// SPDX-License-Identifier: MIT
#import <UIKit/UIKit.h>
#import "DiagnosticRecorder.h"
#import <mach/mach.h>
#include <SDL3/SDL.h>
#include "FramePresentation.h"
#include "EngineThreadMetrics.h"
#include "SystemCpuLoad.h"
#include "GameTrace.h"
#include "HdAtlas.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <sys/resource.h>
namespace {
// Independent of UIKit's main queue: a blocked event/renderer thread must not
// prevent the bounded report. No game files, host paths or user text are read.
class StallMonitor {
    std::atomic<bool> active{false};std::atomic<uint64_t> pumps{0},frames{0};
    std::mutex mutex;std::condition_variable changed;bool stop=false;
    std::thread thread;
    static double cpu_seconds(){struct rusage usage{};if(getrusage(RUSAGE_SELF,&usage))return -1;return usage.ru_utime.tv_sec+usage.ru_stime.tv_sec+(usage.ru_utime.tv_usec+usage.ru_stime.tv_usec)/1000000.0;}
    void run(){
        std::unique_lock<std::mutex> lock(mutex);
        uint64_t previous=0,previous_pumps=0;unsigned stale=0;bool reported=false;
        double stall_cpu=-1;std::string first_calls;
        for(;;){
            changed.wait(lock,[&]{return stop || active.load();});if(stop)return;
            if(changed.wait_for(lock,std::chrono::seconds(1),[&]{return stop || !active.load();})){
                if(stop)return;stale=0;previous=frames.load();continue;
            }
            const auto current=frames.load(),main=pumps.load();
            if(current && current==previous)++stale;else stale=0;
            if(stale==1 && !reported){stall_cpu=cpu_seconds();first_calls=nfs::sampled_game_calls();}
            previous=current;
            if(stale>=10 && !reported){
                reported=true;lock.unlock();
                @autoreleasepool {
                    const auto calls=nfs::sampled_game_calls();
                    const double current_cpu=cpu_seconds();
                    NSString *report=[NSString stringWithFormat:@"NFS II SE presentation stall report\nBuild %@\nNo new frame for at least 10 seconds; cause unconfirmed.\nMain event loop advanced in last sample: %@\nPresented frames: %llu\nLow Power Mode: %@\nFrame limit: %d\nProcess CPU seconds between first and last stall samples: %.4f (-1 means unavailable)\nFirst sample:\n%s\nLast sample:\n%s\n",
                        [NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleVersion"],main!=previous_pumps?@"yes":@"no",(unsigned long long)current,
                        NSProcessInfo.processInfo.lowPowerModeEnabled?@"on":@"off",nfs_frame_limit(),stall_cpu>=0 && current_cpu>=stall_cpu?current_cpu-stall_cpu:-1,first_calls.c_str(),calls.c_str()];
                    NSURL *folder=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
                    if(folder)[report writeToURL:[folder URLByAppendingPathComponent:@"NFSIISE-stall.txt"] atomically:YES encoding:NSUTF8StringEncoding error:nil];
                }
                lock.lock();
            }
            previous_pumps=main;
        }
    }
public:
    StallMonitor():thread([this]{try{run();}catch(...){/* Reporting must not terminate the game. */}}){}
    ~StallMonitor(){{std::lock_guard<std::mutex> lock(mutex);stop=true;}changed.notify_all();thread.join();}
    void activity(int state){
        if(state==2){frames.fetch_add(1,std::memory_order_relaxed);return;}
        if(state==1)pumps.fetch_add(1,std::memory_order_relaxed);
        const bool next=state==1;
        if(active.exchange(next)!=next){std::lock_guard<std::mutex> lock(mutex);changed.notify_all();}
    }
};
StallMonitor &stall_monitor(){static StallMonitor monitor;return monitor;}
}
extern "C" void nfs_monitor_activity(int state){stall_monitor().activity(state);}
extern "C" {extern int32_t fixedFramebufferSize,initialWinWidth,initialWinHeight,windowResized,winWidth,winHeight;}
namespace {
nfs::SystemCpuLoadWindow core_window;
nfs::CpuLoadReading core_reading{};
uint64_t core_read_ns=0;
bool core_provider_rejected=false;
void reset_core_load(){core_window.reset();core_reading={};core_read_ns=0;core_provider_rejected=false;}
NSString *system_core_text(){
    if(core_provider_rejected)return @"";
    const auto now=SDL_GetTicksNS();
    if(!core_read_ns || now<core_read_ns || now-core_read_ns>=1000000000ull){
        nfs::CpuLoadSnapshot snapshot;
        if(!nfs::read_system_cpu_ticks(snapshot)){core_provider_rejected=true;core_window.reset();return @"";}
        core_reading=core_window.observe(snapshot);core_read_ns=now;
    }
    NSMutableString *text=[NSMutableString stringWithString:@"\nSystem cores (whole iPad):"];
    for(std::size_t i=0;i<core_reading.count;++i){
        [text appendString:i%4==0?@"\n":@" · "];
        if(core_reading.valid[i])[text appendFormat:@"C%zu %.0f%%",i+1,core_reading.percent[i]];
        else [text appendFormat:@"C%zu —",i+1];
    }
    return text;
}
}
@interface NFSPerformanceOverlay : NSObject
@property(nonatomic,weak) UIWindow *host;
@property(nonatomic,strong) UILabel *label;
@property(nonatomic,strong) UIButton *keyboardButton;
@property(nonatomic,strong) UIView *limitRow;
@property(nonatomic) BOOL visible;
@property(nonatomic) SDL_Window *window;
@property(nonatomic) BOOL detailed;
@property(nonatomic) BOOL ownsBatteryMonitoring;
- (void)openSettings;
- (void)selectHeight:(NSInteger)height;
- (void)toggleKeyboard;
- (void)changeLimit:(UISwitch *)sender;
- (void)setDeviceMonitoring:(BOOL)enabled;
@end
@implementation NFSPerformanceOverlay
- (void)setDeviceMonitoring:(BOOL)enabled {
    UIDevice *device=UIDevice.currentDevice;
    if(enabled && !device.batteryMonitoringEnabled){device.batteryMonitoringEnabled=YES;self.ownsBatteryMonitoring=YES;}
    if(!enabled && self.ownsBatteryMonitoring){device.batteryMonitoringEnabled=NO;self.ownsBatteryMonitoring=NO;}
}
- (void)changeLimit:(UISwitch *)sender {
    nfs_set_frame_limit(sender.on?30:0);
    [NSUserDefaults.standardUserDefaults setBool:sender.on forKey:@"NFSLock30FPS"];
}
- (void)toggleKeyboard {
    if(!self.window)return;
    if(SDL_TextInputActive(self.window))SDL_StopTextInput(self.window);
    else {
        SDL_PropertiesID properties=SDL_CreateProperties();
        if(!properties)return;
        SDL_SetBooleanProperty(properties,SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN,false);
        SDL_SetBooleanProperty(properties,SDL_PROP_TEXTINPUT_MULTILINE_BOOLEAN,false);
        SDL_StartTextInputWithProperties(self.window,properties);
        SDL_DestroyProperties(properties);
    }
}
- (void)selectHeight:(NSInteger)height {
    if(!self.window)return;
    // Renderer reallocates after pending triangles at its next swap boundary.
    if(height==0){
        SDL_GetWindowSizeInPixels(self.window,&winWidth,&winHeight);
        fixedFramebufferSize=0;
    }else if(height==480 || height==720 || height==960){
        initialWinWidth=int32_t(height*4/3);initialWinHeight=int32_t(height);
        fixedFramebufferSize=1;
    }else return;
    windowResized=1;
    [NSUserDefaults.standardUserDefaults setInteger:height forKey:@"NFSRenderHeight"];
}
- (void)openSettings {
    UIViewController *presenter=self.host.rootViewController;
    while(presenter.presentedViewController)presenter=presenter.presentedViewController;
    if(!presenter || [presenter isKindOfClass:UIAlertController.class])return;
    UIAlertController *sheet=[UIAlertController alertControllerWithTitle:@"Rendering & diagnostics"
        message:@"Pause before changing resolution. Native keeps the original 4:3 view. App CPU is averaged across all cores. Game CPU time measures the engine thread per frame. System-core readings cover the whole iPad when available. Four-finger tap shows or hides diagnostics. Session recording samples about once a second and continues with diagnostics hidden."
        preferredStyle:UIAlertControllerStyleActionSheet];
    NSArray<NSNumber *> *heights=@[@480,@720,@960,@0];
    NSArray<NSString *> *titles=@[@"640 × 480 (original)",@"960 × 720",@"1280 × 960",@"Native screen pixels (4:3)"];
    for(NSUInteger i=0;i<heights.count;++i){
        const NSInteger height=heights[i].integerValue;
        [sheet addAction:[UIAlertAction actionWithTitle:titles[i] style:UIAlertActionStyleDefault handler:^(UIAlertAction *){[self selectHeight:height];}]];
    }
    [sheet addAction:[UIAlertAction actionWithTitle:self.detailed?@"Compact FPS overlay":@"Detailed diagnostics" style:UIAlertActionStyleDefault handler:^(UIAlertAction *){
        self.detailed=!self.detailed;
        reset_core_load();
        [NSUserDefaults.standardUserDefaults setBool:self.detailed forKey:@"NFSDetailedOverlay"];
    }]];
    const int hdMode=nfs_glide_hd_timing_mode();
    UIAlertAction *timing=[UIAlertAction actionWithTitle:hdMode<0?@"HD timing unavailable":(hdMode?@"Timing digits: HD — switch to Original":@"Timing digits: Original — switch to HD") style:UIAlertActionStyleDefault handler:^(UIAlertAction *){
        if(nfs_glide_hd_timing_set_visible(hdMode==0)){
            nfs_reset_engine_metrics();nfs_reset_frame_rate();
            nfs_recording_latest_frame(-1,nfs_render_width(),nfs_render_height());
        }
    }];
    timing.enabled=hdMode>=0;[sheet addAction:timing];
    __weak UIAlertController *weakSheet=sheet;
    if(nfs_recording_finishing()){
        UIAlertAction *saving=[UIAlertAction actionWithTitle:@"Saving recording…" style:UIAlertActionStyleDefault handler:nil];saving.enabled=NO;[sheet addAction:saving];
    }else{
        const bool recording=nfs_recording_active();
        [sheet addAction:[UIAlertAction actionWithTitle:recording?@"Stop & save recording":@"Start session recording" style:UIAlertActionStyleDefault handler:^(UIAlertAction *){
            [weakSheet dismissViewControllerAnimated:YES completion:^{
                if(recording)nfs_recording_stop(@"manual",true);else nfs_recording_start(self.host);
            }];
        }]];
    }
    [sheet addAction:[UIAlertAction actionWithTitle:@"Share last recording" style:UIAlertActionStyleDefault handler:^(UIAlertAction *){
        [weakSheet dismissViewControllerAnimated:YES completion:^{nfs_recording_share(self.host);}];
    }]];
    [sheet addAction:[UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil]];
    sheet.popoverPresentationController.sourceView=self.label;
    sheet.popoverPresentationController.sourceRect=self.label.bounds;
    [presenter presentViewController:sheet animated:YES completion:nil];
}
@end
static NFSPerformanceOverlay *overlay=nil;
extern "C" void nfs_sync_diagnostic_capture(){
    static bool capturing=false;
    const bool needed=overlay.visible || nfs_recording_active();
    [overlay setDeviceMonitoring:needed];
    if(needed!=capturing){capturing=needed;nfs_set_engine_metrics_capture(needed?1:0);}
}
extern "C" void nfs_show_frame_rate(void *pointer,double fps,double cpu,int width,int height){
    auto *window=static_cast<SDL_Window *>(pointer);
    UIWindow *native=(__bridge UIWindow *)SDL_GetPointerProperty(SDL_GetWindowProperties(window),SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER,nullptr);
    if(!native)return;
    if(!overlay)overlay=[[NFSPerformanceOverlay alloc] init];
    if(overlay.label.superview!=native){
        nfs_recording_stop(@"window_changed",false);
        [overlay setDeviceMonitoring:NO];reset_core_load();
        [overlay.label removeFromSuperview];[overlay.keyboardButton removeFromSuperview];[overlay.limitRow removeFromSuperview];overlay.host=native;overlay.window=window;overlay.visible=NO;nfs_sync_diagnostic_capture();
        NSUserDefaults *preferences=NSUserDefaults.standardUserDefaults;
        overlay.detailed=[preferences objectForKey:@"NFSDetailedOverlay"]?[preferences boolForKey:@"NFSDetailedOverlay"]:YES;
        UILabel *label=[[UILabel alloc] init];overlay.label=label;
        label.translatesAutoresizingMaskIntoConstraints=NO;
        label.userInteractionEnabled=YES;label.textColor=UIColor.whiteColor;
        label.backgroundColor=[UIColor colorWithWhite:0 alpha:0.65];
        label.font=[UIFont monospacedDigitSystemFontOfSize:12 weight:UIFontWeightMedium];
        label.textAlignment=NSTextAlignmentCenter;label.numberOfLines=0;
        label.accessibilityIdentifier=@"NativePresentationFPS";
        label.accessibilityHint=@"Tap to select rendering resolution or overlay details";
        [label addGestureRecognizer:[[UITapGestureRecognizer alloc] initWithTarget:overlay action:@selector(openSettings)]];
        [native addSubview:label];
        label.hidden=YES;
        UIView *row=[[UIView alloc] init];overlay.limitRow=row;
        row.translatesAutoresizingMaskIntoConstraints=NO;row.hidden=YES;
        row.backgroundColor=[UIColor colorWithWhite:0 alpha:0.65];
        UILabel *limitText=[[UILabel alloc] init];limitText.translatesAutoresizingMaskIntoConstraints=NO;
        limitText.text=@"Lock FPS to 30";limitText.textColor=UIColor.whiteColor;
        limitText.font=[UIFont systemFontOfSize:13];
        UISwitch *limit=[[UISwitch alloc] init];limit.translatesAutoresizingMaskIntoConstraints=NO;
        limit.on=[preferences boolForKey:@"NFSLock30FPS"];nfs_set_frame_limit(limit.on?30:0);
        limit.accessibilityLabel=@"Lock FPS to 30";
        [limit addTarget:overlay action:@selector(changeLimit:) forControlEvents:UIControlEventValueChanged];
        [row addSubview:limitText];[row addSubview:limit];[native addSubview:row];
        UIButton *keyboard=[UIButton buttonWithType:UIButtonTypeSystem];overlay.keyboardButton=keyboard;
        keyboard.translatesAutoresizingMaskIntoConstraints=NO;
        [keyboard setImage:[UIImage systemImageNamed:@"keyboard"] forState:UIControlStateNormal];
        keyboard.tintColor=UIColor.whiteColor;keyboard.backgroundColor=[UIColor colorWithWhite:0 alpha:0.55];
        keyboard.layer.cornerRadius=8;
        keyboard.accessibilityLabel=@"Show or hide keyboard";
        keyboard.accessibilityIdentifier=@"NativeKeyboardToggle";
        [keyboard addTarget:overlay action:@selector(toggleKeyboard) forControlEvents:UIControlEventTouchUpInside];
        [native addSubview:keyboard];
        [NSLayoutConstraint activateConstraints:@[
            [keyboard.topAnchor constraintEqualToAnchor:native.safeAreaLayoutGuide.topAnchor constant:8],
            [keyboard.leadingAnchor constraintEqualToAnchor:native.safeAreaLayoutGuide.leadingAnchor constant:8],
            [keyboard.widthAnchor constraintEqualToConstant:44],
            [keyboard.heightAnchor constraintEqualToConstant:44],
            [row.topAnchor constraintEqualToAnchor:label.bottomAnchor constant:4],
            [row.leadingAnchor constraintEqualToAnchor:label.leadingAnchor],
            [row.trailingAnchor constraintEqualToAnchor:label.trailingAnchor],
            [row.heightAnchor constraintEqualToConstant:44],
            [limitText.leadingAnchor constraintEqualToAnchor:row.leadingAnchor constant:8],
            [limitText.centerYAnchor constraintEqualToAnchor:row.centerYAnchor],
            [limit.trailingAnchor constraintEqualToAnchor:row.trailingAnchor constant:-8],
            [limit.centerYAnchor constraintEqualToAnchor:row.centerYAnchor],
            [label.topAnchor constraintEqualToAnchor:native.safeAreaLayoutGuide.topAnchor constant:8],
            [label.trailingAnchor constraintEqualToAnchor:native.safeAreaLayoutGuide.trailingAnchor constant:-8],
            [label.widthAnchor constraintEqualToConstant:300],
            [label.heightAnchor constraintGreaterThanOrEqualToConstant:28]]];
        if([preferences objectForKey:@"NFSRenderHeight"])
            [overlay selectHeight:[preferences integerForKey:@"NFSRenderHeight"]];
    }
    nfs_recording_latest_frame(fps,width,height);
    if(!overlay.visible)return;
    const BOOL lowPower=NSProcessInfo.processInfo.lowPowerModeEnabled;
    static BOOL powerKnown=NO,previousPower=NO;
    if(powerKnown && previousPower!=lowPower){if(!nfs_recording_active())nfs_reset_engine_metrics();reset_core_load();}
    powerKnown=YES;previousPower=lowPower;
    mach_task_basic_info_data_t info{};mach_msg_type_number_t count=MACH_TASK_BASIC_INFO_COUNT;
    const BOOL memoryAvailable=task_info(mach_task_self(),MACH_TASK_BASIC_INFO,reinterpret_cast<task_info_t>(&info),&count)==KERN_SUCCESS;
    NSString *cpuText=cpu>=0?[NSString stringWithFormat:@"%.1f%%",cpu]:@"—";
    NSString *memoryText=memoryAvailable?[NSString stringWithFormat:@"%.0f MB",double(info.resident_size)/1048576.0]:@"—";
    [overlay setDeviceMonitoring:YES];
    const float battery=UIDevice.currentDevice.batteryLevel;
    NSString *batteryText=battery>=0 && battery<=1?[NSString stringWithFormat:@"%.0f%%",double(battery)*100]:@"…";
    const double engineCpu=nfs_engine_cpu_ms_per_frame();
    NSString *engineLine=engineCpu>=0?[NSString stringWithFormat:@"Game CPU time: %.2f ms/frame",engineCpu]:@"Game CPU time: measuring…";
    NSString *cores=overlay.detailed?system_core_text():@"";
    overlay.label.text=overlay.detailed?[NSString stringWithFormat:@"FPS %.1f · Frame %.1f ms\n%@\nApp CPU (all cores): %@\nRAM %@ · Battery %@%@\nRender %d × %d\nLow Power Mode: %@",fps,fps>0?1000.0/fps:0,engineLine,cpuText,memoryText,batteryText,cores,width,height,lowPower?@"On":@"Off"]:[NSString stringWithFormat:@"FPS %.1f · Battery %@ · Tap",fps,batteryText];
    overlay.label.hidden=!overlay.visible;overlay.limitRow.hidden=!overlay.visible;
    if(nfs_recording_active())overlay.label.text=[@"● REC · " stringByAppendingString:overlay.label.text];
    [native bringSubviewToFront:overlay.label];
    [native bringSubviewToFront:overlay.limitRow];
    [native bringSubviewToFront:overlay.keyboardButton];
}
extern "C" void nfs_toggle_performance_overlay(void *window){
    if(!overlay || overlay.window!=window)nfs_show_frame_rate(window,0,-1,nfs_render_width(),nfs_render_height());
    overlay.visible=!overlay.visible;
    nfs_sync_diagnostic_capture();reset_core_load();
    overlay.label.hidden=!overlay.visible;overlay.limitRow.hidden=!overlay.visible;
}

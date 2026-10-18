// SPDX-License-Identifier: MIT
#import "DiagnosticRecorder.h"
#import <mach/mach.h>
#include "DiagnosticSession.h"
#include "EngineThreadMetrics.h"
#include "FramePresentation.h"
#include <SDL3/SDL.h>
#include <atomic>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>

@interface NFSDiagnosticRecorder : NSObject {
    FILE *_file;
    std::atomic<unsigned> _pending;
    std::atomic<bool> _failed;
    nfs::DiagnosticSessionLimit _limit;
    nfs::SystemCpuLoadWindow _cores;
    uint64_t _start,_previous,_latestFrame;
    double _latestFPS,_processCPU;
    int64_t _mainCPU;
    int _width,_height;
    BOOL _powerKnown,_power;
}
@property(nonatomic) BOOL recording;
@property(nonatomic) BOOL finishing;
@property(nonatomic,weak) UIWindow *host;
@property(nonatomic,strong) NSURL *lastURL;
@property(nonatomic,strong) NSTimer *timer;
@property(nonatomic,strong) dispatch_queue_t writer;
@property(nonatomic) UIBackgroundTaskIdentifier backgroundTask;
- (void)start:(UIWindow *)host;
- (void)stop:(NSString *)reason notify:(BOOL)notify;
- (void)tick;
- (BOOL)append:(const std::string &)line;
- (void)share:(UIWindow *)host;
- (void)latest:(double)fps width:(int)width height:(int)height;
@end

static UIViewController *recording_presenter(UIWindow *host){
    UIViewController *presenter=host.rootViewController;
    while(presenter.presentedViewController)presenter=presenter.presentedViewController;
    return presenter;
}
static NSURL *recording_folder(){
    NSURL *documents=[NSFileManager.defaultManager URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask].firstObject;
    return documents?[documents URLByAppendingPathComponent:@"Diagnostics" isDirectory:YES]:nil;
}
@implementation NFSDiagnosticRecorder
- (instancetype)init {
    self=[super init];if(self){
        _pending.store(0);_failed.store(false);_mainCPU=-1;_processCPU=-1;
        self.writer=dispatch_queue_create("game.diagnostic-writer",dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL,QOS_CLASS_UTILITY,0));
        self.backgroundTask=UIBackgroundTaskInvalid;
        [NSNotificationCenter.defaultCenter addObserver:self selector:@selector(background:) name:UIApplicationDidEnterBackgroundNotification object:nil];
        NSString *name=[NSUserDefaults.standardUserDefaults stringForKey:@"NFSLastDiagnosticLog"];
        if([name hasPrefix:@"NFSIISE-session-"] && [name.pathExtension isEqualToString:@"jsonl"] && [name isEqualToString:name.lastPathComponent] && name.length<128)
            self.lastURL=[recording_folder() URLByAppendingPathComponent:name];
    }return self;
}
- (void)message:(NSString *)title body:(NSString *)body share:(BOOL)share {
    UIViewController *presenter=recording_presenter(self.host);
    if(!presenter || [presenter isKindOfClass:UIAlertController.class])return;
    UIAlertController *alert=[UIAlertController alertControllerWithTitle:title message:body preferredStyle:UIAlertControllerStyleAlert];
    [alert addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleCancel handler:nil]];
    __weak UIAlertController *weakAlert=alert;
    if(share)[alert addAction:[UIAlertAction actionWithTitle:@"Share log" style:UIAlertActionStyleDefault handler:^(UIAlertAction *){
        [weakAlert dismissViewControllerAnimated:YES completion:^{[self share:self.host];}];
    }]];
    [presenter presentViewController:alert animated:YES completion:nil];
}
- (BOOL)append:(const std::string &)line {
    if(!_file || line.size()>8192 || _pending.load()>=8 || _failed.load())return NO;
    NSData *data=[NSData dataWithBytes:line.data() length:line.size()];FILE *file=_file;
    _pending.fetch_add(1);
    dispatch_async(self.writer,^{
        if(!self->_failed.load() && (std::fwrite(data.bytes,1,data.length,file)!=data.length || std::fflush(file)!=0))self->_failed.store(true);
        self->_pending.fetch_sub(1);
    });return YES;
}
- (void)start:(UIWindow *)host {
    if(self.recording || self.finishing)return;self.host=host;
    NSURL *folder=recording_folder();
    if(!folder || ![NSFileManager.defaultManager createDirectoryAtURL:folder withIntermediateDirectories:YES attributes:nil error:nil]){
        [self message:@"Recording unavailable" body:@"Could not create the Diagnostics folder. Check available storage." share:NO];return;
    }
    NSDateFormatter *format=[[NSDateFormatter alloc] init];format.locale=[[NSLocale alloc] initWithLocaleIdentifier:@"en_US_POSIX"];
    format.timeZone=[NSTimeZone timeZoneForSecondsFromGMT:0];format.dateFormat=@"yyyyMMdd-HHmmss";
    NSString *name=[NSString stringWithFormat:@"NFSIISE-session-%@-%@.jsonl",[format stringFromDate:NSDate.date],[NSUUID.UUID.UUIDString substringToIndex:8]];
    NSURL *url=[folder URLByAppendingPathComponent:name];
    const int fd=open(url.fileSystemRepresentation,O_WRONLY|O_CREAT|O_EXCL,0600);
    if(fd<0){[self message:@"Recording unavailable" body:@"Could not open a new log. Check available storage." share:NO];return;}
    _file=fdopen(fd,"w");if(!_file){close(fd);[self message:@"Recording unavailable" body:@"Could not open a new log." share:NO];return;}
    _failed.store(false);_pending.store(0);_limit=nfs::DiagnosticSessionLimit{};_cores.reset();_powerKnown=NO;
    _start=SDL_GetTicksNS();_previous=0;_mainCPU=-1;_processCPU=-1;
    self.lastURL=url;[NSUserDefaults.standardUserDefaults setObject:name forKey:@"NFSLastDiagnosticLog"];
    self.recording=YES;nfs_sync_diagnostic_capture();nfs_reset_engine_metrics();
    const unsigned build=[[NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleVersion"] intValue];
    if(![self append:nfs::diagnostic_header(build)]){[self stop:@"write_error" notify:YES];return;}
    self.timer=[NSTimer timerWithTimeInterval:1 repeats:YES block:^(NSTimer *){[self tick];}];
    self.timer.tolerance=0.1;[NSRunLoop.mainRunLoop addTimer:self.timer forMode:NSRunLoopCommonModes];
    [self tick];
}
- (void)latest:(double)fps width:(int)width height:(int)height {
    _latestFPS=fps;_latestFrame=SDL_GetTicksNS();_width=width;_height=height;
}
- (void)tick {
    if(!self.recording)return;
    if(_failed.load()){[self stop:@"write_error" notify:YES];return;}
    const uint64_t now=SDL_GetTicksNS();const double elapsed=now>=_start?double(now-_start)/1e9:-1;
    if(_limit.expired(elapsed)){[self stop:@"time_limit" notify:YES];return;}
    if(!_limit.accept(elapsed))return;
    nfs::DiagnosticSample sample;sample.elapsed=elapsed;
    if(_previous && now>_previous)sample.interval=double(now-_previous)/1e9;
    if(_latestFrame && now>=_latestFrame){sample.fps_age=double(now-_latestFrame)/1e9;if(sample.fps_age<=2.5)sample.fps=_latestFPS;}
    sample.low_power=NSProcessInfo.processInfo.lowPowerModeEnabled;
    if(_powerKnown && _power!=sample.low_power){_cores.reset();nfs_reset_engine_metrics();}
    _powerKnown=YES;_power=sample.low_power;
    sample.game_cpu_ms=nfs_engine_cpu_sample(&sample.game_cpu_id);
    sample.logical_cores=unsigned(SDL_max(1,SDL_GetNumLogicalCPUCores()));
    struct rusage usage{};double process=-1;
    if(getrusage(RUSAGE_SELF,&usage)==0)process=usage.ru_utime.tv_sec+usage.ru_stime.tv_sec+(usage.ru_utime.tv_usec+usage.ru_stime.tv_usec)/1e6;
    const int64_t main=nfs::current_thread_cpu_ns();
    if(sample.interval>0){
        if(process>=0 && _processCPU>=0 && process>=_processCPU){sample.process_ms=(process-_processCPU)*1000;sample.app_cpu_percent=sample.process_ms/(sample.interval*10*sample.logical_cores);}
        if(main>=0 && _mainCPU>=0 && main>=_mainCPU)sample.main_ms=double(main-_mainCPU)/1e6;
    }
    _processCPU=process;_mainCPU=main;_previous=now;
    mach_task_basic_info_data_t info{};mach_msg_type_number_t count=MACH_TASK_BASIC_INFO_COUNT;
    if(task_info(mach_task_self(),MACH_TASK_BASIC_INFO,reinterpret_cast<task_info_t>(&info),&count)==KERN_SUCCESS && count==MACH_TASK_BASIC_INFO_COUNT)sample.ram_mb=double(info.resident_size)/1048576;
    UIDevice *device=UIDevice.currentDevice;const float battery=device.batteryLevel;
    sample.battery_percent=battery>=0 && battery<=1?battery*100:-1;
    sample.battery_state=int(device.batteryState);sample.brightness=self.host?self.host.screen.brightness:-1;
    sample.thermal=int(NSProcessInfo.processInfo.thermalState);sample.app_state=int(UIApplication.sharedApplication.applicationState);
    sample.width=_width;sample.height=_height;sample.frame_limit=nfs_frame_limit();
    nfs::CpuLoadSnapshot ticks;if(nfs::read_system_cpu_ticks(ticks))sample.cores=_cores.observe(ticks);else _cores.reset();
    if(![self append:nfs::diagnostic_sample_json(sample)])[self stop:_failed.load()?@"write_error":@"writer_backlog" notify:YES];
}
- (void)background:(NSNotification *)notification {
    (void)notification;if(!self.recording)return;
    self.backgroundTask=[UIApplication.sharedApplication beginBackgroundTaskWithExpirationHandler:^{
        if(self.backgroundTask!=UIBackgroundTaskInvalid){[UIApplication.sharedApplication endBackgroundTask:self.backgroundTask];self.backgroundTask=UIBackgroundTaskInvalid;}
    }];[self stop:@"background" notify:NO];
}
- (void)stop:(NSString *)reason notify:(BOOL)notify {
    if(!self.recording)return;
    self.recording=NO;self.finishing=YES;[self.timer invalidate];self.timer=nil;nfs_sync_diagnostic_capture();
    NSArray *reasons=@[@"manual",@"background",@"window_changed",@"time_limit",@"write_error",@"writer_backlog"];
    if(![reasons containsObject:reason])reason=@"stopped";
    const auto now=SDL_GetTicksNS();const double elapsed=now>=_start?double(now-_start)/1e9:0;
    const auto end=nfs::diagnostic_footer(reason.UTF8String,elapsed,_limit.count());
    NSData *data=[NSData dataWithBytes:end.data() length:end.size()];FILE *file=_file;_file=nullptr;
    dispatch_async(self.writer,^{
        bool failed=self->_failed.load();
        if(std::fwrite(data.bytes,1,data.length,file)!=data.length || std::fflush(file)!=0)failed=true;
        if(std::fclose(file)!=0)failed=true;
        dispatch_async(dispatch_get_main_queue(),^{
            self.finishing=NO;
            if(self.backgroundTask!=UIBackgroundTaskInvalid){[UIApplication.sharedApplication endBackgroundTask:self.backgroundTask];self.backgroundTask=UIBackgroundTaskInvalid;}
            if(notify)[self message:failed?@"Recording saved partially":@"Recording saved"
                body:[NSString stringWithFormat:@"%@\n\nFiles → On My iPad → NFS II SE → Diagnostics. Share this file and tell me the track, cars and game mode.%@",self.lastURL.lastPathComponent,failed?@" Some rows may be missing.":@""] share:YES];
        });
    });
}
- (void)share:(UIWindow *)host {
    self.host=host;if(self.finishing)return;
    if(!self.lastURL || ![NSFileManager.defaultManager fileExistsAtPath:self.lastURL.path]){
        [self message:@"No recording yet" body:@"Start and stop a session first." share:NO];return;
    }
    UIViewController *presenter=recording_presenter(host);if(!presenter || [presenter isKindOfClass:UIAlertController.class])return;
    UIActivityViewController *share=[[UIActivityViewController alloc] initWithActivityItems:@[self.lastURL] applicationActivities:nil];
    share.popoverPresentationController.sourceView=presenter.view;
    share.popoverPresentationController.sourceRect=CGRectMake(presenter.view.bounds.size.width/2,presenter.view.bounds.size.height/2,1,1);
    [presenter presentViewController:share animated:YES completion:nil];
}
@end
static NFSDiagnosticRecorder *recorder(){static NFSDiagnosticRecorder *instance=nil;if(!instance)instance=[[NFSDiagnosticRecorder alloc] init];return instance;}
extern "C" bool nfs_recording_active(){return recorder().recording;}
extern "C" bool nfs_recording_finishing(){return recorder().finishing;}
extern "C" void nfs_recording_start(UIWindow *host){[recorder() start:host];}
extern "C" void nfs_recording_stop(NSString *reason,bool notify){[recorder() stop:reason notify:notify];}
extern "C" void nfs_recording_share(UIWindow *host){[recorder() share:host];}
extern "C" void nfs_recording_latest_frame(double fps,int width,int height){[recorder() latest:fps width:width height:height];}

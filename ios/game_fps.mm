// SPDX-License-Identifier: MIT
#import <UIKit/UIKit.h>
#import <mach/mach.h>
#include <SDL3/SDL.h>
#include "FramePresentation.h"
extern "C" {extern int32_t fixedFramebufferSize,initialWinWidth,initialWinHeight,windowResized,winWidth,winHeight;}
@interface NFSPerformanceOverlay : NSObject
@property(nonatomic,weak) UIWindow *host;
@property(nonatomic,strong) UILabel *label;
@property(nonatomic,strong) UIButton *keyboardButton;
@property(nonatomic) SDL_Window *window;
@property(nonatomic) BOOL detailed;
- (void)openSettings;
- (void)selectHeight:(NSInteger)height;
- (void)toggleKeyboard;
@end
@implementation NFSPerformanceOverlay
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
        message:@"Pause with your controller before changing resolution. Native keeps the original 4:3 view. CPU/app is this game's share of total CPU capacity; GPU usage is unavailable with this renderer."
        preferredStyle:UIAlertControllerStyleActionSheet];
    NSArray<NSNumber *> *heights=@[@480,@720,@960,@0];
    NSArray<NSString *> *titles=@[@"640 × 480 (original)",@"960 × 720",@"1280 × 960",@"Native screen pixels (4:3)"];
    for(NSUInteger i=0;i<heights.count;++i){
        const NSInteger height=heights[i].integerValue;
        [sheet addAction:[UIAlertAction actionWithTitle:titles[i] style:UIAlertActionStyleDefault handler:^(UIAlertAction *){[self selectHeight:height];}]];
    }
    [sheet addAction:[UIAlertAction actionWithTitle:self.detailed?@"Compact FPS overlay":@"Detailed diagnostics" style:UIAlertActionStyleDefault handler:^(UIAlertAction *){
        self.detailed=!self.detailed;
        [NSUserDefaults.standardUserDefaults setBool:self.detailed forKey:@"NFSDetailedOverlay"];
    }]];
    [sheet addAction:[UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil]];
    sheet.popoverPresentationController.sourceView=self.label;
    sheet.popoverPresentationController.sourceRect=self.label.bounds;
    [presenter presentViewController:sheet animated:YES completion:nil];
}
@end
extern "C" void nfs_show_frame_rate(void *pointer,double fps,double cpu,int width,int height){
    auto *window=static_cast<SDL_Window *>(pointer);
    UIWindow *native=(__bridge UIWindow *)SDL_GetPointerProperty(SDL_GetWindowProperties(window),SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER,nullptr);
    if(!native)return;
    static NFSPerformanceOverlay *overlay=nil;
    if(!overlay)overlay=[[NFSPerformanceOverlay alloc] init];
    if(overlay.label.superview!=native){
        [overlay.label removeFromSuperview];[overlay.keyboardButton removeFromSuperview];overlay.host=native;overlay.window=window;
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
            [keyboard.topAnchor constraintEqualToAnchor:label.bottomAnchor constant:6],
            [keyboard.trailingAnchor constraintEqualToAnchor:label.trailingAnchor],
            [keyboard.widthAnchor constraintEqualToConstant:44],
            [keyboard.heightAnchor constraintEqualToConstant:44],
            [label.topAnchor constraintEqualToAnchor:native.safeAreaLayoutGuide.topAnchor constant:8],
            [label.trailingAnchor constraintEqualToAnchor:native.safeAreaLayoutGuide.trailingAnchor constant:-8],
            [label.widthAnchor constraintEqualToConstant:220],
            [label.heightAnchor constraintGreaterThanOrEqualToConstant:28]]];
        if([preferences objectForKey:@"NFSRenderHeight"])
            [overlay selectHeight:[preferences integerForKey:@"NFSRenderHeight"]];
    }
    mach_task_basic_info_data_t info{};mach_msg_type_number_t count=MACH_TASK_BASIC_INFO_COUNT;
    const BOOL memoryAvailable=task_info(mach_task_self(),MACH_TASK_BASIC_INFO,reinterpret_cast<task_info_t>(&info),&count)==KERN_SUCCESS;
    NSString *cpuText=cpu>=0?[NSString stringWithFormat:@"%.1f%%",cpu]:@"—";
    NSString *memoryText=memoryAvailable?[NSString stringWithFormat:@"%.0f MB",double(info.resident_size)/1048576.0]:@"—";
    overlay.label.text=overlay.detailed?[NSString stringWithFormat:@"FPS %.1f  ·  %.1f ms\nCPU/app %@  ·  RAM %@\nRender %d × %d  ·  GPU n/a",fps,fps>0?1000.0/fps:0,cpuText,memoryText,width,height]:[NSString stringWithFormat:@"FPS %.1f  ·  Tap for settings",fps];
    [native bringSubviewToFront:overlay.label];
    [native bringSubviewToFront:overlay.keyboardButton];
}

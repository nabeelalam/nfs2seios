// SPDX-License-Identifier: MIT
#import <UIKit/UIKit.h>
#include <SDL3/SDL.h>
extern "C" void nfs_show_frame_rate(void *pointer,double fps){
    auto *window=static_cast<SDL_Window *>(pointer);
    UIWindow *native=(__bridge UIWindow *)SDL_GetPointerProperty(SDL_GetWindowProperties(window),SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER,nullptr);
    if(!native)return;
    static UILabel *label=nil;
    if(label.superview!=native){
        [label removeFromSuperview];label=[[UILabel alloc] init];
        label.translatesAutoresizingMaskIntoConstraints=NO;
        label.userInteractionEnabled=NO;label.textColor=UIColor.whiteColor;
        label.backgroundColor=[UIColor colorWithWhite:0 alpha:0.6];
        label.font=[UIFont monospacedDigitSystemFontOfSize:14 weight:UIFontWeightMedium];
        label.textAlignment=NSTextAlignmentCenter;label.accessibilityIdentifier=@"NativePresentationFPS";
        [native addSubview:label];
        [NSLayoutConstraint activateConstraints:@[
            [label.topAnchor constraintEqualToAnchor:native.safeAreaLayoutGuide.topAnchor constant:8],
            [label.trailingAnchor constraintEqualToAnchor:native.safeAreaLayoutGuide.trailingAnchor constant:-8],
            [label.widthAnchor constraintEqualToConstant:90],[label.heightAnchor constraintEqualToConstant:24]]];
    }
    label.text=[NSString stringWithFormat:@"FPS %.1f",fps];[native bringSubviewToFront:label];
}

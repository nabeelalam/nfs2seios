// SPDX-License-Identifier: MIT
#pragma once
#import <UIKit/UIKit.h>
extern "C" {
bool nfs_recording_active();
bool nfs_recording_finishing();
void nfs_recording_start(UIWindow *host);
void nfs_recording_stop(NSString *reason,bool notify);
void nfs_recording_share(UIWindow *host);
void nfs_recording_latest_frame(double fps,int width,int height);
void nfs_sync_diagnostic_capture();
}

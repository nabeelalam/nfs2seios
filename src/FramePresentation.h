// SPDX-License-Identifier: MIT
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void nfs_frame_presented(void *window);
void nfs_reset_frame_rate(void);
double nfs_frame_rate_value(void);
void nfs_render_target_changed(int width,int height);
int nfs_render_width(void);
int nfs_render_height(void);
void nfs_toggle_diagnostics(void *window);
void nfs_set_frame_limit(int fps);
int nfs_frame_limit(void);
void nfs_wait_frame_slot(void);
void nfs_diagnostics_activity(int state);
#ifdef __cplusplus
}
#endif

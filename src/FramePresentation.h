// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Swap-submission timestamps; not display photons or GPU execution time. */
typedef struct NfsFrameTiming { uint64_t frame_id,timestamp_ns,interval_ns,window_epoch,swap_call_ns; } NfsFrameTiming;
typedef struct NfsFrameTimingSnapshot { uint64_t latest_frame_id,latest_timestamp_ns,dropped_total,window_epoch,fps_timestamp_ns; double fps; int enabled; } NfsFrameTimingSnapshot;
void nfs_frame_timing_capture(int enabled);
/* The v2 symbol requires records with the host swap duration field. */
unsigned nfs_frame_timing_drain_v2(NfsFrameTiming *out,unsigned capacity);
NfsFrameTimingSnapshot nfs_frame_timing_snapshot(void);
int nfs_swap_window(void *window,uint64_t *elapsed_ns);
void nfs_frame_presented(void *window);
void nfs_frame_presented_with_swap(void *window,uint64_t swap_call_ns);
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

// SPDX-License-Identifier: MIT
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void nfs_frame_presented(void *window);
void nfs_reset_frame_rate(void);
double nfs_frame_rate_value(void);
#ifdef __cplusplus
}
#endif

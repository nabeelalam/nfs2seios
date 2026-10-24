// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_STEERING_BAR_RUNTIME_ABI_H
#define NFS_CONTROLS_STEERING_BAR_RUNTIME_ABI_H
#include <stddef.h>
#include <stdint.h>
#define NFS_CONTROLS_STEERING_BAR_SCHEMA 1u
#define NFS_CONTROLS_STEERING_BAR_LEVELS 4u
#define NFS_CONTROLS_STEERING_BAR_WIDTH 1424u
#define NFS_CONTROLS_STEERING_BAR_HEIGHT 196u
#define NFS_CONTROLS_STEERING_BAR_BYTES ((size_t)1424u*196u*4u)
/* Sealed immutable premultiplied RGBA8. RGB=(straightRGB*alpha+127)/255 once.
   No form, source, native-upload, FIFO or staging ownership is granted here. */
typedef struct NfsControlsSteeringBarView {uint32_t schema,level,width,height;const unsigned char*rgba;size_t bytes;unsigned char sha256[32];float left,top,right,bottom;} NfsControlsSteeringBarView;
#endif

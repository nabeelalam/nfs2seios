// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_STEERING_BAR_GPU_H
#define NFS_CONTROLS_STEERING_BAR_GPU_H
#include "ControlsSteeringBarRuntimeAbi.h"
#include "BackgroundSceneCompositor.h"
typedef struct NfsControlsSteeringBarGpu {uint64_t generation;uintptr_t contextIdentity;GLuint program,textures[4];GLint sampler,matrix;const unsigned char*rgba[4];} NfsControlsSteeringBarGpu;
#ifdef __cplusplus
extern "C" {
#endif
/* Zero initialization suffices. Context-only owner stage accepted for prepare.
   Caller supplies immutable pinned art; warm prepare performs no pixel hash or upload. */
int nfs_controls_steering_bar_gpu_prepare(NfsControlsSteeringBarGpu*,const NfsControlsSteeringBarView*,const NfsBackgroundSceneStage*);
/* Current owned unpublished staging only. No original-source/FIFO/native-upload
   authority, FBO allocation, original-target binding, depth write or presentation.
   Any late error requires discarding the caller's entire staging transaction. */
int nfs_controls_steering_bar_gpu_draw_into_staging(NfsControlsSteeringBarGpu*,const NfsControlsSteeringBarView*,const NfsBackgroundSceneStage*);
void nfs_controls_steering_bar_gpu_forget(NfsControlsSteeringBarGpu*);
void nfs_controls_steering_bar_gpu_retire(NfsControlsSteeringBarGpu*,const NfsBackgroundSceneStage*,int ownerCurrent);
#ifdef __cplusplus
}
#endif
#endif

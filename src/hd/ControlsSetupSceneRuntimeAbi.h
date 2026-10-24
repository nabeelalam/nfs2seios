// SPDX-License-Identifier: MIT
#ifndef NFS_CONTROLS_SETUP_SCENE_RUNTIME_ABI_H
#define NFS_CONTROLS_SETUP_SCENE_RUNTIME_ABI_H
#include "ControlsSetupSceneLayerAbi.h"
#define NFS_CONTROLS_SETUP_SCENE_RUNTIME_SCHEMA 1u
/* Native immutable packet borrows. Required paired lifetime callbacks retain
   every scene/source/font allocation independently of queue consumption.
   Begin retains once before storing borrows; finish/cancel/retire releases once.
   Callbacks must be thread-safe, nonthrowing and must not reenter this renderer.
   Renderer begin copies original565 to its bounded expected-tile witness;
   remaining layer borrows remain retained until the renderer clears them.
   No field asserts GL context, native upload/target authority or suppression. */
typedef struct NfsControlsSetupSceneFrameView {
 uint32_t schema,width,height;uint64_t serial,contentRevision;
 size_t planeWords;const uint16_t*original565;
 NfsControlsSetupSceneLayers layers;
 void*lifetimeOwner;void(*retainOwner)(void*);void(*releaseOwner)(void*);
} NfsControlsSetupSceneFrameView;
#ifdef __cplusplus
extern "C" {
#endif
/* Exact Controls-owned426950 only. All original interleaved upload/select/triangle
   calls continue; finish is queued after them, before original swap/cursor. */
int nfs_private_controls_setup_scene_begin(const NfsControlsSetupSceneFrameView*);
int nfs_private_controls_setup_scene_finish(uint64_t serial);
#ifdef __cplusplus
}
#endif
#endif

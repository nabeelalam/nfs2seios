// SPDX-License-Identifier: MIT
#ifndef NFS_OPPONENT_SCENE_RUNTIME_ABI_H
#define NFS_OPPONENT_SCENE_RUNTIME_ABI_H
#include "BackgroundSceneRuntimeAbi.h"
#include "SoftwareTextLayerAbi.h"
#include "OriginalPreviewLayerAbi.h"
#include "MenuResponsiveCached.h"
#define NFS_OPPONENT_SCENE_RUNTIME_SCHEMA 1u
#define NFS_BACKGROUND_TAIL_OPPONENT_CAPTION 6u
/* Explicit Opponent-only tail. The original Main entry still requires MAIN_SKIP.
   All pointers are retained by a native immutable owner for one synchronous
   renderer callback. No field supplies GPU context/upload/target authority. */
typedef struct NfsOpponentSceneFrameView {
 uint32_t schema;
 NfsBackgroundSceneFrameView scene;
 NfsSoftwareTextLayerView caption;
 NfsOriginalPreviewLayerView preview;
 uint64_t previewArtGeneration;uintptr_t previewArtIdentity;uint32_t previewSelection;
 NfsMenuCachedSnapshot responsive;
} NfsOpponentSceneFrameView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_opponent_scene_begin(const NfsOpponentSceneFrameView*);
int nfs_private_opponent_scene_finish(uint64_t serial);
#ifdef __cplusplus
}
#endif
#endif

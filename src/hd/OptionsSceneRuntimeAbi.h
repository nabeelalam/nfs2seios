// SPDX-License-Identifier: MIT
#ifndef NFS_OPTIONS_SCENE_RUNTIME_ABI_H
#define NFS_OPTIONS_SCENE_RUNTIME_ABI_H
#include "BackgroundSceneRuntimeAbi.h"
#include "SoftwareTextLayerAbi.h"
#include "CompactCaptionWordRuntimeAbi.h"
#define NFS_OPTIONS_SCENE_RUNTIME_SCHEMA 2u
/* Separate tag; Main and Location guards retain their original1/2 values. */
enum {NFS_BACKGROUND_TAIL_OPTIONS_CAPTION=3};
/* Explicit Options-only tail. The original Main entry still requires MAIN_SKIP.
   All pointers are retained by a native immutable owner for one synchronous
   renderer callback. No field supplies GPU context/upload/target authority. */
typedef struct NfsOptionsSceneFrameView {
 uint32_t schema;
 NfsBackgroundSceneFrameView scene;
 NfsSoftwareTextLayerView caption;
 NfsCompactCaptionWordView captionWord;
} NfsOptionsSceneFrameView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_options_scene_begin(const NfsOptionsSceneFrameView*);
int nfs_private_options_scene_finish(uint64_t serial);
#ifdef __cplusplus
}
#endif
#endif

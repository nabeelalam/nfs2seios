// SPDX-License-Identifier: MIT
#ifndef NFS_LOCATION_SCENE_RUNTIME_ABI_H
#define NFS_LOCATION_SCENE_RUNTIME_ABI_H
#include "BackgroundSceneRuntimeAbi.h"
#include "SoftwareTextLayerAbi.h"
#define NFS_LOCATION_SCENE_RUNTIME_SCHEMA 1u
/* Explicit Location-only tail. The original Main entry still requires MAIN_SKIP.
   All pointers are retained by a native immutable owner for one synchronous
   renderer callback. No field supplies GPU context/upload/target authority. */
typedef struct NfsLocationSceneFrameView {
 uint32_t schema;
 NfsBackgroundSceneFrameView scene;
 NfsSoftwareTextLayerView caption;
} NfsLocationSceneFrameView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_location_scene_begin(const NfsLocationSceneFrameView*);
int nfs_private_location_scene_finish(uint64_t serial);
#ifdef __cplusplus
}
#endif
#endif

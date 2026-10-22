// SPDX-License-Identifier: MIT
#ifndef NFS_SETUP_SCENE_RUNTIME_ABI_H
#define NFS_SETUP_SCENE_RUNTIME_ABI_H
#include "BackgroundSceneRuntimeAbi.h"
#include "SoftwareTextLayerAbi.h"
#define NFS_SETUP_SCENE_RUNTIME_SCHEMA 1u
/* Independent Setup tag; Options/Location/Main retain their exact3/2/1 guards. */
enum {NFS_BACKGROUND_TAIL_SETUP_CAPTION=4};
/* Source joins and retained caption are independently Setup-owned. Borrows are
   retained by the immutable queue owner throughout synchronous begin/finish.
   No packet field supplies native context/upload/target authority. */
typedef struct NfsSetupSceneFrameView {
 uint32_t schema;
 NfsBackgroundSceneFrameView scene;
 NfsSoftwareTextLayerView caption;
} NfsSetupSceneFrameView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_setup_scene_begin(const NfsSetupSceneFrameView*);
int nfs_private_setup_scene_finish(uint64_t serial);
#ifdef __cplusplus
}
#endif
#endif

// SPDX-License-Identifier: MIT
#ifndef NFS_PLAYER_SCENE_RUNTIME_ABI_H
#define NFS_PLAYER_SCENE_RUNTIME_ABI_H
#include "BackgroundSceneRuntimeAbi.h"
#include "SoftwareTextLayerAbi.h"
#include "PlayerCaptionRuntimeAbi.h"
#include "OriginalPreviewLayerAbi.h"
#define NFS_PLAYER_SCENE_RUNTIME_SCHEMA 2u
enum {NFS_BACKGROUND_TAIL_PLAYER1_CAPTION=5,NFS_BACKGROUND_TAIL_PLAYER2_CAPTION=6};
/* Immutable synchronous borrows, retained by the producer. Exact form/tail pairs
   remain Player-only; no GL context/upload/target authority is supplied here. */
typedef struct NfsPlayerSceneFrameView {
 uint32_t schema,form;
 NfsBackgroundSceneFrameView scene;
 NfsSoftwareTextLayerView caption;
 NfsOriginalPreviewLayerView preview;
 NfsPlayerCaptionWordView captionWord;
} NfsPlayerSceneFrameView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_player_scene_begin(const NfsPlayerSceneFrameView*);
int nfs_private_player_scene_finish(uint64_t serial);
/* Exact pending serial, current owner only; CPU-only, no GL calls. */
int nfs_private_player_scene_cancel(uint64_t serial);
#ifdef __cplusplus
}
#endif
#endif

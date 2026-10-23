// SPDX-License-Identifier: MIT
#ifndef NFS_PLAYER_INTRO_RUNTIME_ABI_H
#define NFS_PLAYER_INTRO_RUNTIME_ABI_H
#include "BackgroundSceneRuntimeAbi.h"
#include "SoftwareTextLayerAbi.h"
#include "PlayerCaptionRuntimeAbi.h"
#define NFS_PLAYER_INTRO_RUNTIME_SCHEMA 2u
/* Immutable synchronous borrows retained by the producer. Historical Main
   serial/content/events stay historical; requestSerial is a distinct request.
   No field supplies GL/context/target authority or changes native buffers. */
typedef struct NfsPlayerIntroFrameView {
 uint32_t schema,form;
 uint64_t requestSerial,uploadEvent;
 uint32_t sourceOwner,sourceBitmap;
 NfsBackgroundSceneFrameView inheritedMain;
 NfsSoftwareTextLayerView caption;
 const uint16_t*paddedOriginal565;
 size_t wordsCount;
 uint32_t keys[300];
 NfsPlayerCaptionWordView captionWord;
} NfsPlayerIntroFrameView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_player_intro_begin(const NfsPlayerIntroFrameView*);
int nfs_private_player_intro_slot0_finish(uint64_t requestSerial);
int nfs_private_player_intro_tail_finish(uint64_t requestSerial);
int nfs_private_player_intro_end(uint64_t requestSerial);
int nfs_private_player_intro_cancel(uint64_t requestSerial);
#ifdef __cplusplus
}
#endif
#endif

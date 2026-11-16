// SPDX-License-Identifier: MIT
#ifndef NFS_CHILD_INTRO_WIDE_PRELUDE_ABI_H
#define NFS_CHILD_INTRO_WIDE_PRELUDE_ABI_H
#include "BackgroundSceneRuntimeAbi.h"
#include "ChildInitialTransitionAbi.h"
#include "OpponentInitialTransitionAbi.h"
#include "LocationInitialTransitionAbi.h"
#include "MenuTransitionBrandingAbi.h"
/* Disjoint typed normal-slide source. family6/7 is an independently sealed
   Opponents/Location343 upload plus its own initial caption and Main origin.
   Never reinterpret a PlayerInitialNativeProof or select Player photo roles. */
typedef struct NfsChildIntroWidePreludeView {
 uint32_t schema,family,countdown,form,sourceOwner,sourceBitmap,locationInfoReturn;
 uint64_t serial,content,uploadEvent;NfsBackgroundSceneFrameView inheritedMain;
 const uint16_t*paddedOriginal565;size_t wordsCount;uint32_t keys[300];
 NfsChildInitialTransitionLayer childInitial;
 NfsOpponentInitialTransitionView opponentInitial;
 NfsLocationInitialTransitionView locationInitial;
 uint32_t photoFamily;uint64_t photoSerial,ownerEpoch;
 uintptr_t sourceGame,sourceCatalog;uint64_t sourceLife;uint32_t sourceArchive;
 void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(void*);
}NfsChildIntroWidePreludeView;
enum{NFS_CHILD_WIDE_CLEARED=1,NFS_CHILD_WIDE_SLOT1=2,NFS_CHILD_WIDE_PHOTO_BEGIN=3,NFS_CHILD_WIDE_PHOTO_END=4,NFS_CHILD_WIDE_SLOT0=5,NFS_CHILD_WIDE_TAIL=6,NFS_CHILD_WIDE_END=7};
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_child_intro_wide_begin(const NfsChildIntroWidePreludeView*);
int nfs_private_child_intro_wide_stage(uint64_t,unsigned,int32_t,int32_t);
int nfs_private_child_intro_wide_cancel(uint64_t);
int nfs_private_child_intro_branding_begin(const NfsMenuTransitionBrandingView*);
int nfs_private_child_intro_branding_quad(const NfsMenuTransitionBrandingQuad*);
int nfs_private_child_intro_branding_end(uint64_t);
void nfs_private_child_intro_branding_cancel(uint64_t);
#ifdef __cplusplus
}
#endif
#endif

// SPDX-License-Identifier: MIT
#ifndef NFS_PLAYER_INTRO_WIDE_PRELUDE_ABI_H
#define NFS_PLAYER_INTRO_WIDE_PRELUDE_ABI_H
#include "PlayerIntroRuntimeAbi.h"
/* Independent Player source proof; never a MenuTransition packet. Synchronous
   source borrows remain valid only through owner. current() uses retained CPU
   proof validity/GameLife/provider generation only, NEVER reads guest memory. */
typedef struct NfsPlayerIntroWidePreludeView {
 uint32_t schema,countdown;NfsPlayerIntroFrameView frame;
 uint64_t photoSerial,ownerEpoch;uintptr_t sourceGame,sourceCatalog;
 uint64_t sourceLife;uint32_t sourceArchive;
 void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(void*);
}NfsPlayerIntroWidePreludeView;
enum{NFS_PLAYER_WIDE_CLEARED=1,NFS_PLAYER_WIDE_SLOT1=2,NFS_PLAYER_WIDE_PHOTO_BEGIN=3,NFS_PLAYER_WIDE_PHOTO_END=4,NFS_PLAYER_WIDE_SLOT0=5,NFS_PLAYER_WIDE_TAIL=6,NFS_PLAYER_WIDE_END=7};
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_player_intro_wide_begin(const NfsPlayerIntroWidePreludeView*);
int nfs_private_player_intro_wide_stage(uint64_t,unsigned,int32_t,int32_t);
int nfs_private_player_intro_wide_cancel(uint64_t);
#ifdef __cplusplus
}
#endif
#endif

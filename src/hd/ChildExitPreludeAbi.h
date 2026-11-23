// SPDX-License-Identifier: MIT
#ifndef NFS_CHILD_EXIT_PRELUDE_ABI_H
#define NFS_CHILD_EXIT_PRELUDE_ABI_H
#include "SetupSceneRuntimeAbi.h"
#include "OptionsSceneRuntimeAbi.h"
#include "LocationSceneRuntimeAbi.h"
#include "OpponentSceneRuntimeAbi.h"
#include "PlayerSceneRuntimeAbi.h"
/* Historical ChildExit name: original40D1B0 PREVIEW CHANGE (21 frames), not Escape.
 * Source families are disjoint from entrance/MT kinds. Exactly one typed
 * full view is populated. Its immutable CPU copy/upload proof owns the source;
 * a native/displayed framebuffer never supplies semantic authority. */
enum {NFS_CHILD_EXIT_SETUP=1,NFS_CHILD_EXIT_OPTIONS=2,NFS_CHILD_EXIT_LOCATION=3,NFS_CHILD_EXIT_OPPONENT=4,NFS_CHILD_EXIT_PLAYER1=5,NFS_CHILD_EXIT_PLAYER2=6};
enum {NFS_CHILD_EXIT_CLEAR=1,NFS_CHILD_EXIT_FULL=2,NFS_CHILD_EXIT_PHOTO_BEGIN=3,NFS_CHILD_EXIT_PHOTO_END=4,NFS_CHILD_EXIT_SIDEBAR=5,NFS_CHILD_EXIT_WORDS_BEGIN=6,NFS_CHILD_EXIT_WORDS_END=7,NFS_CHILD_EXIT_BRANDING_BEGIN=8,NFS_CHILD_EXIT_BRANDING_END=9,NFS_CHILD_EXIT_CURSOR_BEGIN=10,NFS_CHILD_EXIT_CURSOR_END=11,NFS_CHILD_EXIT_END=12,NFS_CHILD_EXIT_READY=13,NFS_CHILD_EXIT_WIDGETS_BEGIN=14,NFS_CHILD_EXIT_WIDGETS_END=15};
typedef struct NfsChildExitPhotoDescriptor {uint32_t descriptor,words[7];}NfsChildExitPhotoDescriptor;
typedef struct NfsChildExitPreludeView {
 uint32_t schema,family,form,clearedFrame;int32_t motion;
 uint64_t serial,session,ownerEpoch,uploadEvent,sidebarContent,photoSerial,photoEpoch;
 uint32_t photoFamily,photoDescriptorCount,sourceArchive;uintptr_t sourceGame,sourceCatalog;uint64_t sourceLife;
 NfsMenuCachedSnapshot widgets;
 NfsSetupSceneFrameView setup;NfsOptionsSceneFrameView options;
 NfsLocationSceneFrameView location;NfsOpponentSceneFrameView opponent;NfsPlayerSceneFrameView player;
 NfsChildExitPhotoDescriptor photo[100];
 const uint16_t*sidebar565;size_t sidebarWords;uint32_t sidebarKeys[300];
 void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(void*);
 /* Optional whole-transaction rejection for a native-revalidated history lease. */
 void(*rejectLease)(void*);
}NfsChildExitPreludeView;
/* Captured at the actual original497D6C source quad and post-conversion,
 * pre-first native triangle. Only initialized native fields may be compared.
 * Section is a typed lexical call scope, not a generic permission to paint. */
typedef struct NfsChildExitQuad {
 uint64_t serial;uint32_t section,ordinal;
 uint32_t descriptor,descriptorWords[7],tmu,address,mask;
 uint32_t resourceId,resourceCategory;int32_t resourceAnchor,resourceY;
 uint32_t sourceVertices[4][8],indices[4],nativeVertices[4][15];
}NfsChildExitQuad;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_child_exit_begin(const NfsChildExitPreludeView*);
int nfs_private_child_exit_stage(uint64_t,unsigned,int32_t,int32_t);
int nfs_private_child_exit_quad(const NfsChildExitQuad*);
int nfs_private_child_exit_cancel(uint64_t);
#ifdef __cplusplus
}
#endif
#endif

// SPDX-License-Identifier: MIT
#ifndef NFS_BACKGROUND_SCENE_RESOURCE_PREPARE_H
#define NFS_BACKGROUND_SCENE_RESOURCE_PREPARE_H
#include "BackgroundSceneCompositor.h"
typedef struct NfsBackgroundResourceOwner NfsBackgroundResourceOwner;
typedef int(*NfsBackgroundResourceOwnerCurrent)(void*,const NfsBackgroundResourceOwner*);
/* Trusted renderer callback verifies actual context/thread/generation and the
   registered original target FBO. Resource preparation does not change that FBO;
   program/texture bindings are temporarily changed. No packet grants ownership. */
struct NfsBackgroundResourceOwner {uint64_t generation;uintptr_t contextIdentity;NfsBackgroundResourceOwnerCurrent current;void*user;};
#ifdef __cplusplus
extern "C" {
#endif
/* Startup provider bytes are immutable program-lifetime allocations. Fresh
   generation only, or exact already-ready reuse. No FBO/presentation/pass keys.
   Loss abandons names without foreign GL cleanup. Same-owner failure restores
   state and deletes only temporary objects. Full source digest/PMA checked cold. */
int nfs_bg_scene_prepare_owned(NfsBackgroundSceneCompositor*,const NfsBackgroundResourceOwner*);
#ifdef __cplusplus
}
#endif
#endif

// SPDX-License-Identifier: MIT
#pragma once
#include "SetupScenePrepare.h"
#include "SetupCaptionGameHooks.inc"
#include "ContinuousNativeUploadHooks.inc"
#include <memory>
#include <atomic>
// Fixed Setup-only owner and caption; never expands another form owner.
#define NFS_CACHED_MENU_OWNER_SCOPE SetupMenuOwnerScope
#define NFS_CACHED_MENU_SCOPE SetupScope
#define NFS_CACHED_MENU_LIVE_NAMESPACE nfs_setup_scene_live
#define NFS_CACHED_MENU_CPU_NAMESPACE nfs_setup_scene_cpu
#define NFS_CACHED_MENU_CAPTION_NAMESPACE nfs_setup_caption
#define NFS_CACHED_MENU_DESCRIPTOR off_4D2790
#define NFS_CACHED_MENU_FORM_TAG SetupForm
#define NFS_CACHED_MENU_TAIL NFS_BACKGROUND_TAIL_SETUP_CAPTION
#include "CachedMenuSceneLiveGameHooks.inc"
#undef NFS_CACHED_MENU_TAIL
#undef NFS_CACHED_MENU_FORM_TAG
#undef NFS_CACHED_MENU_DESCRIPTOR
#undef NFS_CACHED_MENU_CAPTION_NAMESPACE
#undef NFS_CACHED_MENU_CPU_NAMESPACE
#undef NFS_CACHED_MENU_LIVE_NAMESPACE
#undef NFS_CACHED_MENU_SCOPE
#undef NFS_CACHED_MENU_OWNER_SCOPE

// Observe the original early-return label, not a guessed timeout or renderer
// rejection. This does not grant publication authority to unfinished CPU work.
namespace nfs_setup_scene_live {
struct OriginalAbortReceipt {
 uint64_t ownerEpoch=0,iteration=0,uploadEvent=0,group=0,copy=0,collectorRevision=0;
 unsigned phase=0,groupDraws=0;
};
inline OriginalAbortReceipt&originalAbortReceipt()noexcept {
 static thread_local OriginalAbortReceipt receipt;return receipt;
}
inline void observeOriginalAbort(Game&g)noexcept {
 auto&r=originalAbortReceipt();r={};auto*s=current(g);
 auto*c=nfs_logo_publisher::store();
 if(!s||s->invalid||!s->iteration||s->nativeUploadEvent||s->published||s->invocationDepth!=1||!c||
    c->gameOwner!=s->game||c->thread!=s->thread||c->depth!=1||c->ownerKind!=1||
    c->ownerArg!=uint32_t(guest_address(off_4D2790))||c->group.thread!=s->thread||
    c->invalid||c->group.error||c->copy.error)return;
 const bool beforeGroup=!s->groupSerial&&!s->copySerial&&!s->scene.complete&&
    !c->group.complete&&!c->copy.complete&&!c->groupActive&&!c->copyActive&&
    !c->group.draws&&!c->copy.serial&&c->group.serial==c->iterationRevision&&
    c->group.thread==s->thread&&c->group.kind==1&&
    c->group.ownerArg==uint32_t(guest_address(off_4D2790));
 bool partialGroup=s->groupSerial&&s->groupSerial==c->group.serial&&
    c->group.serial==c->iterationRevision&&c->groupActive&&c->group.stage==1&&
    c->pending==-1&&c->group.pre.valid&&!c->group.complete&&
    (c->group.draws==1||c->group.draws==2)&&!s->copySerial&&!s->scene.complete&&
    !c->copy.serial&&!c->copy.complete&&!c->copyActive;
 if(partialGroup)for(unsigned k=0;k<c->group.draws;++k)
    if(!c->group.draw[k].complete||c->group.draw[k].error)partialGroup=false;
 if(!beforeGroup&&!partialGroup&&(!c->group.complete||!s->groupSerial||c->group.serial!=s->groupSerial))return;
 const bool copied=c->copy.complete&&s->copySerial&&
    c->copy.serial==s->copySerial&&c->copy.linkedGroup==s->groupSerial;
 r={s->ownerEpoch,s->iteration,
    nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire),
    s->groupSerial,copied?s->copySerial:0,c->iterationRevision,beforeGroup?0u:(partialGroup?3u:(copied?2u:1u)),c->group.draws};
 // Receipt is owner/iteration/event scoped; it alone never licenses reuse.
 const int saved=errno;const char*trace=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");
 if(trace&&trace[0]=='1'&&!trace[1])
  std::fprintf(stderr,"SETUP ORIGINAL ABORT phase=%u epoch=%llu iteration=%llu event=%llu group=%llu copy=%llu\n",
   r.phase,(unsigned long long)r.ownerEpoch,(unsigned long long)r.iteration,
   (unsigned long long)r.uploadEvent,(unsigned long long)r.group,(unsigned long long)r.copy);
 errno=saved;
}
}

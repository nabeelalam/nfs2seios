// SPDX-License-Identifier: MIT
#pragma once
#include "OptionsScenePrepare.h"
#include "OptionsCaptionGameHooks.inc"
#include "ContinuousNativeUploadHooks.inc"
#include <memory>
#include <atomic>
// Fixed Options-only owner and caption; never expands another form owner.
#define NFS_CACHED_MENU_OWNER_SCOPE OptionsMenuOwnerScope
#define NFS_CACHED_MENU_SCOPE OptionsScope
#define NFS_CACHED_MENU_LIVE_NAMESPACE nfs_options_scene_live
#define NFS_CACHED_MENU_CPU_NAMESPACE nfs_options_scene_cpu
#define NFS_CACHED_MENU_CAPTION_NAMESPACE nfs_options_caption
#define NFS_CACHED_MENU_DESCRIPTOR off_4D3A88
#define NFS_CACHED_MENU_FORM_TAG OptionsForm
#define NFS_CACHED_MENU_TAIL NFS_BACKGROUND_TAIL_OPTIONS_CAPTION
#include "CachedMenuSceneLiveGameHooks.inc"
#undef NFS_CACHED_MENU_TAIL
#undef NFS_CACHED_MENU_FORM_TAG
#undef NFS_CACHED_MENU_DESCRIPTOR
#undef NFS_CACHED_MENU_CAPTION_NAMESPACE
#undef NFS_CACHED_MENU_CPU_NAMESPACE
#undef NFS_CACHED_MENU_LIVE_NAMESPACE
#undef NFS_CACHED_MENU_SCOPE
#undef NFS_CACHED_MENU_OWNER_SCOPE

// Observe only the exact original redraw early-return label. No retained owner is minted.
namespace nfs_options_scene_live {
inline void observeReturnOriginalAbort(Game&g)noexcept {
 const int saved=errno;const char*t=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");static thread_local unsigned lines;
 if(!t||t[0]!='1'||t[1]||lines>=16){errno=saved;return;}auto*s=current(g);if(!s){errno=saved;return;}++lines;auto*c=nfs_logo_publisher::store();
 std::fprintf(stderr,"OPTIONS RETURN ORIGINAL ABORT epoch=%llu iteration=%llu invalid=%u published=%u stateUpload=%llu liveUpload=%llu stateCopy=%llu stateGroup=%llu invocation=%u collector=%u collectorDepth=%u collectorInvalid=%u collectorGroupComplete=%u collectorCopyComplete=%u collectorRevision=%llu groupDraws=%u\n",(unsigned long long)s->ownerEpoch,(unsigned long long)s->iteration,unsigned(s->invalid),unsigned(bool(s->published)),(unsigned long long)s->nativeUploadEvent,(unsigned long long)nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire),(unsigned long long)s->copySerial,(unsigned long long)s->groupSerial,s->invocationDepth,unsigned(bool(c)),c?c->depth:0,unsigned(c&&c->invalid),unsigned(c&&c->group.complete),unsigned(c&&c->copy.complete),(unsigned long long)(c?c->iterationRevision:0),c?c->group.draws:0);errno=saved;
}
}

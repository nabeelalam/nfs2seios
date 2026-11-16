// SPDX-License-Identifier: MIT
#pragma once
#include "OpponentSceneLiveGameHooks.inc"
#include "BackgroundSceneQueueAdapter.inc"
#include "MainThreadBridge.h"
#include "ChildStaticSceneHistory.h"
#include <stdexcept>
namespace nfs_opponent_scene_live {
struct Intent{std::shared_ptr<const bg::Prepared>owned;uint64_t serial=0;bool beginConsumed=false,finishAttempted=false,finishConsumed=false;};
// Main and Opponent share one renderer/compositor serial and active owner path.
inline uint64_t nextRequest()noexcept{return nfs_background_scene_live::nextRequest();}
inline uint64_t&activeRequest()noexcept{return nfs_background_scene_live::activeRequest();}
struct Builder{State*s=nullptr;nfs_logo_publisher::Collector*c=nullptr;std::shared_ptr<const bg::Prepared>owned;};
inline bool sourceSnapshot(void*context,const nfs_logo_publisher::OwnedPacket&packet)noexcept{auto&b=*static_cast<Builder*>(context);if(!b.s||!b.c||b.s->invalid||!b.s->published||&packet!=&b.c->packet||!packet.prepared.valid||b.s->copySerial!=b.c->copy.serial||b.s->groupSerial!=b.c->group.serial||b.s->nativeUploadEvent!=b.c->nativeUploadEvent||b.s->published->original!=packet.originalCopyPost565||b.s->published->keys!=b.c->rendererAddresses)return false;b.owned=b.s->published;return true;}
inline void cancel(Intent&i)noexcept{if(activeRequest()==i.serial)activeRequest()=0;i.owned.reset();i.serial=0;i.beginConsumed=false;}
inline void enqueueBegin(const std::shared_ptr<const bg::Prepared>&owned,Intent&i,State&source){if(!enabled()||!owned||!owned->valid||activeRequest()||i.serial||i.beginConsumed)return;const uint64_t serial=nextRequest();if(!serial)return;NfsMenuCachedSnapshot responsive{};const int priorErrno=errno;try{nfs_menu_cached_snapshot(0x4d3204u,uint32_t(guest_address(off_4D3204)),source.ownerEpoch,source.game,serial,owned->contentRevision,&responsive);}catch(...){nfs_menu_pointer_metadata_failure();}errno=priorErrno;i.owned=owned;i.serial=serial;nfs::check_worker_stop();const auto receipt=nfs::enqueue_owned_renderer([owned,serial,responsive](){auto view=owned->view();view.scene.serial=serial;view.responsive=responsive;return int32_t(nfs_private_opponent_scene_begin(&view));});if(receipt==nfs::OwnedRendererAcceptance::UseExistingOwnerPath){i.owned.reset();i.serial=0;return;}i.beginConsumed=true;activeRequest()=serial;nfs::check_worker_stop();}
inline void begin(Game&g,Intent&i){auto*s=current(g);auto*c=nfs_logo_publisher::store();if(!s||s->invalid||!s->published||!c||activeRequest()||i.serial||i.beginConsumed)return;if(!s->published->captionProof||!s->published->captionProof->current()||s->published->captionProof->packet()!=s->caption||!captionCurrent(g,*s,c->copy)||!previewCurrent(*s)){s->fail(Reason::Missing);return;}if(!archiveCurrent(*s)){s->fail(Reason::Source,true);return;}Builder b{s,c,{}};if(!nfs_logo_publisher::publishNativeEntry(g,sourceSnapshot,&b,0)){return;}
 // Original40D1B0 may enter after MT slot0 was legitimately retired. Retain
 // only THIS exact accepted completed full+343 source in the current menu scope;
 // no unfinished collector, numeric generation or last-rendered CPU fallback.
 if(b.owned){NfsChildExitPreludeView v{};v.family=NFS_CHILD_EXIT_OPPONENT;v.form=0x4d3204u;v.opponent=b.owned->view();nfs_child_static_scene::Domain d{s->game,s->thread,uint32_t(guest_address(off_4D3204)),s->ownerEpoch,s->archive,s->nativeUploadEvent,s->groupSerial,s->copySerial,s->catalogue};const bool retained=nfs_child_static_scene::remember(d,b.owned,v,c->copy);nfs_child_static_scene::diagnostic(retained?10:11,s->ownerEpoch,s->nativeUploadEvent);}
 enqueueBegin(b.owned,i,*s);}
inline void finish(Intent&i){if(!i.beginConsumed||i.finishAttempted)return;if(!i.serial||activeRequest()!=i.serial){cancel(i);return;}nfs::check_worker_stop();i.finishAttempted=true;const auto owned=i.owned;const uint64_t serial=i.serial;const auto receipt=nfs::enqueue_owned_renderer([owned,serial](){(void)owned;return int32_t(nfs_private_opponent_scene_finish(serial));});if(receipt!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("background scene finish changed renderer owner");i.finishConsumed=true;activeRequest()=0;nfs::check_worker_stop();}
} // namespace nfs_opponent_scene_live

// SPDX-License-Identifier: MIT
#pragma once
#include "OptionsSceneLiveGameHooks.inc"
#include "BackgroundSceneQueueAdapter.inc"
#include "MainThreadBridge.h"
#include <stdexcept>
namespace nfs_options_scene_live {
struct Intent{std::shared_ptr<const bg::Prepared>owned;uint64_t serial=0;bool beginConsumed=false,finishAttempted=false,finishConsumed=false;};
// Main, Location and Options share one renderer/compositor serial and active owner path.
inline uint64_t nextRequest()noexcept{return nfs_background_scene_live::nextRequest();}
inline uint64_t&activeRequest()noexcept{return nfs_background_scene_live::activeRequest();}
struct Builder{State*s=nullptr;nfs_logo_publisher::Collector*c=nullptr;std::shared_ptr<const bg::Prepared>owned;};
inline bool sourceSnapshot(void*context,const nfs_logo_publisher::OwnedPacket&packet)noexcept{auto&b=*static_cast<Builder*>(context);if(!b.s||!b.c||b.s->invalid||!b.s->published||&packet!=&b.c->packet||!packet.prepared.valid||b.s->copySerial!=b.c->copy.serial||b.s->groupSerial!=b.c->group.serial||b.s->nativeUploadEvent!=b.c->nativeUploadEvent||b.s->published->original!=packet.originalCopyPost565||b.s->published->keys!=b.c->rendererAddresses)return false;b.owned=b.s->published;return true;}
inline void cancel(Intent&i)noexcept{if(activeRequest()==i.serial)activeRequest()=0;i.owned.reset();i.serial=0;i.beginConsumed=false;}
inline void enqueueBegin(const std::shared_ptr<const bg::Prepared>&owned,Intent&i){if(!enabled()||!owned||!owned->valid||activeRequest()||i.serial||i.beginConsumed)return;const uint64_t serial=nextRequest();if(!serial)return;i.owned=owned;i.serial=serial;nfs::check_worker_stop();const auto receipt=nfs::enqueue_owned_renderer([owned,serial](){auto view=owned->view();view.scene.serial=serial;return int32_t(nfs_private_options_scene_begin(&view));});if(receipt==nfs::OwnedRendererAcceptance::UseExistingOwnerPath){i.owned.reset();i.serial=0;return;}i.beginConsumed=true;activeRequest()=serial;nfs::check_worker_stop();}
inline void begin(Game&g,Intent&i){auto*s=current(g);auto*c=nfs_logo_publisher::store();if(!s||s->invalid||!s->published||!c||activeRequest()||i.serial||i.beginConsumed)return;if(!captionCurrent(g,*s,c->copy)){s->fail(Reason::Missing);return;}if(!archiveCurrent(*s)){s->fail(Reason::Source,true);return;}Builder b{s,c,{}};if(!nfs_logo_publisher::publishNativeEntry(g,sourceSnapshot,&b,0)){return;}enqueueBegin(b.owned,i);}
inline void finish(Intent&i){if(!i.beginConsumed||i.finishAttempted)return;if(!i.serial||activeRequest()!=i.serial){cancel(i);return;}nfs::check_worker_stop();i.finishAttempted=true;const auto owned=i.owned;const uint64_t serial=i.serial;const auto receipt=nfs::enqueue_owned_renderer([owned,serial](){(void)owned;return int32_t(nfs_private_options_scene_finish(serial));});if(receipt!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("background scene finish changed renderer owner");i.finishConsumed=true;activeRequest()=0;nfs::check_worker_stop();}
} // namespace nfs_options_scene_live

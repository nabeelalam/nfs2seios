// SPDX-License-Identifier: MIT
#pragma once
#include "AudioSidebarLiveGameHooks.inc"
#include "AudioSceneLease.h"
#include "BackgroundSceneQueueAdapter.inc"
#include "MainThreadBridge.h"
#include <stdexcept>
namespace nfs_audio_scene_live {
namespace live=nfs_audio_live;namespace cache=nfs_audio_sidebar_live;
using Prepared=nfs_audio_scene_cpu::Prepared;using Lease=nfs_audio_scene_cpu::Lease;
struct Intent {std::shared_ptr<const Lease>owned;uint64_t serial=0;bool beginConsumed=false,finishAttempted=false,finishConsumed=false;};
inline uint64_t&activeRequest()noexcept{return nfs_background_scene_live::activeRequest();}
inline void cancel(Intent&i)noexcept{if(activeRequest()==i.serial)activeRequest()=0;i.owned.reset();i.serial=0;i.beginConsumed=false;}
inline void begin(Game&g,Intent&i){
 if(!nfs_background_scene_live::enabled()||activeRequest()||i.serial||i.beginConsumed)return;
 auto*s=live::current(g);auto*c=cache::current(g);auto frame=live::lastPacket(g);if(!s||!c||!frame||!live::bound(*s,g,true))return;
 // Exact full640 source entering original426950, not another form's cached pass.
 if(uint32_t(g.eax)!=s->binding.surface.geometry[8])return;
 try{nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D4D24)!=s->binding.owner)return;}catch(...){return;}
 nfs_logo_capture::Target target{},cacheTarget{};std::vector<uint16_t>post;
 if(!live::plane(post,target)||!nfs_logo_capture::same(target,s->target)||!frame->matches(s->binding,s->acceptedTextSerial,post.data(),post.size()))return;
 auto sidebar=cache::acquireForSidebar(g,s->background.sidebar,&cacheTarget);if(!sidebar)return;
 const auto binding=s->binding;const auto cacheBinding=c->binding;
 auto owned=Prepared::create(frame,sidebar,binding,s->acceptedTextSerial,post.data(),post.size(),cacheBinding,cacheTarget,s->background.sidebar.sourceBefore.data(),s->background.sidebar.sourceBefore.size());
 if(!owned||!live::bound(*s,g,true)||!live::plane(post,target)||!nfs_logo_capture::same(target,s->target)||!frame->matches(binding,s->acceptedTextSerial,post.data(),post.size()))return;
 auto lease=Lease::create(owned);if(!lease)return;const uint64_t serial=nfs_background_scene_live::nextRequest();if(!serial)return;i.owned=lease;i.serial=serial;
 nfs::check_worker_stop();const auto receipt=nfs::enqueue_owned_renderer([lease,serial](){auto view=lease->view(serial);return int32_t(nfs_private_audio_scene_begin(&view));});
 if(receipt==nfs::OwnedRendererAcceptance::UseExistingOwnerPath){i.owned.reset();i.serial=0;return;}
 i.beginConsumed=true;activeRequest()=serial;nfs::check_worker_stop();
}
inline void finish(Intent&i){
 if(!i.beginConsumed||i.finishAttempted)return;if(!i.serial||activeRequest()!=i.serial){cancel(i);return;}
 nfs::check_worker_stop();i.finishAttempted=true;const auto owned=i.owned;const uint64_t serial=i.serial;
 const auto receipt=nfs::enqueue_owned_renderer([owned,serial](){(void)owned;return int32_t(nfs_private_audio_scene_finish(serial));});
 if(receipt!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("Audio scene finish changed renderer owner");i.finishConsumed=true;activeRequest()=0;nfs::check_worker_stop();
}
} // namespace nfs_audio_scene_live

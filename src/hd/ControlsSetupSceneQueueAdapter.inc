// SPDX-License-Identifier: MIT
#pragma once
#ifndef NFS_CONTROLS_SETUP_QUEUE_RUNTIME_TEST
#include "ControlsSetupSidebarLiveGameHooks.inc"
#include "ControlsSetupLiveGameHooks.inc"
#include "BackgroundSceneQueueAdapter.inc"
#endif
#include "ControlsSetupScenePrepare.h"
#include "MainThreadBridge.h"
#include <stdexcept>
namespace nfs_controls_setup_scene_live {
namespace live=nfs_controls_setup_live;namespace cache=nfs_controls_setup_sidebar_live;
using Prepared=nfs_controls_setup_scene_cpu::Prepared;using Lease=nfs_controls_setup_scene_cpu::Lease;
struct Intent {std::shared_ptr<const Lease>owned;uint64_t serial=0;bool beginConsumed=false,finishAttempted=false,finishConsumed=false;};
inline uint64_t&activeRequest()noexcept{return nfs_background_scene_live::activeRequest();}
inline void cancel(Intent&i)noexcept{if(activeRequest()==i.serial)activeRequest()=0;i.owned.reset();i.serial=0;i.beginConsumed=false;}
// Bounded existing private trace; copied CPU metadata only, no guest reads.
inline void trace(unsigned reason,const char*name,const live::State*s)noexcept{
 static const bool on=[](){const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");return p&&p[0]=='1'&&!p[1];}();
 static thread_local uint32_t seen=0;if(!on||reason>=32||(seen&(1u<<reason)))return;seen|=1u<<reason;
 std::fprintf(stderr,"CONTROLS SETUP QUEUE admission reason=%s pass=%llu serial=%llu\n",name,(unsigned long long)(s?s->binding.pass:0),(unsigned long long)(s?s->acceptedTextSerial:0));
}
inline void begin(Game&g,Intent&i){
 if(!nfs_background_scene_live::enabled()||activeRequest()||i.serial||i.beginConsumed)return;
 auto*s=live::current(g);if(!s)return;auto*c=cache::current(g);auto frame=live::acquireCurrentPacket(g);if(!c){trace(1,"cache-owner",s);return;}if(!frame||nfs_controls_setup_text::acquireCurrentPacket(g)!=frame->text()){trace(2,"current-frame",s);return;}if(!live::bound(*s,g,true)||!((s->form==uint32_t(guest_address(off_4D349C))&&s->handler==0x4101c0u)||(s->form==uint32_t(guest_address(off_4D35E8))&&s->handler==0x403970u))||s->epoch!=c->epoch){trace(3,"bound-archive",s);return;}
 // Exact full640 source entering original426950, not another form's cached pass.
 if(uint32_t(g.eax)!=s->binding.surface.geometry[8]){trace(4,"native-source-bitmap",s);return;}
 bool sourceOwner=false;try{nfs::GuestCopyTransaction tx;sourceOwner=tx.load<uint32_t>(dword_4D4D24)==s->binding.owner;}catch(...){trace(6,"native-owner-read",s);return;}if(!sourceOwner){trace(5,"native-source-owner",s);return;}
 nfs_logo_capture::Target target{},cacheTarget{};std::vector<uint16_t>post;
 if(!live::plane(post,target)||!nfs_logo_capture::same(target,s->target)||!frame->matches(s->binding,s->acceptedTextSerial,post.data(),post.size())){trace(7,"current-full-plane",s);return;}
 nfs_graphics_frame::SidebarBinding currentSidebar;
 if(!live::acquireCurrentSidebarBinding(g,currentSidebar)){trace(15,"current-sidebar-binding",s);return;}
 auto sidebar=cache::acquireForSidebar(g,s->background.sidebar,currentSidebar,&cacheTarget);if(!sidebar){trace(8,"current-sidebar",s);return;}
 const auto captionProof=c->captionProof;
 if(sidebar->captionWord()||!captionProof||captionProof->packet()!=sidebar->caption()||
    nfs_controls_setup_sidebar_text::acquireRetainedProof(g,captionProof,sidebar->caption(),c->captionSerial,c->captionLifetime,cacheTarget,s->background.sidebar.sourceBefore.data(),s->background.sidebar.sourceBefore.size())!=captionProof){trace(16,"current-sidebar-word",s);return;}
 auto caption=nfs_controls_setup_sidebar_caption::Display::create(captionProof,sidebar->caption(),{s->form,s->handler,reinterpret_cast<uintptr_t>(&g),s->epoch,s->archive,c->captionSerial,c->captionLifetime});
 if(!caption){trace(17,"immutable-sidebar-word",s);return;}
 const auto binding=s->binding;const auto cacheBinding=c->binding;
 auto owned=Prepared::create(frame,sidebar,binding,s->acceptedTextSerial,post.data(),post.size(),currentSidebar,cacheBinding,cacheTarget,s->background.sidebar.sourceBefore.data(),s->background.sidebar.sourceBefore.size(),caption);
 if(!owned){trace(9,"immutable-scene-join",s);return;}
 // A candidate lease is not authority: independently reacquire every current
 // Controls owner, retained343 source/caption and full text packet after join.
 nfs_graphics_frame::SidebarBinding repeatSidebar;nfs_logo_capture::Target repeatCacheTarget{};
 if(live::current(g)!=s||cache::current(g)!=c||!live::bound(*s,g,true)||!(s->binding==binding)||!(c->binding==cacheBinding)||
    live::acquireCurrentPacket(g)!=frame||nfs_controls_setup_text::acquireCurrentPacket(g)!=frame->text()||
    !live::acquireCurrentSidebarBinding(g,repeatSidebar)||!(repeatSidebar==currentSidebar)||
    cache::acquireForSidebar(g,s->background.sidebar,repeatSidebar,&repeatCacheTarget)!=sidebar||!nfs_logo_capture::same(cacheTarget,repeatCacheTarget)||c->captionProof!=captionProof||
    nfs_controls_setup_sidebar_text::acquireRetainedProof(g,captionProof,sidebar->caption(),c->captionSerial,c->captionLifetime,repeatCacheTarget,s->background.sidebar.sourceBefore.data(),s->background.sidebar.sourceBefore.size())!=captionProof||
    !live::plane(post,target)||!nfs_logo_capture::same(target,s->target)||!frame->matches(binding,s->acceptedTextSerial,post.data(),post.size())){trace(10,"repeat-full-plane",s);return;}
 bool repeatOwner=false;try{nfs::GuestCopyTransaction tx;repeatOwner=tx.load<uint32_t>(dword_4D4D24)==binding.owner;}catch(...){trace(6,"native-owner-read",s);return;}
 if(!repeatOwner||uint32_t(g.eax)!=binding.surface.geometry[8]){trace(5,"native-source-owner",s);return;}
 auto lease=Lease::create(owned);if(!lease){trace(11,"lease-allocation",s);return;}const uint64_t serial=nfs_background_scene_live::nextRequest();if(!serial){trace(12,"serial-overflow",s);return;}i.owned=lease;i.serial=serial;
 nfs::check_worker_stop();const auto receipt=nfs::enqueue_owned_renderer([lease,serial](){auto view=lease->view(serial);return int32_t(nfs_private_controls_setup_scene_begin(&view));});
 if(receipt==nfs::OwnedRendererAcceptance::UseExistingOwnerPath){i.owned.reset();i.serial=0;trace(13,"existing-owner-path",s);return;}
 i.beginConsumed=true;activeRequest()=serial;trace(14,"begin-enqueued",s);nfs::check_worker_stop();
}
inline void finish(Intent&i){
 if(!i.beginConsumed||i.finishAttempted)return;if(!i.serial||activeRequest()!=i.serial){cancel(i);return;}
 nfs::check_worker_stop();i.finishAttempted=true;const auto owned=i.owned;const uint64_t serial=i.serial;
 const auto receipt=nfs::enqueue_owned_renderer([owned,serial](){(void)owned;return int32_t(nfs_private_controls_setup_scene_finish(serial));});
 if(receipt!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("Child Controls scene finish changed renderer owner");i.finishConsumed=true;activeRequest()=0;nfs::check_worker_stop();
}
} // namespace nfs_controls_setup_scene_live

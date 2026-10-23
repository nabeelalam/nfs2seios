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
// Bounded existing private trace; copied CPU metadata only, no guest reads.
inline void trace(unsigned reason,const char*name,const live::State*s)noexcept{
 static const bool on=[](){const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");return p&&p[0]=='1'&&!p[1];}();
 static thread_local uint32_t seen=0;if(!on||reason>=32||(seen&(1u<<reason)))return;seen|=1u<<reason;
 std::fprintf(stderr,"AUDIO QUEUE admission reason=%s pass=%llu serial=%llu\n",name,(unsigned long long)(s?s->binding.pass:0),(unsigned long long)(s?s->acceptedTextSerial:0));
}
// Bounded existing private diagnostic: immutable source metadata only.
inline void traceDialog(uint64_t request,const nfs_audio_dialog_word::Proof&proof)noexcept{
 static const bool on=[](){const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");return p&&p[0]=='1'&&!p[1];}();
 static thread_local unsigned count=0,lastSelected=UINT32_MAX;static thread_local std::array<uint16_t,8>lastColors{};
 if(!on||count>=32)return;const auto&e=proof.evidence();std::array<uint16_t,8>colors{};for(unsigned k=0;k<8;++k)colors[k]=proof.packet()->records()[nfs_audio_dialog_word::record(k)+1].command.color;
 if(lastSelected==e.authority.selectedIndex&&lastColors==colors)return;lastSelected=e.authority.selectedIndex;lastColors=colors;++count;
 std::fprintf(stderr,"AUDIO DIALOG source request=%llu selected=%u serial=%llu colors=",(unsigned long long)request,e.authority.selectedIndex,(unsigned long long)e.serial);for(unsigned k=0;k<8;++k)std::fprintf(stderr,"%s%u",k?",":"",unsigned(colors[k]));std::fprintf(stderr,"\n");
}
inline void begin(Game&g,Intent&i){
 if(!nfs_background_scene_live::enabled()||activeRequest()||i.serial||i.beginConsumed)return;
 auto*s=live::current(g);if(!s)return;auto*c=cache::current(g);auto frame=live::lastPacket(g);if(!c){trace(1,"cache-owner",s);return;}if(!frame){trace(2,"current-frame",s);return;}if(!live::bound(*s,g,true)){trace(3,"bound-archive",s);return;}
 // Exact full640 source entering original426950, not another form's cached pass.
 if(uint32_t(g.eax)!=s->binding.surface.geometry[8]){trace(4,"native-source-bitmap",s);return;}
 bool sourceOwner=false;try{nfs::GuestCopyTransaction tx;sourceOwner=tx.load<uint32_t>(dword_4D4D24)==s->binding.owner;}catch(...){trace(6,"native-owner-read",s);return;}if(!sourceOwner){trace(5,"native-source-owner",s);return;}
 nfs_logo_capture::Target target{},cacheTarget{};std::vector<uint16_t>post;
 if(!live::plane(post,target)||!nfs_logo_capture::same(target,s->target)||!frame->matches(s->binding,s->acceptedTextSerial,post.data(),post.size())){trace(7,"current-full-plane",s);return;}
 auto sidebar=cache::acquireForSidebar(g,s->background.sidebar,&cacheTarget);if(!sidebar){trace(8,"current-sidebar",s);return;}
 const auto&word=sidebar->captionWord();if(!word||!word->current()||word->packet()!=sidebar->caption()||!word->proof()||word->proof()->evidence().policy!=nfs_nested_sidebar_caption::Policy::Audio){trace(16,"current-sidebar-word",s);return;}
 auto dialogProof=nfs_audio_text::acquireCurrentDialogProof(g);
 auto dialog=nfs_audio_dialog_display::Prepared::create(dialogProof,frame->text(),{s->binding.form,s->binding.archive,reinterpret_cast<uintptr_t>(&g),s->acceptedTextSerial,s->binding.lifetime});
 if(!dialog){trace(17,"current-dialog-words",s);return;}
 const auto binding=s->binding;const auto cacheBinding=c->binding;
 auto owned=Prepared::create(frame,sidebar,binding,s->acceptedTextSerial,post.data(),post.size(),cacheBinding,cacheTarget,s->background.sidebar.sourceBefore.data(),s->background.sidebar.sourceBefore.size(),dialog);
 if(!owned){trace(9,"immutable-scene-join",s);return;}if(!live::bound(*s,g,true)||!live::plane(post,target)||!nfs_logo_capture::same(target,s->target)||!frame->matches(binding,s->acceptedTextSerial,post.data(),post.size())){trace(10,"repeat-full-plane",s);return;}
 if(nfs_audio_text::acquireCurrentDialogProof(g)!=dialogProof){trace(18,"repeat-dialog-words",s);return;}
 auto lease=Lease::create(owned);if(!lease){trace(11,"lease-allocation",s);return;}const uint64_t serial=nfs_background_scene_live::nextRequest();if(!serial){trace(12,"serial-overflow",s);return;}i.owned=lease;i.serial=serial;traceDialog(serial,*dialogProof);
 nfs::check_worker_stop();const auto receipt=nfs::enqueue_owned_renderer([lease,serial](){auto view=lease->view(serial);return int32_t(nfs_private_audio_scene_begin(&view));});
 if(receipt==nfs::OwnedRendererAcceptance::UseExistingOwnerPath){i.owned.reset();i.serial=0;trace(13,"existing-owner-path",s);return;}
 i.beginConsumed=true;activeRequest()=serial;trace(14,"begin-enqueued",s);nfs::check_worker_stop();
}
inline void finish(Intent&i){
 if(!i.beginConsumed||i.finishAttempted)return;if(!i.serial||activeRequest()!=i.serial){cancel(i);return;}
 nfs::check_worker_stop();i.finishAttempted=true;const auto owned=i.owned;const uint64_t serial=i.serial;
 const auto receipt=nfs::enqueue_owned_renderer([owned,serial](){(void)owned;return int32_t(nfs_private_audio_scene_finish(serial));});
 if(receipt!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("Audio scene finish changed renderer owner");i.finishConsumed=true;activeRequest()=0;nfs::check_worker_stop();
}
} // namespace nfs_audio_scene_live

// SPDX-License-Identifier: MIT
#pragma once
#ifndef NFS_PLAYER_INITIAL_NATIVE_RUNTIME_TEST
#include "PlayerSceneLiveGameHooks.inc"
#endif
#include "PlayerInitialNativeUploadWitness.h"
#include <new>
// Original-preserving slot0 observer. No rendering, trace stream, or slot1
// counter reuse. Every large native buffer is allocated outside guest leases.
namespace nfs_player_initial_native_live {
namespace native=nfs_player_initial_native;namespace scene=nfs_player_scene_live;
using Binding=native::Binding;using Plane=native::Plane;using Source=native::Source;
enum class Failure{None,Owner,Nested,Event,Source,Binding,Read,Stage,Tile,Order,Unwind,Incomplete,Allocation,Retire};
enum class Phase{Entry,Staged,Finished};
struct State{
 native::Witness witness;std::shared_ptr<const Source>source;Plane sourceScratch{},stageScratch{};
 uintptr_t game=0,thread=0;uint64_t event=0;uint32_t stageOwner=0;unsigned depth=0;Phase phase=Phase::Entry;bool invalid=false,tileActive=false,returned=false;Failure failure=Failure::None;
};
inline State*&slot()noexcept{struct Storage{State*p=nullptr;~Storage(){delete p;}};static thread_local Storage storage;return storage.p;}
inline void reject(State&s,Failure reason)noexcept{s.witness.reset();s.source.reset();s.invalid=true;s.tileActive=s.returned=false;s.failure=reason;}
inline void reset()noexcept{if(auto*s=slot())reject(*s,Failure::Owner);}
// No occurrence parameter supplies authority. Independently reconstruct the
// currently owned Player form/archive/thread/epoch and exact pending source.
inline bool currentBinding(Game&g,Binding&out)noexcept{
 out={};auto*l=scene::current(g);if(!l||l->invalid||!l->on||l->game!=reinterpret_cast<uintptr_t>(&g)||l->thread!=nfs_logo_publisher::threadToken()||!l->ownerEpoch||(l->form!=0x4d3050&&l->form!=0x4d3100)||l->formGuest!=scene::relocatedForm(l->form)||l->sourceOwner!=l->formGuest||!scene::initialSidebarCurrent(*l)||!scene::archiveCurrent(*l))return false;
 const auto source=l->pendingInitialSidebar;if(!source||!source->current())return false;const auto&b=source->binding();out.sidebar={l->form,l->formGuest,l->archive,l->game,l->thread,l->ownerEpoch,b.captionEvent};out.sourceOwner=source->post().target.owner;out.sourceBitmap=source->post().target.w[8];
 if(!native::bindingMatches(*source,out,l->catalogue)||l!=scene::current(g)||l->pendingInitialSidebar!=source){out={};return false;}return true;
}
inline bool current(State&s,Game&g,Binding&out)noexcept{
 out={};if(s.invalid||!s.source||s.game!=reinterpret_cast<uintptr_t>(&g)||s.thread!=nfs_logo_publisher::threadToken()||!s.event||native::uploadEvents().load(std::memory_order_acquire)!=s.event)return false;
 auto*l=scene::current(g);return l&&l->pendingInitialSidebar==s.source&&currentBinding(g,out)&&l==scene::current(g)&&l->pendingInitialSidebar==s.source&&native::bindingMatches(*s.source,out,l->catalogue)&&native::uploadEvents().load(std::memory_order_acquire)==s.event;
}
inline bool readSource(Plane&out,const Source&source)noexcept{
 out.valid=0;try{const auto&reference=source.post().target;{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,out.target,reference.owner);if(!nfs_main_sidebar_origin::sameRetainedTarget(reference,out.target)||!nfs_background_scene_cpu::fullClip(out.target,343)||out.target.header[3])return false;out.width=343;out.height=480;nfs_logo_publisher::checked(out.target.w[8],size_t(native::SourceBytes));std::memcpy(out.pixels.data(),nfs_host_address(out.target.w[8]+16,nfs_background_scene_cpu::Side*2),nfs_background_scene_cpu::Side*2);nfs_logo_capture::Target after{};nfs_logo_publisher::target(tx,after,reference.owner);if(!nfs_logo_capture::same(after,out.target))return false;out.valid=1;}return native::sourceMatches(source,out);}catch(...){return false;}
}
// Temporary492820 storage is not one of the four persistent publisher
// handles. Establish its owner only by independently supplied ESP+8 allocation
// and exact selected/descriptor identity; never broaden publisher::target.
inline void stageTarget(nfs::GuestCopyTransaction&tx,nfs_logo_capture::Target&target,uint32_t allocated){
 if(!native::span(allocated,56))throw 1;nfs_logo_publisher::checked(allocated,56);
 const uint32_t selected=uint32_t(guest_address(dword_4DB240));for(unsigned k=0;k<14;++k){target.w[k]=tx.load<uint32_t>(selected+4*k);target.ownerWords[k]=tx.load<uint32_t>(allocated+4*k);}
 for(unsigned k:{0u,1u,2u,7u,8u,10u,11u,12u,13u})if(target.w[k]!=target.ownerWords[k])throw 1;
 const auto&w=target.w;if(w[1]!=640||w[2]!=480||w[7]!=0x7810||w[10]!=1280||w[13]||!native::span(w[8],native::StageBytes)||!native::span(w[12],640*4)||!native::span(w[11],480*4))throw 1;
 nfs_logo_publisher::checked(w[8],size_t(native::StageBytes));nfs_logo_publisher::checked(w[12],640*4);nfs_logo_publisher::checked(w[11],480*4);for(unsigned k=0;k<4;++k)target.header[k]=tx.load<uint32_t>(w[8]+4*k);for(unsigned k=0;k<640;++k)target.x[k]=tx.load<uint32_t>(w[12]+4*k);for(unsigned k=0;k<480;++k)target.y[k]=tx.load<uint32_t>(w[11]+4*k);
 target.ownerState={{tx.load<uint32_t>(dword_4D2314+2),tx.load<uint32_t>(dword_4D231C),tx.load<uint32_t>(dword_4D2320),tx.load<uint32_t>(dword_4D4D24),tx.load<uint32_t>(dword_4DABE4)}};
 target.owner=allocated;if(!nfs_background_scene_cpu::fullClip(target,640)||target.header[3])throw 1;
}
inline bool readStage(Plane&out,uint32_t allocated)noexcept{out.valid=0;try{nfs::GuestCopyTransaction tx;stageTarget(tx,out.target,allocated);out.width=640;out.height=480;std::memcpy(out.pixels.data(),nfs_host_address(out.target.w[8]+16,nfs_background_scene_cpu::Full*2),nfs_background_scene_cpu::Full*2);nfs_logo_capture::Target after{};stageTarget(tx,after,allocated);if(!nfs_logo_capture::same(out.target,after))return false;out.valid=1;return true;}catch(...){return false;}}
struct UploadScope{
 Game&g;State*s=nullptr;bool entered=false;int unwind=0;
 explicit UploadScope(Game&game)noexcept:g(game){if(uint32_t(g.edx)!=0)return;
  // This happens even when optional remastering/owner matching is disabled.
  const uint64_t event=native::overwriteEvent();auto*&p=slot();if(p){p->witness.slot0Overwrite();p->source.reset();p->returned=false;if(p->depth){reject(*p,Failure::Nested);return;}}
  if(!nfs_logo_publisher::enabled()||!event||event==UINT64_MAX)return;Binding binding{};if(!currentBinding(g,binding)||uint32_t(g.eax)!=binding.sourceBitmap)return;auto*l=scene::current(g);if(!l||!l->pendingInitialSidebar)return;
  if(!p)p=new(std::nothrow)State;if(!p)return;s=p;s->source=l->pendingInitialSidebar;s->game=reinterpret_cast<uintptr_t>(&g);s->thread=nfs_logo_publisher::threadToken();s->event=event;s->depth=1;s->stageOwner=0;s->phase=Phase::Entry;s->invalid=s->tileActive=s->returned=false;s->failure=Failure::None;entered=true;unwind=nfs_logo_publisher::exceptions();
  if(!current(*s,g,binding)||!readSource(s->sourceScratch,*s->source)||!current(*s,g,binding))reject(*s,Failure::Source);
 }
 ~UploadScope(){if(!entered)return;Binding binding{};
  // stageFinish already authenticated the temporary640 before original free.
  // Never touch its owner/pixels here: original428250 has retired that storage.
  if(nfs_logo_publisher::exceptions()>unwind)reject(*s,Failure::Unwind);else if(!s->invalid){if(s->phase!=Phase::Finished||s->tileActive||!current(*s,g,binding)||!readSource(s->sourceScratch,*s->source)||!current(*s,g,binding)||!s->witness.proofForUse(s->sourceScratch,binding,scene::current(g)->catalogue))reject(*s,Failure::Incomplete);else s->returned=true;}s->depth=0;
 }
};
// Insert ONCE immediately before the loc4282FB definition. Original loop jumps
// target the label and skip this observation. A duplicate explicit call fails.
inline void stageBegin(Game&g)noexcept{
 auto*s=slot();if(!s||s->depth!=1||s->invalid||s->game!=reinterpret_cast<uintptr_t>(&g)||s->thread!=nfs_logo_publisher::threadToken())return;if(s->phase!=Phase::Entry||s->tileActive){reject(*s,Failure::Order);return;}Binding binding{};if(!current(*s,g,binding)){reject(*s,Failure::Binding);return;}
 uint32_t allocated=0;try{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);if(sp>UINT32_MAX-12)throw 1;allocated=tx.load<uint32_t>(sp+8);}catch(...){reject(*s,Failure::Read);return;}
 if(!readSource(s->sourceScratch,*s->source)||!readStage(s->stageScratch,allocated)||s->stageScratch.target.owner!=allocated||!current(*s,g,binding)){reject(*s,Failure::Stage);return;}
 if(!s->witness.begin(s->source,binding,s->sourceScratch,s->stageScratch,s->event,scene::current(g)->catalogue)){reject(*s,Failure::Stage);return;}s->stageOwner=allocated;s->phase=Phase::Staged;
}
// Insert just before the actual temporary482DA0, after all300 original tiles.
inline void stageFinish(Game&g)noexcept{
 auto*s=slot();if(!s||s->depth!=1||s->invalid||s->game!=reinterpret_cast<uintptr_t>(&g)||s->thread!=nfs_logo_publisher::threadToken())return;if(s->phase!=Phase::Staged||s->tileActive||uint32_t(g.eax)!=s->stageOwner){reject(*s,Failure::Order);return;}Binding binding{};
 if(!current(*s,g,binding)||!readSource(s->sourceScratch,*s->source)||!readStage(s->stageScratch,s->stageOwner)||s->stageScratch.target.owner!=s->stageOwner||!current(*s,g,binding)||!s->witness.finish(s->sourceScratch,s->stageScratch,true,binding,scene::current(g)->catalogue)){reject(*s,Failure::Incomplete);return;}s->phase=Phase::Finished;
}
struct UploadTileScope{
 Game&g;State*s=nullptr;native::TileObservation observation{};bool entered=false;int unwind=0;
 explicit UploadTileScope(Game&game)noexcept:g(game){auto*p=slot();if(!p||p->depth!=1||p->invalid||p->game!=reinterpret_cast<uintptr_t>(&g)||p->thread!=nfs_logo_publisher::threadToken())return;s=p;if(s->phase!=Phase::Staged){reject(*s,Failure::Order);s=nullptr;return;}if(s->tileActive){reject(*s,Failure::Nested);s=nullptr;return;}Binding binding{};if(!current(*s,g,binding)){reject(*s,Failure::Binding);s=nullptr;return;}unwind=nfs_logo_publisher::exceptions();
  try{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);if(sp>UINT32_MAX-12||s->witness.uploaded()>=300)throw 1;observation.descriptorGuest=tx.load<uint32_t>(sp);observation.payloadGuest=tx.load<uint32_t>(sp+4);observation.tableGuest=tx.load<uint32_t>(sp+8);observation.expectedSlotDescriptor=tx.load<uint32_t>(dword_4EB74C+4*s->witness.uploaded());if(!native::span(observation.descriptorGuest,28)||!native::span(observation.payloadGuest,2048))throw 1;nfs_logo_publisher::descriptor(tx,observation.descriptorGuest,observation.before);nfs_logo_publisher::checked(observation.payloadGuest,2048);std::memcpy(observation.pixels.data(),nfs_host_address(observation.payloadGuest,2048),2048);
  }catch(...){reject(*s,Failure::Read);s=nullptr;return;}s->tileActive=true;entered=true;
 }
 ~UploadTileScope(){if(!entered)return;if(s->invalid){s->tileActive=false;return;}if(nfs_logo_publisher::exceptions()>unwind){reject(*s,Failure::Unwind);return;}
  try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::descriptor(tx,observation.descriptorGuest,observation.after);nfs_logo_publisher::checked(observation.payloadGuest,2048);if(std::memcmp(observation.pixels.data(),nfs_host_address(observation.payloadGuest,2048),2048))throw 1;observation.originalUploadReturned=true;}catch(...){reject(*s,Failure::Read);return;}s->tileActive=false;Binding binding{};if(!current(*s,g,binding)||!s->witness.tile(observation,binding,scene::current(g)->catalogue))reject(*s,Failure::Tile);
 }
};
inline std::shared_ptr<const native::Proof>proofForPresentation(Game&g,Binding&binding,Plane&actual343)noexcept{
 binding={};actual343.valid=0;auto*s=slot();if(!s||s->depth||s->invalid||!s->returned||s->phase!=Phase::Finished)return {};if(!current(*s,g,binding)||!readSource(actual343,*s->source)||!current(*s,g,binding)){reject(*s,Failure::Source);binding={};return {};}auto proof=s->witness.proofForUse(actual343,binding,scene::current(g)->catalogue);if(!proof||!current(*s,g,binding)){reject(*s,Failure::Binding);binding={};return {};}return proof;
}
// Called only for actual4848FC allocation retirement, not a descriptor's
// decremented shared-LUT reference. Prefix allocation frees are source retire.
inline void retireAllocation(uint32_t at)noexcept{
 auto*s=slot();if(!s||!s->source||!at)return;const auto&source=s->source->post().target;s->witness.retire(at);if(uint64_t(at)+4==source.w[11])s->witness.retire(source.w[11]);if(uint64_t(at)+4==source.w[12])s->witness.retire(source.w[12]);if(s->phase==Phase::Staged){const auto&stage=s->stageScratch.target;if(uint64_t(at)+4==stage.w[11])s->witness.retire(stage.w[11]);if(uint64_t(at)+4==stage.w[12])s->witness.retire(stage.w[12]);}if(s->witness.failure()==native::Failure::Retire)reject(*s,Failure::Retire);
}
struct SourceLifetimeScope{bool active=false;explicit SourceLifetimeScope(Game&g)noexcept{active=scene::current(g)!=nullptr;if(active)reset();}~SourceLifetimeScope(){if(active)reset();}};
inline Failure failure()noexcept{auto*s=slot();return s?s->failure:Failure::None;}
inline unsigned uploaded()noexcept{auto*s=slot();return s?s->witness.uploaded():0;}
} // namespace nfs_player_initial_native_live

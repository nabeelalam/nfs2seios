// SPDX-License-Identifier: MIT
#pragma once
#ifndef NFS_PLAYER_NATIVE_RUNTIME_TEST
#include "PlayerSceneLiveGameHooks.inc"
#endif
#include "PlayerNativeUploadWitness.h"
#include <new>
namespace nfs_player_native_live {
namespace native=nfs_player_native;namespace scene=nfs_player_scene_live;
using Source=native::Source;using SourceBinding=native::SourceBinding;using CurrentBinding=native::CurrentBinding;using Plane=native::Plane;
// Root integration must obtain source and CURRENT fields independently. The
// occurrence parameter only selects a retained occurrence; it grants no values.
std::shared_ptr<const Source>prepareSource(Game&,uint64_t observedEvent)noexcept;
bool currentBinding(Game&,const SourceBinding&occurrence,CurrentBinding&)noexcept;
enum class Failure{None,Owner,Nested,Event,Source,Binding,Read,Tile,Unwind,Incomplete,Allocation,Retire};
struct State {
 native::Witness witness;std::shared_ptr<const Source>source;Plane scratch{};
 uintptr_t game=0,thread=0;uint64_t event=0;unsigned depth=0;bool active=false,tileActive=false,invalid=false;Failure failure=Failure::None;
};
inline State*&slot()noexcept{struct Storage{State*p=nullptr;~Storage(){delete p;}};static thread_local Storage s;return s.p;}
inline void reject(State&s,Failure f)noexcept{s.witness.reset();s.source.reset();s.active=s.tileActive=false;s.invalid=true;s.failure=f;}
inline void reset()noexcept{auto*s=slot();if(s){reject(*s,Failure::Owner);}}
// VerifiedSource deliberately has no sidebar-pointer authority. Root calls this
// on watched retire sites, including retained343/preview/copy dependencies.
// Conservatively revoke every nonzero notification rather than guess aliases.
inline void retire(uint32_t guest)noexcept{if(guest){auto*s=slot();if(s)reject(*s,Failure::Retire);}}
inline bool current(State&s,Game&g,CurrentBinding&c)noexcept{
 if(s.invalid||!s.source||s.game!=reinterpret_cast<uintptr_t>(&g)||s.thread!=nfs_logo_publisher::threadToken()||!scene::current(g)||!s.event||nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)!=s.event)return false;
 auto source=s.source;c={};return currentBinding(g,source->binding(),c)&&s.source==source&&!s.invalid&&native::bindingMatches(*source,c)&&nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)==s.event;
}
// Anchored actual owner read: current display target may be unrelated after the
// original cached source has been restored. No allocations inside this lease.
inline bool readPlane(Plane&p,const Source&s)noexcept{
 p.valid=0;try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,p.target,s.target().owner);if(!nfs_logo_capture::same(p.target,s.target())||!nfs_background_scene_cpu::fullClip(p.target,640))return false;
 p.width=640;p.height=480;const uint32_t bitmap=p.target.w[8];nfs_logo_publisher::checked(bitmap,614416);std::memcpy(p.pixels.data(),nfs_host_address(bitmap+16,614400),614400);
 nfs_logo_capture::Target after{};nfs_logo_publisher::target(tx,after,s.target().owner);if(!nfs_logo_capture::same(after,p.target))return false;p.valid=1;
 }catch(...){return false;}return native::planeMatches(s,p);
}
struct UploadScope {
 Game&g;State*s=nullptr;bool entered=false;int unwind=0;
 explicit UploadScope(Game&game)noexcept:g(game){
  if(!nfs_logo_publisher::enabled()||uint32_t(g.edx)!=1)return;
  // Generic UploadScope was constructed first and already incremented the one
  // process-wide event. Never increment it again here. Revoke before matching.
  const uint64_t event=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire);auto*&p=slot();if(p){p->witness.slot1Overwrite();p->source.reset();p->active=false;if(p->depth){p->invalid=true;p->failure=Failure::Nested;return;}}
  if(!scene::current(g)||!event||event==UINT64_MAX)return;
  if(!p)p=new(std::nothrow)State;if(!p)return;s=p;s->game=reinterpret_cast<uintptr_t>(&g);s->thread=nfs_logo_publisher::threadToken();s->event=event;s->depth=1;s->active=s->tileActive=s->invalid=false;s->failure=Failure::None;entered=true;unwind=nfs_logo_publisher::exceptions();
  s->source=prepareSource(g,event);CurrentBinding c{};
  if(!s->source||uint32_t(g.eax)!=s->source->target().w[8]){reject(*s,Failure::Source);return;}
  if(!current(*s,g,c)){reject(*s,Failure::Binding);return;}
  if(!readPlane(s->scratch,*s->source)){reject(*s,Failure::Read);return;}
  if(!current(*s,g,c)||!s->witness.begin(s->source,c,s->scratch,event,nfs_logo_publisher::uploadEvents())){reject(*s,Failure::Event);return;}s->active=true;
 }
 ~UploadScope(){if(!entered)return;CurrentBinding c{};if(nfs_logo_publisher::exceptions()>unwind)reject(*s,Failure::Unwind);
  else if(s->active){if(s->tileActive||!current(*s,g,c)||!readPlane(s->scratch,*s->source)||!current(*s,g,c)||!s->witness.finish(s->scratch,true,c))reject(*s,Failure::Incomplete);else s->active=false;}
  s->depth=0;
 }
};
// Wrap only actual49755C within428250, after all three original pushes and
// outside the generated call's ESP return adjustment. Slot contents are inputs.
struct UploadTileScope {
 Game&g;State*s=nullptr;native::TileObservation observation{};bool entered=false;int unwind=0;
 explicit UploadTileScope(Game&game)noexcept:g(game){auto*p=slot();if(!p||p->depth!=1||!p->active||p->invalid||p->game!=reinterpret_cast<uintptr_t>(&g)||p->thread!=nfs_logo_publisher::threadToken())return;s=p;
  if(s->tileActive){reject(*s,Failure::Nested);s=nullptr;return;}CurrentBinding c{};if(!current(*s,g,c)){reject(*s,Failure::Binding);s=nullptr;return;}unwind=nfs_logo_publisher::exceptions();
  try{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);if(sp>UINT32_MAX-12||s->witness.uploaded()>=300)throw 1;observation.descriptorGuest=tx.load<uint32_t>(sp);observation.payloadGuest=tx.load<uint32_t>(sp+4);observation.tableGuest=tx.load<uint32_t>(sp+8);observation.expectedSlotDescriptor=tx.load<uint32_t>(dword_4EB74C+1200+4*s->witness.uploaded());if(!observation.descriptorGuest||uint64_t(observation.descriptorGuest)+28>UINT64_C(0x100000000)||!observation.payloadGuest||uint64_t(observation.payloadGuest)+2048>UINT64_C(0x100000000))throw 1;
   nfs_logo_publisher::descriptor(tx,observation.descriptorGuest,observation.before);nfs_logo_publisher::checked(observation.payloadGuest,2048);std::memcpy(observation.pixels.data(),nfs_host_address(observation.payloadGuest,2048),2048);
  }catch(...){reject(*s,Failure::Read);s=nullptr;return;}s->tileActive=true;entered=true;
 }
 ~UploadTileScope(){if(!entered)return;if(s->invalid){s->tileActive=false;return;}if(nfs_logo_publisher::exceptions()>unwind){reject(*s,Failure::Unwind);return;}
  try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::descriptor(tx,observation.descriptorGuest,observation.after);nfs_logo_publisher::checked(observation.payloadGuest,2048);if(std::memcmp(observation.pixels.data(),nfs_host_address(observation.payloadGuest,2048),2048))throw 1;observation.originalUploadReturned=true;
  }catch(...){reject(*s,Failure::Read);return;}s->tileActive=false;CurrentBinding c{};if(!current(*s,g,c)||!s->witness.tile(observation,c))reject(*s,Failure::Tile);
 }
};
inline std::shared_ptr<const native::Proof>proofForPresentation(Game&g,CurrentBinding&c,Plane&actual)noexcept{
 c={};actual.valid=0;auto*s=slot();if(!s||s->depth||s->active)return {};if(!current(*s,g,c)){reject(*s,Failure::Binding);return {};}if(!readPlane(actual,*s->source)||!current(*s,g,c)){reject(*s,Failure::Source);return {};}auto p=s->witness.proofForPresentation(c,actual);if(!p||!current(*s,g,c)){if(p)reject(*s,Failure::Binding);return {};}return p;
}
} // namespace nfs_player_native_live

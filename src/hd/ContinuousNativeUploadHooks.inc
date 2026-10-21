// SPDX-License-Identifier: MIT
#pragma once
#include "ContinuousLogoGameHooks.inc"
#include <atomic>
namespace nfs_logo_publisher {
// Observed CPU overwrite sequence only; not resource/context generation or async authority.
inline std::atomic<uint64_t>&uploadEvents()noexcept {static std::atomic<uint64_t>n{0};return n;}
inline uint64_t overwriteEvent()noexcept {auto&n=uploadEvents();uint64_t old=n.load(std::memory_order_acquire);for(;;){if(old==UINT64_MAX)return 0;if(n.compare_exchange_weak(old,old+1,std::memory_order_acq_rel))return old+1==UINT64_MAX?0:old+1;}}

// Exact full-plane read anchored to the recorded actual CPU owner, not global-current target.
inline bool readUploadPlane(Plane&p,const Plane&reference)noexcept {
 p.valid=0;try {nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,p.target,reference.target.owner);
  if(!same(p.target,reference.target)||reference.width!=640||reference.height!=480)return false;
  p.width=640;p.height=480;const auto base=p.target.w[8];
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){uint64_t at=uint64_t(base)+p.target.x[x]+p.target.y[y];if(at>UINT32_MAX-1)return false;p.pixels[y*640+x]=tx.load<uint16_t>(uint32_t(at));}
  Target after{};nfs_logo_publisher::target(tx,after,reference.target.owner);if(!same(after,p.target))return false;p.valid=1;
 }catch(...){return false;}
 // Entire equality includes legitimate later tail: if anything changed, do not generalize.
 return p.pixels==reference.pixels;
}
inline void descriptor(nfs::GuestCopyTransaction&tx,uint32_t at,std::array<uint32_t,7>&out){if(!at)throw 1;checked(at,28);for(unsigned i=0;i<7;++i)out[i]=tx.load<uint32_t>(at+4*i);}
struct UploadScope {
 Collector*s=nullptr;bool active=false;int unwind=0;uint64_t event=0;
 explicit UploadScope(Game&g)noexcept {
  if(!enabled()||uint32_t(g.edx)!=1)return;event=overwriteEvent();s=store();if(!s)return;
  // Every observed slot1 upload invalidates before source matching, including unrelated uploads.
  s->slot1Overwrite();if(!event||!owner()||s->gameOwner!=reinterpret_cast<uintptr_t>(&g)||!s->prepared||uint32_t(g.eax)!=s->copy.post.target.w[8])return;
  if(!readUploadPlane(s->sourceScratch,s->copy.post))return;
  active=s->beginUpload(s->sourceScratch,1);s->nativeUploadEvent=event;unwind=exceptions();
 }
 ~UploadScope(){if(!active)return;bool after=false;if(exceptions()==unwind&&uploadEvents().load(std::memory_order_acquire)==event)after=readUploadPlane(s->sourceScratch,s->copy.post);s->finishUpload(s->sourceScratch,after,exceptions()>unwind);}
};
// Scope must wrap exactly the original49755C invocation in428250, after its three pushes.
struct UploadTileScope {
 Collector*s=nullptr;TileObservation observation{};bool active=false;int unwind=0;
 explicit UploadTileScope(Game&g)noexcept {
  if(!owner())return;s=store();if(!s->uploadActive)return;active=true;unwind=exceptions();
  try {nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);if(sp>UINT32_MAX-12||s->uploaded>=300)throw 1;
   observation.descriptorGuest=tx.load<uint32_t>(sp);observation.payloadGuest=tx.load<uint32_t>(sp+4);observation.tableGuest=tx.load<uint32_t>(sp+8);
   observation.expectedSlotDescriptor=tx.load<uint32_t>(dword_4EB74C+1200+s->uploaded*4);
   descriptor(tx,observation.descriptorGuest,observation.before);checked(observation.payloadGuest,2048);std::memcpy(observation.pixels.data(),nfs_host_address(observation.payloadGuest,2048),2048);
  }catch(...){observation.originalUploadReturned=false;s->uploadActive=false;s->bound=false;s->stop=Collector::Stop::UploadPayload;active=false;}
 }
 ~UploadTileScope(){if(!active)return;
  if(exceptions()==unwind)try{nfs::GuestCopyTransaction tx;descriptor(tx,observation.descriptorGuest,observation.after);checked(observation.payloadGuest,2048);if(std::memcmp(observation.pixels.data(),nfs_host_address(observation.payloadGuest,2048),2048))throw 1;observation.originalUploadReturned=true;}catch(...){observation.originalUploadReturned=false;}
  // Native verification occurs after the descriptor lease ends.
  s->uploadTile(observation);
 }
};
// Wrap only the fixed40D3D0 native slot1 invocation to attest its lexical route.
struct NativePresentationScope {
 Collector*s=nullptr;bool active=false;
 explicit NativePresentationScope(Game&g)noexcept {if(!owner())return;s=store();if(s->gameOwner!=reinterpret_cast<uintptr_t>(&g)||!s->bound||s->presentationArmed){s->invalidate(Collector::Stop::Owner);return;}s->presentationArmed=true;active=true;}
 ~NativePresentationScope(){if(active)s->presentationArmed=false;}
};
// Inject only at the exact native slot1 main-menu route, not all427B30 callers.
inline bool publishNativeEntry(Game&g,Publish publish,void*context,uint32_t returnBytes=4)noexcept {
 if(!owner())return false;auto*s=store();if(!s->presentationArmed)return false;if(!s->nativeUploadEvent||uploadEvents().load(std::memory_order_acquire)!=s->nativeUploadEvent){s->invalidate(Collector::Stop::Overwrite);return false;}if(s->gameOwner!=reinterpret_cast<uintptr_t>(&g)||uint32_t(g.eax)!=1||uint32_t(g.edx)||uint32_t(g.ebx)||uint32_t(g.ecx)!=32){s->invalidate(Collector::Stop::Owner);return false;}
 try{nfs::GuestCopyTransaction tx;if((returnBytes!=0&&returnBytes!=4)||uint32_t(g.esp)>UINT32_MAX-8||tx.load<uint32_t>(uint32_t(g.esp)+returnBytes)!=UINT32_MAX){s->invalidate(Collector::Stop::Owner);return false;}}catch(...){s->invalidate(Collector::Stop::Owner);return false;}
 if(!readUploadPlane(s->sourceScratch,s->copy.post)){s->invalidate(Collector::Stop::UploadSource);return false;}
 // All native/guest read scopes have ended; only owned packet reaches callback.
 return s->publishCurrent(reinterpret_cast<uintptr_t>(&g),1,s->sourceScratch,publish,context);
}
inline void resetNativeCache()noexcept {if(!enabled())return;overwriteEvent();if(auto*s=store())s->reset();}
inline void retireNative(Game&g)noexcept {if(!enabled())return;if(auto*s=store()){const bool old=s->prepared;s->retire(uint32_t(g.eax));if(old&&!s->prepared)overwriteEvent();}else overwriteEvent();}
}

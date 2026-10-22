// SPDX-License-Identifier: MIT
#pragma once
#include "Main343OriginProof.h"
#include "BackgroundSceneCatalog.h"
#include <atomic>
#include <new>
namespace nfs_main_sidebar_origin_live {
namespace mo=nfs_main_sidebar_origin;namespace bg=nfs_background_scene_cpu;
struct State{mo::Store origins;bg::Plane scratch;uintptr_t game=0;};
inline std::atomic<uintptr_t>&ownerThread()noexcept{static std::atomic<uintptr_t>value{0};return value;}
inline State*&slot()noexcept{struct Storage{State*p=nullptr;~Storage(){delete p;if(ownerThread().load()==nfs_logo_publisher::threadToken())ownerThread().store(0);}};static thread_local Storage value;return value.p;}
inline State*current(Game&g)noexcept{if(ownerThread().load(std::memory_order_acquire)!=nfs_logo_publisher::threadToken())return nullptr;auto*s=slot();return s&&s->game==reinterpret_cast<uintptr_t>(&g)?s:nullptr;}
inline std::shared_ptr<const mo::Proof>publishMain(Game&g,const bg::JoinedInput&i,const mo::Binding&binding,bg::Workspace&workspace,std::shared_ptr<const bg::SourceCatalog>catalog)noexcept{
 if(!nfs_logo_publisher::enabled()||binding.game!=reinterpret_cast<uintptr_t>(&g)||binding.publisherThread!=nfs_logo_publisher::threadToken()||binding.formGuest!=uint32_t(guest_address(off_4D3EC4)))return {};
 try{nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=binding.archive)return {};}catch(...){return {};}
 uintptr_t expected=0,token=nfs_logo_publisher::threadToken();if(!ownerThread().compare_exchange_strong(expected,token)&&expected!=token)return {};
 auto*&p=slot();if(!p)p=new(std::nothrow)State;if(!p){ownerThread().store(0);return {};}if(p->game!=binding.game){p->origins.clear();p->game=binding.game;}
 return p->origins.publish(i,binding,workspace,std::move(catalog));
}
inline std::shared_ptr<const mo::Proof>acquireForInitialCaption(Game&g)noexcept{
 auto*s=current(g);if(!s)return {};auto catalog=nfs_background_scene_runtime::acquireCatalog();if(!catalog)return {};
 try{uint32_t archive=0,selected=0,handle=0;std::array<uint32_t,4>handles{};
  {nfs::GuestCopyTransaction tx;archive=tx.load<uint32_t>(dword_4D40B8);selected=tx.load<uint32_t>(dword_4D2314+2)>>16;if(selected>1||!archive)return {};
   handles={{tx.load<uint32_t>(dword_4D231C),tx.load<uint32_t>(dword_4D2320),tx.load<uint32_t>(dword_4D4D24),tx.load<uint32_t>(dword_4DABE4)}};handle=handles[selected];if(!handle||handles[0]==handles[1])return {};
  }
  s->scratch.valid=0;uint32_t error=0;nfs_logo_publisher::plane(s->scratch,error);if(error||!bg::plane(s->scratch,343)||s->scratch.target.owner!=handle)return {};
  {nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=archive||tx.load<uint32_t>(dword_4D2314+2)!=s->scratch.target.ownerState[0])return {};
   const std::array<uint32_t,4>now{{tx.load<uint32_t>(dword_4D231C),tx.load<uint32_t>(dword_4D2320),tx.load<uint32_t>(dword_4D4D24),tx.load<uint32_t>(dword_4DABE4)}};if(now!=handles)return {};
   for(uint32_t other:handles)if(other&&other!=handle){nfs_logo_publisher::checked(other,56);if(tx.load<uint32_t>(other+32)==s->scratch.target.w[8])return {};}
  }
  return s->origins.acquire(s->scratch,reinterpret_cast<uintptr_t>(&g),nfs_logo_publisher::threadToken(),archive,catalog);
 }catch(...){s->origins.clear();return {};}
}
inline void beforeOwnerRetire(Game&g,uint32_t at,bool selected=false)noexcept{
 auto*s=current(g);if(!s)return;try{if(!at&&selected){nfs::GuestCopyTransaction tx;nfs_logo_capture::Target t;nfs_logo_publisher::target(tx,t);at=t.owner;}s->origins.retireOwner(at);}catch(...){s->origins.clear();}
}
inline void beforeDescriptorRetire(Game&g,uint32_t at)noexcept{
 auto*s=current(g);if(!s||!at)return;try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(at,56);
  for(const auto&entry:s->origins.entries()){auto p=entry;if(!p)continue;const auto&t=p->post().target;bool retire=at==t.owner||tx.load<uint32_t>(at+32)==t.w[8];
   for(unsigned k:{11u,12u})if(tx.load<uint32_t>(at+4*k)==t.w[k]){if(t.w[k]<4)retire=true;else{const uint32_t refs=tx.load<uint32_t>(t.w[k]-4);if(refs<=1||refs>uint32_t(INT32_MAX))retire=true;}}
   if(retire)s->origins.retireOwner(t.owner);
  }
 }catch(...){s->origins.clear();}
}
inline void beforeAllocationFree(Game&g,uint32_t at)noexcept{
 auto*s=current(g);if(!s||!at)return;
 for(const auto&entry:s->origins.entries()){auto p=entry;if(!p)continue;const auto&t=p->post().target;if(at==p->archive()){s->origins.clear();return;}
  if(at==t.owner||at==t.w[8]||at==t.w[11]||at==t.w[12]||(uint64_t(at)+4==t.w[11])||(uint64_t(at)+4==t.w[12]))s->origins.retireOwner(t.owner);
 }
}
}

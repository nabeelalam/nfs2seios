// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerIntroQueueAdapter.inc"
#include "PlayerPhotoGameHooks.inc"
#include "PlayerIntroWidePreludeAbi.h"
#include "Main343OriginLifetime.h"
#include "MenuTransitionGameHooks.inc"
#include <atomic>
namespace nfs_player_intro_wide_live {
namespace intro=nfs_player_intro_live;namespace native=nfs_player_initial_native;namespace cpu=nfs_player_intro_cpu;namespace scene=nfs_player_scene_live;namespace photo=nfs_player_photo_live;
struct Errno{int value=errno;~Errno(){errno=value;}};
struct Owner final{
 std::atomic<unsigned>refs{1};std::shared_ptr<const cpu::Prepared>prepared;
 std::shared_ptr<const photo::Proof>photoProof;std::shared_ptr<const nfs_main_sidebar_origin::GameLife>life;
 std::shared_ptr<const nfs_menu_transition_live::Packet>returnMap;
 static void retain(void*p){if(p)static_cast<Owner*>(p)->refs.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p){if(p&&static_cast<Owner*>(p)->refs.fetch_sub(1,std::memory_order_acq_rel)==1)delete static_cast<Owner*>(p);}
 static int current(void*p){auto*s=static_cast<Owner*>(p);if(!s||!s->life||!s->life->current()||!s->prepared||!s->prepared->current()||!s->photoProof||!s->photoProof->art||!s->photoProof->art->current())return 0;
  const auto&m=s->returnMap;const auto&b=s->prepared->binding().sidebar;
  if(!s->prepared->source()->showcaseReturn())return !m;
  // Renderer-side checks use retained immutable packet fields/lifetime only.
  // Guest source validation is repeated separately on the worker before FIFO.
  return m&&m->kind==1&&m->slot==1&&m->mapRole<=1&&m->content&&m->life&&m->life->current()&&m->game==b.game&&m->thread==b.thread&&m->archive==b.archive&&m->catalog==s->prepared->catalog();
 }
};
/* Exact lexical40C342 iteration, after countdown ebx is loaded and before
  497680 render-buffer selection. Scope ends after4976B8, before loop backedge.
   The original state/arguments/branching are never changed. */
class PreludeScope final{
 Game&g_;Owner*owner_=nullptr;std::shared_ptr<const native::Proof>proof_;native::Binding binding_{};
 std::unique_ptr<native::Plane>actual_;uint64_t serial_=0;uint32_t form_=0,countdown_=0;unsigned stage_=0;bool accepted_=false;int unwind_=0;
 bool workerCurrent()noexcept{
  auto*l=scene::current(g_);if(!owner_||!Owner::current(owner_)||!l||l->invalid||l->formGuest!=form_||uint32_t(g_.esi)!=form_||photo::published()!=owner_->photoProof||!photo::current(g_,*owner_->photoProof))return false;
  if(owner_->returnMap&&(nfs_menu_transition_live::state().slots[1]!=owner_->returnMap||!nfs_menu_transition_live::unchanged(*owner_->returnMap)))return false;
  native::Binding now{};auto p=nfs_player_initial_native_live::proofForPresentation(g_,now,*actual_);
  return p==proof_&&native::sameBinding(binding_,now)&&proof_->current(now,l->catalogue)&&owner_->prepared->proof()==proof_;
 }
 void cancel()noexcept{
  Errno preserve;
  if(accepted_&&serial_)try{nfs::ScopedWorkerStopDeferral deferral;const uint64_t serial=serial_;(void)nfs::enqueue_owned_renderer([serial]{return int32_t(nfs_private_player_intro_wide_cancel(serial));});}catch(...){}
  accepted_=false;stage_=0;
 }
 bool send(unsigned stage,int32_t x=0,int32_t y=0){
  Errno preserve;if(!accepted_)return false;if(!workerCurrent()){cancel();return false;}nfs::check_worker_stop();Owner::retain(owner_);const auto keep=std::shared_ptr<Owner>(owner_,[](Owner*p){Owner::release(p);});const uint64_t serial=serial_;
  const auto receipt=nfs::enqueue_owned_renderer([keep,serial,stage,x,y]{if(!Owner::current(keep.get()))return int32_t(nfs_private_player_intro_wide_cancel(serial));return int32_t(nfs_private_player_intro_wide_stage(serial,stage,x,y));});
  if(receipt!=nfs::OwnedRendererAcceptance::Consumed){cancel();return false;}nfs::check_worker_stop();stage_=stage;return true;
 }
 void begin(){
  Errno preserve;auto*l=scene::current(g_);if(!l||l->invalid||!l->pendingInitialSidebar||uint32_t(g_.esi)!=l->formGuest||uint32_t(g_.ebx)>2)return;
  countdown_=uint32_t(g_.ebx);form_=l->formGuest;actual_.reset(new(std::nothrow)native::Plane);if(!actual_)return;
  proof_=nfs_player_initial_native_live::proofForPresentation(g_,binding_,*actual_);auto p=photo::published();
  {const int saved=errno;static thread_local unsigned lines=0;const char*flag=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(l->pendingInitialSidebar->showcaseReturn()&&flag&&flag[0]=='1'&&!flag[1]&&lines++<8)std::fprintf(stderr,"PLAYER RETURN WIDE source countdown=%u native=%u photo=%u photoCurrent=%u sourceEpoch=%llu photoEpoch=%llu\n",countdown_,unsigned(bool(proof_)),unsigned(bool(p)),unsigned(p&&photo::current(g_,*p)),(unsigned long long)l->ownerEpoch,(unsigned long long)(p?p->epoch:0));errno=saved;}
  if(!proof_||!p||!photo::current(g_,*p)||binding_.sidebar.game!=reinterpret_cast<uintptr_t>(&g_)||binding_.sidebar.formGuest!=form_)return;
  serial_=nfs_background_scene_live::nextRequest();if(!serial_)return;
  auto prepared=cpu::Prepared::create(proof_,binding_,*actual_,l->catalogue,serial_);if(!prepared)return;
  std::shared_ptr<const nfs_menu_transition_live::Packet>map;
  if(prepared->source()->showcaseReturn()){
   map=nfs_menu_transition_live::state().slots[1];
   if(!map||map->kind!=1||map->slot!=1||map->mapRole>1||!map->content||map->game!=binding_.sidebar.game||map->thread!=binding_.sidebar.thread||map->archive!=binding_.sidebar.archive||map->catalog!=l->catalogue||!nfs_menu_transition_live::unchanged(*map))return;
  }
  auto life=nfs_main_sidebar_origin::acquireGameLife(reinterpret_cast<uintptr_t>(&g_));if(!life)return;
  owner_=new(std::nothrow)Owner;if(!owner_)return;owner_->prepared=std::move(prepared);owner_->photoProof=p;owner_->life=life;owner_->returnMap=std::move(map);if(!workerCurrent())return;
  Owner::retain(owner_);const auto keep=std::shared_ptr<Owner>(owner_,[](Owner*q){Owner::release(q);});const auto catalog=l->catalogue;const auto binding=binding_;const auto countdown=countdown_;
  const auto receipt=nfs::enqueue_owned_renderer([keep,catalog,binding,countdown]{
   if(!Owner::current(keep.get()))return int32_t(0);NfsPlayerIntroWidePreludeView v{};v.schema=1;v.countdown=countdown;v.frame=keep->prepared->view();
   if(keep->returnMap){const auto&m=*keep->returnMap;
    // Join the real slot1 map upload with the independently owned sidebar.
    // The padded slot0/sidebar upload never stands in for this map pass.
    v.frame.inheritedMain.mapRole=m.mapRole;v.frame.inheritedMain.serial=m.content;v.frame.inheritedMain.contentRevision=m.content;v.frame.inheritedMain.original565=m.original.data();std::copy(m.addresses.begin(),m.addresses.end(),v.frame.inheritedMain.rendererAddress);
   }
   v.photoSerial=keep->photoProof->serial;v.ownerEpoch=binding.sidebar.epoch;v.sourceGame=binding.sidebar.game;v.sourceCatalog=reinterpret_cast<uintptr_t>(catalog.get());v.sourceLife=keep->life->serial();v.sourceArchive=binding.sidebar.archive;v.owner=keep.get();v.retain=&Owner::retain;v.release=&Owner::release;v.current=&Owner::current;
   return int32_t(nfs_private_player_intro_wide_begin(&v));});
  accepted_=receipt==nfs::OwnedRendererAcceptance::Consumed;
 }
public:
 explicit PreludeScope(Game&g):g_(g),unwind_(nfs_logo_publisher::exceptions()){try{begin();}catch(...){cancel();Owner::release(owner_);owner_=nullptr;throw;}}
 PreludeScope(const PreludeScope&)=delete;PreludeScope&operator=(const PreludeScope&)=delete;
 void cleared(){if(accepted_&&(!countdown_||stage_))cancel();else(void)send(NFS_PLAYER_WIDE_CLEARED);}
 void slot1Finished(){if(accepted_&&(!countdown_||stage_!=NFS_PLAYER_WIDE_CLEARED))cancel();else(void)send(NFS_PLAYER_WIDE_SLOT1);}
 void photoBegin(){if(!accepted_)return;if((countdown_&&stage_!=NFS_PLAYER_WIDE_SLOT1)||(!countdown_&&stage_)){cancel();return;}const int32_t x=int32_t(g_.eax),y=int32_t(g_.edx);if(x< -1024||x>1024||y!=316){cancel();return;}(void)send(NFS_PLAYER_WIDE_PHOTO_BEGIN,x,y);}
 void photoFinished(){if(accepted_&&stage_!=NFS_PLAYER_WIDE_PHOTO_BEGIN)cancel();else(void)send(NFS_PLAYER_WIDE_PHOTO_END);}
 void slot0Finished(){if(accepted_&&stage_!=NFS_PLAYER_WIDE_PHOTO_END)cancel();else(void)send(NFS_PLAYER_WIDE_SLOT0);}
 void tailFinished(){if(accepted_&&stage_!=NFS_PLAYER_WIDE_SLOT0)cancel();else(void)send(NFS_PLAYER_WIDE_TAIL);}
 ~PreludeScope()noexcept(false){Errno preserve;try{if(nfs_logo_publisher::exceptions()>unwind_||!accepted_||stage_!=NFS_PLAYER_WIDE_TAIL)cancel();else{(void)send(NFS_PLAYER_WIDE_END);accepted_=false;}}catch(...){cancel();Owner::release(owner_);owner_=nullptr;throw;}Owner::release(owner_);}
};
} // namespace nfs_player_intro_wide_live

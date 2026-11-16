// SPDX-License-Identifier: MIT
#pragma once
#include "MenuTransitionGameHooks.inc"
#include "ChildPhotoGameHooks.inc"
#include "ChildIntroWidePreludeAbi.h"
#include "MainThreadBridge.h"
#include "WorkerControl.h"
#include <atomic>
#include <memory>
#include <new>
#include <algorithm>
namespace nfs_child_intro_wide_live {
namespace mt=nfs_menu_transition_live;namespace photo=nfs_child_photo_live;namespace bg=nfs_background_scene_cpu;
struct Errno{int value=errno;~Errno(){errno=value;}};
inline bool typedCurrent(const mt::Packet&p)noexcept{return p.slot==0&&p.life&&p.life->current()&&p.catalog&&p.content&&p.targetBound&&((p.kind==6&&p.opponentInitial&&!p.locationInitial&&!p.childInitial&&p.opponentInitial->current())||(p.kind==7&&p.locationInitial&&!p.opponentInitial&&!p.childInitial&&p.locationInitial->current()));}
inline uint32_t formGuest(const mt::Packet&p)noexcept{return p.kind==6&&p.opponentInitial?p.opponentInitial->binding().formGuest:p.kind==7&&p.locationInitial?p.locationInitial->binding().formGuest:0;}
inline uint64_t epoch(const mt::Packet&p)noexcept{return p.kind==6&&p.opponentInitial?p.opponentInitial->binding().epoch:p.kind==7&&p.locationInitial?p.locationInitial->binding().epoch:0;}
inline std::shared_ptr<const nfs_main_sidebar_origin::Proof>origin(const mt::Packet&p)noexcept{return p.kind==6&&p.opponentInitial?p.opponentInitial->origin():p.kind==7&&p.locationInitial?p.locationInitial->origin():std::shared_ptr<const nfs_main_sidebar_origin::Proof>{};}
struct Owner final{
 std::atomic<unsigned>refs{1};std::shared_ptr<const mt::Packet>packet,infoReturnMap;std::shared_ptr<const photo::Proof>photoProof;std::shared_ptr<const bg::Prepared>main;
 static void retain(void*p){if(p)static_cast<Owner*>(p)->refs.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p){if(p&&static_cast<Owner*>(p)->refs.fetch_sub(1,std::memory_order_acq_rel)==1)delete static_cast<Owner*>(p);}
 static int current(void*p){const auto*s=static_cast<const Owner*>(p);return s&&s->packet&&typedCurrent(*s->packet)&&((s->main&&s->main->valid)||(s->infoReturnMap&&s->infoReturnMap->life->current()))&&s->photoProof&&s->photoProof->owner==s->packet&&photo::ownedCurrent(*s->photoProof);}
};
/* Disjoint family6/7 owner for the exact shared normal40C342 loop. The original
   MT slot0 packet proves its own343/padded640/upload300; no Player proof cast.
   Renderer current() never reads Guest/Game or a widget snapshot. */
class PreludeScope final{
 Game&g_;Owner*owner_=nullptr;uint64_t serial_=0;uint32_t countdown_=0,form_=0;unsigned stage_=0;bool accepted_=false;int unwind_=0;
 std::unique_ptr<mt::NormalBrandingCallerScope>normal_;
 bool sourceCurrent()noexcept{return owner_&&Owner::current(owner_)&&owner_->packet->game==reinterpret_cast<uintptr_t>(&g_)&&owner_->packet->thread==nfs_logo_publisher::threadToken()&&mt::state().slots[0]==owner_->packet&&photo::published()==owner_->photoProof&&mt::unchanged(*owner_->packet)&&(!owner_->infoReturnMap||(mt::state().slots[1]==owner_->infoReturnMap&&mt::unchanged(*owner_->infoReturnMap)))&&photo::current(g_,*owner_->photoProof);}
 bool workerCurrent()noexcept{return uint32_t(g_.esi)==form_&&sourceCurrent();}
 static bool brandingCurrent(void*p)noexcept{return p&&static_cast<PreludeScope*>(p)->sourceCurrent();}
 void cancel()noexcept{Errno preserve;normal_.reset();if(accepted_&&serial_)try{nfs::ScopedWorkerStopDeferral deferral;const uint64_t serial=serial_;(void)nfs::enqueue_owned_renderer([serial]{return int32_t(nfs_private_child_intro_wide_cancel(serial));});}catch(...){}accepted_=false;stage_=0;}
 bool send(unsigned stage,int32_t x=0,int32_t y=0){
  Errno preserve;if(!accepted_)return false;if(!workerCurrent()){cancel();return false;}nfs::check_worker_stop();Owner::retain(owner_);const auto keep=std::shared_ptr<Owner>(owner_,[](Owner*p){Owner::release(p);});const uint64_t serial=serial_;
  const auto receipt=nfs::enqueue_owned_renderer([keep,serial,stage,x,y]{if(!Owner::current(keep.get()))return int32_t(nfs_private_child_intro_wide_cancel(serial));return int32_t(nfs_private_child_intro_wide_stage(serial,stage,x,y));});
  if(receipt!=nfs::OwnedRendererAcceptance::Consumed){cancel();return false;}nfs::check_worker_stop();stage_=stage;return true;
 }
 void begin(){
  Errno preserve;auto p=mt::state().slots[0];if(!p||!typedCurrent(*p)||p->game!=reinterpret_cast<uintptr_t>(&g_)||p->thread!=nfs_logo_publisher::threadToken()||uint32_t(g_.ebx)>2||!mt::unchanged(*p))return;
  form_=formGuest(*p);if(!form_||uint32_t(g_.esi)!=form_)return;auto image=photo::published();if(!image||image->owner!=p||image->family!=p->kind||!photo::current(g_,*image))return;if(p->raw.size()!=16+bg::Side*2||p->sourceTarget.w[8]!=p->source)return;
  const bool infoReturn=p->kind==7&&p->locationInitial->locationInfoReturn();
  auto o=origin(*p);
  std::shared_ptr<const mt::Packet>map;
  if(infoReturn){
   map=mt::state().slots[1];
   if(!map||map->kind!=1||map->slot!=1||map->game!=p->game||map->thread!=p->thread
       ||map->archive!=p->archive||map->catalog!=p->catalog||map->life!=p->life
       ||!map->content||map->raw.size()!=16+bg::Full*2||!mt::unchanged(*map))return;
  }else if(!o||!o->current()||!o->frame()||!o->frame()->valid||!o->catalog()
        ||!nfs_main_sidebar_origin::sameCatalog(*o->catalog(),*p->catalog))return;
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)if(p->original[y*640+x]!=(x<343?bg::le16(p->raw.data()+16+2*(y*343+x)):0))return;
  countdown_=uint32_t(g_.ebx);serial_=nfs_background_scene_live::nextRequest();if(!serial_)return;owner_=new(std::nothrow)Owner;if(!owner_)return;owner_->packet=p;owner_->photoProof=image;owner_->infoReturnMap=std::move(map);if(o)owner_->main=o->frame();if(!workerCurrent())return;
  Owner::retain(owner_);const auto keep=std::shared_ptr<Owner>(owner_,[](Owner*q){Owner::release(q);});const auto serial=serial_;const auto countdown=countdown_;
  const auto receipt=nfs::enqueue_owned_renderer([keep,serial,countdown]{
   if(!Owner::current(keep.get()))return int32_t(0);const auto&p=*keep->packet;NfsChildIntroWidePreludeView v{};v.schema=1;v.family=p.kind;v.countdown=countdown;v.form=p.kind==6?p.opponentInitial->view().form:p.locationInitial->view().form;v.sourceOwner=p.sourceTarget.owner;v.sourceBitmap=p.source;v.serial=serial;v.content=v.uploadEvent=p.content;if(keep->infoReturnMap){
    const auto&map=*keep->infoReturnMap;
    v.locationInfoReturn=1;
    auto&layers=v.inheritedMain;
    layers.schema=1;layers.width=640;layers.height=480;layers.mapRole=map.mapRole;
    layers.serial=layers.contentRevision=map.content;layers.planeWords=bg::Full;
    layers.original565=map.original.data();
    std::copy(map.addresses.begin(),map.addresses.end(),layers.rendererAddress);
    layers.panelClip[2]=343;layers.panelClip[3]=480;
    std::copy(std::begin(p.layers.logos),std::end(p.layers.logos),layers.logos);
   }else v.inheritedMain=keep->main->view();v.paddedOriginal565=p.original.data();v.wordsCount=bg::Full;std::copy(p.addresses.begin(),p.addresses.end(),v.keys);if(p.kind==6)v.opponentInitial=p.opponentInitial->view();else v.locationInitial=p.locationInitial->view();v.photoFamily=keep->photoProof->family;v.photoSerial=keep->photoProof->serial;v.ownerEpoch=epoch(p);v.sourceGame=p.game;v.sourceCatalog=reinterpret_cast<uintptr_t>(p.catalog.get());v.sourceLife=p.life->serial();v.sourceArchive=p.archive;v.owner=keep.get();v.retain=&Owner::retain;v.release=&Owner::release;v.current=&Owner::current;return int32_t(nfs_private_child_intro_wide_begin(&v));});
  accepted_=receipt==nfs::OwnedRendererAcceptance::Consumed;
 }
public:
 explicit PreludeScope(Game&g):g_(g),unwind_(nfs_logo_publisher::exceptions()){try{begin();}catch(...){cancel();Owner::release(owner_);owner_=nullptr;throw;}}
 PreludeScope(const PreludeScope&)=delete;PreludeScope&operator=(const PreludeScope&)=delete;
 void cleared(){if(accepted_&&(!countdown_||stage_))cancel();else(void)send(NFS_CHILD_WIDE_CLEARED);}
 void slot1Finished(){if(accepted_&&(!countdown_||stage_!=NFS_CHILD_WIDE_CLEARED))cancel();else(void)send(NFS_CHILD_WIDE_SLOT1);}
 void photoBegin(){if(!accepted_)return;if((countdown_&&stage_!=NFS_CHILD_WIDE_SLOT1)||(!countdown_&&stage_)){cancel();return;}const int32_t x=int32_t(g_.eax),y=int32_t(g_.edx);if(x< -1024||x>1024||y!=316){cancel();return;}(void)send(NFS_CHILD_WIDE_PHOTO_BEGIN,x,y);}
 void photoFinished(){if(accepted_&&stage_!=NFS_CHILD_WIDE_PHOTO_BEGIN)cancel();else(void)send(NFS_CHILD_WIDE_PHOTO_END);}
 void slot0Finished(){if(accepted_&&stage_!=NFS_CHILD_WIDE_PHOTO_END)cancel();else(void)send(NFS_CHILD_WIDE_SLOT0);}
 void brandingBegin(){Errno preserve;if(!accepted_)return;if(stage_!=NFS_CHILD_WIDE_SLOT0||!workerCurrent()||normal_){cancel();return;}normal_.reset(new(std::nothrow) mt::NormalBrandingCallerScope(g_,owner_->packet,serial_,this,&PreludeScope::brandingCurrent));if(!normal_||!normal_->on)cancel();}
 void tailFinished(){normal_.reset();if(accepted_&&stage_!=NFS_CHILD_WIDE_SLOT0)cancel();else(void)send(NFS_CHILD_WIDE_TAIL);}
 ~PreludeScope()noexcept(false){Errno preserve;try{normal_.reset();if(nfs_logo_publisher::exceptions()>unwind_||!accepted_||stage_!=NFS_CHILD_WIDE_TAIL)cancel();else{(void)send(NFS_CHILD_WIDE_END);accepted_=false;}}catch(...){cancel();Owner::release(owner_);owner_=nullptr;throw;}Owner::release(owner_);}
};
} // namespace nfs_child_intro_wide_live

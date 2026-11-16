// SPDX-License-Identifier: MIT
#pragma once
#include "LocationInitialSidebarProof.h"
#include "LocationCaptionGameHooks.inc"
#include "Main343OriginGameHooks.inc"
namespace nfs_location_initial_sidebar_live {
namespace loc=nfs_location_initial_sidebar;namespace bg=nfs_background_scene_cpu;
struct State {
 loc::Store store;bg::Workspace workspace;bg::Plane actual;
 std::shared_ptr<const bg::SourceCatalog>catalog;uintptr_t game=0,thread=0;
 uint32_t descriptor=0;uint64_t next=0,epoch=0;unsigned depth=0,callerDepth=0;bool invalid=false;
};
// Read-only comparison evidence for the initial Location sidebar. A digest
// never authorizes presentation or grants an old Main proof a new lifetime.
inline void traceSidebar(const char*stage,const State&s,uint32_t incoming,const bg::Plane&plane)noexcept{
 const int saved=errno;
 const char*flag=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");
 static thread_local unsigned lines=0;
 if(flag&&flag[0]=='1'&&!flag[1]&&lines<24&&bg::plane(plane,343)){
  unsigned char digest[32]{};
  if(nfs_texture_sha256(plane.pixels.data(),bg::Side*sizeof(uint16_t),digest)){
   char hex[65]{};
   constexpr char alphabet[]="0123456789abcdef";
   for(unsigned i=0;i<32;++i){hex[2*i]=alphabet[digest[i]>>4];hex[2*i+1]=alphabet[digest[i]&15];}
   ++lines;
   std::fprintf(stderr,"LOCATION INITIAL source stage=%s epoch=%llu incoming=%08x pane=%08x bitmap=%08x sha256=%s\n",
    stage,(unsigned long long)s.epoch,incoming,plane.target.owner,plane.target.w[8],hex);
  }
 }
 errno=saved;
}
inline State*&slot(){struct Storage{State*p=nullptr;~Storage(){delete p;}};static thread_local Storage s;return s.p;}
inline State*current(Game&g)noexcept{auto*s=slot();return s&&!s->invalid&&s->depth==1&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==nfs_logo_publisher::threadToken()?s:nullptr;}
struct OwnerScope {
 State*s=nullptr;bool entered=false;int unwind=0;
 explicit OwnerScope(Game&g)noexcept{
  if(!nfs_logo_publisher::enabled()||uint32_t(g.eax)!=uint32_t(guest_address(off_4D2A20)))return;
  auto*&p=slot();if(!p)p=new(std::nothrow)State;if(!p)return;s=p;entered=true;unwind=nfs_logo_publisher::exceptions();if(s->depth++){s->invalid=true;s->store.clear();return;}
  s->store.clear();s->catalog=nfs_background_scene_runtime::acquireCatalog();s->invalid=false;s->callerDepth=0;
  if(!s->catalog||s->next==UINT64_MAX){s->invalid=true;return;}s->epoch=++s->next;s->game=reinterpret_cast<uintptr_t>(&g);s->thread=nfs_logo_publisher::threadToken();s->descriptor=uint32_t(g.eax);
 }
 ~OwnerScope(){if(!entered)return;if(s->depth)--s->depth;if(!s->depth){s->store.clear();s->catalog.reset();s->invalid=true;s->game=s->thread=0;s->descriptor=0;s->callerDepth=0;}}
};
// This wrapper is emitted ONLY around the first40C110 call in40C160,
// before its slot0 upload/swipe. Later regular Location captions cannot enter.
inline bool initialContext(Game&g,const State&s,const nfs_main_sidebar_origin::Proof&origin,uint32_t incoming)noexcept{
 if(!origin.current()||origin.formGuest()!=uint32_t(guest_address(off_4D3EC4))||uint32_t(g.esi)!=s.descriptor||uint32_t(g.ebp)!=incoming)return false;
 try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(dword_4D243C)==incoming&&tx.load<uint32_t>(dword_4D40B8)==origin.archive();}catch(...){return false;}
}
// The Location information pages return through the same initial caption call.
// Check the actual double-buffer selector, archive and descriptor there.
inline bool locationInfoForm(uint32_t descriptor)noexcept{
 return descriptor==uint32_t(guest_address(dword_4D2944))
     ||descriptor==uint32_t(guest_address(dword_4D2824))
     ||descriptor==uint32_t(guest_address(dword_4D2870));
}
inline bool locationInfoReturnPane(Game&g,const State&s,uint32_t incoming,bg::Plane&pane)noexcept{
 const int saved=errno;
 struct RestoreErrno {int value;~RestoreErrno(){errno=value;}} restore{saved};
 if(!locationInfoForm(incoming)||uint32_t(g.esi)!=s.descriptor||uint32_t(g.ebp)!=incoming)return false;
 try{
  uint32_t selector=0,left=0,right=0,archive=0;
  {
   nfs::GuestCopyTransaction tx;
   selector=tx.load<uint32_t>(dword_4D2314+2);
   left=tx.load<uint32_t>(dword_4D231C);
   right=tx.load<uint32_t>(dword_4D2320);
   archive=tx.load<uint32_t>(dword_4D40B8);
   if((selector>>16)>1||!left||!right||left==right||!archive||tx.load<uint32_t>(dword_4D243C)!=incoming)return false;
  }
  pane.valid=0;
  uint32_t failure=0;
  nfs_logo_publisher::plane(pane,failure);
  nfs::GuestCopyTransaction tx;
  return !failure&&bg::plane(pane,343)&&pane.target.owner==((selector>>16)?right:left)
      &&pane.target.ownerState[0]==selector&&pane.target.ownerState[1]==left&&pane.target.ownerState[2]==right
      &&tx.load<uint32_t>(dword_4D2314+2)==selector
      &&tx.load<uint32_t>(dword_4D231C)==left&&tx.load<uint32_t>(dword_4D2320)==right
      &&tx.load<uint32_t>(dword_4D40B8)==archive&&tx.load<uint32_t>(dword_4D243C)==incoming;
 }catch(...){return false;}
}
struct InitialCallerScope {
 Game&g;State*s=nullptr;std::shared_ptr<const nfs_main_sidebar_origin::Proof>origin;
 std::unique_ptr<bg::Plane>infoBefore;uint32_t incomingEbp=0;uint64_t epoch=0;int unwind=0;bool entered=false;
 explicit InitialCallerScope(Game&game)noexcept:g(game){
  s=current(g);if(!s||!s->catalog||uint32_t(g.esi)!=s->descriptor)return;
  s->store.clear();if(s->callerDepth++){s->invalid=true;return;}entered=true;unwind=nfs_logo_publisher::exceptions();incomingEbp=uint32_t(g.ebp);epoch=s->epoch;
  origin=nfs_main_sidebar_origin_live::acquireForInitialCaption(g);
  if(!origin&&locationInfoForm(incomingEbp)){
   infoBefore.reset(new(std::nothrow) bg::Plane);
   if(infoBefore&&!locationInfoReturnPane(g,*s,incomingEbp,*infoBefore))infoBefore.reset();
  }
  const char*flag=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");
  if(flag&&flag[0]=='1'&&!flag[1]){
   s->actual.valid=0;
   uint32_t error=0;
   nfs_logo_publisher::plane(s->actual,error);
   if(!error)traceSidebar(origin?"main-entry-pre":"reentry-pre",*s,incomingEbp,s->actual);
   if(infoBefore)traceSidebar("records-selected-pre",*s,incomingEbp,*infoBefore);
  }
  if(!origin||!origin->current()||origin->game()!=s->game||origin->publisherThread()!=s->thread||(incomingEbp!=origin->formGuest()&&incomingEbp!=0&&incomingEbp!=UINT32_MAX&&incomingEbp!=UINT32_MAX-1)||!initialContext(g,*s,*origin,incomingEbp))origin.reset();
 }
 ~InitialCallerScope(){
  if(!entered)return;if(s->callerDepth)--s->callerDepth;
  if(s==current(g)&&!s->callerDepth&&s->epoch==epoch&&nfs_logo_publisher::exceptions()<=unwind){
   const char*flag=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");
   if(flag&&flag[0]=='1'&&!flag[1]){
    s->actual.valid=0;
    uint32_t failure=0;
    nfs_logo_publisher::plane(s->actual,failure);
    if(!failure)traceSidebar(origin?"main-entry-post":"reentry-post",*s,incomingEbp,s->actual);
   }
   if(infoBefore&&locationInfoReturnPane(g,*s,incomingEbp,s->actual)){
    auto caption=nfs_location_caption::acquireCurrentProof(g);
    if(caption){
     const auto&e=caption->evidence();
     loc::Binding binding{loc::Form,s->descriptor,e.post.archive,incomingEbp,s->game,s->thread,s->epoch,e.serial,e.lifetime};
     uint32_t logoPhase=UINT32_MAX;
     try{
      nfs::GuestCopyTransaction tx;
      logoPhase=tx.load<uint32_t>(dword_4D2444);
     }catch(...){}
     if(s->store.acceptInfoReturn(caption,binding,*infoBefore,s->actual,s->catalog,logoPhase))
      traceSidebar(incomingEbp==uint32_t(guest_address(dword_4D2944))?"records-sidebar-sealed":
        incomingEbp==uint32_t(guest_address(dword_4D2824))?"stats-sidebar-sealed":"track-info-sidebar-sealed",
        *s,incomingEbp,s->actual);
    }
    return;
   }
  }
  if(!origin||s!=current(g)||s->callerDepth||s->epoch!=epoch||nfs_logo_publisher::exceptions()>unwind||!initialContext(g,*s,*origin,incomingEbp)){s->store.clear();return;}
  auto caption=nfs_location_caption::acquireCurrentProof(g);if(!caption){s->store.clear();return;}
  s->actual.valid=0;uint32_t error=0;nfs_logo_publisher::plane(s->actual,error);if(error||!bg::plane(s->actual,343)){s->store.clear();return;}
  const auto&e=caption->evidence();loc::Binding b{loc::Form,s->descriptor,origin->archive(),incomingEbp,s->game,s->thread,s->epoch,e.serial,e.lifetime};
  (void)s->store.accept(origin,caption,b,s->actual,s->workspace,s->catalog);
 }
};
inline std::shared_ptr<const loc::Proof>acquire(Game&g,uint32_t source,const std::vector<uint8_t>&raw)noexcept{
 auto*s=current(g);if(!s||!s->catalog)return {};auto p=s->store.current();if(!p||!p->sourceMatches(source,raw.data(),raw.size()))return {};
 const auto&b=p->binding();if(b.form!=loc::Form||b.formGuest!=s->descriptor||b.game!=s->game||b.thread!=s->thread||b.epoch!=s->epoch||nfs_location_caption::lastProof(g)!=p->captionProof())return {};
 try{nfs::GuestCopyTransaction tx;bg::Target target{};nfs_logo_publisher::target(tx,target,p->post().target.owner);
  if(tx.load<uint32_t>(dword_4D40B8)!=b.archive||!nfs_main_sidebar_origin::sameRetainedTarget(target,p->post().target)||std::memcmp(raw.data(),target.header.data(),16))return {};
  for(unsigned k=0;k<bg::Side;++k)if(bg::le16(raw.data()+16+2*k)!=p->post().pixels[k])return {};
  return p->current()?p:std::shared_ptr<const loc::Proof>{};
 }catch(...){return {};}
}
inline void retire(Game&g,uint32_t at)noexcept{auto*s=current(g);if(!s||!at)return;auto p=s->store.current();if(!p)return;const auto&t=p->post().target;if(at==p->binding().archive||at==t.owner||at==t.w[8]||at==t.w[11]||at==t.w[12]||(uint64_t(at)+4==t.w[11])||(uint64_t(at)+4==t.w[12]))s->store.clear();}
}

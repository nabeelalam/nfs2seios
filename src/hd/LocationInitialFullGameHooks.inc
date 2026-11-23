// SPDX-License-Identifier: MIT
#pragma once
#include "LocationSceneLiveGameHooks.inc"
#include "LocationInitialSidebarGameHooks.inc"
#include "SetupInitialFullProof.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace nfs_location_initial_full_live {
namespace bg=nfs_background_scene_cpu;namespace live=nfs_location_scene_live;namespace initial=nfs_location_initial_sidebar_live;
struct PreserveErrno{int saved=errno;~PreserveErrno()noexcept{errno=saved;}};
using Child=nfs_location_initial_sidebar::Proof;using Preview=nfs_location_preview::Packet;
inline std::shared_ptr<const Child>currentChild(Game&g)noexcept{auto*c=initial::current(g);return c&&c->descriptor==uint32_t(guest_address(off_4D2A20))?c->store.current():std::shared_ptr<const Child>{};}
struct Proof {std::shared_ptr<const Child>child;std::shared_ptr<const Preview>preview; bg::Plane full,copiedSidebar;std::array<uint32_t,3>refreshPane{};bool refresh=false;uintptr_t game=0,thread=0;uint64_t epoch=0,baseSequence=0,previewSerial=0,previewSequence=0;uint32_t archive=0,mapRole=0;
 NfsOriginalPreviewLayerView previewView()const noexcept{PreserveErrno preserve;if(!preview||!preview->artworkCurrent()||!child||!child->current())return {};auto v=preview->view(); return v;}
};
struct State{std::shared_ptr<const Proof>proof;std::shared_ptr<Proof>pending;bg::Plane before,copySide;std::array<uint32_t,9>table{};std::vector<uint8_t>raw;unsigned depth=0,rows=0;uint32_t key=0;std::array<uint32_t,3>refreshPane{};bool failed=false;};
inline State*&slot()noexcept{struct Storage{State*p=nullptr;~Storage(){delete p;}};static thread_local Storage s;return s.p;}
inline void trace(const char*stage,const char*reason,const State*s,uint64_t a=0,uint64_t b=0,uint64_t c=0,uint64_t d=0)noexcept{PreserveErrno preserve;const char*flag=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(!flag||flag[0]!='1'||flag[1])return;struct Key{const char*stage;const char*reason;unsigned count;};static thread_local Key keys[64]{};static thread_local unsigned used=0;Key*k=nullptr;for(unsigned i=0;i<used;++i)if(!std::strcmp(keys[i].stage,stage)&&!std::strcmp(keys[i].reason,reason)){k=&keys[i];break;}if(!k){if(used==64)return;k=&keys[used++];*k={stage,reason,0};}if(k->count++>=2)return;std::fprintf(stderr,"INITIAL FULL stage=%s reason=%s form=%x depth=%u rows=%u failed=%u pending=%u proof=%u a=%llu b=%llu c=%llu d=%llu\n",stage,reason,0x4d2a20u,s?s->depth:0,s?s->rows:0,unsigned(s&&s->failed),unsigned(s&&bool(s->pending)),unsigned(s&&bool(s->proof)),(unsigned long long)a,(unsigned long long)b,(unsigned long long)c,(unsigned long long)d);}
inline bool owned(Game&g,const Proof&p)noexcept{PreserveErrno preserve;
 auto*s=live::current(g);auto child=currentChild(g);if(!s||s->invalid||s->iteration||!s->baseVerified||!p.child||!p.child->current()||child!=p.child||!s->catalogue||!p.child->catalogMatches(*s->catalogue)||p.child->binding().form!=0x4d2a20u||p.child->binding().game!=p.game||p.child->binding().thread!=p.thread||p.child->binding().archive!=p.archive||s->ownerEpoch!=p.epoch||s->game!=p.game||s->thread!=p.thread||s->archive!=p.archive||s->base.event.ownerEpoch!=p.epoch||s->base.event.sequence!=p.baseSequence||s->base.event.iteration||s->base.mapRole!=p.mapRole||!p.preview||!p.preview->artworkCurrent()||s->preview!=p.preview||s->previewSerial!=p.previewSerial||p.preview->serial()!=p.previewSerial||p.preview->lifetime()!=p.epoch||p.preview->archive()!=p.archive||s->previewBaseEvent.ownerEpoch!=p.epoch||s->previewBaseEvent.sequence!=p.baseSequence||s->previewBaseEvent.iteration||s->previewEvent.ownerEpoch!=p.epoch||s->previewEvent.sequence!=p.previewSequence||s->previewEvent.iteration||p.previewSequence<=p.baseSequence)return false;
 return live::archiveCurrent(*s);
}
// The fade and full cache can use opposite original double buffers. Bind the
// exact pane selected by the copy call and keep the live caption proof for art.
inline bool readInfoReturnCopyTarget(const Proof&p,uint32_t bitmap,bg::Target&target)noexcept{
 try{
  nfs::GuestCopyTransaction tx;
  const auto selector=tx.load<uint32_t>(dword_4D2314+2)>>16;
  const auto left=tx.load<uint32_t>(dword_4D231C);
  const auto right=tx.load<uint32_t>(dword_4D2320);
  const auto&fade=p.child->post().target;
  if(!p.child->locationInfoReturn()||selector>1||!left||!right||left==right
      ||left!=fade.ownerState[1]||right!=fade.ownerState[2]
      ||tx.load<uint32_t>(dword_4D243C)!=p.child->binding().incomingEbp
      ||tx.load<uint32_t>(dword_4D40B8)!=p.archive)return false;
  const uint32_t selected=selector?right:left;
  nfs_logo_publisher::target(tx,target,selected);
  return target.owner==selected&&target.w[8]==bitmap&&bg::fullClip(target,343)&&!target.header[3];
 }catch(...){return false;}
}
// Only the exact40C5C6 refresh copy may read a newly painted selected343
// pane. The entrance proof remains caption/art identity, never these pixels.
inline bool readRefreshCopyTarget(const Proof&p,uint32_t bitmap,bg::Target&target,
                                 std::array<uint32_t,3>&pane)noexcept{
 try{
  nfs::GuestCopyTransaction tx;
  pane={{tx.load<uint32_t>(dword_4D2314+2)>>16,
         tx.load<uint32_t>(dword_4D231C),tx.load<uint32_t>(dword_4D2320)}};
  if(!p.refresh||!p.epoch||!bitmap||!p.child||!p.child->current()||
     p.child->binding().epoch!=p.epoch||p.child->binding().formGuest!=uint32_t(guest_address(off_4D2A20))||
     pane[0]>1||!pane[1]||!pane[2]||pane[1]==pane[2]||
     tx.load<uint32_t>(dword_4D40B8)!=p.archive)return false;
  const uint32_t selected=pane[1+pane[0]];
  if(tx.load<uint32_t>(selected+0x20)!=bitmap)return false;
  nfs_logo_publisher::target(tx,target,selected);
  nfs_soft::SurfaceIdentity identity{};
  return target.owner==selected&&target.w[8]==bitmap&&
   nfs_location_scene_cpu::captionIdentity(target,p.epoch,identity);
 }catch(...){return false;}
}
inline bool captureCopySide(Game&g,const Proof&p,State&s,uint32_t bitmap)noexcept{
 if(!live::readBytes(bitmap,16+bg::Side*2,s.raw))return false;
 if(p.refresh){
  s.copySide.valid=0;
  if(!readRefreshCopyTarget(p,bitmap,s.copySide.target,s.refreshPane))return false;
  for(unsigned i=0;i<4;++i)if(bg::le32(s.raw.data()+4*i)!=s.copySide.target.header[i])return false;
  s.copySide.width=343;s.copySide.height=480;
  for(unsigned i=0;i<bg::Side;++i)s.copySide.pixels[i]=bg::le16(s.raw.data()+16+2*i);
  s.copySide.valid=1;
  return bg::plane(s.copySide,343);
 }
 if(bitmap==p.child->post().target.w[8]){
  if(initial::acquire(g,bitmap,s.raw)!=p.child)return false;
  s.copySide=p.child->post();
  return true;
 }
 s.copySide.valid=0;
 if(!readInfoReturnCopyTarget(p,bitmap,s.copySide.target))return false;
 for(unsigned i=0;i<4;++i)if(bg::le32(s.raw.data()+4*i)!=s.copySide.target.header[i])return false;
 s.copySide.width=343;s.copySide.height=480;
 for(unsigned i=0;i<bg::Side;++i)s.copySide.pixels[i]=bg::le16(s.raw.data()+16+2*i);
 s.copySide.valid=1;
 return bg::plane(s.copySide,343);
}
inline bool copySideCurrent(Game&g,const Proof&p,const State&s)noexcept{
 const auto bitmap=s.copySide.target.w[8];
 std::vector<uint8_t>after;
 if(!live::readBytes(bitmap,16+bg::Side*2,after)||after!=s.raw)return false;
 if(p.refresh){
  bg::Target target{};std::array<uint32_t,3>pane{};
  return readRefreshCopyTarget(p,bitmap,target,pane)&&pane==s.refreshPane&&
   nfs_logo_capture::same(target,s.copySide.target);
 }
 if(bitmap==p.child->post().target.w[8])return initial::acquire(g,bitmap,after)==p.child;
 bg::Target target{};
 return readInfoReturnCopyTarget(p,bitmap,target)&&nfs_logo_capture::same(target,s.copySide.target);
}
// Installed only at the exact40C160 initial/refresh405B60; no ordinary draw admission.
struct CopyScope{Game&g;State*s=nullptr;bool entered=false;int unwind=0;
 explicit CopyScope(Game&game,bool refresh=false)noexcept:g(game){PreserveErrno preserve;auto*liveState=live::current(g);auto child=currentChild(g);if(!liveState||liveState->invalid||liveState->iteration||!liveState->baseVerified||!child||uint32_t(g.esi)!=uint32_t(guest_address(off_4D2A20))){trace("copy-ctor","owner-or-child",s,uint64_t(liveState!=nullptr),liveState?liveState->iteration:0,liveState?unsigned(liveState->baseVerified):0,uint32_t(g.esi));return;}
 auto*&storage=slot();if(!storage)storage=new(std::nothrow)State;if(!storage)return;s=storage;s->proof.reset();entered=true;if(s->depth++){s->pending.reset();s->failed=true;return;}unwind=nfs_logo_publisher::exceptions();s->pending.reset();s->rows=0;s->failed=false;
 try{auto p=std::make_shared<Proof>();p->child=std::move(child);p->preview=liveState->preview;p->game=liveState->game;p->thread=liveState->thread;p->epoch=liveState->ownerEpoch;p->archive=liveState->archive;p->baseSequence=liveState->base.event.sequence;p->previewSerial=liveState->previewSerial;p->previewSequence=liveState->previewEvent.sequence;p->mapRole=liveState->base.mapRole;p->refresh=refresh;
 if(!owned(g,*p)||!p->preview->baseMatches(liveState->base.post)||uint32_t(g.edx)!=0x2800||g.ebx||g.ecx||p->mapRole>1){trace("copy-ctor","binding-or-input",s,uint32_t(g.eax),uint32_t(g.edx),p->previewSerial,p->baseSequence);return;}
 if(!captureCopySide(g,*p,*s,uint32_t(g.eax))||s->copySide.target.header[3]){trace("copy-ctor","child-source",s,s->raw.size());return;}
 {nfs::GuestCopyTransaction tx;nfs_logo_publisher::table(tx,s->table,true);if(nfs_small_logo_live::overlap(uint32_t(g.eax),s->raw.size(),liveState->base.post.target.w[8],16+bg::Full*2)){trace("copy-ctor","source-overlap",s);return;}}
 if(!live::readPlane(s->before)||!bg::plane(s->before,640)||!bg::retainedBaseTarget(liveState->base.post.target,s->before.target)||p->preview->post().size()!=bg::Full||!std::equal(p->preview->post().begin(),p->preview->post().end(),s->before.pixels.begin())){trace("copy-ctor","preview-before",s,s->before.valid,s->before.width,p->preview->post().size());return;}s->pending=std::move(p);trace("copy-ctor","armed",s);
 }catch(...){trace("copy-ctor","exception",s);s->pending.reset();}}
 ~CopyScope(){PreserveErrno preserve;if(!entered)return;if(s->depth==1){auto p=std::move(s->pending);std::array<uint32_t,9>after{};try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::table(tx,after,true);}catch(...){s->failed=true;}
 const bool sourceOkay=p&&copySideCurrent(g,*p,*s);
 if(p&&!s->failed&&s->rows==480&&sourceOkay&&nfs_small_logo_live::tables(s->table,after,true,0,0x7d)&&nfs_logo_publisher::exceptions()==unwind&&owned(g,*p)&&live::readPlane(p->full)&&nfs_setup_initial_full::copied(s->before,s->copySide,p->full,s->key)&&owned(g,*p)){if(p->refresh||p->child->locationInfoReturn()){p->copiedSidebar=s->copySide;if(p->refresh)p->refreshPane=s->refreshPane;}s->proof=std::move(p);trace("copy-dtor","sealed",s);}else{trace("copy-dtor","rejected",s,unsigned(sourceOkay),unsigned(bool(p)),p?p->full.valid:0);s->proof.reset();}}if(s->depth)--s->depth;}
};
inline void row(Game&g)noexcept{PreserveErrno preserve;auto*s=slot();if(!s||s->depth!=1||!s->pending||s->failed)return;auto p=s->pending;
 if(!owned(g,*p)||s->rows>=480||uint32_t(g.ebx)!=343||uint32_t(g.edx)!=uint64_t(s->before.target.w[8])+s->before.target.y[s->rows]+s->before.target.x[0]){trace("row","owner-or-destination",s,uint32_t(g.ebx),uint32_t(g.edx));s->failed=true;return;}
 try{nfs::GuestCopyTransaction tx;const uint32_t source=uint32_t(g.eax),key=tx.load<uint32_t>(dword_4DD03C);if(tx.load<uint32_t>(dword_571264)!=0xf0001f44u||tx.load<uint32_t>(dword_571260)!=uint32_t(game_id_sub_483070)||(s->rows&&key!=s->key)||nfs_small_logo_live::overlap(source,343*4,s->before.target.w[8],16+bg::Full*2)||nfs_small_logo_live::overlap(source,343*4,s->copySide.target.w[8],16+bg::Side*2))throw 1;
 nfs_logo_publisher::checked(source,343*4);for(unsigned x=0;x<343;++x)if(tx.load<uint32_t>(source+4*x)!=nfs_small_logo_live::expanded(s->copySide.pixels[s->rows*343+x],key))throw 1;bg::Target now{};nfs_logo_publisher::target(tx,now);if(!nfs_logo_capture::same(now,s->before.target))throw 1;s->key=key;++s->rows;
 }catch(...){trace("row","record",s,uint32_t(g.eax),uint32_t(g.edx),s->key);s->failed=true;}}
inline std::shared_ptr<const Proof>acquire(Game&g,uint32_t source,const std::vector<uint8_t>&raw)noexcept{PreserveErrno preserve;auto*s=slot();if(!s||s->depth||!s->proof){trace("acquire","no-proof",s);return {};}auto p=s->proof;if(!owned(g,*p)||source!=p->full.target.w[8]||raw.size()!=16+bg::Full*2){trace("acquire","binding-source",s,source,p->full.target.w[8],raw.size());return {};}
 try{nfs::GuestCopyTransaction tx;bg::Target now{};nfs_logo_publisher::target(tx,now,p->full.target.owner);if(!nfs_main_sidebar_origin::sameRetainedTarget(now,p->full.target))return {};}catch(...){return {};}
 for(unsigned k=0;k<4;++k)if(bg::le32(raw.data()+4*k)!=p->full.target.header[k])return {};for(unsigned k=0;k<bg::Full;++k)if(bg::le16(raw.data()+16+2*k)!=p->full.pixels[k])return {};if(!owned(g,*p)){trace("acquire","owner-after",s);return {};}trace("acquire","accepted",s);return p;
}
}

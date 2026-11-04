// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundSceneQueueAdapter.inc"
#include "MenuTransitionAbi.h"
#include "MenuInitialSidebarGameHooks.inc"
namespace nfs_menu_transition_live {
namespace bg=nfs_background_scene_cpu;
struct WorkerErrno{int saved=errno;~WorkerErrno()noexcept{errno=saved;}};
struct Packet{uint64_t content=0;uintptr_t game=0,thread=0;uint32_t archive=0,source=0,slot=0,kind=0,mapRole=0;std::vector<uint8_t>raw;std::array<uint16_t,307200>original{};std::array<uint32_t,300>addresses{};NfsBackgroundSceneLayers layers{};bg::Target sourceTarget{};bool targetBound=false;std::shared_ptr<const nfs_initial_sidebar_transition::Proof>initial;std::shared_ptr<const nfs_main_sidebar_origin::Proof>origin;std::shared_ptr<const nfs_main_sidebar_origin::GameLife>life;};
struct State{std::array<std::shared_ptr<const Packet>,2>slots{};std::shared_ptr<Packet>pending;uint64_t nextContent=0;unsigned tiles=0;bool failed=false;};
inline State&state(){static thread_local State s;return s;}
// Slot number is an original428250 argument, not a source-kind enum. Main's
// exit uploads its343 sidebar toslot0; its gradient still reads whole Main slot1.
inline bool unchanged(const Packet&p)noexcept{
 if(!p.life||!p.life->current()||(p.origin&&!p.origin->current())||(p.initial&&!nfs_initial_sidebar_transition::current(*p.initial)))return false;
 try{nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=p.archive)return false;
  nfs_logo_publisher::checked(p.source,p.raw.size());
  if(std::memcmp(nfs_host_address(p.source,p.raw.size()),p.raw.data(),p.raw.size()))return false;
  if(p.targetBound){bg::Target actual{};if(p.initial)nfs_initial_sidebar_transition::readTarget(tx,actual,p.sourceTarget.owner);else nfs_logo_publisher::target(tx,actual,p.sourceTarget.owner);
   if(!nfs_main_sidebar_origin::sameRetainedTarget(actual,p.sourceTarget))return false;}
  return true;
 }catch(...){return false;}
}
inline bool bindOrigin(Packet&p,Game&g,bool full)noexcept{try{
 auto*history=nfs_main_sidebar_origin_live::current(g);auto catalog=nfs_background_scene_runtime::acquireCatalog();
 if(!history||!catalog)return false;
 for(auto proof:history->origins.entries()){
  if(!proof||!proof->current()||proof->game()!=p.game||proof->publisherThread()!=p.thread||proof->archive()!=p.archive||!nfs_main_sidebar_origin::sameCatalog(*proof->catalog(),*catalog))continue;
  const auto&t=full?proof->copy().post.target:proof->post().target;
  if(t.w[8]!=p.source||p.raw.size()<16||std::memcmp(p.raw.data(),t.header.data(),16))continue;
  auto frame=proof->frame();if(!frame||!frame->valid)continue;
  bool exact=true;
  if(full){if(frame->original!=p.original||frame->keys!=p.addresses)exact=false;}
  else for(unsigned k=0;k<bg::Side;++k)if(bg::le16(p.raw.data()+16+2*k)!=proof->post().pixels[k]){exact=false;break;}
  if(!exact)continue;
  bg::Target actual{};{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,actual,t.owner);}
  if(!nfs_main_sidebar_origin::sameRetainedTarget(actual,t))continue;
  p.origin=proof;p.sourceTarget=t;p.targetBound=true;p.kind=full?3:2;p.mapRole=frame->mapRole;
  p.layers.mapRole=frame->mapRole;p.layers.wordCount=frame->wordCount;p.layers.panelClip[2]=343;p.layers.panelClip[3]=480;
  std::copy(frame->words.begin(),frame->words.end(),p.layers.words);std::copy(frame->logos.begin(),frame->logos.end(),p.layers.logos);
  return unchanged(p);
 }
 return false;
 }catch(...){return false;}
}
// Only real428250 source+300 real49755C uploads can create a packet.
struct UploadScope{State*s=nullptr;Game*g=nullptr;std::shared_ptr<const Packet>previous;bool active=false;int unwind=0;explicit UploadScope(Game&game)noexcept{WorkerErrno preserve;if(!nfs_background_scene_live::enabled()||uint32_t(game.edx)>1)return;g=&game;s=&state();try{nfs::GuestCopyTransaction tx;const uint32_t wh=tx.load<uint32_t>(uint32_t(game.eax)+4);if((wh>>16)!=480||((wh&65535)!=640&&(wh&65535)!=343)){s->slots[uint32_t(game.edx)].reset();return;}}catch(...){s->slots[uint32_t(game.edx)].reset();return;}previous=s->slots[uint32_t(game.edx)];s->slots[uint32_t(game.edx)].reset();if(s->pending){s->failed=true;return;}try{auto p=std::make_shared<Packet>();p->game=reinterpret_cast<uintptr_t>(&game);p->thread=nfs_logo_publisher::threadToken();p->life=nfs_main_sidebar_origin::acquireGameLife(p->game);if(!p->life)return;p->source=uint32_t(game.eax);p->slot=uint32_t(game.edx);auto catalog=nfs_background_scene_runtime::acquireCatalog();if(!catalog)return;uint32_t width=0,height=0;{nfs::GuestCopyTransaction tx;p->archive=tx.load<uint32_t>(dword_4D40B8);const uint32_t wh=tx.load<uint32_t>(p->source+4);width=wh&65535;height=wh>>16;if((width!=640&&width!=343)||height!=480)return;const size_t span=16+size_t(width)*height*2;p->raw.resize(span);nfs_logo_publisher::checked(p->source,span);std::memcpy(p->raw.data(),nfs_host_address(p->source,span),span);}
 if(width==640){for(unsigned role=0;role<2;++role)if(nfs_small_logo_live::hashMatches(p->raw.data(),uint32_t(p->raw.size()),catalog->mapSha[role])){p->kind=1;p->mapRole=role;break;}
  // The whole-Main join is minted by the existing upload-seal destructor,
  // after all actual tiles have been observed. It is never adopted at entry.
  if(!p->kind)p->kind=3;
 }else if(!bindOrigin(*p,game,false)){
  auto first=nfs_initial_sidebar_transition::acquire(game,p->source,p->raw);if(!first)return;
  p->initial=first;p->sourceTarget=first->target;p->targetBound=true;p->layers=first->layers;p->kind=4;p->mapRole=0;
 }
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<width;++x)p->original[y*640+x]=bg::le16(p->raw.data()+16+2*(y*width+x));if(!unchanged(*p)||s->nextContent==UINT64_MAX)return;p->content=++s->nextContent;s->pending=std::move(p);s->tiles=0;s->failed=false;active=true;unwind=nfs_logo_publisher::exceptions();}catch(...){s->pending.reset();}}
 ~UploadScope(){WorkerErrno preserve;if(!active||!s||!g||!s->pending)return;auto p=std::move(s->pending);if(s->failed||nfs_logo_publisher::exceptions()>unwind||!unchanged(*p))return;if(!s->tiles&&previous&&previous->game==p->game&&previous->thread==p->thread&&previous->source==p->source&&previous->kind==p->kind&&previous->raw==p->raw&&unchanged(*previous)){s->slots[p->slot]=std::move(previous);return;}if(s->tiles!=300)return;if(p->kind==3&&!bindOrigin(*p,*g,true))return;if(!unchanged(*p))return;s->slots[p->slot]=std::move(p);}};
struct TileScope{State*s=nullptr;nfs_logo_publisher::TileObservation t{};unsigned index=0;int unwind=0;bool active=false;explicit TileScope(Game&g)noexcept{WorkerErrno preserve;s=&state();if(!s->pending||s->failed||s->tiles>=300)return;index=s->tiles;unwind=nfs_logo_publisher::exceptions();try{nfs::GuestCopyTransaction tx;uint32_t sp=uint32_t(g.esp);t.descriptorGuest=tx.load<uint32_t>(sp);t.payloadGuest=tx.load<uint32_t>(sp+4);t.tableGuest=tx.load<uint32_t>(sp+8);t.expectedSlotDescriptor=tx.load<uint32_t>(dword_4EB74C+s->pending->slot*1200+index*4);nfs_logo_publisher::descriptor(tx,t.descriptorGuest,t.before);nfs_logo_publisher::checked(t.payloadGuest,2048);std::memcpy(t.pixels.data(),nfs_host_address(t.payloadGuest,2048),2048);active=true;}catch(...){s->failed=true;}}
 ~TileScope(){WorkerErrno preserve;if(!active)return;bool ok=nfs_logo_publisher::exceptions()==unwind;try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::descriptor(tx,t.descriptorGuest,t.after);if(std::memcmp(t.pixels.data(),nfs_host_address(t.payloadGuest,2048),2048))ok=false;}catch(...){ok=false;}auto p=s->pending;if(!p){s->failed=true;return;}ok=ok&&t.descriptorGuest&&t.descriptorGuest==t.expectedSlotDescriptor&&!t.tableGuest&&t.before[0]==3&&t.before[1]==3&&t.before[2]==3&&t.before[3]==11&&t.before[6]==3&&t.after[4]==t.payloadGuest&&(t.after[5]&3)==0&&t.after[5]<=0x200000u-2048u;for(unsigned k=0;k<7;++k)if(k!=4&&t.before[k]!=t.after[k])ok=false;for(unsigned k=0;k<index;++k)if(uint64_t(p->addresses[k])<uint64_t(t.after[5])+2048&&uint64_t(t.after[5])<uint64_t(p->addresses[k])+2048)ok=false;unsigned x=(index/15)*32,y=(index%15)*32;for(unsigned yy=0;yy<32;++yy)for(unsigned xx=0;xx<32;++xx)if(bg::le16(t.pixels.data()+2*(yy*32+xx))!=nfs_logo_publisher::to1555(p->original[(y+yy)*640+x+xx]))ok=false;if(!ok){s->failed=true;return;}p->addresses[index]=t.after[5];++s->tiles;}};
struct DrawScope{
 std::shared_ptr<const Packet>packet;uint64_t serial=0;bool accepted=false;
 explicit DrawScope(Game&g,bool plain=false)noexcept{WorkerErrno preserve;
  if(!nfs_background_scene_live::enabled()||nfs_background_scene_live::activeRequest()||uint32_t(g.eax)>1)return;
  auto p=state().slots[uint32_t(g.eax)];
  // Ordinary whole-Main427B30 has its maintained full-scene owner; don't nest.
  if(!p||(plain&&p->kind==3)||p->game!=reinterpret_cast<uintptr_t>(&g)||p->thread!=nfs_logo_publisher::threadToken()||!unchanged(*p))return;
  NfsMenuTransitionView v{};
  try{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);v.originX=int32_t(g.edx);
   if(plain){if(tx.load<uint32_t>(sp+4)!=UINT32_MAX)return;v.drawKind=2;v.originY=int32_t(g.ebx);v.stepX=int32_t(g.ecx);v.minimum=0;v.maximum=255;v.phase=-1;}
   else{v.drawKind=1;v.originY=0;v.stepX=int32_t(g.ebx);v.minimum=int32_t(g.ecx);v.maximum=tx.load<int32_t>(sp+4);v.direction=tx.load<int32_t>(sp+8);v.phase=tx.load<int32_t>(sp+12);}
   if(v.minimum<0||v.maximum>255||v.minimum>v.maximum||v.direction<0||v.direction>1||v.phase< -1||v.phase>255||v.originX< -32768||v.originX>32767||v.originY< -32768||v.originY>32767||v.stepX<0||v.stepX>32)return;
  }catch(...){return;}
  serial=nfs_background_scene_live::nextRequest();if(!serial)return;
  v.serial=serial;v.content=p->content;v.slot=p->slot;v.kind=p->kind;v.mapRole=p->mapRole;v.original565=p->original.data();std::copy(p->addresses.begin(),p->addresses.end(),v.addresses);v.layers=p->layers;packet=p;
  try{const auto receipt=nfs::enqueue_owned_renderer([p,v](){return int32_t(nfs_private_menu_transition_begin(&v));});accepted=receipt==nfs::OwnedRendererAcceptance::Consumed;}
  catch(...){packet.reset();serial=0;}
 }
 ~DrawScope(){WorkerErrno preserve;if(!accepted||!packet||!serial)return;auto p=packet;const uint64_t id=serial;try{(void)nfs::enqueue_owned_renderer([p,id](){return int32_t(nfs_private_menu_transition_finish(id));});}catch(...){}}
};
struct PlainDrawScope:DrawScope{explicit PlainDrawScope(Game&g)noexcept:DrawScope(g,true){}};
}

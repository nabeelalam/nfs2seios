// SPDX-License-Identifier: MIT
// Exact Main pre-base sidebar copy. No title-frame or previously displayed
// framebuffer is adopted as an artwork source.
#pragma once
#include "BackgroundSceneQueueAdapter.inc"
#include "BackgroundSceneLayersAbi.h"
#include <cerrno>
namespace nfs_initial_sidebar_transition {
namespace bg=nfs_background_scene_cpu;
struct ErrnoScope{int saved=errno;~ErrnoScope(){errno=saved;}};
struct CopyLife{std::atomic<bool>alive{true};};
struct Proof{
 uintptr_t game=0,thread=0;uint32_t archive=0;uint64_t epoch=0,iteration=0;
 bg::Event before{},after{};bg::Target target{};
 std::array<uint16_t,bg::Side>pixels{};NfsBackgroundSceneLayers layers{};
 std::shared_ptr<const nfs_main_sidebar_origin::GameLife>gameLife;
 std::shared_ptr<CopyLife>life;
};
struct CopyScope;
struct State{
 std::shared_ptr<const Proof>proof;CopyScope*copying=nullptr;
 void clear()noexcept{if(proof&&proof->life)proof->life->alive.store(false,std::memory_order_release);proof.reset();}
 ~State(){clear();}
};
inline State&state(){static thread_local State s;return s;}
// This lexical temporary is NOT one of Main's four global handles. Bind its
// exact allocation owner from both original stack locals; never make generic
// target() accept an unknown owner by matching pixels alone.
inline void readTarget(nfs::GuestCopyTransaction&tx,bg::Target&t,uint32_t owner,bool ambient=false){
 if(!owner)throw 1;nfs_logo_publisher::checked(owner,56);t={};t.owner=owner;
 for(unsigned k=0;k<14;++k){t.ownerWords[k]=tx.load<uint32_t>(owner+4*k);t.w[k]=ambient?tx.load<uint32_t>(uint32_t(guest_address(dword_4DB240))+4*k):t.ownerWords[k];}
 if(t.w[0]!=0x57494e44u||t.w[1]!=343||t.w[2]!=480||t.w[7]!=0x7810||!t.w[8]||!t.w[11]||!t.w[12])throw 1;
 nfs_logo_publisher::checked(t.w[8],16+bg::Side*2);nfs_logo_publisher::checked(t.w[11],480*4);nfs_logo_publisher::checked(t.w[12],343*4);
 for(unsigned k=0;k<4;++k)t.header[k]=tx.load<uint32_t>(t.w[8]+4*k);
 for(unsigned k=0;k<343;++k)t.x[k]=tx.load<uint32_t>(t.w[12]+4*k);
 for(unsigned k=0;k<480;++k)t.y[k]=tx.load<uint32_t>(t.w[11]+4*k);
 t.ownerState[0]=tx.load<uint32_t>(dword_4D2314+2);
 const uint32_t globals[]={uint32_t(guest_address(dword_4D231C)),uint32_t(guest_address(dword_4D2320)),uint32_t(guest_address(dword_4D4D24)),uint32_t(guest_address(dword_4DABE4))};
 for(unsigned k=0;k<4;++k){t.ownerState[k+1]=tx.load<uint32_t>(globals[k]);if(t.ownerState[k+1]==owner)throw 1;}
 if(!bg::fullClip(t,343)||t.header[3])throw 1;
}
inline bool current(const Proof&p)noexcept{
 if(!p.life||!p.life->alive.load(std::memory_order_acquire)||!p.gameLife||!p.gameLife->current()||p.thread!=nfs_logo_publisher::threadToken())return false;
 auto*s=nfs_background_scene_live::slot();
 // The independently completed copy survives unrelated native upload resets,
 // but not the first map draw, another Main owner, archive or Game lifetime.
 return s&&s->on&&s->mainDepth==1&&s->game==p.game&&s->thread==p.thread&&s->ownerEpoch==p.epoch&&!s->baseSeen&&s->archive==p.archive;
}
inline bool initialLayers(nfs_background_scene_live::State&s,const nfs_logo_publisher::Collector&c,NfsBackgroundSceneLayers&out)noexcept{
 try{
  const auto&scene=s.scene;const auto&g=c.group;const auto&copy=c.copy;
  // Admission below replays the exact343 sidebar and group/copy independently.
  // Collector::prepared admits a different640 native composition (including
  // its opaque-underlay/upload lifetime); it is not authority for this copy.
  if(s.invalid||s.baseSeen||s.baseVerified||s.invocationDepth||!s.iteration||!s.catalogue||!bg::validCatalog(*s.catalogue)||!scene.complete||scene.error||scene.form!=bg::MainForm||scene.draws.empty()||scene.draws.size()>5||c.depth||c.invocationDepth||!g.complete||!copy.complete||c.invalid||c.gameOwner!=s.game||c.thread!=s.thread||g.thread!=s.thread||g.serial!=s.groupSerial||copy.serial!=s.copySerial||copy.linkedGroup!=g.serial)return false;
  const bg::Event events[]={scene.event,s.groupEvent,s.tailEvent,s.copyEvent};
  for(unsigned k=0;k<4;++k)if(events[k].ownerEpoch!=s.ownerEpoch||events[k].iteration!=s.iteration||!events[k].sequence||(k&&events[k].sequence<=events[k-1].sequence))return false;
  if(!bg::plane(scene.preSide,343)||!nfs_logo_capture::same(scene.preSide.target,scene.postSide.target)||!nfs_logo_capture::same(scene.preSide.target,scene.postWords.target))return false;
  auto failure=bg::side(scene.draws[0],scene.preSide.target,s.workspace,*s.catalogue);
  if(failure!=bg::Failure::None||!bg::plane(scene.postSide,343)||!std::equal(s.workspace.side.begin(),s.workspace.side.end(),scene.postSide.pixels.begin()))return false;
  out={};out.panelClip[2]=343;out.panelClip[3]=480;out.wordCount=unsigned(scene.draws.size()-1);
  for(unsigned k=1;k<scene.draws.size();++k)if(bg::word(scene.draws[k],scene.preSide.target,s.workspace,out.words[k-1],*s.catalogue)!=bg::Failure::None)return false;
  if(!bg::plane(scene.postWords,343)||!std::equal(s.workspace.side.begin(),s.workspace.side.end(),scene.postWords.pixels.begin())||!bg::samePlane(scene.postWords,g.pre,343))return false;
  auto logos=std::unique_ptr<nfs_small_logo_live::Prepared>(new nfs_small_logo_live::Prepared);
  if(nfs_small_logo_live::verifyAndPrepare(g,copy,s.workspace.logo,*logos,s.catalogue->logos)!=nfs_small_logo_live::Failure::None||copy.sourceOwner.header[3])return false;
  if(scene.tailKind!=NFS_BACKGROUND_TAIL_MAIN_SKIP||scene.tailPredicate!=bg::MainForm||scene.tailSymbol!=bg::MainForm||!bg::samePlane(scene.tailPre,g.post,343)||!bg::samePlane(scene.tailPre,scene.tailPost,343))return false;
  for(unsigned k=0;k<bg::Side;++k)if(bg::le16(copy.sourceBefore.data()+16+2*k)!=scene.tailPost.pixels[k]||bg::le16(copy.sourceAfter.data()+16+2*k)!=scene.tailPost.pixels[k])return false;
  std::copy(logos->layers.begin(),logos->layers.end(),out.logos);
  return true;
 }catch(...){return false;}
}
struct CopyScope{
 Game&g;State*store=nullptr;std::shared_ptr<Proof>proof;bg::Target source{};
 std::vector<uint8_t>raw;uint32_t mainStack=0;size_t copied=0;unsigned rows=0;bool failed=false,active=false;int unwind=0;
 explicit CopyScope(Game&game)noexcept:g(game){ErrnoScope preserve;
  auto*s=nfs_background_scene_live::current(g);if(!s||s->invalid||s->baseSeen||s->baseVerified||s->sequence>UINT64_MAX-2||uint32_t(g.edx)!=0x400||g.ebx||g.ecx)return;
  store=&state();if(store->copying){store->copying->failed=true;return;}store->clear();
  auto*c=nfs_logo_publisher::store();if(!c)return;
  try{
   {nfs::GuestCopyTransaction tx;if(!tx.load<uint16_t>(uint32_t(guest_address(word_4D232C))))return;}
   auto p=std::make_shared<Proof>();if(!initialLayers(*s,*c,p->layers))return;
   p->game=reinterpret_cast<uintptr_t>(&g);p->thread=nfs_logo_publisher::threadToken();p->archive=s->archive;p->epoch=s->ownerEpoch;p->iteration=s->iteration;
   p->gameLife=nfs_main_sidebar_origin::acquireGameLife(p->game);p->life=std::make_shared<CopyLife>();if(!p->gameLife)return;
   const auto&tail=s->scene.tailPost;
   {nfs::GuestCopyTransaction tx;
    if(uint32_t(g.eax)!=tail.target.w[8]||tx.load<uint32_t>(dword_4D40B8)!=p->archive)return;
    nfs_logo_publisher::target(tx,source,tail.target.owner);
    const uint32_t sp=uint32_t(g.esp);if(sp>UINT32_MAX-0x11c)throw 1;mainStack=sp;
    const uint32_t temporary=tx.load<uint32_t>(sp+0x118);if(tx.load<uint32_t>(sp+0x114)!=temporary)throw 1;
    readTarget(tx,p->target,temporary,true);
   }
   if(!bg::fullClip(p->target,343)||!nfs_main_sidebar_origin::sameRetainedTarget(source,tail.target)||source.header[3]||p->target.header[3]||nfs_small_logo_live::overlap(source.w[8],16+bg::Side*2,p->target.w[8],16+bg::Side*2))return;
   if(!nfs_background_scene_live::readBytes(source.w[8],16+bg::Side*2,raw))return;
   if(std::memcmp(raw.data(),source.header.data(),16))return;
   for(unsigned k=0;k<bg::Side;++k){const auto pixel=bg::le16(raw.data()+16+2*k);if(pixel!=tail.pixels[k])return;p->pixels[k]=pixel;}
   p->before=s->event(s->iteration);if(!p->before.sequence)return;
   proof=std::move(p);unwind=nfs_logo_publisher::exceptions();store->copying=this;active=true;
  }catch(...){proof.reset();}
 }
 ~CopyScope()noexcept{ErrnoScope preserve;if(!active||!store)return;store->copying=nullptr;
  try{
   auto*s=nfs_background_scene_live::current(g);
   if(failed||uint32_t(g.esp)!=mainStack||nfs_logo_publisher::exceptions()>unwind||!proof||!s||s->invalid||s->baseSeen||s->ownerEpoch!=proof->epoch||s->iteration!=proof->iteration||copied!=bg::Side*2||(rows!=1&&rows!=480))return;
   bg::Target sourceNow{},targetNow{};{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,sourceNow,source.owner);readTarget(tx,targetNow,proof->target.owner,true);if(tx.load<uint32_t>(mainStack+0x114)!=proof->target.owner||tx.load<uint32_t>(mainStack+0x118)!=proof->target.owner)return;if(tx.load<uint32_t>(dword_4D40B8)!=proof->archive)return;}
   if(!nfs_main_sidebar_origin::sameRetainedTarget(sourceNow,source)||!nfs_logo_capture::same(targetNow,proof->target))return;
   std::vector<uint8_t>after,destination;
   if(!nfs_background_scene_live::readBytes(source.w[8],raw.size(),after)||after!=raw||!nfs_background_scene_live::readBytes(proof->target.w[8],raw.size(),destination)||std::memcmp(destination.data(),proof->target.header.data(),16)||std::memcmp(destination.data()+16,raw.data()+16,bg::Side*2))return;
   proof->after=s->event(s->iteration);if(!proof->after.sequence||proof->after.sequence<=proof->before.sequence)return;
   store->proof=std::move(proof);
  }catch(...){}
 }
};
// The real4A14D4 byte-copy entry is observed once per native opaque row/block.
// Before/after bytes and sequential offsets prove the copy actually occurred.
struct CopyLeafScope{
 CopyScope*copy=nullptr;size_t offset=0,count=0;uint32_t source=0,destination=0;int unwind=0;bool active=false;
 explicit CopyLeafScope(Game&g)noexcept{ErrnoScope preserve;copy=state().copying;if(!copy||copy->failed||!copy->proof)return;
  source=uint32_t(g.eax);destination=uint32_t(g.edx);count=uint32_t(g.ebx);
  const uint64_t first=uint64_t(copy->proof->target.w[8])+16,last=first+bg::Side*2;
  if(destination<first||uint64_t(destination)>=last){if(uint64_t(destination)<last&&first<uint64_t(destination)+count)copy->failed=true;return;}
  offset=copy->copied;
  if(!count||(count&1)||copy->rows>=480||offset>bg::Side*2||count>bg::Side*2-offset||source!=uint64_t(copy->source.w[8])+16+offset||destination!=first+offset){copy->failed=true;return;}
  try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(source,count);if(std::memcmp(nfs_host_address(source,count),copy->raw.data()+16+offset,count)){copy->failed=true;return;}unwind=nfs_logo_publisher::exceptions();active=true;}catch(...){copy->failed=true;}
 }
 ~CopyLeafScope()noexcept{ErrnoScope preserve;if(!active)return;
  if(copy->failed||nfs_logo_publisher::exceptions()>unwind){copy->failed=true;return;}
  try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(source,count);nfs_logo_publisher::checked(destination,count);if(std::memcmp(nfs_host_address(source,count),copy->raw.data()+16+offset,count)||std::memcmp(nfs_host_address(destination,count),copy->raw.data()+16+offset,count)){copy->failed=true;return;}copy->copied+=count;++copy->rows;}catch(...){copy->failed=true;}
 }
};
inline std::shared_ptr<const Proof>acquire(Game&g,uint32_t bitmap,const std::vector<uint8_t>&raw)noexcept{
 auto p=state().proof;if(!p||p->game!=reinterpret_cast<uintptr_t>(&g)||!current(*p)||p->target.w[8]!=bitmap||raw.size()!=16+bg::Side*2||std::memcmp(raw.data(),p->target.header.data(),16))return {};
 for(unsigned k=0;k<bg::Side;++k)if(bg::le16(raw.data()+16+2*k)!=p->pixels[k])return {};
 return p;
}
inline void retire(Game&g,uint32_t at)noexcept{
 auto&s=state();auto p=s.proof;if(p&&p->game==reinterpret_cast<uintptr_t>(&g)&&at&&(at==p->archive||at==p->target.owner||at==p->target.w[8]))s.clear();
 if(s.copying&&s.copying->proof&&s.copying->proof->game==reinterpret_cast<uintptr_t>(&g)&&(at==s.copying->proof->target.owner||at==s.copying->proof->target.w[8]))s.copying->failed=true;
}
}

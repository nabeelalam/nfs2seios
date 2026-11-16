// SPDX-License-Identifier: MIT
#pragma once
#include "ChildStaticSceneHistory.h"
#include "LocationInitialFullGameHooks.inc"
#include "OpponentInitialFullGameHooks.inc"
namespace nfs_child_static_scene {
// This is a distinct complete initial-full source, never an ordinary iterative
// menu Prepared manufactured from a collector. Its immutable views retain
// both the caption/sidebar proof and the preview/full-copy proof.
template<class Proof> struct InitialFullSnapshot {
 std::shared_ptr<const Proof>proof;
 std::shared_ptr<const bg::Prepared>full;
};
using LocationFullProof=nfs_location_initial_full_live::Proof;
using OpponentFullProof=nfs_opponent_initial_full_live::Proof;

inline bool fullOwned(Game&g,const LocationFullProof&p){return nfs_location_initial_full_live::owned(g,p);}
inline bool fullOwned(Game&g,const OpponentFullProof&p){return nfs_opponent_initial_full_live::owned(g,p);}
inline bool fullAcquired(Game&g,uint32_t source,const std::vector<uint8_t>&raw,const std::shared_ptr<const LocationFullProof>&p){
 return nfs_location_initial_full_live::acquire(g,source,raw)==p;
}
inline bool fullAcquired(Game&g,uint32_t source,const std::vector<uint8_t>&raw,const std::shared_ptr<const OpponentFullProof>&p){
 return nfs_opponent_initial_full_live::acquire(g,source,raw)==p;
}
inline const bg::Plane& historySidebar(const LocationFullProof&p){return p.child->post();}
inline const bg::Plane& historySidebar(const OpponentFullProof&p){return p.copiedSidebar;}

inline bool sidebarMatches(const LocationFullProof&p,const std::vector<uint8_t>&raw){
 return p.child->sourceMatches(historySidebar(p).target.w[8],raw.data(),raw.size());
}
inline bool sidebarMatches(const OpponentFullProof&p,const std::vector<uint8_t>&raw){
 const auto&side=historySidebar(p);
 if(!p.refresh||!bg::plane(side,343)||raw.size()!=16+bg::Side*2)return false;
 bg::Plane current{};
 if(!nfs_opponent_initial_full_live::captureRefreshSidebar(side.target.w[8],p.epoch,current)
     || !nfs_logo_capture::same(current.target,side.target)
     || !std::equal(current.pixels.begin(),current.pixels.begin()+bg::Side,side.pixels.begin()))
  return false;
 for(unsigned i=0;i<4;++i)if(bg::le32(raw.data()+4*i)!=side.target.header[i])return false;
 for(unsigned i=0;i<bg::Side;++i)if(bg::le16(raw.data()+16+2*i)!=side.pixels[i])return false;
 return true;
}
inline bool initialFullView(NfsChildExitPreludeView&v,const bg::Prepared&full,const LocationFullProof&p){
 const auto caption=p.child->view();
 const auto preview=p.previewView();
 if(!caption.schema||!caption.caption.schema||!preview.schema)return false;
 v.family=NFS_CHILD_EXIT_LOCATION;
 v.form=0x4d2a20;
 v.location.schema=NFS_LOCATION_SCENE_RUNTIME_SCHEMA;
 v.location.scene=full.view();
 v.location.scene.tailKind=NFS_BACKGROUND_TAIL_LOCATION_CAPTION;
 v.location.captionWord=caption.caption;
 v.location.preview=preview;
 return true;
}
inline bool initialFullView(NfsChildExitPreludeView&v,const bg::Prepared&full,const OpponentFullProof&p){
 const auto caption=p.child->view();
 const auto preview=p.previewView();
 if(!caption.schema||!caption.caption.font.schema||!preview.schema)return false;
 v.family=NFS_CHILD_EXIT_OPPONENT;
 v.form=0x4d3204;
 v.opponent.schema=NFS_OPPONENT_SCENE_RUNTIME_SCHEMA;
 v.opponent.scene=full.view();
 v.opponent.scene.tailKind=NFS_BACKGROUND_TAIL_OPPONENT_CAPTION;
 v.opponent.caption=caption.caption;
 v.opponent.preview=preview;
 v.opponent.previewSelection=p.preview->selection();
 if(p.art&&p.art->current()){
  v.opponent.previewArtGeneration=p.art->generation();
  v.opponent.previewArtIdentity=p.art->identity();
 }
 return true;
}
template<class Proof>
inline bool rememberInitialFull(Game&g,const std::shared_ptr<const Proof>&proof,
 const std::shared_ptr<const bg::SourceCatalog>&catalog,const std::shared_ptr<const nfs_main_sidebar_origin::GameLife>&gameLife,
 uint64_t content,uint32_t source,const std::vector<uint8_t>&fullRaw,const std::array<uint16_t,bg::Full>&pixels,const std::array<uint32_t,300>&keys)noexcept{
 unsigned stage=0;bool success=false;struct Trace{unsigned&stage;bool&success;~Trace(){diagnostic(success?7:16+stage,stage);}}trace{stage,success};
 const int saved=errno;struct Restore{int n;~Restore(){errno=n;}}restore{saved};try{
 stage=1;auto*s=active();const uintptr_t game=reinterpret_cast<uintptr_t>(&g),thread=nfs_logo_publisher::threadToken();const uint32_t form=proof&&proof->child?proof->child->binding().formGuest:0;
 if(!s||!s->token()||!matching(*s->token(),game,thread,form)||!proof||!catalog||!gameLife||!gameLife->current()||!content||nfs_background_scene_runtime::acquireCatalog()!=catalog||!fullOwned(g,*proof))return false;
 stage=2;const auto child=proof->child;const auto origin=child?child->origin():nullptr;const auto base=origin?origin->frame():nullptr;
 if(!child||!child->current()||!origin||!origin->current()||!base||!base->valid||!origin->catalog()||!nfs_main_sidebar_origin::sameCatalog(*origin->catalog(),*catalog)||proof->game!=game||proof->thread!=thread||!child->binding().epoch||!proof->epoch||proof->archive!=child->binding().archive||child->binding().formGuest!=form||!child->catalogMatches(*catalog)||gameLife!=nfs_main_sidebar_origin::acquireGameLife(game)||pixels!=proof->full.pixels||!fullAcquired(g,source,fullRaw,proof))return false;
 stage=3;const uint64_t event=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire);if(!event)return false;
 // Same verified child resource is read before/after exact current descriptors.
 // These keys only identify actual uploaded tiles: renderer pixels/pass proofs
 // remain mandatory and are not replaced by this CPU snapshot.
 const auto&side=historySidebar(*proof);
 stage=4;std::vector<uint8_t>raw,after;if(!nfs_location_scene_live::readBytes(side.target.w[8],16+bg::Side*2,raw)||!sidebarMatches(*proof,raw))return false;
 stage=5;{nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=proof->archive)return false;
  for(unsigned i=0;i<300;++i){const uint32_t d=tx.load<uint32_t>(dword_4EB74C+1200+i*4);std::array<uint32_t,7>w{};nfs_logo_publisher::descriptor(tx,d,w);
   if(!d||w[0]!=3||w[1]!=3||w[2]!=3||w[3]!=11||w[6]!=3||w[5]!=keys[i]||(w[5]&3)||w[5]>0x200000u-2048u)return false;
   for(unsigned j=0;j<i;++j)if(uint64_t(keys[j])<uint64_t(keys[i])+2048&&uint64_t(keys[i])<uint64_t(keys[j])+2048)return false;
  }
 }
 stage=6;if(!nfs_location_scene_live::readBytes(side.target.w[8],raw.size(),after)||after!=raw||!sidebarMatches(*proof,after)||!fullOwned(g,*proof)||event!=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire))return false;
 // Full-copy proof and 300 actual upload observations authenticate replacing the
 // base snapshot's native comparison plane/keys. Base word/logo borrows remain
 // owned by origin through the retained proof chain.
 stage=8;auto full=std::make_shared<bg::Prepared>(*base);full->original=pixels;full->keys=keys;full->serial=content;full->contentRevision=content;
 auto typed=std::make_shared<InitialFullSnapshot<Proof>>();typed->proof=proof;typed->full=full;
 auto record=std::make_shared<Record>();record->typed=typed;record->full=full;record->gameLife=gameLife;record->token=s->token();
 record->domain={game,thread,form,proof->epoch,proof->archive,event,origin->group().serial,origin->copy().serial,catalog};
 record->sidebarRaw=std::make_shared<const std::vector<uint8_t>>(std::move(raw));record->sidebarTarget=side.target;
 if(!nfs_texture_sha256(record->sidebarRaw->data(),record->sidebarRaw->size(),record->sidebarSha.data()))return false;
 auto&v=record->view;v.schema=1;v.ownerEpoch=proof->epoch;v.uploadEvent=event;v.sourceArchive=proof->archive;v.sourceGame=game;v.sourceCatalog=reinterpret_cast<uintptr_t>(catalog.get());v.sourceLife=gameLife->serial();
 if(!initialFullView(v,*full,*proof))return false;
 if(!matching(*s->token(),game,thread,form)||!fullOwned(g,*proof)||event!=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire))return false;s->store(record);success=true;return true;
 }catch(...){return false;}}
} // namespace nfs_child_static_scene

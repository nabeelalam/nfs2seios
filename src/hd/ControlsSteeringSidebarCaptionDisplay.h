// SPDX-License-Identifier: MIT
#pragma once
#include "ControlsSteeringSidebarCaptionProof.h"
#include "CompactCaptionWordArtwork.h"
#include "CompactCaptionWordRuntimeAbi.h"

namespace nfs_controls_steering_sidebar_caption {
// Immutable presentation binding supplied independently by the caller. This is
// historical identity, never permission to publish through a live source/GPU owner.
struct Binding {
 uint32_t form=0,handler=0;uintptr_t game=0;
 uint64_t ownerEpoch=0;uint32_t archive=0;uint64_t serial=0,lifetime=0;
};
class Display final {
 std::shared_ptr<const Proof>proof_;
 std::shared_ptr<const nfs_soft::FramePacket>packet_;
 std::shared_ptr<const nfs_compact_caption_display::Artwork>art_;
 Binding binding_{};
 Display()=default;
 static bool target(const Snapshot&s,const nfs_soft::SurfaceIdentity&identity)noexcept {
  const auto&t=s.target;const auto&w=t.w;
  if(!(s.identity==identity)||!identity.valid()||identity.geometry[1]!=343||
     !t.owner||w[1]!=343||w[2]!=480||w[3]||w[4]||w[5]!=343||w[6]!=480||
     w[7]!=0x7810||!w[8]||w[10]!=686||!w[11]||!w[12]||w[13]||
     t.header!=std::array<uint32_t,4>{{0x78,343u|(480u<<16),0,0}}||
     uint64_t(w[8])+16+343u*480u*2u>0x100000000ULL)return false;
  for(unsigned k=0;k<14;++k)if(identity.geometry[k]!=w[k])return false;
  for(unsigned k=0;k<343;++k)if(t.x[k]!=2*k)return false;
  for(unsigned k=0;k<480;++k)if(t.y[k]!=16+686*k)return false;
  for(unsigned k:{0u,1u,2u,7u,8u,10u,11u,12u,13u})if(t.ownerWords[k]!=w[k])return false;
  return true;
 }
 static bool fontCore(const Snapshot&a,const Snapshot&b)noexcept {
  return a.font==b.font&&a.callback==b.callback&&a.mode==b.mode&&a.flags==b.flags&&
   a.format==b.format&&a.originX==b.originX&&a.originY==b.originY&&a.size==b.size&&a.fontHeader==b.fontHeader;
 }
 static bool matches(const Proof&p,const std::shared_ptr<const nfs_soft::FramePacket>&packet,const Binding&b)noexcept {
  const auto&e=p.evidence();
  if(!b.form||b.handler!=0x4101c0u||!b.game||!b.ownerEpoch||!b.archive||!b.serial||!b.lifetime||
     !p.current()||!packet||p.packet()!=packet||e.form!=b.form||e.handler!=b.handler||e.game!=b.game||
     e.ownerEpoch!=b.ownerEpoch||e.serial!=b.serial||e.lifetime!=b.lifetime||!e.thread||!e.textGuest||
     e.incomingEbp!=b.form||e.authority.form!=b.form||e.authority.archive!=b.archive||!e.authority.table||
     e.authority.assignment||e.authority.parent.selectedIndex!=1||
     e.authority.sensitivity>3||
     packet->serial()!=b.serial||packet->surface().lifetime!=b.lifetime||packet->pixels()!=343u*480u||
     packet->commands()!=1||packet->records().size()!=1||!packet->writes()||packet->vertices().empty()||
     e.measureWidth!=48||e.measureAnchor!=168||e.measureKind!=2||
     !nfs_nested_sidebar_caption::exactCommand(e.command)||
     !nfs_nested_sidebar_caption::exactCommand(packet->records()[0].command))return false;
  for(const auto*s:{&e.pre,&e.measure,&e.before,&e.after,&e.post}){
   if(s->archive!=b.archive||s->selector!=6||!s->entry||s->entry!=e.pre.entry||
      s->entryWord!=e.pre.entryWord||(s->entryWord>>16)!=7||!target(*s,packet->surface())||
      !nfs_logo_capture::same(s->target,e.pre.target))return false;
  }
  // Original405D10 changes color/palette after measurement. Authenticate the
  // common font core there, then exact white entry/exit font/palette state.
  if(!e.before.font||!e.before.callback||!e.before.palette||e.before.mode||e.before.flags||
     e.before.format!=16||e.before.size!=18||e.before.originX||e.before.originY||
     e.before.color!=0xffff||e.after.color!=0xffff||!fontCore(e.before,e.after)||
     e.before.palette!=e.after.palette||e.before.paletteWords!=e.after.paletteWords||
     !fontCore(e.measure,e.before))return false;
  return packet->clean().size()==343u*480u&&packet->original().size()==343u*480u;
 }
public:
 static std::shared_ptr<const Display>create(std::shared_ptr<const Proof>proof,
   std::shared_ptr<const nfs_soft::FramePacket>packet,const Binding&binding)noexcept {
  if(!proof||!matches(*proof,packet,binding))return {};
  auto art=nfs_compact_caption_display::artwork(3);if(!art)return {};
  try{auto p=std::shared_ptr<Display>(new Display);p->proof_=std::move(proof);
   p->packet_=std::move(packet);p->art_=std::move(art);p->binding_=binding;return p;
  }catch(...){return {};}
 }
 const std::shared_ptr<const Proof>&proof()const noexcept{return proof_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 const Binding&binding()const noexcept{return binding_;}
 bool current()const noexcept{return proof_&&art_&&matches(*proof_,packet_,binding_);}
 NfsCompactCaptionWordView view()const noexcept {
  NfsCompactCaptionWordView v{};if(!current())return v;
  v.schema=NFS_COMPACT_CAPTION_WORD_SCHEMA;v.word=3;v.width=192;v.height=128;
  v.rgba=art_->rgba.data();v.bytes=art_->rgba.size();std::copy(art_->sha.begin(),art_->sha.end(),v.sha256);
  v.left=120;v.top=364;v.right=168;v.bottom=396;return v;
 }
};
}

// SPDX-License-Identifier: MIT
#pragma once
#include "NestedSidebarCaptionProof.h"
#include "CompactCaptionWordArtwork.h"
#include "CompactCaptionWordRuntimeAbi.h"
namespace nfs_nested_sidebar_caption {
class Display final {
 std::shared_ptr<const Proof>proof_;std::shared_ptr<const nfs_soft::FramePacket>packet_;
 std::shared_ptr<const nfs_compact_caption_display::Artwork>art_;Binding binding_{};Display()=default;
 static bool matches(const Proof&p,const std::shared_ptr<const nfs_soft::FramePacket>&packet,const Binding&b)noexcept{
  const auto&e=p.evidence();if((b.policy!=Policy::Audio&&b.policy!=Policy::Graphics)||!b.form||!b.archive||!b.game||!b.serial||!b.lifetime||!p.current()||!packet||p.packet()!=packet||packet->serial()!=b.serial||packet->surface().lifetime!=b.lifetime||packet->pixels()!=343*480||packet->commands()!=1||packet->records().size()!=1||!packet->writes()||packet->vertices().empty()||e.policy!=b.policy||e.form!=b.form||e.incomingEbp!=b.form||e.game!=b.game||!e.thread||e.serial!=b.serial||e.lifetime!=b.lifetime||e.measureWidth!=48||e.measureAnchor!=168||e.measureKind!=2||!exactCommand(e.command)||!exactCommand(packet->records()[0].command))return false;
  for(const auto*s:{&e.pre,&e.measure,&e.before,&e.after,&e.post})if(s->archive!=b.archive||s->selector!=6||!s->entry||s->entry!=e.pre.entry||s->entryWord!=e.pre.entryWord||(s->entryWord>>16)!=7||!(s->identity==packet->surface()))return false;return true;
 }
public:
 static std::shared_ptr<const Display>create(std::shared_ptr<const Proof>proof,std::shared_ptr<const nfs_soft::FramePacket>packet,const Binding&binding)noexcept{
  if(!proof||!matches(*proof,packet,binding))return {};auto art=nfs_compact_caption_display::artwork(3);if(!art)return {};try{auto out=std::shared_ptr<Display>(new Display);out->proof_=std::move(proof);out->packet_=std::move(packet);out->art_=std::move(art);out->binding_=binding;return out;}catch(...){return {};}
 }
 const std::shared_ptr<const Proof>&proof()const noexcept{return proof_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 bool current()const noexcept{return proof_&&art_&&matches(*proof_,packet_,binding_);}
 NfsCompactCaptionWordView view()const noexcept{
  NfsCompactCaptionWordView v{};if(!current())return v;v.schema=NFS_COMPACT_CAPTION_WORD_SCHEMA;v.word=3;v.width=192;v.height=128;v.rgba=art_->rgba.data();v.bytes=art_->rgba.size();std::copy(art_->sha.begin(),art_->sha.end(),v.sha256);v.left=120;v.top=364;v.right=168;v.bottom=396;return v;
 }
};
}

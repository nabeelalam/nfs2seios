// SPDX-License-Identifier: MIT
#pragma once
#include "LocationCaptionProof.h"
#include "CompactCaptionWordRuntimeAbi.h"
#include "CompactCaptionWordArtwork.h"
namespace nfs_location_caption_display {
struct Binding {
 uint32_t form=0,formGuest=0,archive=0;uintptr_t game=0;uint64_t serial=0,lifetime=0;
 bool empty()const noexcept{return !form&&!formGuest&&!archive&&!game&&!serial&&!lifetime;}
};
class Prepared final {
 std::shared_ptr<const nfs_location_caption_source::Proof>proof_;
 std::shared_ptr<const nfs_soft::FramePacket>packet_;
 std::shared_ptr<const nfs_compact_caption_display::Artwork>art_;Binding binding_{};
 Prepared()=default;
 static bool matches(const nfs_location_caption_source::Proof&p,const std::shared_ptr<const nfs_soft::FramePacket>&packet,const Binding&b)noexcept{
  const auto&e=p.evidence();
  if(b.form!=nfs_location_caption_source::LogicalForm||!b.formGuest||!b.archive||!b.game||!b.serial||!b.lifetime||!p.current()||!packet||p.packet()!=packet||packet->serial()!=b.serial||packet->surface().lifetime!=b.lifetime||packet->commands()!=1||packet->records().size()!=1||!packet->writes()||packet->pixels()!=343*480||packet->vertices().empty()||e.logicalForm!=b.form||e.formGuest!=b.formGuest||e.game!=b.game||e.serial!=b.serial||e.lifetime!=b.lifetime||!e.policyThread)return false;
  const auto&c=e.command;
  if(c.role!=1||c.sourceSize!=18||c.alignment||c.color!=0xffff||c.x!=116||c.y!=162||c.originX||c.originY||c.advance!=52||c.length!=8||std::memcmp(c.text.data(),"location",8)||c.clip.left||c.clip.top||c.clip.right!=343||c.clip.bottom!=480)return false;
  for(size_t i=8;i<c.text.size();++i)if(c.text[i])return false;
  for(const auto*s:{&e.pre,&e.stringBefore,&e.stringAfter,&e.post})if(s->archive!=b.archive||s->identity.lifetime!=b.lifetime||!(s->identity==packet->surface()))return false;
  return true;
 }
public:
 Prepared(const Prepared&)=delete;Prepared&operator=(const Prepared&)=delete;
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const nfs_location_caption_source::Proof>proof,std::shared_ptr<const nfs_soft::FramePacket>packet,const Binding&binding)noexcept{
  if(!proof||!matches(*proof,packet,binding))return {};auto image=nfs_compact_caption_display::artwork(2);if(!image)return {};
  try{std::shared_ptr<Prepared>out(new Prepared);out->proof_=std::move(proof);out->packet_=std::move(packet);out->art_=std::move(image);out->binding_=binding;return out;}catch(...){return {};}
 }
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 bool current()const noexcept{return proof_&&art_&&matches(*proof_,packet_,binding_);}
 NfsCompactCaptionWordView view()const noexcept{
  NfsCompactCaptionWordView v{};if(!current())return v;v.schema=NFS_COMPACT_CAPTION_WORD_SCHEMA;v.word=2;v.width=art_->width;v.height=art_->height;v.rgba=art_->rgba.data();v.bytes=art_->rgba.size();std::copy(art_->sha.begin(),art_->sha.end(),v.sha256);v.left=116;v.top=162;v.right=168;v.bottom=194;return v;
 }
};
} // namespace nfs_location_caption_display

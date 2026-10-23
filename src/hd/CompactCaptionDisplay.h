// SPDX-License-Identifier: MIT
#pragma once
#include "CompactCaptionProof.h"
#include "CompactCaptionWordRuntimeAbi.h"
#include "CompactCaptionWordArtwork.h"
#include <algorithm>
namespace nfs_compact_caption_display {
struct Binding {
 uint32_t form=0,formGuest=0,archive=0;uintptr_t game=0;uint64_t serial=0,lifetime=0;
 bool empty()const noexcept{return !form&&!formGuest&&!archive&&!game&&!serial&&!lifetime;}
};
class Prepared final {
 std::shared_ptr<const nfs_compact_caption::Proof>proof_;
 std::shared_ptr<const nfs_soft::FramePacket>packet_;
 std::shared_ptr<const Artwork>art_;Binding binding_{};
 Prepared()=default;
 static bool matches(const nfs_compact_caption::Proof&p,const std::shared_ptr<const nfs_soft::FramePacket>&packet,const Binding&b)noexcept{
  const bool setup=b.form==0x4d2790;
  if(!setup&&b.form!=0x4d3a88)return false;
  const unsigned selector=setup?0:6,resource=setup?0:7,length=setup?10:7;
  const int left=setup?93:120,y=setup?126:364,width=setup?75:48;
  const auto&e=p.evidence();
  if(!b.formGuest||!b.archive||!b.game||!b.serial||!b.lifetime||!p.current()||!packet||p.packet()!=packet||packet->serial()!=b.serial||packet->surface().lifetime!=b.lifetime||packet->commands()!=1||!packet->writes()||packet->pixels()!=343*480||e.policy!=(setup?nfs_compact_caption::Policy::Setup:nfs_compact_caption::Policy::Options)||e.route!=nfs_compact_caption::Route::OrdinaryEbp||e.formGuest!=b.formGuest||e.incomingEbp!=b.formGuest||e.game!=b.game||e.serial!=b.serial||e.lifetime!=b.lifetime||!e.policyThread||!e.resourceSerial||e.requestedResource!=resource||e.inputCategory!=1||e.inputAnchor!=168||e.inputY!=y||e.measureWidth!=width||e.measureAnchor!=168||e.measureKind!=2)return false;
  const auto&c=e.command;
  if(c.role!=1||c.sourceSize!=18||c.alignment!=2||c.color!=0xffff||c.x!=left||c.y!=y||c.originX||c.originY||c.advance!=width||c.length!=length||std::memcmp(c.text.data(),setup?"game setup":"options",length)||c.clip.left!=0||c.clip.top!=0||c.clip.right!=343||c.clip.bottom!=480)return false;
  for(size_t i=length;i<c.text.size();++i)if(c.text[i])return false;
  for(const auto*s:{&e.resolved,&e.measure,&e.stringBefore,&e.stringAfter})
   if(s->archive!=b.archive||s->selector!=selector||s->resource!=resource||!s->entry||s->entry!=e.resolved.entry||s->entryWord!=e.resolved.entryWord||(s->entryWord>>16)!=resource||s->identity.lifetime!=b.lifetime)return false;
  return e.stringAfter.identity==packet->surface()&&p.current();
 }
public:
 Prepared(const Prepared&)=delete;Prepared&operator=(const Prepared&)=delete;
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const nfs_compact_caption::Proof>proof,std::shared_ptr<const nfs_soft::FramePacket>packet,const Binding&binding)noexcept{
  if(!proof||!matches(*proof,packet,binding))return {};
  auto image=artwork(binding.form==0x4d2790?1:3);if(!image)return {};
  try{std::shared_ptr<Prepared>out(new Prepared);out->proof_=std::move(proof);out->packet_=std::move(packet);out->art_=std::move(image);out->binding_=binding;return out->current()?out:std::shared_ptr<const Prepared>{};}catch(...){return {};}
 }
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 bool current()const noexcept{return proof_&&art_&&matches(*proof_,packet_,binding_);}
 NfsCompactCaptionWordView view()const noexcept{
  NfsCompactCaptionWordView v{};if(!current())return v;
  const bool setup=binding_.form==0x4d2790;v.schema=NFS_COMPACT_CAPTION_WORD_SCHEMA;v.word=art_->word;v.width=art_->width;v.height=art_->height;v.rgba=art_->rgba.data();v.bytes=art_->rgba.size();std::copy(art_->sha.begin(),art_->sha.end(),v.sha256);v.left=setup?93:120;v.top=setup?126:364;v.right=168;v.bottom=v.top+32;return current()?v:NfsCompactCaptionWordView{};
 }
};
} // namespace nfs_compact_caption_display

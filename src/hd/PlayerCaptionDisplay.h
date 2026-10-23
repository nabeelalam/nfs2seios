// SPDX-License-Identifier: MIT
#pragma once
#include "CompactCaptionProof.h"
#include "PlayerCaptionRuntimeAbi.h"
#include "PlayerCaptionWordArtwork.h"
#include <algorithm>
namespace nfs_player_caption_display {
struct Binding {uint32_t form=0,formGuest=0,archive=0;uintptr_t game=0;uint64_t serial=0,lifetime=0;};
class Prepared final {
 std::shared_ptr<const nfs_compact_caption::Proof>proof_;std::shared_ptr<const nfs_soft::FramePacket>packet_;std::shared_ptr<const Artwork>art_;Binding binding_{};
 Prepared()=default;
 static bool matches(const nfs_compact_caption::Proof&p,const std::shared_ptr<const nfs_soft::FramePacket>&packet,const Binding&b)noexcept{
  const auto&e=p.evidence();const bool one=b.form==0x4d3050;const unsigned resource=one?2:3;
  // Archive is held by each independently resolved source snapshot, not Evidence.
  if((!one&&b.form!=0x4d3100)||!b.formGuest||!b.archive||!b.game||!b.serial||!b.lifetime||!p.current()||!packet||p.packet()!=packet||packet->serial()!=b.serial||packet->surface().lifetime!=b.lifetime||packet->commands()!=1||!packet->writes()||packet->pixels()!=343*480||e.policy!=(one?nfs_compact_caption::Policy::Player1:nfs_compact_caption::Policy::Player2)||e.formGuest!=b.formGuest||e.game!=b.game||e.serial!=b.serial||e.lifetime!=b.lifetime||!e.policyThread||!e.resourceSerial||e.requestedResource!=resource||e.inputCategory!=1||e.inputAnchor!=168||e.inputY!=(one?198:234)||e.measureWidth!=(one?68:70)||e.measureAnchor!=168||e.measureKind!=2)return false;
  const auto&c=e.command;if(c.role!=1||c.sourceSize!=18||c.alignment!=2||c.color!=0xffff||c.x!=(one?100:98)||c.y!=e.inputY||c.originX||c.originY||c.advance!=e.measureWidth||c.length!=10||memcmp(c.text.data(),one?"player one":"player two",10)||c.clip.left!=0||c.clip.top!=0||c.clip.right!=343||c.clip.bottom!=480)return false;
  for(size_t i=10;i<c.text.size();++i)if(c.text[i])return false;
  if(e.route!=nfs_compact_caption::Route::OrdinaryEbp&&e.route!=nfs_compact_caption::Route::InitialMain)return false;
  for(const auto*s:{&e.resolved,&e.measure,&e.stringBefore,&e.stringAfter})if(s->archive!=b.archive||s->selector!=resource||s->resource!=resource||s->identity.lifetime!=b.lifetime)return false;
  return e.stringAfter.identity==packet->surface()&&p.current();
 }
public:
 Prepared(const Prepared&)=delete;Prepared&operator=(const Prepared&)=delete;
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const nfs_compact_caption::Proof>proof,std::shared_ptr<const nfs_soft::FramePacket>packet,const Binding&binding)noexcept{
  if(!proof||!matches(*proof,packet,binding))return {};auto image=artwork(binding.form==0x4d3050?1:2);if(!image)return {};
  try{std::shared_ptr<Prepared>p(new Prepared);p->proof_=std::move(proof);p->packet_=std::move(packet);p->art_=std::move(image);p->binding_=binding;return p->current()?p:std::shared_ptr<const Prepared>{};}catch(...){return {};}
 }
 bool current()const noexcept{return proof_&&art_&&matches(*proof_,packet_,binding_);}
 NfsPlayerCaptionWordView view()const noexcept{
  NfsPlayerCaptionWordView v{};if(!current())return v;v.schema=NFS_PLAYER_CAPTION_WORD_SCHEMA;v.word=art_->word;v.width=art_->width;v.height=art_->height;v.rgba=art_->rgba.data();v.bytes=art_->rgba.size();std::copy(art_->sha.begin(),art_->sha.end(),v.sha256);v.left=v.word==1?100:98;v.top=v.word==1?198:234;v.right=168;v.bottom=v.top+32;return current()?v:NfsPlayerCaptionWordView{};
 }
};
} // namespace nfs_player_caption_display

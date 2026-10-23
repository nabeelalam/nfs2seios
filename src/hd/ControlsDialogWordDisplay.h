// SPDX-License-Identifier: MIT
#pragma once
#include "ControlsDialogWordProof.h"
#include "ControlsDialogWordRuntimeAbi.h"
#include "ControlsDialogWordArtwork.h"
namespace nfs_controls_dialog_display {
struct Binding {uint32_t form=0,archive=0;uintptr_t game=0;uint64_t serial=0,lifetime=0;};
class Prepared final {
 std::shared_ptr<const nfs_controls_dialog_word::Proof>proof_;
 std::shared_ptr<const nfs_soft::FramePacket>packet_;
 // Canonical image order is art ID1..5, independently of original draw order.
 std::array<std::shared_ptr<const Artwork>,nfs_controls_dialog_word::Words>art_{};
 Binding binding_{};Prepared()=default;
 static bool matches(const nfs_controls_dialog_word::Proof&p,const std::shared_ptr<const nfs_soft::FramePacket>&packet,const Binding&b)noexcept{const auto&e=p.evidence();return b.form&&b.archive&&b.game&&b.serial&&b.lifetime&&p.current()&&p.packet()==packet&&e.form==b.form&&e.authority.archive==b.archive&&e.game==b.game&&e.serial==b.serial&&e.lifetime==b.lifetime;}
public:
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const nfs_controls_dialog_word::Proof>proof,std::shared_ptr<const nfs_soft::FramePacket>packet,const Binding&b)noexcept{
  if(!proof||!matches(*proof,packet,b))return {};
  try{auto o=std::shared_ptr<Prepared>(new Prepared);
   for(unsigned i=0;i<nfs_controls_dialog_word::Words;++i){const auto id=nfs_controls_dialog_word::artId(i);if(!id||id>o->art_.size())return {};auto a=artwork(id);if(!a||a->word!=id||a->width!=4*nfs_controls_dialog_word::width(i)||a->height!=128||a->rgba.size()!=size_t(a->width)*a->height*4)return {};o->art_[id-1]=std::move(a);}
   o->proof_=std::move(proof);o->packet_=std::move(packet);o->binding_=b;return o;
  }catch(...){return {};}
 }
 const Binding&binding()const noexcept{return binding_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 const std::shared_ptr<const nfs_controls_dialog_word::Proof>&proof()const noexcept{return proof_;}
 // Historical immutable consistency only. Caller must independently acquire
 // current source authority before joining this retained display to a live scene.
 bool current()const noexcept{return proof_&&matches(*proof_,packet_,binding_);}
 NfsControlsDialogWordsView view()const noexcept{
  NfsControlsDialogWordsView v{};if(!current())return v;v.schema=NFS_CONTROLS_DIALOG_WORD_SCHEMA;v.count=NFS_CONTROLS_DIALOG_WORD_PLACEMENTS;
  for(unsigned i=0;i<nfs_controls_dialog_word::Words;++i){const auto&a=*art_[i];auto&image=v.images[i];image.word=i+1;image.width=a.width;image.height=a.height;image.rgba=a.rgba.data();image.bytes=a.rgba.size();std::copy(a.sha.begin(),a.sha.end(),image.sha256);}
  for(unsigned i=0;i<nfs_controls_dialog_word::Words;++i)for(unsigned j=0;j<2;++j){const auto&r=packet_->records()[nfs_controls_dialog_word::record(i)+j];auto&placement=v.placements[2*i+j];placement.word=nfs_controls_dialog_word::artId(i);placement.color565=r.command.color;placement.vertexFirst=r.vertexFirst;placement.vertexCount=r.vertexCount;placement.left=float(r.command.x);placement.top=float(r.command.y);placement.right=placement.left+nfs_controls_dialog_word::width(i);placement.bottom=placement.top+32;}
  return v;
 }
};
} // namespace nfs_controls_dialog_display

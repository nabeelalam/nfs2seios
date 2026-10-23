// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerInitialNativeUploadWitness.h"
#include "PlayerIntroRuntimeAbi.h"
namespace nfs_player_intro_cpu {
namespace native=nfs_player_initial_native;namespace bg=nfs_background_scene_cpu;
// Owned layout only. Creation consumes genuine initial/sidebar/native proofs;
// there is no Prepared.valid, synthetic Copy, or Player scene adoption API.
class Prepared final{
 std::shared_ptr<const native::Proof>proof_;std::shared_ptr<const native::Source>source_;
 std::shared_ptr<const bg::Prepared>main_;std::shared_ptr<const nfs_soft::FramePacket>caption_;
 std::shared_ptr<const native::Plane>stage_;std::shared_ptr<const bg::SourceCatalog>catalog_;
 native::Binding binding_{};std::array<uint32_t,300>keys_{};std::array<unsigned char,32>fontSha_{};uint64_t request_=0,event_=0;
 Prepared()=default;
public:
 Prepared(const Prepared&)=delete;Prepared&operator=(const Prepared&)=delete;
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const native::Proof>proof,const native::Binding&binding,const native::Plane&actual343,std::shared_ptr<const bg::SourceCatalog>catalog,uint64_t requestSerial)noexcept{
  if(!proof||!requestSerial||!catalog||!proof->current(binding,catalog))return {};auto source=proof->source(binding,catalog);auto stage=proof->stage(binding,catalog);const uint64_t event=proof->uploadEvent(binding,catalog);std::array<uint32_t,300>keys{};
  if(!source||!source->current()||!source->origin()||!source->origin()->current()||!source->origin()->frame()||!source->origin()->frame()->valid||!source->caption()||!source->caption()->font()||!source->caption()->commands()||source->caption()->pixels()!=bg::Side||!native::sourceMatches(*source,actual343)||!stage||!native::padded(*source,*stage)||!event||!proof->keys(binding,catalog,keys))return {};
  try{std::shared_ptr<Prepared>p(new Prepared);p->proof_=std::move(proof);p->source_=std::move(source);p->stage_=std::move(stage);p->main_=p->source_->origin()->frame();p->caption_=p->source_->caption();p->catalog_=std::move(catalog);p->binding_=binding;p->keys_=keys;p->request_=requestSerial;p->event_=event;const auto&font=*p->caption_->font();if(!nfs_texture_sha256(font.alpha().data(),font.alpha().size(),p->fontSha_.data())||!p->current())return {};return p;}catch(...){return {};}
 }
 bool current()const noexcept{return request_&&event_&&proof_&&source_&&source_->current()&&proof_->current(binding_,catalog_)&&proof_->uploadEvent(binding_,catalog_)==event_;}
 const std::shared_ptr<const native::Proof>&proof()const noexcept{return proof_;}const std::shared_ptr<const native::Source>&source()const noexcept{return source_;}const std::shared_ptr<const bg::SourceCatalog>&catalog()const noexcept{return catalog_;}const native::Binding&binding()const noexcept{return binding_;}uint64_t requestSerial()const noexcept{return request_;}uint64_t uploadEvent()const noexcept{return event_;}
 NfsPlayerIntroFrameView view()const noexcept{
  NfsPlayerIntroFrameView v{};if(!current())return v;v.schema=NFS_PLAYER_INTRO_RUNTIME_SCHEMA;v.form=binding_.sidebar.form;v.requestSerial=request_;v.uploadEvent=event_;v.sourceOwner=binding_.sourceOwner;v.sourceBitmap=binding_.sourceBitmap;v.inheritedMain=main_->view();
  const auto&font=*caption_->font();auto&text=v.caption;text.logicalWidth=343;text.logicalHeight=480;text.font.schema=1;text.font.width=font.width;text.font.height=font.height;text.font.identity=font.identity();text.font.alpha=font.alpha().data();text.font.bytes=font.alpha().size();std::copy(fontSha_.begin(),fontSha_.end(),text.font.sha256);text.vertices=caption_->vertices().data();text.vertexCount=caption_->vertices().size();v.paddedOriginal565=stage_->pixels.data();v.wordsCount=bg::Full;std::copy(keys_.begin(),keys_.end(),v.keys);return current()?v:NfsPlayerIntroFrameView{};
 }
};
} // namespace nfs_player_intro_cpu

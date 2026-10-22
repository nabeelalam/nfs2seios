// SPDX-License-Identifier: MIT
#pragma once
#include "CompactCaptionProof.h"
namespace nfs_player_initial_sidebar {
namespace bg=nfs_background_scene_cpu;
struct Binding {
 uint32_t form=0,formGuest=0,archive=0;uintptr_t game=0,thread=0;
 uint64_t epoch=0;bg::Event captionEvent{};
};
inline bool sameBinding(const Binding&a,const Binding&b)noexcept{return a.form==b.form&&a.formGuest==b.formGuest&&a.archive==b.archive&&a.game==b.game&&a.thread==b.thread&&a.epoch==b.epoch&&a.captionEvent.ownerEpoch==b.captionEvent.ownerEpoch&&a.captionEvent.sequence==b.captionEvent.sequence&&a.captionEvent.iteration==b.captionEvent.iteration;}
class Store;
// Historical Main ownership/events and current Player caption ownership remain
// distinct. This pending source proof grants no copy/upload/GPU authorization.
class Proof final {
 struct Life{std::atomic<bool>alive{true};};
 std::shared_ptr<Life>life_;std::shared_ptr<const nfs_main_sidebar_origin::Proof>origin_;
 std::shared_ptr<const nfs_compact_caption::Proof>caption_;Binding binding_;bg::Plane post_;
 Proof()=default;friend class Store;
public:
 Proof(const Proof&)=delete;Proof&operator=(const Proof&)=delete;
 const std::shared_ptr<const nfs_main_sidebar_origin::Proof>&origin()const noexcept{return origin_;}
 const std::shared_ptr<const nfs_compact_caption::Proof>&captionProof()const noexcept{return caption_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&caption()const noexcept{return caption_->packet();}
 const bg::Plane&post()const noexcept{return post_;}const Binding&binding()const noexcept{return binding_;}
 uint64_t captionSerial()const noexcept{return caption_->evidence().serial;}
 uint64_t captionLifetime()const noexcept{return caption_->evidence().lifetime;}
 bool current()const noexcept{return life_&&life_->alive.load(std::memory_order_acquire)&&origin_&&origin_->current()&&caption_&&caption_->current();}
 bool catalogMatches(const bg::SourceCatalog&catalog)const noexcept{return origin_&&origin_->catalog()&&nfs_main_sidebar_origin::sameCatalog(*origin_->catalog(),catalog);}
 bool catalogMatches(const std::shared_ptr<const bg::SourceCatalog>&catalog)const noexcept{return catalog&&catalogMatches(*catalog);}
};
class Store final {
 std::shared_ptr<const Proof>pending_;
 static bool planeMatches(const Proof&p,const bg::Plane&actual)noexcept{return bg::plane(actual,343)&&nfs_main_sidebar_origin::sameRetainedTarget(p.post().target,actual.target)&&std::equal(p.post().pixels.begin(),p.post().pixels.begin()+bg::Side,actual.pixels.begin());}
public:
 Store()=default;Store(const Store&)=delete;Store&operator=(const Store&)=delete;~Store(){clear();}
 void clear()noexcept{if(pending_&&pending_->life_)pending_->life_->alive.store(false,std::memory_order_release);pending_.reset();}
 void reset()noexcept{clear();}
 // Current here means unrevoked historical source. Caller must acquire with
 // independently captured live Binding/343 before reuse, and reset on Player
 // owner exit/retirement. Numeric form/epoch alone never seals a source.
 std::shared_ptr<const Proof>current()const noexcept{return pending_&&pending_->current()?pending_:std::shared_ptr<const Proof>{};}
 std::shared_ptr<const Proof>accept(std::shared_ptr<const nfs_main_sidebar_origin::Proof>origin,std::shared_ptr<const nfs_compact_caption::Proof>caption,const Binding&binding,const bg::Plane&actual,bg::Workspace&workspace,std::shared_ptr<const bg::SourceCatalog>catalog)noexcept{
  clear();if(!origin||!origin->current()||!origin->frame()||!origin->catalog()||!caption||!caption->current()||caption->origin()!=origin||!catalog||!bg::validCatalog(*catalog)||!nfs_main_sidebar_origin::sameCatalog(*origin->catalog(),*catalog)||!bg::plane(actual,343))return {};
  const auto&e=caption->evidence();const auto&packet=caption->packet();
  const bool player1=e.policy==nfs_compact_caption::Policy::Player1;
  if((!player1&&e.policy!=nfs_compact_caption::Policy::Player2)||binding.form!=(player1?0x4d3050u:0x4d3100u)||!binding.formGuest||binding.formGuest!=e.formGuest||!binding.game||binding.game!=origin->game()||binding.game!=e.game||!binding.thread||binding.thread!=origin->publisherThread()||!binding.archive||binding.archive!=origin->archive()||!binding.epoch||binding.captionEvent.ownerEpoch!=binding.epoch||!binding.captionEvent.sequence||binding.captionEvent.iteration||e.route!=nfs_compact_caption::Route::InitialMain||!e.policyThread||!e.lifetime||!e.serial||!e.resourceSerial||!packet||packet->serial()!=e.serial||packet->surface().lifetime!=e.lifetime||packet->pixels()!=bg::Side||packet->commands()!=1||!packet->font())return {};
  if(e.incomingEbp!=origin->formGuest()&&e.incomingEbp!=UINT32_MAX&&e.incomingEbp!=UINT32_MAX-1)return {};
  // Recheck exact immutable source metadata; policy-local TLS is deliberately
  // not equated to the independently supplied publisher-thread occurrence.
  if(e.requestedResource!=(player1?2u:3u)||e.inputCategory!=1||e.inputAnchor!=168||e.inputY!=(player1?198:234)||e.measureWidth!=(player1?68:70)||e.measureAnchor!=168||e.measureKind!=2||e.command.role!=1||e.command.sourceSize!=18||e.command.alignment!=2||e.command.color!=0xffff||e.command.x!=(player1?100:98)||e.command.y!=e.inputY||e.command.advance!=e.measureWidth||e.command.length!=10||std::memcmp(e.command.text.data(),player1?"player one":"player two",10))return {};
  const auto&original=origin->post();if(!bg::plane(original,343)||!nfs_main_sidebar_origin::sameRetainedTarget(original.target,actual.target)||!packet->surface().valid()||packet->clean().size()!=bg::Side||packet->original().size()!=bg::Side)return {};
  for(const auto*s:{&e.resolved,&e.measure,&e.stringBefore,&e.stringAfter})if(s->archive!=binding.archive||!nfs_main_sidebar_origin::sameRetainedTarget(s->target,actual.target)||s->identity.lifetime!=e.lifetime||s->selector!=(player1?2u:3u)||s->resource!=(player1?2u:3u))return {};
  if(!(e.stringAfter.identity==packet->surface()))return {};for(unsigned k=0;k<14;++k)if(packet->surface().geometry[k]!=actual.target.w[k])return {};
  if(nfs_small_logo_live::verifyGroup(origin->group(),workspace.logo,catalog->logos)!=nfs_small_logo_live::Failure::None)return {};
  for(unsigned k=0;k<bg::Side;++k)if(packet->clean()[k]!=original.pixels[k]||packet->original()[k]!=actual.pixels[k]||(workspace.logo.groupUnion[k]&&packet->original()[k]!=packet->clean()[k]))return {};
  if(!origin->current()||!caption->current())return {};
  try{std::shared_ptr<Proof>p(new Proof);p->life_=std::make_shared<Proof::Life>();p->origin_=std::move(origin);p->caption_=std::move(caption);p->binding_=binding;p->post_=actual;if(!p->current())return {};pending_=p;return p;}catch(...){return {};}
 }
 std::shared_ptr<const Proof>acquire(const Binding&binding,const bg::Plane&actual,const std::shared_ptr<const bg::SourceCatalog>&catalog)noexcept{
  if(!pending_||!pending_->current()||!sameBinding(pending_->binding(),binding)||!pending_->catalogMatches(catalog)||!planeMatches(*pending_,actual)||!pending_->current()){clear();return {};}return pending_;
 }
};
} // namespace nfs_player_initial_sidebar

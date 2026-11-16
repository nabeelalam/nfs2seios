// SPDX-License-Identifier: MIT
#pragma once
#include "Main343OriginProof.h"
#include "LocationCaptionDisplay.h"
#include "LocationInitialTransitionAbi.h"
#include "BackgroundSceneLayersAbi.h"
namespace nfs_location_initial_sidebar {
namespace bg=nfs_background_scene_cpu;
constexpr uint32_t Form=0x4d2a20u;
struct Binding {
 uint32_t form=0,formGuest=0,archive=0,incomingEbp=0;uintptr_t game=0,thread=0;
 uint64_t epoch=0,captionSerial=0,captionLifetime=0;
};
inline bool sameBinding(const Binding&a,const Binding&b)noexcept{return a.form==b.form&&a.formGuest==b.formGuest&&a.archive==b.archive&&a.incomingEbp==b.incomingEbp&&a.game==b.game&&a.thread==b.thread&&a.epoch==b.epoch&&a.captionSerial==b.captionSerial&&a.captionLifetime==b.captionLifetime;}
class Store;
// Historical Main source and lexical Location caption remain distinct owners.
// This seal grants neither native copy/upload nor full Location preview authority.
class Proof final {
 struct Life{std::atomic<bool>alive{true};};
 std::shared_ptr<Life>life_;std::shared_ptr<const nfs_main_sidebar_origin::Proof>origin_;
 std::shared_ptr<const nfs_location_caption_source::Proof>caption_;
 std::shared_ptr<const nfs_location_caption_display::Prepared>display_;
 std::shared_ptr<const bg::SourceCatalog>infoReturnCatalog_;
 std::array<NfsSmallLogoLayer,3>infoReturnLogos_{};
 Binding binding_;bg::Plane post_;Proof()=default;friend class Store;
public:
 Proof(const Proof&)=delete;Proof&operator=(const Proof&)=delete;
 bool current()const noexcept{return life_&&life_->alive.load(std::memory_order_acquire)&&((origin_&&origin_->current())||infoReturnCatalog_)&&caption_&&caption_->current()&&display_&&display_->current();}
 const std::shared_ptr<const nfs_main_sidebar_origin::Proof>&origin()const noexcept{return origin_;}
 const std::shared_ptr<const nfs_location_caption_source::Proof>&captionProof()const noexcept{return caption_;}
 const bg::Plane&post()const noexcept{return post_;}const Binding&binding()const noexcept{return binding_;}
 NfsLocationInitialTransitionView view()const noexcept{NfsLocationInitialTransitionView v{};if(!current())return v;v.schema=NFS_LOCATION_INITIAL_TRANSITION_SCHEMA;v.form=Form;v.ownerEpoch=binding_.epoch;v.captionSerial=binding_.captionSerial;v.captionLifetime=binding_.captionLifetime;v.caption=display_->view();return v.caption.schema?v:NfsLocationInitialTransitionView{};}
 bool sourceMatches(uint32_t source,const uint8_t*raw,size_t bytes)const noexcept{
  if(!current()||!raw||source!=post_.target.w[8]||bytes!=16+bg::Side*2||std::memcmp(raw,post_.target.header.data(),16))return false;
  for(unsigned k=0;k<bg::Side;++k)if(bg::le16(raw+16+2*k)!=post_.pixels[k])return false;return true;
 }
 bool catalogMatches(const bg::SourceCatalog&c)const noexcept{const auto catalog=origin_?origin_->catalog():infoReturnCatalog_;return catalog&&nfs_main_sidebar_origin::sameCatalog(*catalog,c);}
 bool locationInfoReturn()const noexcept{return bool(infoReturnCatalog_);}
 bool layers(NfsBackgroundSceneLayers&out)const noexcept{
  if(!current())return false;
  out={};out.panelClip[2]=343;out.panelClip[3]=480;
  if(infoReturnCatalog_){
   std::copy(infoReturnLogos_.begin(),infoReturnLogos_.end(),out.logos);
   return true;
  }
  const auto&frame=origin_->frame();
  if(!frame||!frame->valid)return false;
  out.mapRole=frame->mapRole;out.wordCount=frame->wordCount;
  std::copy(frame->words.begin(),frame->words.end(),out.words);
  std::copy(frame->logos.begin(),frame->logos.end(),out.logos);
  return true;
 }
};
class Store final {
 std::shared_ptr<const Proof>pending_;
public:
 Store()=default;Store(const Store&)=delete;Store&operator=(const Store&)=delete;~Store(){clear();}
 void clear()noexcept{if(pending_&&pending_->life_)pending_->life_->alive.store(false,std::memory_order_release);pending_.reset();}
 std::shared_ptr<const Proof>current()const noexcept{return pending_&&pending_->current()?pending_:std::shared_ptr<const Proof>{};}
 std::shared_ptr<const Proof>accept(std::shared_ptr<const nfs_main_sidebar_origin::Proof>origin,std::shared_ptr<const nfs_location_caption_source::Proof>caption,const Binding&b,const bg::Plane&actual,bg::Workspace&workspace,std::shared_ptr<const bg::SourceCatalog>catalog)noexcept{
  clear();if(!origin||!origin->current()||!origin->frame()||!origin->frame()->valid||!origin->catalog()||!caption||!caption->current()||!catalog||!bg::validCatalog(*catalog)||!nfs_main_sidebar_origin::sameCatalog(*origin->catalog(),*catalog)||!bg::plane(actual,343))return {};
  const auto&e=caption->evidence();const auto&packet=caption->packet();
  if(b.form!=Form||!b.formGuest||e.formGuest!=b.formGuest||e.logicalForm!=nfs_location_caption_source::LogicalForm||!b.archive||b.archive!=origin->archive()||!b.game||b.game!=origin->game()||b.game!=e.game||!b.thread||b.thread!=origin->publisherThread()||!b.epoch||!b.captionSerial||!b.captionLifetime||b.captionSerial!=e.serial||b.captionLifetime!=e.lifetime||!e.policyThread||!packet||packet->serial()!=e.serial||packet->surface().lifetime!=e.lifetime||packet->clean().size()!=bg::Side||packet->original().size()!=bg::Side)return {};
  if(b.incomingEbp!=origin->formGuest()&&b.incomingEbp!=0&&b.incomingEbp!=UINT32_MAX&&b.incomingEbp!=UINT32_MAX-1)return {};
  const auto&pre=origin->post();if(!bg::plane(pre,343)||!nfs_main_sidebar_origin::sameRetainedTarget(pre.target,actual.target))return {};
  for(const auto*s:{&e.pre,&e.stringBefore,&e.stringAfter,&e.post})if(s->archive!=b.archive||s->identity.lifetime!=e.lifetime||!nfs_main_sidebar_origin::sameRetainedTarget(s->target,actual.target))return {};
  if(nfs_small_logo_live::verifyGroup(origin->group(),workspace.logo,catalog->logos)!=nfs_small_logo_live::Failure::None)return {};
  for(unsigned k=0;k<bg::Side;++k)if(packet->clean()[k]!=pre.pixels[k]||packet->original()[k]!=actual.pixels[k]||(workspace.logo.groupUnion[k]&&packet->original()[k]!=packet->clean()[k]))return {};
  nfs_location_caption_display::Binding displayBinding{nfs_location_caption_source::LogicalForm,b.formGuest,b.archive,b.game,b.captionSerial,b.captionLifetime};
  auto display=nfs_location_caption_display::Prepared::create(caption,packet,displayBinding);if(!display||!origin->current()||!caption->current())return {};
  try{std::shared_ptr<Proof>p(new Proof);p->life_=std::make_shared<Proof::Life>();p->origin_=std::move(origin);p->caption_=std::move(caption);p->display_=std::move(display);p->binding_=b;p->post_=actual;if(!p->current())return {};pending_=p;return p;}catch(...){return {};}
 }
 // The scoped information-page return draws the authored Location panel and live caption.
 // Main's word and logo metadata has a separate lifetime.
 bool acceptInfoReturn(const std::shared_ptr<const nfs_location_caption_source::Proof>&caption,
  const Binding&b,const bg::Plane&before,const bg::Plane&after,
  std::shared_ptr<const bg::SourceCatalog>catalog,unsigned logoPhase)noexcept{
  clear();
  if(logoPhase>=320||!caption||!caption->current()||!catalog||!bg::validCatalog(*catalog)
      ||!bg::plane(before,343)||!bg::plane(after,343)
      ||!nfs_logo_capture::same(before.target,after.target))return false;
  const auto&packet=caption->packet();
  if(!packet||packet->clean().size()!=bg::Side||packet->original().size()!=bg::Side
      ||!std::equal(packet->clean().begin(),packet->clean().end(),before.pixels.begin())
      ||!std::equal(packet->original().begin(),packet->original().end(),after.pixels.begin()))return false;
  const auto&e=caption->evidence();
  if(b.form!=Form||!b.epoch||!b.thread||!b.incomingEbp
      ||!nfs_logo_capture::same(e.pre.target,before.target)||!nfs_logo_capture::same(e.post.target,after.target))return false;
  nfs_location_caption_display::Binding binding{
   nfs_location_caption_source::LogicalForm,b.formGuest,b.archive,b.game,b.captionSerial,b.captionLifetime};
  auto display=nfs_location_caption_display::Prepared::create(caption,packet,binding);
  if(!display)return false;
  try{
   std::shared_ptr<Proof>p(new Proof);
   p->life_=std::make_shared<Proof::Life>();
   // Mirror the original four-colour logo cycle in 40BBE0.
   const unsigned first=logoPhase/80;
   const unsigned roles[]={first+1,(first+1)%4+1,0};
   const unsigned scalars[]={256,logoPhase%80*255/80,256};
   for(unsigned i=0;i<3;++i){
    const auto&asset=catalog->logos.identities[roles[i]];
    auto&layer=p->infoReturnLogos_[i];
    layer.role=roles[i];layer.scalar256=scalars[i];
    layer.left=float(nfs_small_logo_live::signed12(asset.header[3]));
    layer.top=float(nfs_small_logo_live::signed12(asset.header[3]>>16));
    layer.right=layer.left+asset.width;layer.bottom=layer.top+asset.height;
    layer.clip[2]=343;layer.clip[3]=480;
   }
   p->infoReturnCatalog_=std::move(catalog);
   p->caption_=caption;
   p->display_=std::move(display);
   p->binding_=b;p->post_=after;
   if(!p->current())return false;
   pending_=std::move(p);
   return true;
  }catch(...){return false;}
 }
 std::shared_ptr<const Proof>acquire(const Binding&b,const bg::Plane&actual,const std::shared_ptr<const bg::SourceCatalog>&catalog)noexcept{
  if(!pending_||!pending_->current()||!sameBinding(pending_->binding(),b)||!catalog||!pending_->catalogMatches(*catalog)||!bg::plane(actual,343)||!nfs_main_sidebar_origin::sameRetainedTarget(pending_->post().target,actual.target)||!std::equal(pending_->post().pixels.begin(),pending_->post().pixels.begin()+bg::Side,actual.pixels.begin())){clear();return {};}return pending_;
 }
};
}

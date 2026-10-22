// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundScenePrepare.h"
#include "Main343OriginLifetime.h"
#include <atomic>
#include <memory>
namespace nfs_main_sidebar_origin {
namespace bg=nfs_background_scene_cpu;
struct Binding{uintptr_t game=0,publisherThread=0;uint32_t archive=0,formGuest=0;uint64_t epoch=0;};
inline bool sameCatalog(const bg::SourceCatalog&a,const bg::SourceCatalog&b)noexcept{
 if(a.roles!=b.roles||a.mapSha!=b.mapSha||a.mapRoles!=b.mapRoles||a.logos.roles!=b.logos.roles)return false;
 for(unsigned k=0;k<13;++k){const auto&x=a.identities[k];const auto&y=b.identities[k];if(x.span!=y.span||x.width!=y.width||x.height!=y.height||x.paletteOffset!=y.paletteOffset||x.header!=y.header||x.sha!=y.sha)return false;}
 for(unsigned k=0;k<5;++k){const auto&x=a.logos.identities[k];const auto&y=b.logos.identities[k];if(x.span!=y.span||x.width!=y.width||x.height!=y.height||x.paletteOffset!=y.paletteOffset||x.paletteCount!=y.paletteCount||x.maxIndex!=y.maxIndex||x.header!=y.header||x.sha!=y.sha)return false;}return true;
}
// Only the ambient selected-double-buffer word may differ on a later read.
// Actual owner, all descriptor words, header, full X/Y LUTs and other handles
// remain independently exact. This does not relabel any historical record.
inline bool sameRetainedTarget(const bg::Target&a,const bg::Target&b)noexcept{
 if(a.w!=b.w||a.header!=b.header||a.x!=b.x||a.y!=b.y||a.owner!=b.owner||a.ownerWords!=b.ownerWords)return false;
 for(unsigned k=1;k<5;++k)if(a.ownerState[k]!=b.ownerState[k])return false;return true;
}
class Store;
class Proof final{
 struct Lifetime{std::atomic<bool>alive{true};};
 std::shared_ptr<Lifetime>life_;std::shared_ptr<const GameLife>gameLife_;Binding binding_;bg::Plane post_;bg::Base base_;bg::Scene scene_;nfs_logo_capture::Group group_;nfs_logo_capture::Copy copy_;
 bg::Event groupEvent_{},tailEvent_{},copyEvent_{},uploadEvent_{};std::shared_ptr<const bg::Prepared>frame_;std::shared_ptr<const bg::SourceCatalog>catalog_;
 Proof()=default;friend class Store;
public:
 Proof(const Proof&)=delete;Proof&operator=(const Proof&)=delete;
 bool current()const noexcept{return life_&&life_->alive.load(std::memory_order_acquire)&&gameLife_&&gameLife_->current();}
 const bg::Plane&post()const noexcept{return post_;}const bg::Base&base()const noexcept{return base_;}const bg::Scene&scene()const noexcept{return scene_;}const nfs_logo_capture::Group&group()const noexcept{return group_;}const nfs_logo_capture::Copy&copy()const noexcept{return copy_;}
 const std::shared_ptr<const bg::Prepared>&frame()const noexcept{return frame_;}const std::shared_ptr<const bg::SourceCatalog>&catalog()const noexcept{return catalog_;}
 uint32_t archive()const noexcept{return binding_.archive;}uint32_t formGuest()const noexcept{return binding_.formGuest;}uintptr_t game()const noexcept{return binding_.game;}uintptr_t publisherThread()const noexcept{return binding_.publisherThread;}uint64_t epoch()const noexcept{return binding_.epoch;}
 const bg::Event&groupEvent()const noexcept{return groupEvent_;}const bg::Event&tailEvent()const noexcept{return tailEvent_;}const bg::Event&copyEvent()const noexcept{return copyEvent_;}const bg::Event&uploadEvent()const noexcept{return uploadEvent_;}
};
// Caller supplies independently observed live Main identity, synchronous owner
// events and a trusted immutable catalog. No Prepared.valid adoption exists.
// Normal Main exit intentionally does not clear this bounded history; actual
// owner/bitmap retirement, archive/game/thread/catalog changes must revoke it.
class Store final{
 mutable std::array<std::shared_ptr<const Proof>,2>entries_{};
 static void revoke(std::shared_ptr<const Proof>&p)noexcept{if(p&&p->life_)p->life_->alive.store(false,std::memory_order_release);p.reset();}
 bool context(uintptr_t game,uintptr_t thread,uint32_t archive,const std::shared_ptr<const bg::SourceCatalog>&catalog)const noexcept{
  for(const auto&p:entries_)if(p&&(p->game()!=game||p->publisherThread()!=thread||p->archive()!=archive||!catalog||!sameCatalog(*p->catalog(),*catalog))){clear();return false;}return true;
 }
 void alias(uint32_t owner,uint32_t bitmap)const noexcept{for(auto&p:entries_)if(p&&(p->post().target.owner==owner||p->post().target.w[8]==bitmap))revoke(p);}
public:
 const std::array<std::shared_ptr<const Proof>,2>&entries()const noexcept{return entries_;}
 Store()=default;Store(const Store&)=delete;Store&operator=(const Store&)=delete;~Store(){clear();}
 void clear()const noexcept{for(auto&p:entries_)revoke(p);}void reset()noexcept{clear();}
 void retireOwner(uint32_t owner)noexcept{if(owner)for(auto&p:entries_)if(p&&p->post().target.owner==owner)revoke(p);}
 void retireBitmap(uint32_t bitmap)noexcept{if(bitmap)for(auto&p:entries_)if(p&&p->post().target.w[8]==bitmap)revoke(p);}
 std::shared_ptr<const Proof>publish(const bg::JoinedInput&i,const Binding&binding,bg::Workspace&workspace,std::shared_ptr<const bg::SourceCatalog>catalog)noexcept{
  context(binding.game,binding.publisherThread,binding.archive,catalog);alias(i.copy.sourceOwner.owner,i.copy.sourceOwner.w[8]);
  if(!binding.game||!binding.publisherThread||!binding.archive||!binding.formGuest||!binding.epoch||!catalog||i.base.form!=bg::MainForm||i.scene.form!=bg::MainForm||i.base.event.ownerEpoch!=binding.epoch||i.scene.event.ownerEpoch!=binding.epoch||i.logos.thread!=binding.publisherThread)return {};
  try{auto frame=std::make_shared<bg::Prepared>();if(bg::verifyAndPrepare(i,workspace,*frame,*catalog)!=bg::Failure::None)return {};
   if(!bg::plane(i.scene.tailPost,343)||!sameRetainedTarget(i.scene.tailPost.target,i.copy.sourceOwner))return {};
   for(unsigned k=0;k<bg::Side;++k)if(i.scene.tailPost.pixels[k]!=bg::le16(i.copy.sourceBefore.data()+16+2*k)||i.scene.tailPost.pixels[k]!=bg::le16(i.copy.sourceAfter.data()+16+2*k))return {};
   std::shared_ptr<Proof>p(new Proof);p->life_=std::make_shared<Proof::Lifetime>();p->gameLife_=acquireGameLife(binding.game);if(!p->gameLife_||!p->gameLife_->current())return {};p->binding_=binding;p->post_=i.scene.tailPost;p->base_=i.base;p->scene_=i.scene;p->group_=i.logos;p->copy_=i.copy;p->groupEvent_=i.groupEvent;p->tailEvent_=i.tailEvent;p->copyEvent_=i.copyEvent;p->uploadEvent_=i.uploadEvent;p->frame_=std::move(frame);p->catalog_=std::make_shared<const bg::SourceCatalog>(*catalog);
   // Preserve the untouched double buffer. A third distinct source evicts only
   // the oldest retained entry; every external handle sees explicit revocation.
   if(!entries_[0])entries_[0]=p;else if(!entries_[1])entries_[1]=p;else{revoke(entries_[0]);entries_[0]=std::move(entries_[1]);entries_[1]=p;}return p;
  }catch(...){return {};}
 }
 std::shared_ptr<const Proof>acquire(const bg::Plane&actual,uintptr_t game,uintptr_t thread,uint32_t archive,const std::shared_ptr<const bg::SourceCatalog>&catalog)const noexcept{
  if(!context(game,thread,archive,catalog)||!game||!thread||!archive||!catalog)return {};
  for(auto&p:entries_)if(p&&(p->post().target.owner==actual.target.owner||p->post().target.w[8]==actual.target.w[8])){
   if(!p->current()||!bg::plane(actual,343)||!sameRetainedTarget(p->post().target,actual.target)||!std::equal(p->post().pixels.begin(),p->post().pixels.begin()+bg::Side,actual.pixels.begin())){alias(actual.target.owner,actual.target.w[8]);return {};}return p;
  }return {};
 }
};
}

// Dedicated immutable car-information display lease. Live source construction stays in Adapter.
#pragma once
#include "ResultsTextDisplay.h"
#include "ResultsFont.h"
#include "CarInfoLayerAbi.h"
#include "CarInfoArtProvider.h"
#include "CarInfoSourceCatalog.h"
#include "hd/OriginalUiRecords.h"
#include <atomic>
namespace nfs_car_info_ui_live {
struct Adapter;
struct StaticSource {uint32_t index=0,address=0;std::array<int32_t,4>clip{};};
struct StaticProof {nfs_logo_capture::Target target{};std::array<StaticSource,2>sources{};std::vector<uint16_t>plane;uint32_t archive=0,archiveIndex=0,locale=0;uint64_t epoch=0,serial=0;unsigned completed=0,expected=0,mode=0;std::vector<unsigned char>overlayBytes;NfsCarInfoArtImage originalOverlay{};};
struct StaticArt {
 NfsCarInfoArtView view{};std::shared_ptr<const StaticProof>proofOwner;
 ~StaticArt(){if(view.owner&&view.release)view.release(view.owner);}
 static std::shared_ptr<const StaticArt>acquire(std::shared_ptr<const StaticProof>p){try{
  if(!p||!p->expected||p->expected>2||p->completed!=p->expected)return {};
  const uint32_t ids[2]={p->sources[0].index,p->expected==2?p->sources[1].index:NFS_CAR_INFO_NO_SOURCE};
  auto art=std::make_shared<StaticArt>();if(!nfs_car_info_art_acquire(ids,&art->view))return {};
  if(art->view.count==1&&p->expected==2){if(p->overlayBytes.empty()||p->originalOverlay.sourceIndex!=ids[1]||p->originalOverlay.pixels!=p->overlayBytes.data())return {};art->proofOwner=p;art->view.images[1]=p->originalOverlay;art->view.count=2;}
  if(art->view.count!=p->expected)return {};return art;
 }catch(...){return {};}}
};
struct Payload {
 nfs_logo_capture::Target target{};
 std::vector<uint16_t>clean,original;
 std::vector<nfs_soft::TextCommand>commands;
 std::shared_ptr<const StaticProof>statics;std::shared_ptr<const StaticArt>art;
 uint32_t form=0,carIndex=UINT32_MAX,page=0;uint16_t formType=0;
 uint64_t epoch=0,serial=0;
};
class Frame final {
 friend struct Adapter;std::shared_ptr<const Payload>data_;std::shared_ptr<const nfs_results_text::PreparedText>display_;
 Frame()=default;
public:
 const Payload&data()const noexcept{return *data_;}
 const std::shared_ptr<const nfs_results_text::PreparedText>&display()const noexcept{return display_;}
};
class Lease final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const Frame>frame_;Lease()=default;
 static void retain(void*p){static_cast<Lease*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p){auto*s=static_cast<Lease*>(p);if(s->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
public:
 static std::shared_ptr<const Lease>create(std::shared_ptr<const Frame>f)noexcept{if(!f||!f->display())return {};try{auto p=std::shared_ptr<Lease>(new Lease,[](const Lease*x){release(const_cast<Lease*>(x));});p->frame_=std::move(f);return p;}catch(...){return {};}}
 NfsCarInfoLiveView view(uint64_t request)const noexcept{NfsCarInfoLiveView v{};if(!request||!frame_)return v;const auto&d=frame_->data();v.schema=1;v.formType=d.formType;v.carIndex=d.carIndex;v.page=d.page;v.width=640;v.height=480;v.request=request;v.content=d.serial;v.clean=d.clean.data();v.original=d.original.data();v.pixels=d.original.size();v.text=frame_->display()->layer();if(d.art)v.art=d.art->view;v.owner=const_cast<Lease*>(this);v.retain=retain;v.release=release;return v;}
};
}

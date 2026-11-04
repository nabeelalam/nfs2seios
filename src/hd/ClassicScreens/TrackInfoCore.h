// Dedicated immutable track-information display lease. Live source construction stays in Adapter.
#pragma once
#include "ResultsTextDisplay.h"
#include "ResultsFont.h"
#include "TrackInfoLayerAbi.h"
#include "TrackInfoArtProvider.h"
#include "TrackInfoSourceCatalog.h"
#include "hd/OriginalUiRecords.h"
#include <atomic>
namespace nfs_track_info_ui_live {
struct Adapter;
struct StaticSource {uint32_t index=0,address=0;std::array<int32_t,4>clip{};};
struct StaticProof {nfs_logo_capture::Target target{};std::array<StaticSource,3>sources{};std::vector<uint16_t>plane;uint32_t archive=0,archiveIndex=0,locale=0;uint64_t epoch=0,serial=0;unsigned completed=0;};
struct StaticArt {NfsTrackInfoArtView view{};~StaticArt(){if(view.owner&&view.release)view.release(view.owner);}static std::shared_ptr<const StaticArt>acquire(const StaticProof&p){try{uint32_t ids[3];for(unsigned k=0;k<3;++k)ids[k]=p.sources[k].index;auto art=std::make_shared<StaticArt>();if(!nfs_track_info_art_acquire(ids,&art->view))return {};return art;}catch(...){return {};}}};
struct Payload {
 nfs_logo_capture::Target target{};
 std::vector<uint16_t>clean,original;
 std::vector<nfs_soft::TextCommand>commands;
 std::shared_ptr<const StaticProof>statics;std::shared_ptr<const StaticArt>art;
 uint32_t form=0;uint16_t formType=0;
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
 NfsTrackInfoLiveView view(uint64_t request)const noexcept{NfsTrackInfoLiveView v{};if(!request||!frame_)return v;const auto&d=frame_->data();v.schema=3;v.formType=d.formType;v.width=640;v.height=480;v.request=request;v.content=d.serial;v.clean=d.clean.data();v.original=d.original.data();v.pixels=d.original.size();v.text=frame_->display()->layer();if(d.art)v.art=d.art->view;v.owner=const_cast<Lease*>(this);v.retain=retain;v.release=release;return v;}
};
}

// PRIVATE immutable Results display lease. Live source construction stays in Adapter.
#pragma once
#include "ResultsTextDisplay.h"
#include "ResultsFont.h"
#include "ResultsLayerAbi.h"
#include "hd/OriginalUiRecords.h"
#include <atomic>
namespace nfs_results_ui_live {
struct Adapter;
struct Payload {
 nfs_logo_capture::Target target{};
 std::vector<uint16_t>clean,original;
 std::vector<nfs_soft::TextCommand>commands;
 uint32_t form=0;uint16_t formType=0;unsigned headingIndex=0;
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
 NfsResultsLiveView view(uint64_t request)const noexcept{NfsResultsLiveView v{};if(!request||!frame_)return v;const auto&d=frame_->data();v.schema=2;v.headingIndex=d.headingIndex;v.width=640;v.height=480;v.request=request;v.content=d.serial;v.clean=d.clean.data();v.original=d.original.data();v.pixels=d.original.size();v.text=frame_->display()->layer();v.owner=const_cast<Lease*>(this);v.retain=retain;v.release=release;return v;}
};
}

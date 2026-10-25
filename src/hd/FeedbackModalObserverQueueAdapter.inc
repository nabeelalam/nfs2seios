// SPDX-License-Identifier: MIT
#pragma once
#include "FeedbackModalWordDisplay.h"
#include "FeedbackModalNativeObserverAbi.h"
#include "BackgroundSceneQueueAdapter.inc"
#include <atomic>
#include <stdexcept>
// Observe-only source queue. No artwork staging, suppression or publication.
namespace nfs_feedback_modal_observer_queue {
using Words=nfs_feedback_modal_word_display::Prepared;
using Joined=nfs_feedback_modal_scene_live::Joined;
class Lease final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const Words>words_;
 explicit Lease(std::shared_ptr<const Words>w):words_(std::move(w)){}
 ~Lease()=default;
 static void retain(void*p)noexcept{static_cast<Lease*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p)noexcept{auto*s=static_cast<Lease*>(p);if(s->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
public:
 static std::shared_ptr<const Lease>create(std::shared_ptr<const Words>w)noexcept{
  if(!w||!w->scene()||!w->historicalConsistent()||w->scene()->original().size()!=307200)return {};
  try{auto*p=new Lease(std::move(w));return std::shared_ptr<const Lease>(p,[p](const Lease*){release(p);});}catch(...){return {};}
 }
 NfsFeedbackModalNativeView view(uint64_t serial)const noexcept{
  NfsFeedbackModalNativeView v{};if(!serial||!words_||!words_->historicalConsistent())return v;
  const auto&s=words_->scene();v.schema=1;v.width=640;v.height=480;v.serial=serial;v.modalEpoch=s->invocation()->modalEpoch();
  v.final565=s->original().data();v.planeWords=s->original().size();v.lifetimeOwner=const_cast<Lease*>(this);v.retainOwner=retain;v.releaseOwner=release;return v;
 }
};
struct Intent {std::shared_ptr<const Lease>lease;uint64_t serial=0,epoch=0;bool consumed=false,finished=false;};
inline void begin(Game&g,const std::shared_ptr<const Joined>&joined,Intent&i){
 if(!nfs_background_scene_live::enabled()||nfs_background_scene_live::activeRequest()||i.serial||i.consumed||!joined||!joined->current(g))return;
 auto words=Words::create(joined);auto lease=Lease::create(words);if(!lease||!joined->current(g))return;
 const uint64_t serial=nfs_background_scene_live::nextRequest();if(!serial)return;
 const uint64_t epoch=joined->invocation()->modalEpoch();i.lease=lease;i.serial=serial;i.epoch=epoch;
 nfs::check_worker_stop();const auto acceptance=nfs::enqueue_owned_renderer([lease,serial](){auto v=lease->view(serial);return int32_t(nfs_private_feedback_modal_observer_begin(&v));});
 if(acceptance==nfs::OwnedRendererAcceptance::UseExistingOwnerPath){i.lease.reset();i.serial=i.epoch=0;return;}
 i.consumed=true;nfs_background_scene_live::activeRequest()=serial;nfs::check_worker_stop();
}
inline void finish(Intent&i){
 if(!i.consumed||i.finished)return;if(!i.serial||nfs_background_scene_live::activeRequest()!=i.serial)throw std::logic_error("Feedback observer queue owner changed");
 const auto lease=i.lease;const auto serial=i.serial,epoch=i.epoch;nfs::check_worker_stop();
 const auto acceptance=nfs::enqueue_owned_renderer([lease,serial,epoch](){
  (void)lease;(void)nfs_private_feedback_modal_observer_finish(serial,epoch);NfsFeedbackModalNativeReceipt r{},again{};const int taken=nfs_private_feedback_modal_observer_take_receipt(serial,epoch,&r);
  const int duplicate=nfs_private_feedback_modal_observer_take_receipt(serial,epoch,&again);
  std::fprintf(stderr,"FEEDBACK MODAL QUEUE receipt=%u admitted=%u batches=%u duplicate=%u context=%llu\n",unsigned(taken),r.admitted,r.batches,unsigned(duplicate),(unsigned long long)r.contextGeneration);
  if(!taken)nfs_private_feedback_modal_observer_cancel(serial,epoch);return int32_t(taken&&r.admitted&&!duplicate);
 });
 if(acceptance!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("Feedback observer finish changed renderer owner");
 i.finished=true;nfs_background_scene_live::activeRequest()=0;i.lease.reset();nfs::check_worker_stop();
}
inline void cancel(Intent&i)noexcept{
 if(i.consumed&&!i.finished&&i.serial){const auto lease=i.lease;const auto serial=i.serial,epoch=i.epoch;
  try{(void)nfs::enqueue_owned_renderer([lease,serial,epoch](){(void)lease;nfs_private_feedback_modal_observer_cancel(serial,epoch);return int32_t(0);});}catch(...){}
 }
 if(nfs_background_scene_live::activeRequest()==i.serial)nfs_background_scene_live::activeRequest()=0;i.lease.reset();i.finished=true;
}
struct Context {Game*game=nullptr;std::shared_ptr<const Joined>joined;Intent*intent=nullptr;};
inline Context&context()noexcept{static thread_local Context c;return c;}
class Scope final {
 Intent*intent_=nullptr;
public:
 Scope(Game&g,std::shared_ptr<const Joined>joined,Intent&i)noexcept{
  if(context().game||!joined||!joined->current(g))return;context()={&g,std::move(joined),&i};intent_=&i;
 }
 ~Scope(){if(intent_&&context().intent==intent_){cancel(*intent_);context()={};}}
};
class NativeScope final {
 Intent*intent_=nullptr;int unwind_=0;
public:
 explicit NativeScope(Game&g){const auto&c=context();if(c.game!=&g||!c.joined||!c.intent||c.intent->consumed||
   uint32_t(g.eax)!=c.joined->invocation()->binding().surface.geometry[8])return;
  intent_=c.intent;unwind_=nfs_logo_publisher::exceptions();begin(g,c.joined,*intent_);
 }
 ~NativeScope()noexcept(false){if(!intent_)return;if(nfs_logo_publisher::exceptions()>unwind_){cancel(*intent_);return;}finish(*intent_);}
};
} // namespace nfs_feedback_modal_observer_queue

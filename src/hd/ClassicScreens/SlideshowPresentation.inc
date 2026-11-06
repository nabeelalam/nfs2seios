// SPDX-License-Identifier: MIT
#pragma once
#include "SlideshowGameBridge.inc"
#include "SlideshowLayerAbi.h"
#include <atomic>
namespace nfs_slideshow_live {
class Lease final {
 std::atomic<unsigned>refs_{1};
 std::shared_ptr<const nfs_slideshow::ImageFrame>image_;
 std::shared_ptr<const nfs_slideshow_text::Payload>controls_;
 std::shared_ptr<const nfs_results_text::PreparedText>display_;
 std::array<unsigned char,32>sha_{};
 nfs_logo_capture::Target target_{};
 Lease()=default;
 static void retain(void*p){static_cast<Lease*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p){auto*x=static_cast<Lease*>(p);if(x->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete x;}
public:
 static std::shared_ptr<const Lease>acquire(Game&g)noexcept {
  try {
   auto&b=bridge();auto image=presentationFrame();if(b.game!=&g||!image)return {};
   auto&s=nfs_slideshow_text::state();
   if(s.failed||s.capturing||s.string||s.word||!s.published||!s.display||
      s.published->epoch!=image->epoch||
      !nfs_slideshow::sourceSurfaceMatches(*image,s.published->baseline))return {};
   // Other screens and gameplay return above without reading the guest target.
   const auto currentTarget=nfs_slideshow_text::target();
   if(nfs_slideshow_text::plane(currentTarget)!=s.published->original)return {};
   {nfs::GuestCopyTransaction tx;if(!exactForm(tx,b.form)||tx.load<uint32_t>(dword_4D40B8)!=b.archive)return {};}
   auto p=std::shared_ptr<Lease>(new Lease,[](const Lease*x){release(const_cast<Lease*>(x));});
   p->image_=image;p->controls_=s.published;p->display_=s.display;p->target_=currentTarget;
   // A backbuffer may have a different address. Admission requires its entire
   // original plane to match the captured controls, then retains this exact
   // current target for the paired request; no historic address grants access.
   p->sha_=image->rgbaSha; // Already validated once when the selected image was decoded.
   return p;
  }catch(...){return {};}
 }
 bool current(Game&g)const noexcept {
  try {auto&b=bridge();auto&s=nfs_slideshow_text::state();
   if(b.game!=&g||presentationFrame()!=image_||s.published!=controls_||s.display!=display_||
      !nfs_logo_capture::same(target_,nfs_slideshow_text::target())||
      nfs_slideshow_text::plane(target_)!=controls_->original)return false;
   nfs::GuestCopyTransaction tx;return exactForm(tx,b.form)&&tx.load<uint32_t>(dword_4D40B8)==b.archive;
  }catch(...){return false;}
 }
 NfsSlideshowLiveView view(uint64_t request)const noexcept {
  NfsSlideshowLiveView v{};if(!request||!image_||!controls_||!display_)return v;
  v.schema=2;v.width=640;v.height=480;v.imageWidth=image_->width;v.imageHeight=image_->height;
  v.request=request;v.content=image_->epoch;v.original=controls_->original.data();v.pixels=controls_->original.size();
  v.rgba=image_->rgba.data();v.rgbaBytes=image_->rgba.size();std::copy(sha_.begin(),sha_.end(),v.imageSha);
  if(image_->footer){const auto&f=*image_->footer;v.footerWidth=f.width();v.footerHeight=f.height();v.footer=f.rgba().data();v.footerBytes=f.rgba().size();std::copy(f.sha().begin(),f.sha().end(),v.footerSha);}
  v.text=display_->layer();v.owner=const_cast<Lease*>(this);v.retain=retain;v.release=release;return v;
 }
};
}

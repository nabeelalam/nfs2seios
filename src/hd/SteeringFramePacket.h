// SPDX-License-Identifier: MIT
#pragma once
#include "GraphicsFramePacket.h"
#include "SteeringChartPacket.h"

// Immutable presentation join only. The caller supplies independent current
// observations; this grants no guest, form, parent, source-hash or native authority.
namespace nfs_steering_frame {
namespace af=nfs_audio_frame;
class SteeringFramePacket final {
 af::Binding binding_{};
 uint64_t publication_=0;uint32_t handler_=0;
 std::shared_ptr<const nfs_graphics_frame::VerifiedBackground>background_;
 std::shared_ptr<const nfs_steering_chart::Packet>chart_;
 SteeringFramePacket()=default;
 static bool compatible(const af::Binding&current,uint32_t handler,const nfs_steering_chart::Binding&b,uint64_t serial)noexcept {
  return handler&&b.handler==handler&&b.valid()&&b.form==current.form&&b.archive==current.archive&&
   b.lifetime==current.lifetime&&b.ownerEpoch==current.ownerEpoch&&
   b.textSerial==serial&&b.targetBitmap==current.surface.geometry[8];
 }
 static bool storesValid(const nfs_steering_chart::Packet&p)noexcept {
  if(p.source().size()!=nfs_steering_chart::Builder::sourceSize||p.rows()!=49||
     p.original().size()!=nfs_graphics_frame::Pixels)return false;
  // Count source-selected actual stores, including equal-value/opaque-black
  // writes. Transparent entries require no store; do not assume a fixed count.
  unsigned stores=0;const auto&s=p.source();
  for(size_t i=16;i<17460;++i)if(s[17476+size_t(s[i])*4+3]>=16)++stores;
  return p.stores()==stores;
 }
public:
 static std::shared_ptr<const SteeringFramePacket>create(
   const std::shared_ptr<const nfs_graphics_frame::VerifiedBackground>&background,
   const std::shared_ptr<const nfs_steering_chart::Packet>&chart,
   const af::Binding&current,uint32_t currentHandler,uint64_t acceptedTextSerial,
   const uint16_t*currentPost,size_t count)noexcept {
  if(!current.valid()||!background||!chart||!(background->binding()==current)||
     !acceptedTextSerial||!currentPost||count!=nfs_graphics_frame::Pixels||
     !compatible(current,currentHandler,chart->binding(),acceptedTextSerial)||!storesValid(*chart))return {};
  const auto&text=chart->text();
  if(!text||!text->font()||!text->commands()||!text->writes()||text->vertices().empty()||
     text->pixels()!=nfs_graphics_frame::Pixels||!(text->surface()==current.surface)||
     text->serial()!=acceptedTextSerial||text->clean()!=background->pixels()||
     !chart->matches(chart->binding(),currentPost,count))return {};
  // The chart has changed the plane since text publication. Intentionally never
  // call text->matches(...currentPost...) here: only the final chart owns it.
  try{auto p=std::shared_ptr<SteeringFramePacket>(new SteeringFramePacket);
   p->binding_=current;p->handler_=currentHandler;p->publication_=acceptedTextSerial;p->background_=background;p->chart_=chart;return p;
  }catch(...){return {};}
 }
 const af::Binding&binding()const noexcept{return binding_;}
 uint64_t publication()const noexcept{return publication_;}
 const std::shared_ptr<const nfs_graphics_frame::VerifiedBackground>&background()const noexcept{return background_;}
 const std::shared_ptr<const nfs_steering_chart::Packet>&chart()const noexcept{return chart_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&text()const noexcept{return chart_->text();}
 const std::vector<uint16_t>&original()const noexcept{return chart_->original();}
 bool matches(const af::Binding&current,uint32_t currentHandler,uint64_t serial,const uint16_t*post,size_t count)const noexcept {
  return current.valid()&&current==binding_&&currentHandler==handler_&&serial==publication_&&
   compatible(current,currentHandler,chart_->binding(),serial)&&chart_->matches(chart_->binding(),post,count);
 }
};
}

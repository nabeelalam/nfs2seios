// SPDX-License-Identifier: MIT
#pragma once
#include "CreditsSceneSource.h"
#include "hd/SoftwareFramePacket.h"
#include <atomic>
#include <memory>
#include <cmath>
#include <vector>
namespace nfs_credits_ui {
struct PartnerPlacement {int32_t x=0,y=0;std::array<float,4>clip{{0,0,640,480}};};
struct FrameEvent {enum Kind:uint8_t {Text=1,Partners=2};Kind kind=Text;uint32_t index=0;};
class FrameAssembly;
// Complete original ordering, independently of the legacy renderer's fragment
// snapshots. This is source content, not cached-native-pass/GL/swap authority.
class WholeFrame final {
 friend class FrameAssembly;
 std::shared_ptr<const SceneSource>scene_;std::shared_ptr<const std::atomic<uint64_t>>live_;uint64_t serial_=0;
 std::vector<nfs_soft::TextCommand>commands_;std::vector<uint32_t>colors_;
 std::vector<FrameEvent>events_;PartnerPlacement partner_{};bool hasPartner_=false;
 WholeFrame()=default;
public:
 bool current()const noexcept{return scene_&&scene_->current()&&live_&&live_->load(std::memory_order_acquire)==serial_;}
 uint64_t serial()const noexcept{return serial_;}
 const std::shared_ptr<const SceneSource>&scene()const noexcept{return scene_;}
 const std::vector<nfs_soft::TextCommand>&commands()const noexcept{return commands_;}
 const std::vector<uint32_t>&colors()const noexcept{return colors_;}
 const std::vector<FrameEvent>&events()const noexcept{return events_;}
 const PartnerPlacement*partner()const noexcept{return hasPartner_?&partner_:nullptr;}
};
class FrameAssembly final {
 std::shared_ptr<WholeFrame>building_;std::shared_ptr<std::atomic<uint64_t>>live_;uint64_t serial_=0;bool partnerOpen_=false;
 static bool clip(const std::array<float,4>&c)noexcept{for(float v:c)if(!std::isfinite(v))return false;return c[0]>=0&&c[1]>=0&&c[2]<=640&&c[3]<=480&&c[0]<=c[2]&&c[1]<=c[3];}
public:
 FrameAssembly()=default;FrameAssembly(const FrameAssembly&)=delete;FrameAssembly&operator=(const FrameAssembly&)=delete;
 ~FrameAssembly(){abort();}
 void abort()noexcept{if(live_)live_->store(0,std::memory_order_release);building_.reset();partnerOpen_=false;}
 bool active()const noexcept{return building_&&building_->current();}
 // Called only after the genuine base-cache draw completed successfully.
 bool begin(std::shared_ptr<const SceneSource>scene)noexcept{
  abort();if(!scene||!scene->current()||serial_==UINT64_MAX)return false;
  try{if(!live_)live_=std::make_shared<std::atomic<uint64_t>>(0);auto p=std::shared_ptr<WholeFrame>(new WholeFrame);p->scene_=std::move(scene);p->serial_=++serial_;p->live_=live_;live_->store(p->serial_,std::memory_order_release);p->commands_.reserve(32);p->colors_.reserve(32);p->events_.reserve(33);building_=std::move(p);return true;}catch(...){abort();return false;}
 }
 bool append(const nfs_soft::TextCommand&command,uint32_t argb)noexcept{
  if(!active()||partnerOpen_||building_->commands_.size()>=128){abort();return false;}
  if(!command.length||command.length>95||command.text[command.length]||command.advance<=0||command.advance>4096||command.role>1||command.alignment||!clip({{command.clip.left,command.clip.top,command.clip.right,command.clip.bottom}})||(command.sourceSize!=16&&command.sourceSize!=18&&command.sourceSize!=24&&command.sourceSize!=36)){abort();return false;}
  for(unsigned i=0;i<command.length;++i){const unsigned ch=command.text[i];if(ch<32||(ch>126&&ch!=0x99&&ch!=0xa9&&ch!=0xe0)||ch==96||ch==123||ch==124){abort();return false;}}
  try{const auto i=uint32_t(building_->commands_.size());building_->commands_.push_back(command);building_->colors_.push_back(argb|0xff000000u);building_->events_.push_back({FrameEvent::Text,i});return true;}catch(...){abort();return false;}
 }
 bool beginPartner(const PartnerPlacement&p)noexcept{
  if(!active()||partnerOpen_||building_->hasPartner_||p.x||p.y<= -100||p.y>=450||!clip(p.clip)){abort();return false;}
  building_->partner_=p;partnerOpen_=true;return true;
 }
 // Called after, not before, the original translated cache draw returns.
 bool finishPartner()noexcept{
  if(!active()||!partnerOpen_){abort();return false;}
  try{building_->events_.push_back({FrameEvent::Partners,0});building_->hasPartner_=true;partnerOpen_=false;return true;}catch(...){abort();return false;}
 }
 // A qualified no-text frame is legitimate. An incomplete partner draw is not.
 std::shared_ptr<const WholeFrame>seal()noexcept{
  if(!active()||partnerOpen_){abort();return {};}
  std::shared_ptr<const WholeFrame>out=std::move(building_);partnerOpen_=false;return out;
 }
};
}

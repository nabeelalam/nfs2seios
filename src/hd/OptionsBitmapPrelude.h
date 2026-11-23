// SPDX-License-Identifier: MIT
#pragma once
#include "OptionsSidebarOrigin.h"
#include "OptionsBitmapPreludeAbi.h"
#include <atomic>
namespace nfs_options_bitmap_cpu {
namespace bg=nfs_background_scene_cpu;
struct Binding {
 uintptr_t game=0,thread=0;uint64_t epoch=0,baseSequence=0,sequence=0;
 uint32_t form=0,previous=0,family=0,archive=0,mapRole=0,phase=0;
 bool valid()const noexcept{return game&&thread&&epoch&&baseSequence&&sequence&&form&&previous&&form!=previous&&family>=1&&family<=5&&archive&&mapRole<2&&phase>=1001&&phase<=1551&&(phase-1001)%50==0;}
};
inline bool copied(const bg::Plane&base,const bg::Plane&after,const std::vector<uint8_t>&side)noexcept{
 if(!bg::plane(base,640)||!bg::plane(after,640)||!nfs_logo_capture::same(base.target,after.target)||side.size()!=16+bg::Side*2||bg::le32(side.data())!=0x78||bg::le32(side.data()+4)!=(343u|(480u<<16))||bg::le32(side.data()+8)||bg::le32(side.data()+12))return false;
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){const auto foreground=x<343?bg::le16(side.data()+16+2*(y*343+x)):0;if(after.pixels[y*640+x]!=(foreground?foreground:base.pixels[y*640+x]))return false;}return true;
}
class Prepared final {
 Binding binding_;bg::Plane full_;std::shared_ptr<const nfs_options_sidebar_origin::Origin>side_;
 Prepared()=default;
public:
 static std::shared_ptr<const Prepared>create(const Binding&b,const bg::Plane&base,const bg::Plane&after,const std::shared_ptr<const nfs_options_sidebar_origin::Origin>&side,const std::vector<uint8_t>&before,const std::vector<uint8_t>&repeat,unsigned originalCalls)noexcept{
  if(!b.valid()||originalCalls!=1||!side||!side->matches(b.game,b.thread,b.archive,before.data(),before.size())||before!=repeat||!side->frame||!side->frame->wordView().schema||!copied(base,after,before))return {};
  try{auto p=std::shared_ptr<Prepared>(new Prepared);p->binding_=b;p->full_=after;p->side_=side;return p;}catch(...){return {};}
 }
 const Binding&binding()const noexcept{return binding_;}const bg::Plane&full()const noexcept{return full_;}
 const auto&side()const noexcept{return side_;}
 NfsOptionsBitmapPreludeView view(uint64_t serial)const noexcept{
  NfsOptionsBitmapPreludeView v{};if(!serial||!side_||!side_->current())return v;v.schema=1;v.phase=binding_.phase;v.serial=serial;v.content=binding_.sequence;v.planeWords=bg::Full;v.original565=full_.pixels.data();v.background.mapRole=binding_.mapRole;v.background.wordCount=side_->frame->wordCount;v.background.panelClip[2]=343;v.background.panelClip[3]=480;std::copy(side_->frame->words.begin(),side_->frame->words.end(),v.background.words);std::copy(side_->frame->logos.begin(),side_->frame->logos.end(),v.background.logos);v.caption=side_->frame->wordView();return v;
 }
};
class Lease final {
 std::atomic<unsigned>refs_{1};std::atomic<bool>allowed_{true};std::shared_ptr<const Prepared>source_;
 std::shared_ptr<const std::atomic<bool>>ownerLife_,frameLife_;
 Lease(std::shared_ptr<const Prepared>p,std::shared_ptr<const std::atomic<bool>>o,std::shared_ptr<const std::atomic<bool>>f):source_(std::move(p)),ownerLife_(std::move(o)),frameLife_(std::move(f)){}
 static void retain(void*p)noexcept{static_cast<Lease*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p)noexcept{auto*s=static_cast<Lease*>(p);if(s->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
 static int current(void*p)noexcept{auto*s=static_cast<Lease*>(p);return s&&s->allowed_.load(std::memory_order_acquire)&&s->ownerLife_&&s->ownerLife_->load(std::memory_order_acquire)&&s->frameLife_&&s->frameLife_->load(std::memory_order_acquire)&&s->source_&&s->source_->side()->current();}
public:
 static std::shared_ptr<Lease>create(std::shared_ptr<const Prepared>p,std::shared_ptr<const std::atomic<bool>>o,std::shared_ptr<const std::atomic<bool>>f)noexcept{if(!p||!o||!f)return {};try{return std::shared_ptr<Lease>(new Lease(std::move(p),std::move(o),std::move(f)),[](Lease*p){release(p);});}catch(...){return {};}}
 void cancel()noexcept{allowed_.store(false,std::memory_order_release);}
 NfsOptionsBitmapPreludeView view(uint64_t serial)noexcept{auto v=source_->view(serial);if(v.schema&&current(this)){v.owner=this;v.retain=retain;v.release=release;v.current=current;}else v={};return v;}
};
}

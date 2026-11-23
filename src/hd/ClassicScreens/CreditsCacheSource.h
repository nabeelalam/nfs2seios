// SPDX-License-Identifier: MIT
#pragma once
#include "CreditsCacheAbi.h"
#include "CreditsSceneSource.h"
#include <vector>
#include <algorithm>
namespace nfs_credits_ui {
// Original draw result captured immediately after the named bitmap primitive.
// Renderer receipts and the actual original cached pass are required separately.
class CacheSource final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const SourceLease>source_;
 std::shared_ptr<const std::vector<uint16_t>>pixels_;std::shared_ptr<const std::atomic<uint64_t>>live_;
 uintptr_t game_=0;uint32_t archive_=0,slot_=0,locale_=0;uint64_t serial_=0;
 std::array<uint32_t,300>descriptors_{},addresses_{};std::array<StaticDraw,2>sprites_{};
 CacheSource()=default;
 static void retain(void*p)noexcept{static_cast<CacheSource*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p)noexcept{auto*s=static_cast<CacheSource*>(p);if(s->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
 static int valid(const void*p)noexcept{const auto*s=static_cast<const CacheSource*>(p);return s&&s->source_&&s->source_->current(s->game_,s->archive_)&&s->live_&&s->live_->load(std::memory_order_acquire)==s->serial_;}
public:
 static std::shared_ptr<const CacheSource>create(std::shared_ptr<const SourceLease>source,uintptr_t game,uint32_t archive,unsigned slot,unsigned locale,uint64_t serial,std::shared_ptr<const std::atomic<uint64_t>>live,std::shared_ptr<const std::vector<uint16_t>>pixels,const std::array<uint32_t,300>&descriptors,const std::array<uint32_t,300>&addresses,const std::array<StaticDraw,4>&draws)noexcept{
  if(draws[0].sourceIndex!=5||draws[1].sourceIndex!=8)return {};
  if(!source||!source->current(game,archive)||slot>1||locale>=6||!serial||!live||live->load()!=serial||!pixels||pixels->size()!=640*480)return {};
  for(unsigned i=0;i<300;++i){if(!descriptors[i]||(descriptors[i]&3)||(addresses[i]&3)||addresses[i]>0x200000u-2048)return {};for(unsigned j=0;j<i;++j)if(descriptors[i]==descriptors[j]||(uint64_t(addresses[i])<uint64_t(addresses[j])+2048&&uint64_t(addresses[j])<uint64_t(addresses[i])+2048))return {};}
  try{auto p=std::shared_ptr<CacheSource>(new CacheSource,[](const CacheSource*x){release(const_cast<CacheSource*>(x));});p->source_=std::move(source);p->game_=game;p->archive_=archive;p->slot_=slot;p->locale_=locale;p->serial_=serial;p->live_=std::move(live);p->pixels_=std::move(pixels);p->descriptors_=descriptors;p->addresses_=addresses;p->sprites_[0]=draws[0];p->sprites_[1]=draws[1];return p;}catch(...){return {};}
 }
 bool current()const noexcept{return valid(this)!=0;}
 const std::array<uint32_t,300>&descriptors()const noexcept{return descriptors_;}
 const std::array<uint32_t,300>&addresses()const noexcept{return addresses_;}
 unsigned slot()const noexcept{return slot_;}
 NfsCreditsCacheView view()const noexcept{NfsCreditsCacheView v{};if(!current())return v;v.schema=1;v.slot=slot_;v.locale=locale_;v.content=serial_;v.sourceDomain=reinterpret_cast<uintptr_t>(source_.get());for(unsigned i=0;i<2;++i){const auto&d=sprites_[i];const auto&m=nfs_credits_sources[d.sourceIndex];v.sprite[i]={float(d.x),float(d.y),float(d.x)+m.width,float(d.y)+m.height};}v.original565=pixels_->data();v.pixels=pixels_->size();std::copy(addresses_.begin(),addresses_.end(),v.addresses);v.owner=const_cast<CacheSource*>(this);v.retain=retain;v.release=release;v.current=valid;return v;}
};
}

// SPDX-License-Identifier: MIT
#pragma once
#include "CreditsSourceIdentity.h"
#include <array>
namespace nfs_credits_ui {
// Records from the original 4A10BC call after a verified resource resolve.
// Coordinates are actual edx/ebx, NOT the negative header hotspot at byte8.
struct StaticDraw {uint32_t sourceIndex=0;int32_t x=0,y=0;std::array<uint32_t,14>target{};};
// This proves original static-resource calls and their placements. It does not
// grant cached-texture upload, native pass, GL target or presentation authority.
class SceneSource final {
 std::shared_ptr<const SourceLease>source_;uintptr_t game_=0;uint32_t archive_=0,locale_=0;
 std::array<StaticDraw,4>draws_{};
 SceneSource()=default;
public:
 static std::shared_ptr<const SceneSource>create(std::shared_ptr<const SourceLease>source,uintptr_t game,uint32_t archive,unsigned locale,const std::array<StaticDraw,4>&draws)noexcept{
  if(!source||!source->current(game,archive)||locale>=6)return {};
  const unsigned recipe[4]={5,8,0,nfs_credits_title_source_indices[locale]};
  for(unsigned i=0;i<4;++i){const auto&d=draws[i];if(d.sourceIndex!=recipe[i]||d.x< -4096||d.x>4096||d.y< -4096||d.y>4096||!d.target[8]||d.target[1]!=640||d.target[2]!=480||d.target[3]||d.target[4]||d.target[5]!=640||d.target[6]!=480||d.target[7]!=0x7810||d.target[10]!=1280||!d.target[11]||!d.target[12]||d.target[13])return {};}
  if(draws[2].x||draws[2].y)return {};
  try{auto p=std::shared_ptr<SceneSource>(new SceneSource);p->source_=std::move(source);p->game_=game;p->archive_=archive;p->locale_=locale;p->draws_=draws;return p;}catch(...){return {};}
 }
 bool current()const noexcept{return source_&&source_->current(game_,archive_);}
 uintptr_t domain()const noexcept{return reinterpret_cast<uintptr_t>(source_.get());}
 unsigned locale()const noexcept{return locale_;}
 const std::array<StaticDraw,4>&draws()const noexcept{return draws_;}
};
}

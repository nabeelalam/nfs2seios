// SPDX-License-Identifier: MIT
#pragma once
#include "ControlsBindingPhrasePlan.h"
#include "ControlsBindingPhraseArtwork.h"
#include "ControlsBindingPhraseRuntimeAbi.h"
namespace nfs_controls_binding_phrase {
class Prepared final {
 std::shared_ptr<const Plan>plan_;
 std::array<std::shared_ptr<const Artwork>,20>images_{};
 NfsControlsBindingPhrasesView view_{};
 Prepared()=default;
public:
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const nfs_soft::FramePacket>packet)noexcept{
  auto plan=Plan::create(std::move(packet));if(!plan)return {};
  try{auto p=std::shared_ptr<Prepared>(new Prepared);p->plan_=std::move(plan);if(p->plan_->groups().empty())return p;
   p->view_.schema=NFS_CONTROLS_BINDING_PHRASE_SCHEMA;
   for(const auto&g:p->plan_->groups()){
    unsigned slot=0;while(slot<p->view_.imageCount&&p->images_[slot]->image!=g.image)++slot;
    if(slot==p->view_.imageCount){if(slot>=20)return {};auto a=artwork(g.image);if(!a)return {};p->images_[slot]=a;auto&v=p->view_.images[slot];v.phrase=a->image;v.width=a->width;v.height=a->height;v.rgba=a->rgba.data();v.bytes=a->rgba.size();std::copy(a->sha.begin(),a->sha.end(),v.sha256);++p->view_.imageCount;}
    if(p->view_.count>=20)return {};auto&v=p->view_.placements[p->view_.count++];v.slot=slot+1;v.foreground565=g.foregroundColor;v.shadow565=g.shadowColor;v.vertexFirst=g.vertexFirst;v.vertexCount=g.vertexCount;v.left=float(g.x);v.top=float(g.y);v.right=float(g.x+g.width);v.bottom=float(g.y+32);
   }
   return p;
  }catch(...){return {};}
 }
 const std::shared_ptr<const Plan>&plan()const noexcept{return plan_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return plan_->packet();}
 NfsControlsBindingPhrasesView view()const noexcept{return view_;}
};
} // namespace nfs_controls_binding_phrase

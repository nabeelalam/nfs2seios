// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include <algorithm>
#include "ControlsSteeringWordRuntimeAbi.h"
#include "ControlsSteeringWordArtwork.h"
namespace nfs_controls_steering_word_display {
// Presentation of a finite list of complete labels. The source frame remains
// immutable; this display never supplies a form, live-owner or GPU authority.
class Prepared final {
 std::shared_ptr<const nfs_soft::FramePacket>packet_;
 std::array<std::shared_ptr<const Artwork>,NFS_CONTROLS_STEERING_WORD_IMAGES>art_{};
 std::array<NfsControlsSteeringWordPlacement,NFS_CONTROLS_STEERING_WORD_PLACEMENTS>placements_{};
 unsigned count_=0;Prepared()=default;
 static unsigned image(const nfs_soft::TextCommand&c)noexcept{
  if(!c.length||c.length>=c.text.size())return 0;unsigned found=0;
  for(unsigned i=0;i<NFS_CONTROLS_STEERING_WORD_IMAGES;++i){
   const char*t=nfs_controls_steering_word_text[i];
   if(!t||std::strlen(t)!=c.length||c.sourceSize!=nfs_controls_steering_word_source_size[i]||
      c.advance!=int32_t(nfs_controls_steering_word_advance[i])||std::memcmp(c.text.data(),t,c.length))continue;
   if(found)return 0;found=i+1;
  }
  return found;
 }
public:
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const nfs_soft::FramePacket>packet)noexcept{
  if(!packet||!packet->serial()||!packet->surface().valid()||packet->surface().geometry[1]!=640||
     packet->pixels()!=640u*480u||packet->records().size()!=packet->commands()||packet->vertices().empty())return {};
  try{
   auto p=std::shared_ptr<Prepared>(new Prepared);
   for(unsigned i=0;i<NFS_CONTROLS_STEERING_WORD_IMAGES;++i){
    auto a=artwork(i+1);if(!a||a->word!=i+1||a->width!=nfs_controls_steering_word_width[i]||a->height!=128||
       a->width!=4u*nfs_controls_steering_word_advance[i]||a->rgba.size()!=size_t(a->width)*a->height*4)return {};
    p->art_[i]=std::move(a);
   }
   size_t end=0;
   for(const auto&r:packet->records()){
    if(r.vertexFirst<end||r.vertexFirst>packet->vertices().size()||r.vertexCount>packet->vertices().size()-r.vertexFirst)return {};
    end=r.vertexFirst+r.vertexCount;
    const unsigned id=image(r.command);if(!id)continue;
    // Empty, clipped or translated source strings keep their existing glyph
    // spans instead of replacing them with a wider unobserved whole-word box.
    const auto&c=r.command;
    if(!r.vertexCount||c.originX||c.originY||c.x<0||c.y<0||int64_t(c.x)+c.advance>640||int64_t(c.y)+32>480||
       c.x<c.clip.left||c.y<c.clip.top||int64_t(c.x)+c.advance>c.clip.right||int64_t(c.y)+32>c.clip.bottom)continue;
    if(p->count_==NFS_CONTROLS_STEERING_WORD_PLACEMENTS)return {};
    auto&placement=p->placements_[p->count_++];placement.word=id;placement.color565=c.color;
    placement.vertexFirst=r.vertexFirst;placement.vertexCount=r.vertexCount;
    placement.left=float(c.x);placement.top=float(c.y);placement.right=float(c.x+c.advance);placement.bottom=float(c.y+32);
   }
   if(!p->count_)return {};p->packet_=std::move(packet);return p;
  }catch(...){return {};}
 }
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 // Historical immutable consistency; outer complete-frame/current-source queue
 // admission remains responsible for whether this display can be rendered now.
 bool current()const noexcept{return packet_&&count_&&count_<=NFS_CONTROLS_STEERING_WORD_PLACEMENTS;}
 NfsControlsSteeringWordsView view()const noexcept{
  NfsControlsSteeringWordsView v{};if(!current())return v;
  v.schema=NFS_CONTROLS_STEERING_WORD_SCHEMA;v.count=count_;
  for(unsigned i=0;i<NFS_CONTROLS_STEERING_WORD_IMAGES;++i){
   const auto&a=*art_[i];auto&image=v.images[i];image.word=i+1;image.width=a.width;image.height=a.height;
   image.rgba=a.rgba.data();image.bytes=a.rgba.size();std::copy(a.sha.begin(),a.sha.end(),image.sha256);
  }
  std::copy_n(placements_.begin(),count_,v.placements);return v;
 }
};
} // namespace nfs_controls_steering_word_display

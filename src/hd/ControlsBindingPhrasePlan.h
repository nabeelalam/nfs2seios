// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include "ControlsBindingPhraseIdentity.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <vector>
namespace nfs_controls_binding_phrase {
// Presentation of bounded complete binding values only. No form/current-owner,
// source replay, FIFO or native renderer authority is created by this plan.
struct Group {
 unsigned image=0,row=0,column=0,sourceSize=0;
 size_t firstRecord=0,recordCount=0,vertexFirst=0,vertexCount=0;
 int x=0,y=0,width=0;
 uint16_t shadowColor=0,foregroundColor=0;
};
class Plan final {
 std::shared_ptr<const nfs_soft::FramePacket>packet_;
 std::vector<Group>groups_;
 struct Bounds {float l=640,t=480,r=0,b=0;};
 Plan()=default;
 static bool text(const nfs_soft::TextCommand&c,const char*t)noexcept{
  return t&&c.length==std::strlen(t)&&c.length<c.text.size()&&!std::memcmp(c.text.data(),t,c.length);
 }
 static bool clip(const nfs_soft::Clip&a,const nfs_soft::Clip&b)noexcept{
  return a.left==b.left&&a.top==b.top&&a.right==b.right&&a.bottom==b.bottom;
 }
 static bool bounds(const nfs_soft::FramePacket&p,const nfs_soft::RecordedCommand&r,Bounds&b)noexcept{
  if(!r.vertexCount||r.vertexFirst%6||r.vertexCount%6||r.vertexFirst>p.vertices().size()||r.vertexCount>p.vertices().size()-r.vertexFirst)return false;
  for(size_t i=r.vertexFirst;i<r.vertexFirst+r.vertexCount;++i){const auto&v=p.vertices()[i];if(!std::isfinite(v.x)||!std::isfinite(v.y)||v.x<r.command.clip.left||v.x>r.command.clip.right||v.y<r.command.clip.top||v.y>r.command.clip.bottom)return false;b.l=std::min(b.l,v.x);b.t=std::min(b.t,v.y);b.r=std::max(b.r,v.x);b.b=std::max(b.b,v.y);}
  return b.l<b.r&&b.t<b.b;
 }
 static bool match(const nfs_soft::FramePacket&p,size_t first,const NfsControlsBindingPhraseIdentity&id,Group&g)noexcept{
  if(!id.count||id.count>4||!id.width||id.width%4||id.height!=128||first>p.records().size()||2u*id.count>p.records().size()-first)return false;
  const auto&shadow=p.records()[first].command;const auto&foreground=p.records()[first+1].command;
  if(foreground.sourceSize!=id.sourceSize||foreground.x<0||foreground.y<0)return false;
  const int x=foreground.x,y=foreground.y,width=int(id.width/4);
  unsigned row=0,column=0;
  if(id.sourceSize==18){if(x!=419||y<144||(y-144)%26)return false;row=unsigned(y-144)/26;}
  else if(id.sourceSize==16){if((x!=363&&x!=501)||y<129||(y-129)%26)return false;row=unsigned(y-129)/26;column=x==501;}
  else return false;
  if(row>=10)return false;
  const auto&c=foreground.clip;
  for(float n:{c.left,c.top,c.right,c.bottom})if(!std::isfinite(n))return false;
  // Both complete word boxes, including the translated source shadow, must
  // fit. A clipped or exceptional layout keeps all original command spans.
  if(c.left<0||c.top<0||c.right>640||c.bottom>480||x<c.left||y<c.top||int64_t(x)+width+1>c.right||int64_t(y)+33>c.bottom)return false;
  size_t next=p.records()[first].vertexFirst;std::array<Bounds,4>occupied{};
  for(unsigned token=0;token<id.count;++token){
   for(unsigned layer=0;layer<2;++layer){const auto&r=p.records()[first+2*token+layer];const auto&q=r.command;const int offset=layer?0:1;
    if(!text(q,id.tokens[token])||q.role!=1||q.sourceSize!=id.sourceSize||q.alignment||q.originX||q.originY||q.advance!=int(id.advances[token])||int64_t(q.x)!=int64_t(x)+id.positions[token]+offset||q.y!=y+offset||q.color!=(layer?foreground.color:shadow.color)||!clip(q.clip,c)||r.vertexFirst!=next||!bounds(p,r,occupied[token]))return false;
    next+=r.vertexCount;
   }
   // Reordering component shadow/foreground pairs into whole-phrase layers is
   // allowed only when their actual source-vertex boxes do not intersect.
   for(unsigned prior=0;prior<token;++prior){const auto&a=occupied[prior];const auto&b=occupied[token];if(a.l<b.r&&b.l<a.r&&a.t<b.b&&b.t<a.b)return false;}
  }
  g.image=id.image;g.row=row;g.column=column;g.sourceSize=id.sourceSize;g.firstRecord=first;g.recordCount=2u*id.count;g.vertexFirst=p.records()[first].vertexFirst;g.vertexCount=next-g.vertexFirst;g.x=x;g.y=y;g.width=width;g.shadowColor=shadow.color;g.foregroundColor=foreground.color;return true;
 }
public:
 static std::shared_ptr<const Plan>create(std::shared_ptr<const nfs_soft::FramePacket>packet)noexcept{
  if(!packet||!packet->serial()||!packet->surface().valid()||packet->surface().geometry[1]!=640||packet->pixels()!=640u*480u||packet->records().size()!=packet->commands())return {};
  try{auto p=std::shared_ptr<Plan>(new Plan);p->packet_=std::move(packet);p->groups_.reserve(20);
   const auto&records=p->packet_->records();for(size_t at=0;at+1<records.size()&&p->groups_.size()<20;){bool found=false;const unsigned size=records[at].command.sourceSize;
    if(size==16||size==18){const unsigned base=size==18?180:0;for(unsigned i=base;i<base+180;++i){const auto&id=nfs_controls_binding_phrase_images[i];if(!text(records[at].command,id.tokens[0]))continue;Group group;if(match(*p->packet_,at,id,group)){p->groups_.push_back(group);at+=group.recordCount;found=true;break;}}}
    if(!found)++at;
   }
   return p;
  }catch(...){return {};}
 }
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 const std::vector<Group>&groups()const noexcept{return groups_;}
};
} // namespace nfs_controls_binding_phrase

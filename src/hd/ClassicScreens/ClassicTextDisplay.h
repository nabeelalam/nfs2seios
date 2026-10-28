// PRIVATE display-only adapter. Original commands/values remain immutable.
// This class does not grant live source ownership or replace global fonts.
#pragma once
#include "hd/SoftwareFramePacket.h"
#include "hd/SoftwareTextVertices.h"
#include "hd/SoftwareTextLayerAbi.h"
#include "TextureDigest.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <utility>
namespace nfs_classic_ui {
struct TextBox {float width=0;unsigned role=0;float scale=1;};
class PreparedText final {
 std::shared_ptr<const void>owner_;
 std::shared_ptr<const nfs_soft::FramePacket>packet_;
 std::shared_ptr<const nfs_soft::FontCatalog>font_;
 std::vector<nfs_soft::TextCommand>commands_;
 std::vector<NfsSoftwareTextVertex>vertices_;
 std::vector<std::pair<size_t,size_t>>ranges_;
 std::array<unsigned char,32>sha_{};
 PreparedText()=default;
 static bool visible(const nfs_soft::TextCommand&c,std::array<uint8_t,96>&text,unsigned&length)noexcept{
  if(!c.length||c.length>95||c.text[c.length])return false;
  length=c.length;bool terminal=false;
  for(unsigned n=0;n<c.length;++n){unsigned k=uint8_t(c.text[n]);
   if(k==9){if(!terminal){length=n;terminal=true;}continue;}
   if(terminal||nfs_soft::index(uint8_t(k))<0)return false;
  }
  std::copy_n(c.text.begin(),length,text.begin());text[length]=0;return true;
 }
public:
 static std::shared_ptr<const PreparedText>createCommands(
  const std::vector<nfs_soft::TextCommand>&commands,
  std::shared_ptr<const nfs_soft::FontCatalog>font,
  std::shared_ptr<const void>sourceOwner,
  const std::vector<TextBox>&boxes={},bool allowEmptyInk=false)noexcept{
  if(!font||!sourceOwner||commands.empty()||commands.size()>256||(!boxes.empty()&&boxes.size()!=commands.size()))return {};
  try{
   auto p=std::shared_ptr<PreparedText>(new PreparedText);p->owner_=std::move(sourceOwner);p->font_=std::move(font);p->commands_=commands;p->vertices_.resize(16384);size_t used=0;
   for(size_t n=0;n<commands.size();++n){
    const auto&c=commands[n];std::array<uint8_t,96>text{};unsigned length=0;if(!visible(c,text,length))return {};
    if(!length){p->ranges_.emplace_back(used,0);continue;}const TextBox box=boxes.empty()?TextBox{}:boxes[n];
    if(box.role>1||!std::isfinite(box.width)||box.width<0||box.width>640||!std::isfinite(box.scale)||box.scale<=0||box.scale>4||c.alignment>2||c.advance<0)return {};
    const float scale=float(c.sourceSize)/18.f*box.scale;
    if(c.sourceSize!=16&&c.sourceSize!=18&&c.sourceSize!=24&&c.sourceSize!=36)return {};
    for(float v:{c.clip.left,c.clip.top,c.clip.right,c.clip.bottom})if(!std::isfinite(v))return {};
    if(c.clip.left<0||c.clip.top<0||c.clip.right>640||c.clip.bottom>480||c.clip.left>c.clip.right||c.clip.top>c.clip.bottom)return {};
    nfs_soft::Layout layout;layout.length=length;int64_t pen=0;int previous=-1;
    const auto&description=p->font_->description();
    // Build unclipped whole-string geometry, then fit and clip exactly once.
    for(unsigned k=0;k<length;++k){int at=nfs_soft::index(text[k]);if(at<0)return {};if(previous>=0)pen+=description.kern[box.role][previous*96+at];const auto&g=description.glyphs[box.role][at];
     if(g.w&&g.h){float l=float(pen/65536.0)+(g.left-1)*.25f,t=(g.top-1)*.25f;
      layout.quads[layout.count++]={l,t,l+(g.w+2)*.25f,t+(g.h+2)*.25f,(g.x-1.f)/1024,(g.y-1.f)/512,(g.x+g.w+1.f)/1024,(g.y+g.h+1.f)/512,text[k]};}
     pen+=g.advance;previous=at;
    }
    layout.advance=pen;float natural=float(pen/65536.0)*scale;
    float limit=box.width>0?box.width:float(c.advance);
    float fit=limit>0&&natural>limit?limit/natural:1.f;
    const float width=natural*fit;
    const float left=float(c.x)+float(c.originX)+(c.alignment==2?float(c.advance)-width:c.alignment==1?(float(c.advance)-width)*.5f:0.f);
    unsigned shown=0;
    for(unsigned k=0;k<layout.count;++k){
     auto q=layout.quads[k];q.left=left+q.left*scale*fit;q.right=left+q.right*scale*fit;q.top=float(c.y)+float(c.originY)+24.f+q.top*scale;q.bottom=float(c.y)+float(c.originY)+24.f+q.bottom*scale;
     float l=std::max(q.left,c.clip.left),t=std::max(q.top,c.clip.top),r=std::min(q.right,c.clip.right),b=std::min(q.bottom,c.clip.bottom);
     if(l>=r||t>=b)continue;
     const float u=q.u1-q.u0,v=q.v1-q.v0,w=q.right-q.left,h=q.bottom-q.top;
     q.u1=q.u0+(r-q.left)/w*u;q.v1=q.v0+(b-q.top)/h*v;q.u0+=(l-q.left)/w*u;q.v0+=(t-q.top)/h*v;q.left=l;q.top=t;q.right=r;q.bottom=b;
     layout.quads[shown++]=q;
    }
    layout.count=shown;
    // append_vertices accepts logical RGB565 colour, preserving original selection/shadow colours.
    size_t first=used;if(!nfs_soft::append_vertices(layout,c.color,p->vertices_.data(),p->vertices_.size(),used))return {};p->ranges_.emplace_back(first,used-first);
   }
   p->vertices_.resize(used);if((!used&&!allowEmptyInk)||!nfs_texture_sha256(p->font_->alpha().data(),p->font_->alpha().size(),p->sha_.data()))return {};
   return p;
  }catch(...){return {};}
 }
 static std::shared_ptr<const PreparedText>create(std::shared_ptr<const nfs_soft::FramePacket>packet,
  std::shared_ptr<const nfs_soft::FontCatalog>font,const std::vector<TextBox>&boxes={})noexcept{
  if(!packet||packet->pixels()!=640*480||!packet->surface().valid()||packet->records().empty())return {};
  try{std::vector<nfs_soft::TextCommand>commands;commands.reserve(packet->records().size());for(const auto&r:packet->records())commands.push_back(r.command);
   auto out=createCommands(commands,std::move(font),packet,boxes);if(!out)return {};auto mutableOut=std::const_pointer_cast<PreparedText>(out);mutableOut->packet_=std::move(packet);return out;
  }catch(...){return {};}
 }
 NfsSoftwareTextLayerView layer()const noexcept{
  NfsSoftwareTextLayerView v{};if(!owner_||!font_||vertices_.empty())return v;
  v.logicalWidth=640;v.logicalHeight=480;v.font.schema=1;v.font.width=1024;v.font.height=512;v.font.identity=font_->identity();v.font.alpha=font_->alpha().data();v.font.bytes=font_->alpha().size();std::copy(sha_.begin(),sha_.end(),v.font.sha256);v.vertices=vertices_.data();v.vertexCount=vertices_.size();return v;
 }
 const std::vector<std::pair<size_t,size_t>>&ranges()const noexcept{return ranges_;}
 const std::vector<nfs_soft::TextCommand>&originalCommands()const noexcept{return commands_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&sourcePacket()const noexcept{return packet_;}
 const std::shared_ptr<const void>&sourceOwner()const noexcept{return owner_;}
};
} // namespace nfs_classic_ui

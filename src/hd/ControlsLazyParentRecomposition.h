// SPDX-License-Identifier: MIT
#pragma once
#include "ControlsSceneRuntimeAbi.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include <vector>
namespace nfs_controls_lazy_parent_recomposition {
constexpr size_t Width=640,Height=480,SidebarWidth=343,Pixels=Width*Height;
enum class Failure {None,Span,Header,View,ProtectedBackground,UntrackedText,ActualPlane,Allocation};
class Verifier;
class VerifiedOriginalRecipe final {
 std::vector<uint16_t>final_;
 explicit VerifiedOriginalRecipe(std::vector<uint16_t>p):final_(std::move(p)){}
 friend class Verifier;
public:
 const std::vector<uint16_t>&finalPlane()const noexcept{return final_;}
};
struct Result {Failure failure=Failure::Span;std::unique_ptr<const VerifiedOriginalRecipe>recipe;};
namespace detail {
inline uint16_t le16(const uint8_t*p)noexcept{return uint16_t(uint16_t(p[0])|(uint16_t(p[1])<<8));}
inline uint32_t le32(const uint8_t*p)noexcept{return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
inline bool protectedCells(const NfsControlsSceneFrameView&view,std::vector<uint8_t>&cells)noexcept{
 const auto&t=view.layers.text;const auto&d=view.layers.dialogWords;
 if(view.schema!=NFS_CONTROLS_SCENE_RUNTIME_SCHEMA||view.width!=Width||view.height!=Height||t.logicalWidth!=Width||t.logicalHeight!=Height||t.vertexCount!=696||!t.vertices||d.schema!=NFS_CONTROLS_DIALOG_WORD_SCHEMA||d.count!=NFS_CONTROLS_DIALOG_WORD_PLACEMENTS||cells.size()!=Pixels)return false;
 auto mark=[&](float l,float top,float r,float b)noexcept{
  if(!std::isfinite(l)||!std::isfinite(top)||!std::isfinite(r)||!std::isfinite(b)||l<0||top<0||r>Width||b>Height||l>=r||top>=b)return false;
  const int x0=std::max(0,int(std::floor(l))-1),y0=std::max(0,int(std::floor(top))-1),x1=std::min(640,int(std::ceil(r))+1),y1=std::min(480,int(std::ceil(b))+1);
  for(int y=y0;y<y1;++y)for(int x=x0;x<x1;++x)cells[size_t(y)*Width+size_t(x)]=1;return true;
 };
 for(size_t first=0;first<696;first+=6){float l=640,top=480,r=0,b=0;
  for(unsigned k=0;k<6;++k){const auto&v=t.vertices[first+k];
   if(!std::isfinite(v.x)||!std::isfinite(v.y)||!std::isfinite(v.u)||!std::isfinite(v.v)||!std::isfinite(v.r)||!std::isfinite(v.g)||!std::isfinite(v.b)||!std::isfinite(v.a)||v.x<0||v.x>Width||v.y<0||v.y>Height||v.u<0||v.u>1||v.v<0||v.v>1||v.a<0||v.a>1||v.r<0||v.g<0||v.b<0||v.r>v.a||v.g>v.a||v.b>v.a)return false;
   l=std::min(l,v.x);top=std::min(top,v.y);r=std::max(r,v.x);b=std::max(b,v.y);
  }if(!mark(l,top,r,b))return false;
 }
 for(unsigned i=0;i<NFS_CONTROLS_DIALOG_WORD_PLACEMENTS;++i){const auto&p=d.placements[i];if(!mark(p.left,p.top,p.right,p.bottom))return false;}return true;
}
} // namespace detail
class Verifier final {
public:
 // Pure candidate recipe proof. It does not construct a new source/text packet
 // or authenticate Game ownership; the actual producer supplies typed owners.
 static Result verify(const std::vector<uint8_t>&mapBytes,const std::vector<uint8_t>&freshSidebar,const std::vector<uint16_t>&oldCleanWords,const std::vector<uint16_t>&oldOriginalWords,const std::vector<uint8_t>&actualAfterBytes,const NfsControlsSceneFrameView&view)noexcept{
  auto fail=[](Failure f){return Result{f,nullptr};};
  if(mapBytes.size()!=16+Pixels*2||freshSidebar.size()!=16+SidebarWidth*Height*2||oldCleanWords.size()!=Pixels||oldOriginalWords.size()!=Pixels||actualAfterBytes.size()!=Pixels*2)return fail(Failure::Span);
  // The original map catalogue requires zero auxiliary header fields. Sidebar
  // mode1000 consumes a fixed left343 image; its hotspot is not an offset here.
  if(detail::le32(mapBytes.data())!=0x78||detail::le32(mapBytes.data()+4)!=(640u|(480u<<16))||detail::le32(mapBytes.data()+8)||detail::le32(mapBytes.data()+12)||detail::le32(freshSidebar.data())!=0x78||detail::le32(freshSidebar.data()+4)!=(343u|(480u<<16)))return fail(Failure::Header);
  try{std::vector<uint8_t>protect(Pixels,0);if(!detail::protectedCells(view,protect))return fail(Failure::View);
   std::vector<uint16_t>candidate(Pixels);
   for(size_t y=0;y<Height;++y)for(size_t x=0;x<Width;++x){const size_t at=y*Width+x;uint16_t background=detail::le16(mapBytes.data()+16+at*2);
    if(x<SidebarWidth){const uint16_t sidebar=detail::le16(freshSidebar.data()+16+(y*SidebarWidth+x)*2);if(sidebar)background=sidebar;}
    if(protect[at]){if(background!=oldCleanWords[at])return fail(Failure::ProtectedBackground);candidate[at]=oldOriginalWords[at];}
    else{if(oldOriginalWords[at]!=oldCleanWords[at])return fail(Failure::UntrackedText);candidate[at]=background;}
    if(candidate[at]!=detail::le16(actualAfterBytes.data()+at*2))return fail(Failure::ActualPlane);
   }
   return Result{Failure::None,std::unique_ptr<const VerifiedOriginalRecipe>(new VerifiedOriginalRecipe(std::move(candidate)))};
  }catch(...){return fail(Failure::Allocation);}
 }
};
inline Result verify(const std::vector<uint8_t>&mapBytes,const std::vector<uint8_t>&freshSidebar,const std::vector<uint16_t>&oldCleanWords,const std::vector<uint16_t>&oldOriginalWords,const std::vector<uint8_t>&actualAfterBytes,const NfsControlsSceneFrameView&view)noexcept{return Verifier::verify(mapBytes,freshSidebar,oldCleanWords,oldOriginalWords,actualAfterBytes,view);}
} // namespace nfs_controls_lazy_parent_recomposition

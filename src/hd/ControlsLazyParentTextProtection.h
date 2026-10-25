// SPDX-License-Identifier: MIT
#pragma once
#include "ControlsLazyParentCopyRecords.h"
#include "ControlsSceneRuntimeAbi.h"
#include <algorithm>
#include <cmath>
#include <vector>
namespace nfs_controls_lazy_parent_text_protection {
// Numeric occlusion guard only. The caller separately authenticates the actual
// copy, immutable sidebar, retained scene and current original Main plane.
inline bool preserved(const nfs_controls_lazy_parent_copy::Geometry&g,const std::vector<uint8_t>&sidebar,uint32_t key,const NfsControlsSceneFrameView&view)noexcept{
 namespace copy=nfs_controls_lazy_parent_copy;
 constexpr size_t MainTextVertices=696; // Exact finite Controls-dialog proof.
 if(sidebar.size()!=copy::SourceBytes||g.x< -2048||g.x>2047||g.y< -2048||g.y>2047||g.left<0||g.top<0||g.right>640||g.bottom>480||g.left>=g.right||g.top>=g.bottom||g.left-g.x<0||g.top-g.y<0||g.right-g.x>343||g.bottom-g.y>480)return false;
 const auto&text=view.layers.text;const auto&dialog=view.layers.dialogWords;
 if(view.schema!=NFS_CONTROLS_SCENE_RUNTIME_SCHEMA||view.width!=640||view.height!=480||text.logicalWidth!=640||text.logicalHeight!=480||text.vertexCount!=MainTextVertices||text.vertexCount%6||!text.vertices||dialog.schema!=NFS_CONTROLS_DIALOG_WORD_SCHEMA||dialog.count!=NFS_CONTROLS_DIALOG_WORD_PLACEMENTS)return false;
 auto protect=[&](float l,float t,float r,float b)noexcept{
  if(!std::isfinite(l)||!std::isfinite(t)||!std::isfinite(r)||!std::isfinite(b)||l<0||t<0||r>640||b>480||l>=r||t>=b)return false;
  // Outward rounding plus one logical pixel protects filtered/subpixel edges.
  const int left=std::max(g.left,std::max(0,int(std::floor(l))-1));
  const int top=std::max(g.top,std::max(0,int(std::floor(t))-1));
  const int right=std::min(g.right,std::min(640,int(std::ceil(r))+1));
  const int bottom=std::min(g.bottom,std::min(480,int(std::ceil(b))+1));
  for(int y=top;y<bottom;++y)for(int x=left;x<right;++x){
   const size_t sx=size_t(x-g.x),sy=size_t(y-g.y);
   // Geometry above proves sx<343,sy<480 before any address arithmetic.
   const size_t at=16+(sy*343+sx)*2;
   const uint32_t pixel=uint32_t(sidebar[at])|(uint32_t(sidebar[at+1])<<8);
   if(pixel!=key)return false; // Full32 key: greater than FFFF never matches.
  }
  return true;
 };
 for(size_t first=0;first<MainTextVertices;first+=6){
  float l=640,t=480,r=0,b=0;
  for(unsigned j=0;j<6;++j){const auto&v=text.vertices[first+j];
   if(!std::isfinite(v.x)||!std::isfinite(v.y)||!std::isfinite(v.u)||!std::isfinite(v.v)||!std::isfinite(v.r)||!std::isfinite(v.g)||!std::isfinite(v.b)||!std::isfinite(v.a)||v.x<0||v.x>640||v.y<0||v.y>480||v.u<0||v.u>1||v.v<0||v.v>1||v.a<0||v.a>1||v.r<0||v.g<0||v.b<0||v.r>v.a||v.g>v.a||v.b>v.a)return false;
   l=std::min(l,v.x);t=std::min(t,v.y);r=std::max(r,v.x);b=std::max(b,v.y);
  }
  if(!protect(l,t,r,b))return false;
 }
 for(unsigned i=0;i<NFS_CONTROLS_DIALOG_WORD_PLACEMENTS;++i){const auto&p=dialog.placements[i];if(!protect(p.left,p.top,p.right,p.bottom))return false;}
 return true;
}
} // namespace nfs_controls_lazy_parent_text_protection

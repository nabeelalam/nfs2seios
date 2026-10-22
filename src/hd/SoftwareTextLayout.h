// SPDX-License-Identifier: MIT
// Authenticated caller-owned font metadata; logical original-game coordinates.
#pragma once
#include "SoftwareFontCatalog.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
namespace nfs_soft {
constexpr unsigned max_text=95;
struct Clip {float left,top,right,bottom;};
struct Quad {float left,top,right,bottom,u0,v0,u1,v1;uint8_t code;};
struct Layout {std::array<Quad,max_text> quads{};std::array<int64_t,max_text> positions{};unsigned count=0,length=0;int64_t advance=0;float fittedWidth=0;};
inline int index(uint8_t code){return code>=32&&code<=126?int(code-32):code==224?95:-1;}
inline bool layout(const FontCatalog&font,const uint8_t *text,size_t bytes,unsigned role,float x,float y,float boxWidth,unsigned alignment,Clip clip,Layout &result){
 if(!text||!bytes||bytes>max_text+1||role>=2||alignment>2)return false;
 for(float v:{x,y,boxWidth,clip.left,clip.top,clip.right,clip.bottom})if(!std::isfinite(v))return false;
 if(std::abs(x)>8192||std::abs(y)>8192||boxWidth<0||boxWidth>4096||clip.left<0||clip.top<0||clip.right>640||clip.bottom>480||clip.right<clip.left||clip.bottom<clip.top)return false;
 unsigned length=0;std::array<int,max_text> indices{};
 for(;length<bytes && text[length];++length){if(length>=max_text)return false;indices[length]=index(text[length]);if(indices[length]<0)return false;}
 if(length==bytes)return false;
 Layout next;next.length=length;int64_t pen=0;
 for(unsigned i=0;i<length;++i){if(i)pen+=font.description().kern[role][indices[i-1]*96+indices[i]];next.positions[i]=pen;pen+=font.description().glyphs[role][indices[i]].advance;}
 next.advance=pen;
 // Fit the complete ink and logical advance; punctuation can overhang its advance.
 // Natural, unboxed layout preserves the shaping engine's exact origin.
 double inkLeft=0,inkRight=pen/65536.0;
 for(unsigned i=0;i<length;++i){const auto &g=font.description().glyphs[role][indices[i]];if(!g.w||!g.h)continue;const double at=next.positions[i]/65536.0;inkLeft=std::min(inkLeft,at+g.left*.25);inkRight=std::max(inkRight,at+(g.left+g.w)*.25);}
 const float natural=float(inkRight-inkLeft);if(!std::isfinite(natural)||natural<0)return false;const float scale=boxWidth>0&&natural>boxWidth?boxWidth/natural:1.f;next.fittedWidth=natural*scale;
 const float gap=boxWidth>0?boxWidth-next.fittedWidth:0;const float origin=x+(alignment==1?gap*.5f:alignment==2?gap:0)-(boxWidth>0?float(inkLeft)*scale:0);
 for(unsigned i=0;i<length;++i){const auto &g=font.description().glyphs[role][indices[i]];if(!g.w||!g.h)continue;
  // Include one transparent texel around each ink box for filtered edges.
  float left=origin+float(next.positions[i]/65536.0)*scale+(g.left-1)*.25f*scale,top=y+font.description().baseline[role]+(g.top-1)*.25f;
  float right=left+(g.w+2)*.25f*scale,bottom=top+(g.h+2)*.25f;
  float cl=std::max(left,clip.left),ct=std::max(top,clip.top),cr=std::min(right,clip.right),cb=std::min(bottom,clip.bottom);if(cl>=cr||ct>=cb)continue;
  const float u0=(g.x-1.f)/FontCatalog::width,v0=(g.y-1.f)/FontCatalog::height,u1=(g.x+g.w+1.f)/FontCatalog::width,v1=(g.y+g.h+1.f)/FontCatalog::height;
  next.quads[next.count++]={cl,ct,cr,cb,u0+(cl-left)/(right-left)*(u1-u0),v0+(ct-top)/(bottom-top)*(v1-v0),u0+(cr-left)/(right-left)*(u1-u0),v0+(cb-top)/(bottom-top)*(v1-v0),text[i]};
 }
 result=next;return true;
}
}

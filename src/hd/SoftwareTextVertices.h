// SPDX-License-Identifier: MIT
// Renderer bridge: bounded caller-owned batch, original game coordinates.
#pragma once
#include "SoftwareTextLayout.h"
#include "SoftwareTextVertex.h"
namespace nfs_soft {
using Vertex=NfsSoftwareTextVertex;
inline bool append_vertices(const Layout& layout,uint16_t color,Vertex* output,size_t capacity,size_t&used){
 if(layout.count>max_text||layout.length>max_text||layout.count>layout.length||used>capacity||used%6||layout.count>(capacity-used)/6||(!output&&layout.count))return false;
 // Validate the entire batch before writing any caller-owned vertex.
 for(unsigned i=0;i<layout.count;++i){const auto&q=layout.quads[i];for(float v:{q.left,q.top,q.right,q.bottom,q.u0,q.v0,q.u1,q.v1})if(!std::isfinite(v))return false;
  if(q.left<0||q.top<0||q.right>640||q.bottom>480||q.left>=q.right||q.top>=q.bottom||q.u0<0||q.v0<0||q.u1>1||q.v1>1||q.u0>=q.u1||q.v0>=q.v1)return false;
 }
 unsigned red=(color>>11)&31,green=(color>>5)&63,blue=color&31;
 const float cr=float((red<<3)|(red>>2))/255,cg=float((green<<2)|(green>>4))/255,cb=float((blue<<3)|(blue>>2))/255;
 for(unsigned i=0;i<layout.count;++i){const auto&q=layout.quads[i];const Vertex a{q.left,q.top,q.u0,q.v0,cr,cg,cb,1},b{q.left,q.bottom,q.u0,q.v1,cr,cg,cb,1},c{q.right,q.top,q.u1,q.v0,cr,cg,cb,1},d{q.right,q.bottom,q.u1,q.v1,cr,cg,cb,1};
  for(const Vertex& v:{a,b,c,c,b,d})output[used++]=v;
 }
 return true;
}
}

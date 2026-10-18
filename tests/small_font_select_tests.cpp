// SPDX-License-Identifier: MIT
#include <cstdio>
#include <limits>
#include <initializer_list>
#include "HdGlyphSelect.h"

int main(){unsigned count=0;float ignored[6]{};
 if(nfs_hd_small_font_triangle(nullptr,4)||nfs_hd_small_font_triangle(ignored,0)||nfs_hd_small_font_triangle(ignored,16))return 9;
 for(unsigned edge:{4u,8u}){float low=127.5f/edge;const float p[4][2]={{low,low},{255,low},{low,255},{255,255}};
  for(unsigned a=0;a<4;++a)for(unsigned b=0;b<4;++b)for(unsigned c=0;c<4;++c){
   float uv[6]={p[a][0],p[a][1],p[b][0],p[b][1],p[c][0],p[c][1]};
   if(bool(nfs_hd_small_font_triangle(uv,edge))!=(a!=b && a!=c && b!=c))return 1;++count;
  }
  float uv[6]={low,low,255,low,255,255};
  for(unsigned i=0;i<6;++i){float saved=uv[i];
   for(float delta:{-.1f,.1f}){uv[i]=saved+delta;if(nfs_hd_small_font_triangle(uv,edge))return 2;++count;}
   for(float invalid:{std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}){
    uv[i]=invalid;if(nfs_hd_small_font_triangle(uv,edge))return 3;++count;
   }uv[i]=saved;
  }
 }
 std::printf("PASS:%u exact/reordered/degenerate/clipped/nonfinite cases across4and8pixel font sources\n",count);
}

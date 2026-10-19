// SPDX-License-Identifier: MIT
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <limits>
#include <cstdint>
#include "TextureDigest.h"
struct GrVertex {float x,y,z,r,g,b,ooz,a,oow;};
#include "StartLightPixels.inc"
#include "StartLightRuntime.inc"
#define CHECK(v) do{if(!(v)){std::fprintf(stderr,"Start light check failed at %d\n",__LINE__);std::abort();}}while(0)
int main(){
 const size_t capacity=4194304;std::vector<unsigned char> memory(capacity+32,0x5b),source(222*222*4);auto rgba=memory.data()+16;
 CHECK(!nfs_start_light_expand(nullptr,capacity));CHECK(!nfs_start_light_expand(rgba,capacity-1));
 for(auto c:memory)CHECK(c==0x5b);CHECK(nfs_start_light_expand(rgba,capacity));
 for(unsigned y=0;y<222;++y)for(unsigned x=0;x<222;++x){unsigned bank=y>=110,ay=188+y-bank*110,ax=248+x+bank*222;std::memcpy(source.data()+(y*222+x)*4,rgba+(ay*1024+ax)*4,4);}
 unsigned char digest[32];CHECK(nfs_texture_sha256(source.data(),source.size(),digest) && !std::memcmp(digest,nfs_start_light_raster_digest,32));
 for(unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x){auto p=rgba+(y*1024+x)*4;
  if(y>=188 && y<300 && x>=248 && x<692){unsigned bank=x>=470,sx=x-248-bank*222,sy=y-188+bank*110;CHECK(!std::memcmp(p,source.data()+(sy*222+sx)*4,4));}
  else for(unsigned c=0;c<4;++c)CHECK(p[c]==0x5b);
 }
 for(unsigned i=0;i<16;++i)CHECK(memory[i]==0x5b && memory[capacity+16+i]==0x5b);
 const unsigned permutations[6][3]={{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
 for(unsigned omitted=0;omitted<4;++omitted){GrVertex original[3]{};float original_uv[6];unsigned j=0;
  for(unsigned corner=0;corner<4;++corner)if(corner!=omitted){original[j].oow=65535.0f/65536;original_uv[2*j]=((corner&1)?209.0f:159.0f)*255/256;original_uv[2*j+1]=((corner&2)?180.0f:130.0f)*255/256;++j;}
  for(const auto &order:permutations){GrVertex v[3],before[3];float uv[6],saved_uv[6];
   for(unsigned i=0;i<3;++i){v[i]=original[order[i]];uv[2*i]=original_uv[2*order[i]];uv[2*i+1]=original_uv[2*order[i]+1];}
   std::memcpy(before,v,sizeof v);std::memcpy(saved_uv,uv,sizeof uv);CHECK(nfs_start_light_triangle(v,v+1,v+2,uv));CHECK(!std::memcmp(v,before,sizeof v) && !std::memcmp(uv,saved_uv,sizeof uv));
   for(unsigned i=0;i<3;++i){float q=v[i].oow;for(float bad:{0.0f,-1.0f,.5f,1.0f,std::numeric_limits<float>::quiet_NaN()}){v[i].oow=bad;CHECK(!nfs_start_light_triangle(v,v+1,v+2,uv));}v[i].oow=q;}
   for(unsigned i=0;i<6;++i){float good=uv[i];for(float bad:{good+.1f,0.0f,std::numeric_limits<float>::quiet_NaN()}){uv[i]=bad;CHECK(!nfs_start_light_triangle(v,v+1,v+2,uv));}uv[i]=good;}
   for(unsigned i=0;i<3;++i){uv[2*i]=uv[0];uv[2*i+1]=uv[1];}CHECK(!nfs_start_light_triangle(v,v+1,v+2,uv));
  }
 }
 std::puts("PASS countdown raster digest, banks/seams/canaries, 24 triangle orientations, immutable input and invalid UV/depth/NaN rejection");
}

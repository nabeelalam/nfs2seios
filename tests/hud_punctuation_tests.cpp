// SPDX-License-Identifier: MIT
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <initializer_list>
struct Tmu {float sow,tow,oow;};
struct GrVertex {float x,y,z,r,g,b,ooz,a,oow;Tmu tmuvtx[2];};
#define VertexSnap 786432
static int nfs_hd_builtin_enabled=1,nfs_hd_visible=1,nfs_hd_aux_texture=1,nfs_hd_selected=1;
#include "ClockPunctuationPixels.inc"
#include "SilverSlashPixels.inc"
#include "LabelCellLayout.inc"
#include "SilverSlashRuntime.inc"
#define CHECK(v) do {if(!(v)){std::fprintf(stderr,"Punctuation check failed at %d\n",__LINE__);std::abort();}}while(0)
int main(){
 nfs_hd_cell_layout_available=1;
 for(unsigned half=0;half<2;++half)for(unsigned place=0;place<2;++place)for(unsigned white=0;white<2;++white)for(unsigned omitted=0;omitted<4;++omitted){
  float x[4]={0,5,8,3},y[4]={27,0,0,27};if(white){x[0]=1;x[1]=6;x[2]=7;x[3]=2;y[0]=y[3]=26;y[1]=y[2]=1;}
  GrVertex v[3]={},before[3],out[3];unsigned count=0;
  for(unsigned j=0;j<4;++j)if(j!=omitted){auto &a=v[count++];a.x=VertexSnap+(place?602:455)+x[j];a.y=VertexSnap+5+241*half+y[j];a.oow=1;a.a=255;a.r=a.b=white?247:16;a.g=white?251:16;a.z=37;a.tmuvtx[1].sow=91;}
  std::memcpy(before,v,sizeof v);CHECK(nfs_hd_silver_slash(v,v+1,v+2,out)==(white?2:1));CHECK(!std::memcmp(v,before,sizeof v));
  if(!white)for(unsigned j=0;j<3;++j){CHECK(out[j].tmuvtx[0].sow>=53.125f && out[j].tmuvtx[0].sow<=61.875f);CHECK(out[j].tmuvtx[0].tow>=47.125f && out[j].tmuvtx[0].tow<=74.875f);out[j].x=v[j].x;out[j].y=v[j].y;out[j].tmuvtx[0].sow=v[j].tmuvtx[0].sow;out[j].tmuvtx[0].tow=v[j].tmuvtx[0].tow;CHECK(!std::memcmp(out+j,v+j,sizeof(GrVertex)));}
  for(int *flag:{&nfs_hd_builtin_enabled,&nfs_hd_visible,&nfs_hd_aux_texture,&nfs_hd_selected,&nfs_hd_cell_layout_available}){*flag=0;CHECK(nfs_hd_silver_slash(v,v+1,v+2,out)==0);*flag=1;}
  v[0].oow=2;CHECK(nfs_hd_silver_slash(v,v+1,v+2,out)==0);v[0]=before[0];
  for(auto &a:v)a.x+=.125f;CHECK(nfs_hd_silver_slash(v,v+1,v+2,out)==0);
 }
 for(unsigned half=0;half<2;++half)for(unsigned omitted=0;omitted<4;++omitted){GrVertex v[3]={},before[3],out[3];float uv[6];unsigned count=0;
  for(unsigned k=0;k<4;++k)if(k!=omitted){auto &a=v[count];a.x=VertexSnap+272+(k&1)*33;a.y=VertexSnap+8+241*half+(k>>1)*22;a.oow=1;a.z=17;uv[2*count]=(k&1)?32.87109375f:.498046875f;uv[2*count+1]=(k>>1)?168.33984375f:146.923828125f;++count;}
  CHECK(nfs_clock_punctuation_triangle(uv));uv[0]+=.25f;CHECK(!nfs_clock_punctuation_triangle(uv));std::memcpy(before,v,sizeof v);
  CHECK(nfs_hd_label_geometry(69,v,v+1,v+2,out)==1);CHECK(!std::memcmp(before,v,sizeof v));
  for(unsigned i=0;i<3;++i){CHECK(out[i].y==VertexSnap+5+241*half || out[i].y==VertexSnap+31+241*half);out[i].y=v[i].y;CHECK(!std::memcmp(out+i,v+i,sizeof(GrVertex)));}
  nfs_hd_cell_layout_available=0;CHECK(nfs_hd_label_geometry(69,v,v+1,v+2,out)==0);nfs_hd_cell_layout_available=1;
 }
 const size_t capacity=4194304;std::vector<unsigned char> image(capacity+32,0xa5),before=image;
 CHECK(!nfs_clock_punctuation_expand(nullptr,capacity));CHECK(!nfs_clock_punctuation_expand(image.data()+16,capacity-1));CHECK(!nfs_silver_slash_expand(image.data()+16,capacity-1));CHECK(image==before);
 CHECK(nfs_clock_punctuation_expand(image.data()+16,capacity));auto clock=image;CHECK(nfs_silver_slash_expand(image.data()+16,capacity));
 for(unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x){size_t at=16+((size_t)y*1024+x)*4;bool p=x>=80 && x<212 && y>=188 && y<292,s=x>=212 && x<248 && y>=188 && y<300;
  for(unsigned c=0;c<4;++c){if(p)CHECK(image[at+c]==clock[at+c]);if(!p && !s)CHECK(image[at+c]==0xa5);}}
 for(unsigned i=0;i<16;++i)CHECK(image[i]==0xa5 && image[capacity+16+i]==0xa5);
 std::puts("PASS punctuation/slash rows, immutable attributes, fallback guards and disjoint atlas/canary writes");
}

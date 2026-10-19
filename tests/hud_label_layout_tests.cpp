// SPDX-License-Identifier: MIT
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <cstdlib>
struct GrVertex { float x,y,z,r,g,b,ooz,a,oow,attributes[6]; };
#define VertexSnap 786432
#include "LabelCellLayout.inc"
#include "WideLabelPixels.inc"
#define CHECK(v) do {if(!(v)){std::fprintf(stderr,"Label check failed at %d\n",__LINE__);std::abort();}}while(0)
int main(){
 const int selectors[2][3]={{27,26,30},{23,12,27}};
 nfs_hd_cell_layout_available=1;
 for(unsigned half=0;half<2;++half)for(unsigned word=0;word<2;++word)for(unsigned letter=0;letter<3;++letter)for(unsigned omitted=0;omitted<4;++omitted){
  GrVertex input[3]={},before[3],out[3];unsigned count=0;float start=word?560:401;
  for(unsigned k=0;k<4;++k)if(k!=omitted){auto &a=input[count++];a.x=VertexSnap+start+9*letter+8*(k&1);a.y=VertexSnap+14+241*half+8*(k>>1);a.oow=1;a.r=123;a.attributes[3]=37;}
  std::memcpy(before,input,sizeof input);CHECK(nfs_hd_label_geometry(selectors[word][letter],input,input+1,input+2,out)==1);
  CHECK(!std::memcmp(before,input,sizeof input));float shift=-.22f*(int(letter)*9-9);
  for(unsigned k=0;k<3;++k){CHECK(std::fabs(out[k].x-input[k].x-shift)<.04f);out[k].x=input[k].x;CHECK(!std::memcmp(out+k,input+k,sizeof(GrVertex)));}
  nfs_hd_cell_layout_available=0;CHECK(nfs_hd_label_geometry(selectors[word][letter],input,input+1,input+2,out)==0);nfs_hd_cell_layout_available=1;
  for(auto &a:input)a.y+=1;CHECK(nfs_hd_label_geometry(selectors[word][letter],input,input+1,input+2,out)==0);
 }
 for(unsigned half=0;half<2;++half)for(int selector=26;selector<=48;++selector){
  if(selector>26 && selector<38)continue;
  bool raised=selector==38 || selector==47 || selector==48;float width=raised?4:8;
  for(unsigned omitted=0;omitted<4;++omitted){GrVertex input[3]={},out[3],original[3];unsigned n=0;
   for(unsigned k=0;k<4;++k)if(k!=omitted){auto &a=input[n++];a.x=VertexSnap+31+width*(k&1);a.y=VertexSnap+(raised?227:228)+240*half+8*(k>>1);a.oow=1;a.g=117;a.attributes[4]=91;}
   std::memcpy(original,input,sizeof input);CHECK(nfs_hd_label_geometry(selector,input,input+1,input+2,out)==1);CHECK(!std::memcmp(input,original,sizeof input));
   float center=VertexSnap+31+width*.5f;
   for(unsigned k=0;k<3;++k){CHECK(std::fabs(out[k].x-(center-3.5f+(input[k].x-center)*(selector==38?1.5f:1.0f)))<.04f);CHECK(out[k].y==input[k].y+raised);out[k].x=input[k].x;out[k].y=input[k].y;CHECK(!std::memcmp(out+k,input+k,sizeof(GrVertex)));}
  }
 }
 const size_t capacity=4194304;std::vector<unsigned char> image(capacity+32,0xa5),before=image;
 CHECK(!nfs_wide_label_expand(nullptr,capacity));CHECK(!nfs_wide_label_expand(image.data()+16,capacity-1));CHECK(image==before);
 CHECK(nfs_wide_label_expand(image.data()+16,capacity));unsigned alpha=0;
 for(unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x){size_t at=16+((size_t)y*1024+x)*4;bool owned=x<80 && y>=188 && y<252;
  if(owned)alpha+=image[at+3];else for(unsigned c=0;c<4;++c)CHECK(image[at+c]==0xa5);
 }
 CHECK(alpha>0);for(unsigned i=0;i<16;++i)CHECK(image[i]==0xa5 && image[16+capacity+i]==0xa5);
 std::puts("PASS both caption rows/all triangles, natural widths and immutable attributes; wide W strip, capacity and atlas canaries");
}

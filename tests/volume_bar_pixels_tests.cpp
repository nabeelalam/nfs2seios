// SPDX-License-Identifier: MIT
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "PauseHeaderPixels.inc"
#include "VolumeBarPixels.inc"
#define CHECK(v) do{if(!(v)){std::fprintf(stderr,"Volume material check failed at %d\n",__LINE__);std::abort();}}while(0)
static std::vector<unsigned char> independentDecode(const unsigned char*p,size_t bytes){
 std::vector<unsigned char> out;
 for(size_t i=0;i<bytes;i+=6){CHECK(i+6<=bytes);unsigned count=p[i]+256u*p[i+1];CHECK(count && out.size()/4+count<=80u*112u);
  for(unsigned j=0;j<count;++j)out.insert(out.end(),p+i+2,p+i+6);}
 CHECK(out.size()==80u*112u*4u);return out;
}
int main(){
 const size_t capacity=4194304,guard=32;auto pixels=independentDecode(nfs_volume_bar_rle,sizeof(nfs_volume_bar_rle));
 std::vector<unsigned char> image(capacity+2*guard,0xa5);CHECK(nfs_pause_header_expand(image.data()+guard,capacity));auto before=image;
 auto reject=[&](const unsigned char*p,size_t bytes,size_t cap=4194304){CHECK(!nfs_volume_bar_expand_checked(image.data()+guard,cap,p,bytes));CHECK(image==before);};
 CHECK(!nfs_volume_bar_expand(nullptr,capacity));
 reject((const unsigned char*)1,6,capacity-1);reject(nullptr,0);
 std::vector<unsigned char> encoded(nfs_volume_bar_rle,nfs_volume_bar_rle+sizeof(nfs_volume_bar_rle));
 reject(encoded.data(),encoded.size()-1);
 for(unsigned mutation=0;mutation<4;++mutation){auto bad=encoded;
  if(mutation==0)bad[0]=bad[1]=0;
  if(mutation==1)bad[0]=bad[1]=255;
  if(mutation==2)bad.insert(bad.end(),{1,0,0,0,0,0});
  if(mutation==3)bad.erase(bad.end()-6,bad.end());
  reject(bad.data(),bad.size());
 }
 CHECK(nfs_volume_bar_expand(image.data()+guard,capacity));bool clear=false,solid=false,smooth=false;
 for(unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x){size_t at=guard+((size_t)y*1024+x)*4;
  bool inside=x>=920 && x<1000 && y>=398 && y<510;
  for(unsigned c=0;c<4;++c)CHECK(image[at+c]==(inside?pixels[((size_t)(y-398)*80+x-920)*4+c]:before[at+c]));
  if(inside){unsigned a=image[at+3];clear|=a==0;solid|=a==255;smooth|=a>0 && a<255;}
 }
 CHECK(clear && solid && smooth);
 for(unsigned y:{396u,397u,510u,511u})for(unsigned x=0;x<1024;++x)for(unsigned k=0;k<4;++k)CHECK(image[guard+((size_t)y*1024+x)*4+k]==before[guard+((size_t)y*1024+x)*4+k]);
 // Source y0,26,27 remain exterior; no opaque capsule stretches into margins.
 for(unsigned y=0;y<112;++y)for(unsigned x=0;x<80;++x)if(y<4 || y>=104)CHECK(pixels[((size_t)y*80+x)*4+3]==0);
 for(unsigned y=8;y<104;++y)CHECK(pixels[((size_t)y*80+79)*4+3]==255);
 for(size_t i=0;i<guard;++i)CHECK(image[i]==0xa5 && image[guard+capacity+i]==0xa5);
 auto once=image;CHECK(nfs_volume_bar_expand(image.data()+guard,capacity));CHECK(image==once);
 std::puts("PASS volume material independent decode, transparent silhouette, fixed atlas writes, header/gutter preservation and malformed refusal");
}

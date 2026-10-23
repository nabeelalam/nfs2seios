// SPDX-License-Identifier: MIT
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "PauseHeaderPixels.inc"
#define CHECK(v) do{if(!(v)){std::fprintf(stderr,"Pause header check failed at %d\n",__LINE__);std::abort();}}while(0)
static std::vector<unsigned char> decode(const unsigned char *p,size_t n,size_t pixels){
 std::vector<unsigned char> out;
 for(size_t at=0;at<n;at+=6){CHECK(at+6<=n);unsigned run=unsigned(p[at])+256u*p[at+1];CHECK(run && out.size()/4+run<=pixels);
  for(unsigned j=0;j<run;++j)out.insert(out.end(),p+at+2,p+at+6);}
 CHECK(out.size()==pixels*4);return out;
}
int main(){
 const size_t capacity=4194304,guard=32;
 auto title=decode(nfs_pause_header_title_rle,sizeof(nfs_pause_header_title_rle),520u*112u);
 auto caution=decode(nfs_pause_header_caution_rle,sizeof(nfs_pause_header_caution_rle),152u*112u);
 std::vector<unsigned char> image(capacity+2*guard,0xa5),original=image;
 auto unchanged=[&](){CHECK(image==original);};
 CHECK(!nfs_pause_header_expand(nullptr,capacity));
 CHECK(!nfs_pause_header_expand(image.data()+guard,capacity-1));unchanged();
 CHECK(!nfs_pause_header_expand_checked(image.data()+guard,capacity-1,(const unsigned char*)1,6,(const unsigned char*)1,6));unchanged();
 auto badTitle=std::vector<unsigned char>(nfs_pause_header_title_rle,nfs_pause_header_title_rle+sizeof(nfs_pause_header_title_rle));
 auto badCaution=std::vector<unsigned char>(nfs_pause_header_caution_rle,nfs_pause_header_caution_rle+sizeof(nfs_pause_header_caution_rle));
 auto reject=[&](const unsigned char*t,size_t tn,const unsigned char*c,size_t cn){CHECK(!nfs_pause_header_expand_checked(image.data()+guard,capacity,t,tn,c,cn));unchanged();};
 reject(nullptr,0,badCaution.data(),badCaution.size());
 reject(badTitle.data(),badTitle.size(),nullptr,0);
 reject(badTitle.data(),badTitle.size()-1,badCaution.data(),badCaution.size());
 // A bad second stream must not write the otherwise valid first tile.
 reject(badTitle.data(),badTitle.size(),badCaution.data(),badCaution.size()-1);
 for(unsigned family=0;family<2;++family)for(unsigned mutation=0;mutation<5;++mutation){
  auto t=badTitle,c=badCaution;auto&bad=family?c:t;
  if(mutation==0){bad[0]=bad[1]=0;}
  if(mutation==1){bad[0]=bad[1]=255;}
  if(mutation==2){bad[5]=254;}
  if(mutation==3){bad.insert(bad.end(),{1,0,0,0,0,255});}
  if(mutation==4){bad.erase(bad.end()-6,bad.end());}
  reject(t.data(),t.size(),c.data(),c.size());
 }
 CHECK(nfs_pause_header_expand(image.data()+guard,capacity));
 for(unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x){
  size_t at=guard+((size_t)y*1024+x)*4;
  const bool a=x>=240 && x<760 && y>=398 && y<510,b=x>=768 && x<920 && y>=398 && y<510;
  for(unsigned k=0;k<4;++k){
   if(a)CHECK(image[at+k]==title[((size_t)(y-398)*520+x-240)*4+k]);
   else if(b)CHECK(image[at+k]==caution[((size_t)(y-398)*152+x-768)*4+k]);
   else CHECK(image[at+k]==0xa5);
  }
  if(a||b)CHECK(image[at+3]==255);
 }
 for(size_t i=0;i<guard;++i)CHECK(image[i]==0xa5 && image[guard+capacity+i]==0xa5);
 // Both numerical and small-letter GL_LINEAR tolerance gutters stay byte-exact.
 for(unsigned y:{396u,397u,510u,511u})for(unsigned x=0;x<1024;++x)for(unsigned k=0;k<4;++k)CHECK(image[guard+((size_t)y*1024+x)*4+k]==0xa5);
 auto once=image;CHECK(nfs_pause_header_expand(image.data()+guard,capacity));CHECK(once==image);
 std::puts("PASS pause header independent run decode, full-plane/gutter preservation, all-or-nothing malformed rejection and canaries");
}

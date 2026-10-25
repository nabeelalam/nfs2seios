// SPDX-License-Identifier: MIT
#include "WarningWordPixels.inc"
#include "WarningWordSelect.h"
#include <array>
#include <vector>
#include <cmath>
#include <limits>
#include <cstdio>
#include <cstdlib>
#define CK(x) do{if(!(x)){std::fprintf(stderr,"warning check line%d\n",__LINE__);std::abort();}}while(0)
static void pixels(){
 std::vector<unsigned char> alpha;for(size_t i=0;i<sizeof(nfs_warning_word_rle);i+=3){unsigned n=nfs_warning_word_rle[i]|unsigned(nfs_warning_word_rle[i+1])<<8;alpha.insert(alpha.end(),n,nfs_warning_word_rle[i+2]);}CK(alpha.size()==618*57+18*57);
 constexpr unsigned cap=4194304,guard=32;std::vector<unsigned char> image(cap+2*guard),before;
 for(size_t i=0;i<image.size();++i)image[i]=unsigned(i*37+19);before=image;
 CK(!nfs_warning_word_expand(nullptr,cap));CK(!nfs_warning_word_expand_checked(image.data()+guard,cap-1,reinterpret_cast<const unsigned char*>(1),3));CK(image==before);
 auto bad=std::vector<unsigned char>(nfs_warning_word_rle,nfs_warning_word_rle+sizeof(nfs_warning_word_rle));
 for(unsigned mutation=0;mutation<5;++mutation){auto b=bad;if(mutation==0)b.pop_back();if(mutation==1)b[0]=b[1]=0;if(mutation==2)b[2]^=1;if(mutation==3)b.insert(b.end(),{1,0,0});if(mutation==4)b.erase(b.end()-3,b.end());CK(!nfs_warning_word_expand_checked(image.data()+guard,cap,b.data(),b.size()));CK(image==before);}
 CK(nfs_warning_word_expand(image.data()+guard,cap));
 const unsigned banks[5][4]={{0,318,1,963},{318,126,1,129},{444,62,129,1},{506,62,129,61},{568,50,633,1}};
 for(unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x){bool changed=false;unsigned expected=0;for(const auto &b:banks)if(x>=b[2]-1&&x<=b[2]+b[1]&&y>=b[3]-1&&y<=b[3]+57){changed=true;int sx=int(b[0])+int(x)-int(b[2]),sy=int(y)-int(b[3]);if(sx>=0&&sx<618&&sy>=0&&sy<57)expected=alpha[sy*618+sx];}
  if(x>=128&&x<148&&y>=128&&y<187){changed=true;expected=x>=129&&x<147&&y>=129&&y<186?alpha[35226+(y-129)*18+x-129]:0;}
  size_t at=guard+(size_t(y)*1024+x)*4;for(unsigned k=0;k<4;++k)CK(image[at+k]==(changed?(k==3?expected:0):before[at+k]));}
 for(unsigned i=0;i<guard;++i)CK(image[i]==before[i]&&image[guard+cap+i]==before[guard+cap+i]);auto once=image;CK(nfs_warning_word_expand(image.data()+guard,cap)&&image==once);
}
static void selectors(){
 for(unsigned layout=0;layout<3;++layout)for(unsigned a=0;a<4;++a)for(unsigned b=0;b<4;++b)for(unsigned c=0;c<4;++c){float top=layout==1?248.f:7.f;bool single=layout==2;unsigned corners[3]={a,b,c};NfsWarningWordVertex v[3];float uv[6];for(unsigned i=0;i<3;++i){unsigned k=corners[i];v[i]={single?(k&1?539.f:283.f):(k&1?522.f:299.f),top+(k&2?21.f:0.f),1,255,255,255,255};uv[2*i]=k&1?(single?255.f:222.12890625f):.498046875f;uv[2*i+1]=k&2?20.91796875f:.498046875f;}bool complete=a!=b&&b!=c&&a!=c;CK(nfs_warning_word_triangle(v,uv)==(complete?(single?77:76):0));
 if(complete){for(unsigned i=0;i<6;++i){float saved=uv[i];uv[i]=std::nextafter(saved,std::numeric_limits<float>::infinity());CK(!nfs_warning_word_triangle(v,uv));uv[i]=saved;}
  for(unsigned i=0;i<3;++i)for(unsigned f=0;f<7;++f){float *fields[]={&v[i].x,&v[i].y,&v[i].oow,&v[i].r,&v[i].g,&v[i].b,&v[i].a};float *p=fields[f],saved=*p;for(float invalid:{saved+.0625f,std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity()}){*p=invalid;CK(!nfs_warning_word_triangle(v,uv));}*p=saved;}}
 }
 CK(!nfs_warning_word_triangle(nullptr,nullptr));
}
int main(){pixels();selectors();std::puts("PASS exact warning geometry/finite/perspective/color fallback, full-plane/gutter decode, canaries and malformed all-or-nothing");}
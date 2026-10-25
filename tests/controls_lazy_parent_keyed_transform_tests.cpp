// SPDX-License-Identifier: MIT
// Pure synthetic tests; no original assets and no source/native authority.
#include "hd/ControlsLazyParentKeyedTransform.h"
#include <cassert>
#include <limits>
#include <vector>
using namespace nfs_controls_lazy_parent_keyed_transform;
static void put16(std::vector<uint8_t>&v,size_t i,uint16_t p){v[i]=uint8_t(p);v[i+1]=uint8_t(p>>8);}
static void put32(std::vector<uint8_t>&v,size_t i,uint32_t p){for(unsigned k=0;k<4;++k)v[i+k]=uint8_t(p>>(8*k));}
struct Fixture {
 std::vector<uint8_t>s,t,b,a;RowView r;
 Fixture(size_t n,uint32_t key,uint32_t first=0):s(n*2),t(n*4),b(n*2),a(n*2){
  for(size_t i=0;i<n;++i){const uint16_t p=uint16_t(first+i),before=uint16_t(0x5a5au^uint16_t(i*257u));
   // Independent channel formulation, rather than copying verifier masks.
   const uint32_t red=p/2048u,green=(p/32u)%64u,blue=p%32u;
   const uint32_t argb=uint32_t(p)==key?0u:0xff000000u+red*0x80000u+green*0x400u+blue*8u;
   put16(s,i*2,p);put32(t,i*4,argb);put16(b,i*2,before);put16(a,i*2,uint32_t(p)==key?before:p);
  }
  r.wrapper=WrapperId;r.transform=TransformId;r.forward=ForwardId;r.key=key;r.pixels=n;
  r.source565={s.data(),s.size()};r.temporaryArgb={t.data(),t.size()};r.destinationBefore565={b.data(),b.size()};r.destinationAfter565={a.data(),a.size()};
 }
};
int main(){
 // Exercise all source565 values in bounded rows for keys at boundaries, in the
 // middle, and outside the 16-bit range. No large-row exception is introduced.
 for(uint32_t key:{0u,1u,0x7befu,0xffffu,0x10000u,0xffffffffu}){
  for(uint32_t first=0;first<65536u;){const size_t n=65536u-first<MaxPixels?65536u-first:MaxPixels;Fixture f(n,key,first);assert(verify(f.r)==Failure::None);first+=uint32_t(n);}
 }
 Fixture f(MaxPixels,0x1234,0x1200);assert(verify(f.r)==Failure::None);
 for(unsigned which=0;which<3;++which){auto r=f.r;if(which==0)r.wrapper^=1;if(which==1)r.transform^=1;if(which==2)r.forward^=1;assert(verify(r)==Failure::Identity);}
 for(size_t n:{size_t(0),MaxPixels+1,std::numeric_limits<size_t>::max()}){auto r=f.r;r.pixels=n;assert(verify(r)==Failure::Count);}
 for(unsigned which=0;which<4;++which){for(unsigned mode=0;mode<3;++mode){auto r=f.r;Bytes*spans[]={&r.source565,&r.temporaryArgb,&r.destinationBefore565,&r.destinationAfter565};if(mode==0)spans[which]->data=nullptr;if(mode==1)--spans[which]->size;if(mode==2)++spans[which]->size;assert(verify(r)==Failure::Span);}}
 // Every intermediate byte is checked, including all four bytes of the last pixel.
 for(size_t i=0;i<f.t.size();++i){f.t[i]^=1;assert(verify(f.r)==Failure::Temporary);f.t[i]^=1;}
 for(size_t i=0;i<f.a.size();++i){f.a[i]^=1;assert(verify(f.r)==Failure::Destination);f.a[i]^=1;}
 // Keyed destination is retained exactly; setting it to source is not accepted.
 Fixture keyed(1,0x1234,0x1234);assert(verify(keyed.r)==Failure::None);put16(keyed.a,0,0x1234);assert(verify(keyed.r)==Failure::Destination);
 // A plausible nonzero partial alpha cannot be accepted as an opaque conversion.
 Fixture opaque(1,0x10000,0xffff);opaque.t[3]=0x80;assert(verify(opaque.r)==Failure::Temporary);
 // Comparing only low16(key) would incorrectly make this source transparent.
 Fixture highKey(1,0x10001,1);assert(verify(highKey.r)==Failure::None);put32(highKey.t,0,0);assert(verify(highKey.r)==Failure::Temporary);
 Fixture single(1,0xffffffffu,0);assert(verify(single.r)==Failure::None);
}

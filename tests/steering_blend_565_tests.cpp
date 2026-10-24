// SPDX-License-Identifier: MIT
// The generated original-engine config reserves this macro.
#define packed __attribute__((__packed__))
#include "hd/SteeringBlend565.h"
#undef packed
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
using nfs_steering_chart::Blend565;
using nfs_steering_chart::blend565;
static uint64_t checks=0;
#define CHECK(v) do{++checks;if(!(v)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#v);std::exit(1);}}while(0)
// Expanded original-register oracle. It deliberately retains the MOV AX / MOV
// AL partial-register semantics, original instruction order, unsigned MUL's two
// registers, and wrapping ADDs rather than using the helper's packed formula.
static Blend565 originalRegisters(uint32_t source,uint16_t destination,uint32_t incomingEax){
 uint32_t eax=incomingEax,ebx=source,ecx=0,edx=0;
 if(ebx<UINT32_C(0x10000000))return {destination,false};
 if(ebx<UINT32_C(0xfc000000)){
  eax=(eax&UINT32_C(0xffff0000))|destination; // MOV AX,[ESI]
  ecx=ebx;edx=eax;
  ecx^=UINT32_MAX;
  eax<<=16;
  eax=(eax&UINT32_C(0xffffff00))|(edx&UINT32_C(0xff));
  edx&=UINT32_C(0x7e0);
  edx<<=8;
  eax&=UINT32_C(0xf800001f);
  ecx>>=24;
  eax=uint32_t(uint64_t(eax)+edx);
  const uint64_t edxEax=uint64_t(eax)*ecx;
  eax=uint32_t(edxEax);edx=uint32_t(edxEax>>32);
  edx<<=16;
  ecx=eax;
  ecx>>=5;
  ebx=uint32_t(uint64_t(ebx)+edx);
  eax>>=11;
  ecx&=UINT32_C(0xff);
  eax&=UINT32_C(0xff00);
  ebx=uint32_t(uint64_t(ebx)+ecx);
  ebx=uint32_t(uint64_t(ebx)+eax);
 }
 eax=ebx;ebx&=UINT32_C(0xf80000);ebx>>=8;edx=eax;eax>>=3;
 edx&=UINT32_C(0xfc00);edx>>=5;eax&=UINT32_C(0x1f);
 ebx=uint32_t(uint64_t(ebx)+eax);ebx=uint32_t(uint64_t(ebx)+edx);
 return {uint16_t(ebx&UINT32_C(0xffff)),true};
}
// Expanded original483288 instructions, independently keeping its indexed
// palette representation and direct BX store. No converter correctness assumed.
static Blend565 originalIndexedRegisters(uint32_t palette,uint16_t destination){
 uint32_t eax=0,ebx=palette,ecx=0,edx=0;
 if(ebx<UINT32_C(0x10000000))return {destination,false};
 if(ebx<UINT32_C(0xfc000000)){
  eax=0;eax=(eax&UINT32_C(0xffff0000))|destination;
  ecx=ebx;edx=eax;ecx^=UINT32_MAX;eax<<=16;
  eax=(eax&UINT32_C(0xffffff00))|(edx&UINT32_C(0xff));
  edx&=UINT32_C(0x7e0);edx<<=8;eax&=UINT32_C(0xf800001f);
  ecx>>=24;eax=uint32_t(uint64_t(eax)+edx);
  const uint64_t edxEax=uint64_t(eax)*ecx;
  eax=uint32_t(edxEax);edx=uint32_t(edxEax>>32);
  edx<<=8;ecx=eax;eax>>=16;edx&=UINT32_C(0xf800);
  ecx>>=8;eax&=UINT32_C(0x7e0);
  ebx=uint32_t(uint64_t(ebx)+edx);ecx&=UINT32_C(0x1f);
  ebx=uint32_t(uint64_t(ebx)+eax);ebx=uint32_t(uint64_t(ebx)+ecx);
 }
 return {uint16_t(ebx&UINT32_C(0xffff)),true};
}
static void compareIndexed(uint32_t palette,uint16_t destination){
 auto actual=nfs_steering_chart::blendIndexedPalette565(palette,destination);
 auto expected=originalIndexedRegisters(palette,destination);
 CHECK(actual.value==expected.value&&actual.stored==expected.stored);
}
static void indexedCases(){
 using nfs_steering_chart::blendIndexedPalette565;
 for(uint32_t destination=0;destination<=65535;++destination){
  for(uint32_t palette:{0u,UINT32_C(0x0fffffff),UINT32_C(0x10000000),UINT32_C(0x10ffffff),UINT32_C(0xfbffffff),UINT32_C(0xfc000000),UINT32_MAX})compareIndexed(palette,uint16_t(destination));
  for(uint32_t alpha:{0u,1u,15u}){auto result=blendIndexedPalette565((alpha<<24)|UINT32_C(0xffffff),uint16_t(destination));CHECK(!result.stored&&result.value==destination);}
 }
 // Every packed palette color at all opaque alpha values and diverse prior
 // destinations must store BX directly, rather than reinterpret RGB888.
 const std::array<uint16_t,12>destinations{{0,1,31,32,0x7e0,0x800,0xf800,0xffff,0x1234,0x7bef,0x8410,0xf81f}};
 for(uint32_t alpha=252;alpha<256;++alpha)for(uint32_t color=0;color<=65535;++color)for(uint16_t before:destinations){auto result=blendIndexedPalette565((alpha<<24)|color,before);CHECK(result.stored&&result.value==color);compareIndexed((alpha<<24)|color,before);}
 const std::array<uint16_t,18>colors{{0,1,7,8,15,16,31,32,0x7e0,0x7ff,0x800,0xf800,0xffff,0x1234,0x7bef,0x8410,0xf81f,0x07ff}};
 // All alpha/color/destination fields, plus arbitrary bits16..23, exercise
 // packed carries/truncation without promising these are valid palette entries.
 for(unsigned alpha=0;alpha<256;++alpha)for(uint16_t color:colors)for(uint16_t before:colors)for(unsigned middle:{0u,1u,127u,255u})compareIndexed((alpha<<24)|(middle<<16)|color,before);
 auto skipped=blendIndexedPalette565(0,0),stored=blendIndexedPalette565(UINT32_C(0xfc000000),0);CHECK(skipped.value==stored.value&&!skipped.stored&&stored.stored);
 CHECK(blendIndexedPalette565(UINT32_C(0xfc00f800),0).value==0xf800);
 CHECK(blend565(UINT32_C(0xfc00f800),0).value!=0xf800);
}
static uint16_t packOpaque(uint32_t source){return uint16_t(((source>>16)&0xf8)<<8|((source>>8)&0xfc)<<3|(source&0xff)>>3);}
static void compare(uint32_t source,uint16_t before,uint32_t seed=UINT32_C(0xdeadbeef)){
 auto actual=blend565(source,before),expected=originalRegisters(source,before,seed);
 CHECK(actual.value==expected.value&&actual.stored==expected.stored);
}
int main(){
 // Exact cutoff boundaries, including every possible destination. Skipping is
 // distinct from storing an equal value and must preserve the prior word.
 for(uint32_t destination=0;destination<=65535;++destination){for(uint32_t source:{0u,UINT32_C(0x0fffffff),UINT32_C(0x10000000),UINT32_C(0x10000001),UINT32_C(0xfbffffff),UINT32_C(0xfc000000),UINT32_MAX})compare(source,uint16_t(destination));
  for(uint32_t alpha:{0u,1u,15u}){auto result=blend565(alpha<<24|UINT32_C(0xffffff),uint16_t(destination));CHECK(!result.stored&&result.value==destination);}
 }
 // All opaque-alpha destinations and independent straight RGB-to565 packing.
 const std::array<uint32_t,12>colors{{0,1,7,8,0xff,0xff00,0xff0000,0xffffff,0x123456,0x808080,0xf8fcf8,0x070307}};
 for(uint32_t alpha=252;alpha<256;++alpha)for(uint32_t color:colors)for(uint32_t destination=0;destination<=65535;++destination){const uint32_t source=alpha<<24|color;auto result=blend565(source,uint16_t(destination));CHECK(result.stored&&result.value==packOpaque(color));compare(source,uint16_t(destination));}
 // Every alpha, color thresholds around 5/6-bit packing boundaries, diverse
 // destination fields, and intentionally non-premultiplied extremes establish
 // the original overflow/truncation behavior without assuming valid palettes.
 const std::array<unsigned,12>channel{{0,1,3,7,8,15,16,31,63,127,248,255}};
 const std::array<uint16_t,12>destinations{{0,1,31,32,0x7e0,0x800,0xf800,0xffff,0x1234,0x7bef,0x8410,0xf81f}};
 for(unsigned alpha=0;alpha<256;++alpha)for(unsigned red:channel)for(unsigned green:channel)for(unsigned blue:channel){const uint32_t source=(alpha<<24)|(red<<16)|(green<<8)|blue;for(uint16_t before:destinations)compare(source,before);}
 // Independent incoming upper EAX bits cannot leak through MOV AX then SHL16.
 for(uint32_t seed:{0u,UINT32_MAX,UINT32_C(0x12340000),UINT32_C(0xabcdffff)})for(uint32_t source:{UINT32_C(0x10ffffff),UINT32_C(0x7f804020),UINT32_C(0xfbffffff)})for(uint16_t before:destinations)compare(source,before,seed);
 // An equal resulting pixel may still require an original store.
 auto skipped=blend565(0,0),stored=blend565(UINT32_C(0xfc000000),0);CHECK(skipped.value==stored.value&&!skipped.stored&&stored.stored);
 // Premultiplied black at alpha251 retains tiny original destination bits;
 // alpha252 takes the opaque branch. Guard the transition against rounding.
 CHECK(blend565(UINT32_C(0xfb000000),0xffff).value==originalRegisters(UINT32_C(0xfb000000),0xffff,0).value);
 CHECK(blend565(UINT32_C(0xfc000000),0xffff).value==0);
 indexedCases();
 std::printf("PASS steering original565 blend checks=%llu pure-arithmetic=483070+483288 actual-palette-premultiplication=unverified chart-source-proof=none\n",static_cast<unsigned long long>(checks));
}

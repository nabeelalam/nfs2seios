// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>
namespace nfs_steering_chart {
// Pure arithmetic corresponding to original483070's 32-bit-source/565 row.
// The input is the original source32 word; asset alpha semantics are separate.
// Indexed483288 uses a different packed565 palette representation and is not
// supported by this483070 helper. No actual chart callback route is assumed.
// This helper grants no owner, source-write, chart or native-renderer authority.
// Palette conversion and asset alpha semantics require their own evidence.
struct Blend565 {uint16_t value;bool stored;};
inline Blend565 blend565(uint32_t sourceWord,uint16_t before)noexcept{
 if(sourceWord<UINT32_C(0x10000000))return {before,false};
 uint32_t mixed=sourceWord;
 if(sourceWord<UINT32_C(0xfc000000)){
  // Original SHL EAX,16 / MOV AL,DL / masks interleave the destination's
  // red, green and blue fields before the unsigned EDX:EAX multiplication.
  const uint32_t destinationFields=((uint32_t(before)<<16)&UINT32_C(0xf8000000))+
                        ((uint32_t(before)&UINT32_C(0x07e0))<<8)+
                        (uint32_t(before)&UINT32_C(0x001f));
  const uint64_t product=uint64_t(destinationFields)*(255u-(sourceWord>>24));
  const uint32_t low=uint32_t(product),high=uint32_t(product>>32);
  // Keep the original low32 truncation of SHL EDX,16 and each ADD. Do not
  // replace this packed arithmetic with rounded or clamped channel blending.
  mixed+=high<<16;
  mixed+=(low>>5)&UINT32_C(0xff);
  mixed+=(low>>11)&UINT32_C(0xff00);
 }
 const uint32_t packed565=((mixed&UINT32_C(0xf80000))>>8)+
                          ((mixed>>3)&UINT32_C(0x1f))+
                          ((mixed&UINT32_C(0xfc00))>>5);
 return {uint16_t(packed565),true};
}
// Independent original483288 indexed-row arithmetic AFTER the caller resolves
// the current palette entry. The low16 palette field contains packed
// RGB565, while the upper byte supplies alpha; this helper assumes no converter
// multiplication or asset alpha convention. This function neither resolves
// nor authenticates a palette, converter, index, source row or actual chart.
inline Blend565 blendIndexedPalette565(uint32_t paletteEntry,uint16_t before)noexcept{
 if(paletteEntry<UINT32_C(0x10000000))return {before,false};
 uint32_t mixed=paletteEntry;
 if(paletteEntry<UINT32_C(0xfc000000)){
  const uint32_t destinationFields=((uint32_t(before)<<16)&UINT32_C(0xf8000000))+
                        ((uint32_t(before)&UINT32_C(0x07e0))<<8)+
                        (uint32_t(before)&UINT32_C(0x001f));
  const uint64_t product=uint64_t(destinationFields)*(255u-(paletteEntry>>24));
  const uint32_t low=uint32_t(product),high=uint32_t(product>>32);
  // Unlike483070, all three contributions are already destination565 fields.
  // Preserve wrapping ADD and low16 store; there is no final RGB888 conversion.
  mixed+=(high<<8)&UINT32_C(0xf800);
  mixed+=(low>>16)&UINT32_C(0x07e0);
  mixed+=(low>>8)&UINT32_C(0x001f);
 }
 return {uint16_t(mixed),true};
}
} // namespace nfs_steering_chart

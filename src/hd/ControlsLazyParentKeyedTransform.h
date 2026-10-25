// SPDX-License-Identifier: MIT
#pragma once
#include <cstddef>
#include <cstdint>
namespace nfs_controls_lazy_parent_keyed_transform {
// A numeric consistency check only. No Game, source ownership or native authority.
// The observed Controls sidebar is 343 pixels wide; wider unrelated draws are
// deliberately outside this contract, even though the original stack is larger.
constexpr size_t MaxPixels=343;
constexpr uint32_t WrapperId=0xf000211cu, TransformId=0xf0001f44u, ForwardId=0xf0001844u;
struct Bytes { const uint8_t* data=nullptr;size_t size=0; };
struct RowView {
 uint32_t wrapper=0,transform=0,forward=0,key=0;
 size_t pixels=0;
 Bytes source565{},temporaryArgb{},destinationBefore565{},destinationAfter565{};
};
enum class Failure {None,Identity,Count,Span,Temporary,Destination};
inline uint16_t le16(const uint8_t*p)noexcept{return uint16_t(uint16_t(p[0])|(uint16_t(p[1])<<8));}
inline uint32_t le32(const uint8_t*p)noexcept{return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
inline Failure verify(const RowView&r)noexcept{
 if(r.wrapper!=WrapperId||r.transform!=TransformId||r.forward!=ForwardId)return Failure::Identity;
 // Bounds precede all multiplication, pointer arithmetic and data access.
 if(!r.pixels||r.pixels>MaxPixels)return Failure::Count;
 const size_t sourceBytes=r.pixels*2,temporaryBytes=r.pixels*4;
 auto span=[](Bytes b,size_t n){return b.data&&b.size==n;};
 if(!span(r.source565,sourceBytes)||!span(r.temporaryArgb,temporaryBytes)||!span(r.destinationBefore565,sourceBytes)||!span(r.destinationAfter565,sourceBytes))return Failure::Span;
 for(size_t i=0;i<r.pixels;++i){
  const uint32_t pixel=le16(r.source565.data+i*2);
  // Original 499A88 compares the zero-extended source word to the full key32.
  // There is no bit replication, and a key greater than FFFF never matches.
  const uint32_t expanded=pixel==r.key?0u:0xff000000u|((pixel&0xf800u)<<8)|((pixel&0x07e0u)<<5)|((pixel&0x001fu)<<3);
  if(le32(r.temporaryArgb.data+i*4)!=expanded)return Failure::Temporary;
  // 483070 skips zero and repacks the opaque expansion exactly. Its intermediate
  // alpha branch cannot occur for a correctly validated 499A88 result.
  const uint16_t expected=pixel==r.key?le16(r.destinationBefore565.data+i*2):uint16_t(pixel);
  if(le16(r.destinationAfter565.data+i*2)!=expected)return Failure::Destination;
 }
 return Failure::None;
}
} // namespace nfs_controls_lazy_parent_keyed_transform

// SPDX-License-Identifier: MIT
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

// Pure replay of bounded, caller-owned PlayerGraphPhotoCapture observations.
// A match is NOT a verified source or rendering/upload authorization. In
// particular the 32-bit converted palette's provenance is not authenticated
// here: its entries are observed inputs to row replay, not trusted asset colors.
namespace nfs_player_graph_replay {
enum class Failure {None,Domain,Canvas,Recipe,Fill,Asset,Table,Palette,Row,Pixel,Allocation};
struct Result {
 Failure failure=Failure::None;unsigned phase=0,draw=0,row=0;
 bool matched=false;
 // Intentionally always false until a separately reviewed palette-production
 // proof binds off_4DD044 to each original indexed asset's embedded palette.
 bool sourceAuthenticated=false;
 bool dispatchTableTailsAuthenticated=false;
};
inline uint32_t le(const uint8_t*p,unsigned bytes)noexcept{uint32_t v=0;for(unsigned k=0;k<bytes;++k)v|=uint32_t(p[k])<<(8*k);return v;}
inline void put(uint8_t*p,uint32_t v,unsigned bytes)noexcept{for(unsigned k=0;k<bytes;++k)p[k]=uint8_t(v>>(8*k));}
inline uint16_t pack565(uint32_t c)noexcept{return uint16_t(((c>>8)&0xf800)|((c>>5)&0x7e0)|((c>>3)&31));}
inline uint16_t blend565(uint16_t old,uint32_t c)noexcept{
 if(c<0x10000000u)return old;
 if(c>=0xfc000000u)return uint16_t(c);
 const uint32_t spread=(old&31u)|((old&0x7e0u)<<8)|((old&0xf800u)<<16);
 const uint64_t p=uint64_t(spread)*(255-(c>>24));
 return uint16_t(c+(((p>>32)<<8)&0xf800)+((p>>16)&0x7e0)+((p>>8)&31));
}
inline uint32_t blend32(uint32_t old,uint32_t src)noexcept{
 const unsigned a=src>>24,b=old>>24;if(!a)return old;if(!b)return src;
 uint32_t out=uint32_t(255-(((255-a)*(255-b))>>8))<<24;
 for(unsigned shift:{0u,8u,16u})out|=uint32_t((((src>>shift)&255)+((((old>>shift)&255)*(255-a))>>8))&255)<<shift;
 return out;
}
inline std::array<uint32_t,4>statistics(unsigned car)noexcept{
 static constexpr uint32_t v[4][12]={{10,12,1,11,7,5,9,6,3,4,2,8},{12,8,4,11,5,3,10,7,1,6,2,9},{12,8,5,2,11,4,3,10,7,6,9,1},{9,8,4,6,12,5,1,11,3,10,7,2}};
 std::array<uint32_t,4>out{};if(car<12)for(unsigned k=0;k<4;++k)out[k]=v[k][car];return out;
}
inline bool tablePrefix(const std::array<uint32_t,9>&t)noexcept{
 static constexpr uint32_t prefix[]={0xf000006c,0xf000183c,0xf0001840,0xf0001844,0xf0001848,0xf000184c};
 for(unsigned k=0;k<6;++k)if(t[k]!=prefix[k])return false;
 // Tail transition semantics have not yet been source-reviewed. Do not turn
 // an observed/guessed transition into a canonical acceptance rule. Result
 // explicitly records that this part of source authentication is missing.
 return true;
}
template<class Canvas>bool canvas(const Canvas&c,unsigned bits)noexcept{
 const auto&w=c.words;const unsigned stride=bits/8;
 if(c.bits!=bits||w[0]!=0x57494e44||w[1]!=384||w[2]!=160||w[3]||w[4]||w[5]!=384||w[6]!=160||(w[7]&255)!=bits||!w[8]||w[10]!=384*stride||!w[11]||!w[12]||c.header[1]!=(384u|(160u<<16))||c.pixels.size()!=384*160*stride||uint64_t(w[8])+16+c.pixels.size()>UINT64_C(0x100000000))return false;
 for(unsigned x=0;x<384;++x)if(c.x[x]!=x*stride)return false;
 for(unsigned y=0;y<160;++y)if(c.y[y]!=16+y*384*stride)return false;
 return true;
}
template<class Capture>Result replay(const Capture&c)noexcept{
 Result result;auto reject=[&](Failure f){result.failure=f;return result;};
 if(!c.game||!c.thread||!c.epoch||!c.archive||!c.formGuest||(c.form!=0x4d3050&&c.form!=0x4d3100)||c.car>=12)return reject(Failure::Domain);
 try{for(unsigned phase=0;phase<2;++phase){
  result.phase=phase;const auto&p=c.photos[phase];const unsigned bits=phase?16:32,stride=bits/8;
  if(!canvas(p.before,bits)||!canvas(p.after,bits)||p.before.words!=p.after.words||p.before.header!=p.after.header||p.before.x!=p.after.x||p.before.y!=p.after.y)return reject(Failure::Canvas);
  if(!p.colorObserved||!p.fillObserved||!p.consumedObserved||p.consumed.selectedCar!=c.car||p.consumed.cachedCarIndex!=c.car||p.consumed.graphEnabled!=1||p.consumed.graphValues!=statistics(c.car)||p.draws.size()!=5)return reject(Failure::Recipe);
  std::vector<uint8_t>pixels=p.before.pixels;const uint64_t base=uint64_t(p.before.words[8])+16;
  static constexpr std::array<uint32_t,9>fillTable{{0xf0001aa0,0xf0001aa4,0xf0001aa8,0xf0001aa8,0xf0001aac,0xf0001ab0,0,0,0}};
  const uint32_t fillColor=phase?0:0xff000000u;
  if(p.rawColor!=0xff000000u||p.packedColor!=fillColor||p.fillColor!=fillColor||p.fillArgs!=std::array<uint32_t,4>{{0,0,280,128}}||p.fillTableBefore!=fillTable||p.fillTableAfter!=fillTable||p.fillRows.size()!=128)return reject(Failure::Fill);
  for(unsigned y=0;y<128;++y){result.row=y;const auto&r=p.fillRows[y];
   if(r.callback!=(phase?0xf0001aa8u:0xf0001ab0u)||r.format!=bits||r.converter||r.args[1]!=base+y*384*stride||r.args[2]!=280||r.before.size()!=280*stride||r.after.size()!=280*stride||!r.sourceBefore.empty()||!r.sourceAfter.empty())return reject(Failure::Row);
   for(unsigned x=0;x<280;++x){const size_t at=(y*384+x)*stride;if(le(r.before.data()+x*stride,stride)!=le(pixels.data()+at,stride)||le(r.after.data()+x*stride,stride)!=fillColor)return reject(Failure::Pixel);put(pixels.data()+at,fillColor,stride);}
  }
  static const char names[5][5]={"grph","grpg","grpb","grpr","grpy"};
  for(unsigned k=0;k<5;++k){result.draw=k;result.row=0;const auto&d=p.draws[k];const auto&raw=d.sourceBefore;
   const int x=k?26-9*(12-int(p.consumed.graphValues[k-1])):0,y=k?11+30*int(k-1):0;
   if(std::memcmp(d.name.data(),names[k],4)||d.args[1]!=0x3000||int32_t(d.args[2])!=x||int32_t(d.args[3])!=y||!d.args[0]||raw.size()<32||raw.size()>16+640*480+3+16+256*4||raw!=d.sourceAfter||uint64_t(d.args[0])+raw.size()>UINT64_C(0x100000000))return reject(Failure::Asset);
   if(phase&&raw!=c.photos[0].draws[k].sourceBefore)return reject(Failure::Asset);
   const uint32_t first=le(raw.data(),4),wh=le(raw.data()+4,4),width=wh&65535,height=wh>>16,offset=first>>8;
   const uint64_t end=16+uint64_t(width)*height;
   if((first&255)!=0x7b||!width||width>640||!height||height>480||offset<end||offset-end>3||uint64_t(offset)+16>raw.size())return reject(Failure::Asset);
   const unsigned count=le(raw.data()+offset+4,2);
   if((le(raw.data()+offset,4)&255)!=0x2a||!count||count>256||uint64_t(offset)+16+4*count>raw.size()||raw.size()-(uint64_t(offset)+16+4*count)>3)return reject(Failure::Palette);
   if(!tablePrefix(d.tableBefore)||!tablePrefix(d.tableAfter))return reject(Failure::Table);
   const int left=std::max(0,x),top=std::max(0,y),right=std::min(384,x+int(width)),bottom=std::min(160,y+int(height));
   const unsigned rows=left<right&&top<bottom?unsigned(bottom-top):0;
   if(d.rows.size()!=rows)return reject(Failure::Row);
   for(unsigned yy=0;yy<rows;++yy){result.row=yy;const auto&r=d.rows[yy];const unsigned n=unsigned(right-left),sy=unsigned(top+int(yy)-y),sx=unsigned(left-x);const size_t source=16+size_t(sy)*width+sx;
    if(r.callback!=(phase?0xf0001850u:0xf000211cu)||r.format!=bits||r.converter!=(phase?0u:0xf0002218u)||(!phase&&r.forward!=0xf000184c)||!r.sourceSpanKnown||r.args[0]!=uint64_t(d.args[0])+source||r.args[1]!=base+uint64_t((top+int(yy))*384+left)*stride||r.args[2]!=n||r.sourceBefore.size()!=n||r.sourceBefore!=r.sourceAfter||std::memcmp(r.sourceBefore.data(),raw.data()+source,n)||r.before.size()!=n*stride||r.after.size()!=n*stride||!r.palette||r.paletteBefore!=r.paletteAfter)return reject(Failure::Row);
    if(phase){for(unsigned index=0;index<count;++index){const uint32_t original=le(raw.data()+offset+16+4*index,4);if(r.paletteBefore[index]!=((original&0xff000000u)|pack565(original)))return reject(Failure::Palette);}if(count<256&&r.paletteBefore[255])return reject(Failure::Palette);}
    for(unsigned xx=0;xx<n;++xx){const unsigned index=r.sourceBefore[xx];if(index>=count&&index!=255)return reject(Failure::Palette);const size_t at=size_t((top+int(yy))*384+left+int(xx))*stride;const uint32_t old=le(pixels.data()+at,stride),want=phase?blend565(uint16_t(old),r.paletteBefore[index]):blend32(old,r.paletteBefore[index]);if(le(r.before.data()+xx*stride,stride)!=old||le(r.after.data()+xx*stride,stride)!=want)return reject(Failure::Pixel);put(pixels.data()+at,want,stride);}
   }
  }
  if(pixels!=p.after.pixels)return reject(Failure::Pixel);
 }}catch(...){return reject(Failure::Allocation);}
 result.matched=true;return result;
}
} // namespace nfs_player_graph_replay

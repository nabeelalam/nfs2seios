// SPDX-License-Identifier: MIT
#pragma once
#include "SetupBadgeData.h"
#include "CompletedTrackWordData.h"
#include <vector>
#include <array>
#include <cstring>
namespace nfs_location_completed_flag {
inline const std::vector<unsigned char>& cloth() {
 static const std::vector<unsigned char> pixels = [] {
  constexpr size_t bytes = 444 * 344 * 4;
  static_assert(sizeof nfs_setup_trnm_rle % 6 == 0,
                "Cloth artwork needs complete run/RGBA records");
  std::vector<unsigned char> decoded;
  decoded.reserve(bytes);
  for (size_t at = 0; at < sizeof nfs_setup_trnm_rle; at += 6) {
   const auto* record = nfs_setup_trnm_rle + at;
   const unsigned run = record[0] | unsigned(record[1]) << 8;
   if (!run || run > (bytes - decoded.size()) / 4 ||
       record[2] > record[5] || record[3] > record[5] || record[4] > record[5])
    return std::vector<unsigned char>{};
   for (unsigned j = 0; j < run; ++j)
    decoded.insert(decoded.end(), record + 2, record + 6);
  }
  if (decoded.size() != bytes) return std::vector<unsigned char>{};
  return decoded;
 }();
 return pixels;
}
// Decode all six finite authored words once; refreshes reuse immutable alpha.
inline const std::vector<unsigned char>& completedAlpha(unsigned index = 0) {
 static const std::array<std::vector<unsigned char>,6> pixels = [] {
  std::array<std::vector<unsigned char>,6> decoded;
  for(unsigned index=0;index<6;++index){
   const auto& asset=nfs_completed_words[index];auto& word=decoded[index];
   const size_t bytes=size_t(asset.width)*asset.height;
   if(asset.count%2)continue;word.reserve(bytes);
   for(size_t k=0;k<asset.count;k+=2){
    const unsigned run=asset.rle[k],alpha=asset.rle[k+1];
    if(!run||alpha>255||run>bytes-word.size()){word.clear();break;}
    word.insert(word.end(),run,static_cast<unsigned char>(alpha));
   }
   if(word.size()!=bytes)word.clear();
  }
  return decoded;
 }();
 static const std::vector<unsigned char> empty;
 return index<6?pixels[index]:empty;
}
inline int completedWord(const unsigned char*name,const unsigned char*sha){
 if(!name||!sha)return -1;
 for(unsigned k=0;k<6;++k)if(!std::memcmp(name,nfs_completed_words[k].name,4)&&
                            !std::memcmp(sha,nfs_completed_words[k].source,32))return int(k);
 return -1;
}
inline void blendCompletedWord(std::vector<unsigned char>& surface,
                               unsigned width, unsigned height,
                               int left, int top, unsigned index=0) {
 const auto& alpha=completedAlpha(index);if(alpha.empty()||index>=6)return;
 const auto& word=nfs_completed_words[index];
 // One logical pixel black shadow followed by white glyphs, both kept within
 // this locale's exact original bitmap canvas and original source clipping.
 for(unsigned pass=0;pass<2;++pass){
  const unsigned offset=pass?0:4;
  for(unsigned y=0;y+offset<word.height;++y)for(unsigned x=0;x+offset<word.width;++x){
   const unsigned coverage=alpha[size_t(y)*word.width+x];if(!coverage)continue;
   const int xx=left+int(x+offset),yy=top+int(y+offset);
   if(xx<0||yy<0||xx>=int(width)||yy>=int(height))continue;
   auto*destination=surface.data()+4*(size_t(yy)*width+unsigned(xx));
   const unsigned opacity=pass?coverage:(coverage*210+127)/255;
   for(unsigned channel=0;channel<4;++channel){
    const unsigned source=(pass||channel==3)?opacity:0;
    destination[channel]=static_cast<unsigned char>(source+(destination[channel]*(255-opacity)+127)/255);
   }
  }
 }
}

// Called ONLY after unchanged whole original fill/draw/row replay acceptance.
// Original post/pixels/coverage remain intact for all owner/lifetime proofs.
template<class I,class P>std::vector<unsigned char>prepare(const I&i,const P&p){
 static const unsigned char expected[32]={0x7d,0xc9,0x74,0x14,0xe0,0x60,0x6f,0xd5,0x1d,0xbe,0xeb,0x8d,0x6b,0x33,0x9b,0x3d,0x09,0xe2,0xda,0xd7,0x3f,0x09,0xe2,0x05,0x12,0x05,0xd6,0xe9,0x01,0x62,0x09,0xc4};
 if(i.draws.size()!=3||std::memcmp(i.draws[1].name.data(),"trnm",4)||std::memcmp(i.draws[1].sha.data(),expected,32))return {};
 const auto&r=p.bounds();if(!r.width||!r.height||r.width>640||r.height>480)return {};
 const auto&f=i.draws[1];if(f.args[2]!=392||f.args[3]!=361)return {}; // exact Location source bases211/316
 std::vector<unsigned char>v(size_t(r.width)*r.height*16*4);const auto&tex=cloth();if(tex.empty())return {};
 const unsigned W=r.width*4,H=r.height*4;
 auto store=[&](int x,int y,unsigned value,unsigned alpha,bool blend){
  if(x<int(r.left)||y<int(r.top)||x>=int(r.left+r.width)||y>=int(r.top+r.height))return;
  unsigned R=(value>>11)&31,G=(value>>5)&63,B=value&31;unsigned c[4]={((R<<3)|(R>>2))*alpha/255,((G<<2)|(G>>4))*alpha/255,((B<<3)|(B>>2))*alpha/255,alpha};
  for(unsigned dy=0;dy<4;++dy)for(unsigned dx=0;dx<4;++dx){auto*q=v.data()+4*((unsigned(y-r.top)*4+dy)*W+unsigned(x-r.left)*4+dx);for(unsigned k=0;k<4;++k)q[k]=static_cast<unsigned char>(blend?c[k]+(q[k]*(255-alpha)+127)/255:c[k]);}
 };
 // Rebuild pre-flag fill/track prefix, never paint on the old baked flag.
 for(unsigned y=316;y<444;++y)for(unsigned x=211;x<491;++x)store(x,y,0,255,false);
 const auto&track=i.draws[0];for(const auto&row:track.rows){size_t at=(uint64_t(row.args[1])-(uint64_t(track.before.w[8])+16))/2;unsigned x=at%640,y=at/640;for(size_t k=0;k<row.after.size();++k)store(int(x+k),int(y),row.after[k],255,false);}
 // The authored cloth is a separate PMA surface at its ACTUAL resolved draw.
 for(unsigned y=0;y<344;++y)for(unsigned x=0;x<444;++x){int xx=int(f.args[2]*4+x)-int(r.left*4),yy=int(f.args[3]*4+y)-int(r.top*4);if(xx<0||yy<0||xx>=int(W)||yy>=int(H))continue;const auto*s=tex.data()+4*(y*444+x);auto*d=v.data()+4*(yy*W+xx);for(unsigned k=0;k<4;++k)d[k]=static_cast<unsigned char>(s[k]+(d[k]*(255-s[3])+127)/255);}
 // Completion label stays ABOVE cloth. Names, casing and diacritics are
 // independently checked against the six original localized bitmap images.
 const auto&label=i.draws[2];const int word=completedWord(label.name.data(),label.sha.data());
 if(word>=0&&label.args[2]==392&&label.args[3]==326){
  if(completedAlpha(unsigned(word)).empty())return {};
  blendCompletedWord(v,W,H,int(label.args[2]*4)-int(r.left*4),
                     int(label.args[3]*4)-int(r.top*4),unsigned(word));
 }else for(const auto&row:label.rows){size_t at=(uint64_t(row.args[1])-(uint64_t(label.before.w[8])+16))/2;unsigned x=at%640,y=at/640;for(size_t k=0;k<row.source.size();++k){auto raw=row.paletteBefore[row.source[k]];store(int(x+k),int(y),raw&65535,raw>>24,true);}}
 return v;
}
}

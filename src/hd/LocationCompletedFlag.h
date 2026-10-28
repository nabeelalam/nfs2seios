// SPDX-License-Identifier: MIT
#pragma once
#include "SetupBadgeData.h"
#include <vector>
#include <array>
#include <cstring>
namespace nfs_location_completed_flag {
inline std::vector<unsigned char>cloth(){
 std::vector<unsigned char>v;v.reserve(444*344*4);for(size_t at=0;at<sizeof nfs_setup_trnm_rle;at+=6){if(sizeof nfs_setup_trnm_rle-at<6)return {};const auto*p=nfs_setup_trnm_rle+at;unsigned n=p[0]|unsigned(p[1])<<8;if(!n||n>(444*344*4-v.size())/4||p[2]>p[5]||p[3]>p[5]||p[4]>p[5])return {};for(unsigned j=0;j<n;++j)v.insert(v.end(),p+2,p+6);}if(v.size()!=444*344*4)return {};return v;
}
// Called ONLY after unchanged whole original fill/draw/row replay acceptance.
// Original post/pixels/coverage remain intact for all owner/lifetime proofs.
template<class I,class P>std::vector<unsigned char>prepare(const I&i,const P&p){
 static const unsigned char expected[32]={0x7d,0xc9,0x74,0x14,0xe0,0x60,0x6f,0xd5,0x1d,0xbe,0xeb,0x8d,0x6b,0x33,0x9b,0x3d,0x09,0xe2,0xda,0xd7,0x3f,0x09,0xe2,0x05,0x12,0x05,0xd6,0xe9,0x01,0x62,0x09,0xc4};
 if(i.draws.size()!=3||std::memcmp(i.draws[1].name.data(),"trnm",4)||std::memcmp(i.draws[1].sha.data(),expected,32))return {};
 const auto&r=p.bounds();if(!r.width||!r.height||r.width>640||r.height>480)return {};
 const auto&f=i.draws[1];if(f.args[2]!=392||f.args[3]!=361)return {}; // exact Location source bases211/316
 std::vector<unsigned char>v(size_t(r.width)*r.height*16*4);auto tex=cloth();if(tex.empty())return {};
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
 // Original localized completion label stays ABOVE the cloth, same coordinates,
 // source alpha and original palette. No English-only word substitution.
 const auto&label=i.draws[2];for(const auto&row:label.rows){size_t at=(uint64_t(row.args[1])-(uint64_t(label.before.w[8])+16))/2;unsigned x=at%640,y=at/640;for(size_t k=0;k<row.source.size();++k){auto raw=row.paletteBefore[row.source[k]];store(int(x+k),int(y),raw&65535,raw>>24,true);}}
 return v;
}
}

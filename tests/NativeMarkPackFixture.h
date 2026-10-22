// SPDX-License-Identifier: MIT
#pragma once
#include "HdArtworkPackFixture.h"
#include "hd/NativeMarkProvider.h"
#include <limits>
static uint64_t read64(const Bytes&b,size_t p){uint64_t v=0;for(unsigned i=0;i<8;++i)v|=uint64_t(b.at(p+i))<<(8*i);return v;}
struct NativeFixture {
 Bytes bytes;std::vector<size_t>at;
 NativeFixture(unsigned textures=2,unsigned bindings=3,bool mixed=false,unsigned mapEdge=0,unsigned secondMapEdge=0){
  Fixture original;std::vector<Bytes>parts;
  for(unsigned i=0;i<24;++i){const auto off=read64(original.bytes,72+32*i),n=read64(original.bytes,80+32*i);parts.emplace_back(original.bytes.begin()+off,original.bytes.begin()+off+n);}
  for(unsigned i=0;i<2;++i){const unsigned edge=i?secondMapEdge:mapEdge;if(!edge)continue;auto&t=parts[i+1];t.resize(112+size_t(edge)*edge*4);p32(t,0,edge);p32(t,4,edge);p64(t,40,size_t(edge)*edge*4);for(size_t k=112;k<t.size();k+=4){t[k]=60;t[k+1]=30;t[k+2]=10;t[k+3]=255;}CHECK(nfs_texture_sha256(t.data()+112,t.size()-112,t.data()+48));}
  Bytes keys(16+64*bindings);p32(keys,0,1);p32(keys,4,bindings);p32(keys,8,64);
  for(unsigned i=0;i<bindings;++i){const size_t q=16+64*i;p32(keys,q,1);const bool cursor=mixed&&i%textures==1;p32(keys,q+4,cursor?2:1);p32(keys,q+8,cursor?32:128);p32(keys,q+12,cursor?2048:32768);p32(keys,q+16,i%textures);p32(keys,q+20,cursor?32:128);p32(keys,q+24,cursor?32:128);fill(keys,q+32,uint8_t(0x31+i));}parts.push_back(std::move(keys));
  for(unsigned i=0;i<textures;++i){const unsigned edge=mixed&&i==1?128:512;Bytes t(112+edge*edge*4);p32(t,0,edge);p32(t,4,edge);p32(t,8,1);p32(t,12,i);p32(t,16,1);p64(t,32,0xabc+i);p64(t,40,edge*edge*4);
   // Synthetic PMA: distinct opaque pixel plus transparent padding.
   t[112]=uint8_t(20+i);t[113]=30;t[114]=40;t[115]=255;CHECK(nfs_texture_sha256(t.data()+112,t.size()-112,t.data()+48));parts.push_back(std::move(t));}
  bytes.resize(64+32*parts.size());std::memcpy(bytes.data(),mixed?"NFSHDPK3":"NFSHDPK2",8);p32(bytes,8,mixed?3:2);p32(bytes,12,64);p32(bytes,24,unsigned(parts.size()));p32(bytes,28,32);p64(bytes,32,64);
  for(unsigned i=0;i<parts.size();++i){const unsigned tag=i==0?1:i<=5?2:i==6?3:i<=18?4:i<=23?5:i==24?6:7;const unsigned role=i==0?0:i<=5?i-1:i==6?0:i<=18?i-6:i<=23?i-19:i==24?0:i-25;at.push_back(bytes.size());p32(bytes,64+32*i,tag);p32(bytes,68+32*i,role);p64(bytes,72+32*i,bytes.size());p64(bytes,80+32*i,parts[i].size());bytes.insert(bytes.end(),parts[i].begin(),parts[i].end());}p64(bytes,16,bytes.size());
 }
};

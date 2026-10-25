#pragma once
// SPDX-License-Identifier: MIT
// Entirely synthetic artwork; no original pixels, game files or local paths.
#include "AudioArtworkPackFixture.h"
#include <limits>
static uint32_t read32(const Bytes&b,size_t p){uint32_t v=0;for(unsigned i=0;i<4;++i)v|=uint32_t(b.at(p+i))<<(8*i);return v;}
struct FeedbackFixture {
 Bytes bytes;std::array<size_t,2>imageAt{};size_t imageTable=0;
 explicit FeedbackFixture(bool include=true){
  AudioPackFixture base;const unsigned count=read32(base.bytes,24);std::vector<Bytes>parts;
  for(unsigned i=0;i<count;++i){size_t at=size_t(read64(base.bytes,72+32*i)),n=size_t(read64(base.bytes,80+32*i));parts.emplace_back(base.bytes.begin()+at,base.bytes.begin()+at+n);}
  if(include)for(unsigned k=0;k<2;++k){unsigned w=k?468:1376,h=k?244:188;size_t n=size_t(w)*h*4;Bytes art(112+n);p32(art,0,w);p32(art,4,h);p32(art,8,1);p32(art,12,k+1);p32(art,16,1);p64(art,40,n);
   // Straight RGBA intentionally violates RGB<=alpha, including alpha zero.
   art[112]=251;art[113]=127;art[114]=55;art[115]=uint8_t(k);
   CHECK(nfs_texture_sha256(art.data()+112,n,art.data()+48));parts.push_back(std::move(art));
  }
  bytes.resize(64+32*parts.size());std::copy_n(base.bytes.begin(),64,bytes.begin());bytes[7]='5';p32(bytes,8,5);p32(bytes,24,unsigned(parts.size()));
  for(unsigned i=0;i<parts.size();++i){if(i<count)std::copy_n(base.bytes.begin()+64+32*i,32,bytes.begin()+64+32*i);else{p32(bytes,64+32*i,10);p32(bytes,68+32*i,i-count+1);imageAt[i-count]=bytes.size();}p64(bytes,72+32*i,bytes.size());p64(bytes,80+32*i,parts[i].size());bytes.insert(bytes.end(),parts[i].begin(),parts[i].end());}
  imageTable=64+32*count;p64(bytes,16,bytes.size());
 }
};

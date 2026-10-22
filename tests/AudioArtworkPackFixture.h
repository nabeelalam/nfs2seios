// SPDX-License-Identifier: MIT
#pragma once
#include "NativeMarkPackFixture.h"
#include "hd/AudioBarReplay.h"
#include <functional>
struct AudioPackFixture {
 NativeFixture legacy{2,3,true};Bytes bytes;size_t catalog=0,atlas=0;
 std::array<Bytes,6> sources;
 AudioPackFixture(){
  std::vector<Bytes> parts;
  for(unsigned k=0;k<27;++k){auto off=read64(legacy.bytes,72+32*k),n=read64(legacy.bytes,80+32*k);parts.emplace_back(legacy.bytes.begin()+off,legacy.bytes.begin()+off+n);}
  Bytes c(496);p32(c,0,1);p32(c,4,6);p32(c,8,80);
  for(unsigned k=0;k<6;++k){using namespace nfs_audio_frame;unsigned w=Width[k],h=Height[k],pal=16+w*h;auto&b=sources[k];b.resize(pal+24);p32(b,0,(pal<<8)|0x7b);p32(b,4,w|(h<<16));p32(b,8,uint16_t(HotX[k])|(uint32_t(uint16_t(HotY[k]))<<16));p32(b,pal,0x2a);p32(b,pal+4,2);p32(b,pal+16,0xff000000);p32(b,pal+20,0xffc0c0c0);
   size_t q=16+80*k;p32(c,q,k);p32(c,q+4,b.size());p32(c,q+8,w);p32(c,q+12,h);p32(c,q+16,pal);p32(c,q+20,2);p32(c,q+24,uint32_t(HotX[k]));p32(c,q+28,uint32_t(HotY[k]));std::copy_n(b.begin(),16,c.begin()+q+32);auto d=sha(b);std::copy(d.begin(),d.end(),c.begin()+q+48);
  }parts.push_back(c);
  Bytes art(112+NFS_AUDIO_ART_BYTES);p32(art,0,NFS_AUDIO_ART_WIDTH);p32(art,4,NFS_AUDIO_ART_HEIGHT);p32(art,8,NFS_AUDIO_ART_SCHEMA);p32(art,16,1);p32(art,20,1);p64(art,32,0xabc123);p64(art,40,NFS_AUDIO_ART_BYTES);art[112]=20;art[113]=10;art[114]=5;art[115]=100;CHECK(nfs_texture_sha256(art.data()+112,NFS_AUDIO_ART_BYTES,art.data()+48));parts.push_back(art);
  bytes.resize(64+32*parts.size());std::copy_n(legacy.bytes.begin(),64,bytes.begin());bytes[7]='4';p32(bytes,8,4);p32(bytes,24,parts.size());
  for(unsigned k=0;k<parts.size();++k){if(k<27)std::copy_n(legacy.bytes.begin()+64+32*k,32,bytes.begin()+64+32*k);else{p32(bytes,64+32*k,k==27?8:9);p32(bytes,68+32*k,0);}p64(bytes,72+32*k,bytes.size());p64(bytes,80+32*k,parts[k].size());if(k==27)catalog=bytes.size();if(k==28)atlas=bytes.size();bytes.insert(bytes.end(),parts[k].begin(),parts[k].end());}p64(bytes,16,bytes.size());
 }
};

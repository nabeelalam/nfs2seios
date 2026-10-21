// SPDX-License-Identifier: MIT
#pragma once
#include "hd/ArtworkPack.h"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <thread>
#include <atomic>
#include <stdexcept>
using nfs_hd::ArtworkPack;using nfs_hd::Digest;
#define CHECK(x) do{if(!(x))throw std::runtime_error(#x);}while(0)
using Bytes=std::vector<uint8_t>;
static void p32(Bytes&b,size_t o,uint32_t v){for(unsigned k=0;k<4;++k)b.at(o+k)=uint8_t(v>>(8*k));}
static void p64(Bytes&b,size_t o,uint64_t v){for(unsigned k=0;k<8;++k)b.at(o+k)=uint8_t(v>>(8*k));}
static Digest sha(const Bytes&b){Digest d{};CHECK(nfs_texture_sha256(b.data(),b.size(),d.data()));return d;}
static void fill(Bytes&b,size_t at,uint8_t v,size_t n=32){std::fill_n(b.begin()+at,n,v);}
struct Fixture {
 Bytes bytes;std::array<size_t,24>at{};
 Fixture(){
  std::vector<Bytes>parts;Bytes c(1616);p32(c,0,0x31544143);p32(c,4,1);p32(c,8,20);p32(c,12,80);
  for(unsigned k=0;k<20;++k){const size_t o=16+80*k;const unsigned kind=k<13?1:k<18?2:3,role=k<13?k:k<18?k-13:k-18;p32(c,o,kind);p32(c,o+4,role);
   if(kind==1){unsigned w=k?1:343,h=k?1:480;const unsigned palette=16+w*h;p32(c,o+8,palette+1040);p32(c,o+12,w);p32(c,o+16,h);p32(c,o+20,palette);p32(c,o+32,0x7b);p32(c,o+36,w|(h<<16));fill(c,o+48,uint8_t(0x11+k));}
   else if(kind==2){p32(c,o+8,37);p32(c,o+12,1);p32(c,o+16,1);p32(c,o+20,17);p32(c,o+24,1);p32(c,o+32,0x7b);p32(c,o+36,0x10001);p32(c,o+44,0xfffeffff);fill(c,o+48,uint8_t(0x50+role));}
   else fill(c,o+48,uint8_t(0x77+role));
  }parts.push_back(c);
  for(unsigned k=0;k<6;++k){Bytes p(112+32*8*4);p32(p,0,32);p32(p,4,8);p32(p,8,1);p32(p,12,k<5?k:0);p32(p,16,1);p32(p,24,k<2);p64(p,32,0x100+k);p64(p,40,32*8*4);
   for(size_t z=112;z<p.size();z+=4){p[z]=60;p[z+1]=30;p[z+2]=10;p[z+3]=k<2?255:200;}
   CHECK(nfs_texture_sha256(p.data()+112,p.size()-112,p.data()+48));if(k<2)fill(p,80,uint8_t(0x77+k));else if(k==2)fill(p,80,0x11);parts.push_back(std::move(p));
  }
  static const char names[12][5]={"nedb","neds","spdb","spds","forb","fors","bspb","bsps","bndb","bnds","bfrb","bfrs"};
  for(unsigned k=0;k<12;++k){Bytes p(112);p32(p,0,1);p32(p,4,k+1);p32(p,8,3);p32(p,12,k);for(unsigned z:{20u,24u,28u,32u,36u})p32(p,z,1);std::memcpy(p.data()+40,names[k],5);fill(p,48,uint8_t(0x12+k));fill(p,80,uint8_t(0x90+k));parts.push_back(std::move(p));}
  for(unsigned k=0;k<5;++k){Bytes p(40);p32(p,0,k);p32(p,4,k);for(unsigned z:{12u,16u,20u,24u,28u})p32(p,z,1);p32(p,32,0xffffffff);p32(p,36,0xfffffffe);parts.push_back(std::move(p));}
  CHECK(parts.size()==24);bytes.resize(832);std::memcpy(bytes.data(),"NFSHDPK1",8);p32(bytes,8,1);p32(bytes,12,64);p32(bytes,24,24);p32(bytes,28,32);p64(bytes,32,64);
  for(unsigned k=0;k<24;++k){unsigned tag=k==0?1:k<=5?2:k==6?3:k<=18?4:5,id=k==0?0:k<=5?k-1:k==6?0:k<=18?k-6:k-19;at[k]=bytes.size();p32(bytes,64+32*k,tag);p32(bytes,68+32*k,id);p64(bytes,72+32*k,bytes.size());p64(bytes,80+32*k,parts[k].size());bytes.insert(bytes.end(),parts[k].begin(),parts[k].end());}p64(bytes,16,bytes.size());
 }
};

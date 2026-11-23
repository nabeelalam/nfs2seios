// SPDX-License-Identifier: MIT
#include "hd/ClassicScreens/HistoryWideArtwork.h"
#include <cassert>
#include <sstream>
#include "hd/ClassicScreens/HistoryWideRegistry.h"
#include "hd/ClassicScreens/HistoryWidePack.h"
using namespace nfs_history_wide;
int main(){
 std::vector<unsigned char>pixels(32*18*4,255);Input i{};i.sourceIndex=0;i.width=32;i.height=18;i.baseGeneration=7;i.bytes=pixels.size();i.pixels=pixels.data();nfs_texture_sha256(i.pixels,i.bytes,i.sha);
 auto a=Artwork::create(i,7);assert(a);Artwork::View v{};assert(a->view(7,0,v)&&v.width==32&&v.height==18);pixels[0]=0;assert(v.pixels[0]==255);auto retained=a;a.reset();assert(retained->view(7,0,v)&&v.pixels[0]==255);
 Artwork::View sentinel{123,456,789,nullptr};assert(!retained->view(0,0,sentinel)&&sentinel.width==123);assert(!retained->view(8,0,sentinel));assert(!retained->view(7,7,sentinel));
 pixels[0]=255;
 for(unsigned bad=0;bad<8;++bad){auto q=i;if(bad==0)q.sourceIndex=1;if(bad==1)q.sourceIndex=49;if(bad==2)q.baseGeneration=8;if(bad==3)q.width=4097;if(bad==4)q.bytes--;if(bad==5)q.sha[0]^=1;if(bad==6)q.height=16;if(bad==7)pixels[3]=254;assert(!Artwork::create(q,7));pixels[3]=255;}
 assert(!Artwork::create(i,0));
 Registry registry;assert(registry.install(retained,7,0));assert(registry.acquire(7,0)==retained);assert(!registry.acquire(8,0));assert(!registry.acquire(7,7));assert(!registry.install(retained,7,1));
 auto replacement=Artwork::create(i,7);assert(replacement&&registry.install(replacement,7,0));assert(!registry.current(retained,7,0));assert(registry.current(replacement,7,0));registry.clear();assert(!registry.current(replacement,7,0));assert(retained->view(7,0,v)&&v.pixels[0]==255);

 std::vector<unsigned char>pack(32+pixels.size(),0);std::memcpy(pack.data(),"NFSHWID1",8);
 auto put=[](unsigned char*p,uint32_t v){for(unsigned k=0;k<4;++k)p[k]=static_cast<unsigned char>(v>>(8*k));};
 put(pack.data()+8,1);put(pack.data()+16,32);put(pack.data()+20,18);put(pack.data()+24,pixels.size());std::memcpy(pack.data()+32,pixels.data(),pixels.size());
 unsigned char trusted[32];nfs_texture_sha256(pack.data(),pack.size(),trusted);
 assert(decode(pack.data(),pack.size(),trusted,7));
 std::istringstream stream(std::string(reinterpret_cast<const char*>(pack.data()),pack.size()));assert(load(stream,trusted,7));
 auto changed=pack;changed.back()^=1;assert(!decode(changed.data(),changed.size(),trusted,7));
 for(unsigned bad=0;bad<7;++bad){auto q=pack;if(bad==0)q[0]^=1;if(bad==1)q[8]=2;if(bad==2)q[12]=1;if(bad==3)q[28]=1;if(bad==4)q.pop_back();if(bad==5)q.push_back(0);if(bad==6)q[16]=0;unsigned char pin[32];nfs_texture_sha256(q.data(),q.size(),pin);assert(!decode(q.data(),q.size(),pin,7));}
 unsigned char zero[32]{};assert(!decode(pack.data(),pack.size(),zero,7));assert(!decode(pack.data(),pack.size(),nullptr,7));assert(!decode(nullptr,pack.size(),trusted,7));
 std::istringstream shortStream("short");assert(!load(shortStream,trusted,7));
 std::vector<unsigned char>compressed(32+compressBound(pixels.size()));std::memcpy(compressed.data(),pack.data(),32);std::memcpy(compressed.data(),"NFSHWID2",8);put(compressed.data()+8,2);uLongf length=compressed.size()-32;assert(compress2(compressed.data()+32,&length,pixels.data(),pixels.size(),9)==Z_OK);compressed.resize(32+length);
 unsigned char compressedPin[32];nfs_texture_sha256(compressed.data(),compressed.size(),compressedPin);auto decoded=decode(compressed.data(),compressed.size(),compressedPin,7);assert(decoded&&decoded->view(7,0,v)&&v.bytes==pixels.size()&&!std::memcmp(v.pixels,pixels.data(),pixels.size()));
 for(unsigned bad=0;bad<4;++bad){auto q=compressed;if(bad==0)q.push_back(0);if(bad==1)q.pop_back();if(bad==2)q[24]++;if(bad==3)q[32]^=1;unsigned char pin[32];nfs_texture_sha256(q.data(),q.size(),pin);assert(!decode(q.data(),q.size(),pin,7));}


}

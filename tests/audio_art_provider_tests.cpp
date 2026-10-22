// SPDX-License-Identifier: MIT
#include "hd/AudioArtProvider.h"
#include <array>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>
#define CHECK(x) do{if(!(x))throw std::runtime_error(#x);}while(0)
static std::array<unsigned char,NFS_AUDIO_ART_BYTES>pixels{};
static unsigned variant=0;
static int atlas(void*user,NfsAudioArt*out){
 if(user!=pixels.data())return 0;
 NfsAudioArt a{};a.pixels=pixels.data();a.bytes=pixels.size();a.width=NFS_AUDIO_ART_WIDTH;a.height=NFS_AUDIO_ART_HEIGHT;a.schema=NFS_AUDIO_ART_SCHEMA;a.premultiplied=a.opacityBaked=1;a.identity=123;std::memset(a.sha256,0x42,32);
 switch(variant){case 1:a.pixels=nullptr;break;case 2:--a.bytes;break;case 3:++a.width;break;case 4:--a.height;break;case 5:a.schema=1;break;case 6:a.premultiplied=0;break;case 7:a.opacityBaked=0;break;case 8:a.identity=0;break;case 9:std::memset(a.sha256,0,32);break;}
 *out=a;return variant!=10;
}
static int failed(void*,NfsAudioArt*out){std::memset(out,0x7b,sizeof *out);return 0;}
static NfsAudioArtProvider provider(){return {NFS_AUDIO_ART_PROVIDER_SCHEMA,pixels.data(),atlas};}
int main(int argc,char**argv){try{
 CHECK(argc==2);NfsAudioArt value;std::memset(&value,0x5a,sizeof value);auto saved=value;
 if(!std::strcmp(argv[1],"missing")){
  CHECK(!nfs_hd_audio_art_provider_available());CHECK(!nfs_hd_audio_art(&value));CHECK(!std::memcmp(&value,&saved,sizeof value));auto p=provider();CHECK(!nfs_hd_install_audio_art_provider(&p));
 }else if(!std::strcmp(argv[1],"null-first")){
  CHECK(!nfs_hd_audio_art(nullptr));auto p=provider();CHECK(!nfs_hd_install_audio_art_provider(&p));CHECK(!nfs_hd_audio_art_provider_available());
 }else if(!std::strcmp(argv[1],"failed-callback")){
  auto p=provider();p.atlas=failed;CHECK(nfs_hd_install_audio_art_provider(&p));CHECK(!nfs_hd_audio_art(&value));CHECK(!std::memcmp(&value,&saved,sizeof value));CHECK(nfs_hd_audio_art_provider_available());
 }else if(!std::strcmp(argv[1],"metadata")){
  auto p=provider();CHECK(nfs_hd_install_audio_art_provider(&p));
  for(variant=1;variant<=10;++variant){CHECK(!nfs_hd_audio_art(&value));CHECK(!std::memcmp(&value,&saved,sizeof value));}
  variant=0;CHECK(nfs_hd_audio_art(&value));CHECK(value.identity==123&&value.pixels==pixels.data());CHECK(!nfs_hd_install_audio_art_provider(&p));
 }else{
  CHECK(!std::strcmp(argv[1],"concurrent"));auto p=provider(),bad=p;bad.schema=0;CHECK(!nfs_hd_install_audio_art_provider(&bad));bad=p;bad.atlas=nullptr;CHECK(!nfs_hd_install_audio_art_provider(&bad));CHECK(!nfs_hd_install_audio_art_provider(nullptr));CHECK(nfs_hd_install_audio_art_provider(&p));CHECK(nfs_hd_audio_art_provider_available());
  p.schema=0;p.user=nullptr;p.atlas=failed;
  std::atomic<bool>okay{true};std::atomic<unsigned>calls{0};std::vector<std::thread>threads;
  for(unsigned k=0;k<8;++k)threads.emplace_back([&]{for(unsigned n=0;n<1000;++n){NfsAudioArt a{};if(!nfs_hd_audio_art(&a)||a.pixels!=pixels.data()||a.bytes!=NFS_AUDIO_ART_BYTES||a.width!=NFS_AUDIO_ART_WIDTH||a.height!=NFS_AUDIO_ART_HEIGHT||a.schema!=NFS_AUDIO_ART_SCHEMA||a.identity!=123)okay=false;if(nfs_hd_audio_art(nullptr))okay=false;calls+=2;}});
  for(auto&t:threads)t.join();CHECK(okay&&calls==16000);CHECK(!nfs_hd_install_audio_art_provider(&p));
 }
 std::printf("PASS immutable Audio art provider %s\n",argv[1]);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}

// SPDX-License-Identifier: MIT
#include "hd/HdArtProvider.h"
#include <array>
#include <atomic>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <thread>
#include <vector>
#define CHECK(v) do{if(!(v))throw std::runtime_error(#v);}while(0)
static const unsigned char pixel[4]={1,2,3,255};
static int texture(void*,uint32_t role,NfsBackgroundSceneTexture*out){*out={};out->pixels=pixel;out->bytes=4;out->role=role;return 1;}
static int word(void*,uint32_t role,NfsBackgroundSceneWord*out){*out={};out->sourceRole=role;return 1;}
static int logo(void*,NfsSmallLogoArt*out){*out={};out->pixels=pixel;out->bytes=4;return 1;}
static int region(void*,uint32_t role,NfsSmallLogoRegion*out){*out={};out->role=role;return 1;}
static int bad(void*,uint32_t,NfsBackgroundSceneTexture*out){std::memset(out,7,sizeof *out);return 0;}
static NfsHdArtProvider provider(){return {1,nullptr,texture,word,logo,region};}
int main(int argc,char**argv){try{CHECK(argc==2);NfsBackgroundSceneTexture value;std::memset(&value,0x5a,sizeof value);auto original=value;
 if(!std::strcmp(argv[1],"missing")){CHECK(!nfs_hd_art_provider_available());CHECK(!nfs_private_background_scene_texture(0,&value));CHECK(!std::memcmp(&value,&original,sizeof value));auto p=provider();CHECK(!nfs_hd_install_art_provider(&p));}
 else if(!std::strcmp(argv[1],"failure")){auto p=provider();p.texture=bad;CHECK(nfs_hd_install_art_provider(&p));CHECK(!nfs_private_background_scene_texture(0,&value));CHECK(!std::memcmp(&value,&original,sizeof value));CHECK(!nfs_private_background_scene_texture(5,&value));CHECK(!std::memcmp(&value,&original,sizeof value));CHECK(!nfs_private_background_scene_texture(0,nullptr));CHECK(!nfs_hd_install_art_provider(&p));}
 else{CHECK(!std::strcmp(argv[1],"concurrent"));auto p=provider(),invalid=p;invalid.schema=0;CHECK(!nfs_hd_install_art_provider(&invalid));invalid=p;invalid.word=nullptr;CHECK(!nfs_hd_install_art_provider(&invalid));CHECK(nfs_hd_install_art_provider(&p));CHECK(nfs_hd_art_provider_available());p.texture=nullptr;std::atomic<unsigned>count{0};std::atomic<bool>okay{true};std::vector<std::thread>threads;
  for(unsigned t=0;t<8;++t)threads.emplace_back([&](){for(unsigned j=0;j<1000;++j){NfsBackgroundSceneTexture a{};NfsBackgroundSceneWord b{};NfsSmallLogoArt c{};NfsSmallLogoRegion d{};if(!nfs_private_background_scene_texture(j%5,&a)||a.role!=j%5||a.pixels!=pixel||!nfs_private_background_scene_word(j%12+1,&b)||b.sourceRole!=j%12+1||!nfs_private_small_logo_art(&c)||c.pixels!=pixel||!nfs_private_small_logo_region(j%5,&d)||d.role!=j%5)okay=false;count+=4;}});
  for(auto&t:threads)t.join();CHECK(okay&&count==32000);CHECK(!nfs_hd_install_art_provider(&invalid));}
 std::printf("PASS %s immutable provider no artwork data\n",argv[1]);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}

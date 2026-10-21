// SPDX-License-Identifier: MIT
#include "hd/NativeMarkProvider.h"
#include <array>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <vector>
#define CHECK(v) do{if(!(v))throw std::runtime_error(#v);}while(0)
static std::array<unsigned char,1048576>pixels{};
static std::array<unsigned char,32>knownKey(){std::array<unsigned char,32>k{};k.fill(0x31);return k;}
static int lookup(void*,uint32_t encoding,uint32_t edge,const uint8_t key[32],NfsNativeMarkArt*out){auto expected=knownKey();if(encoding!=1||edge!=128||!key||std::memcmp(key,expected.data(),32)||!out)return 0;*out={};out->pixels=pixels.data();out->bytes=pixels.size();out->width=out->height=512;out->logicalWidth=out->logicalHeight=128;out->schema=out->premultiplied=1;out->identity=123;std::memset(out->sha256,0x42,32);return 1;}
static int failed(void*,uint32_t,uint32_t,const uint8_t[32],NfsNativeMarkArt*out){std::memset(out,0x7b,sizeof *out);return 0;}
static NfsNativeMarkProvider provider(){return {1,nullptr,lookup};}
int main(int argc,char**argv){try{CHECK(argc==2);auto key=knownKey();NfsNativeMarkArt value;std::memset(&value,0x5a,sizeof value);auto before=value;
 if(!std::strcmp(argv[1],"missing")){
  CHECK(!nfs_hd_native_mark_provider_available());CHECK(!nfs_hd_native_mark_art(2,128,key.data(),&value));CHECK(!std::memcmp(&value,&before,sizeof value));auto rejected=provider();CHECK(!nfs_hd_install_native_mark_provider(&rejected));CHECK(!nfs_hd_native_mark_art(1,128,key.data(),&value));CHECK(!std::memcmp(&value,&before,sizeof value));auto p=provider();CHECK(!nfs_hd_install_native_mark_provider(&p));CHECK(!nfs_hd_native_mark_provider_available());
 }else if(!std::strcmp(argv[1],"failure")){
  auto p=provider();p.lookup=failed;CHECK(nfs_hd_install_native_mark_provider(&p));CHECK(nfs_hd_native_mark_provider_available());CHECK(!nfs_hd_native_mark_art(1,128,key.data(),&value));CHECK(!std::memcmp(&value,&before,sizeof value));CHECK(!nfs_hd_native_mark_art(0,128,key.data(),&value));CHECK(!nfs_hd_native_mark_art(1,64,key.data(),&value));CHECK(!nfs_hd_native_mark_art(1,128,nullptr,&value));CHECK(!nfs_hd_native_mark_art(1,128,key.data(),nullptr));CHECK(!std::memcmp(&value,&before,sizeof value));CHECK(!nfs_hd_install_native_mark_provider(&p));
 }else{
  CHECK(!std::strcmp(argv[1],"concurrent"));auto p=provider(),invalid=p;invalid.schema=0;CHECK(!nfs_hd_install_native_mark_provider(&invalid));invalid=p;invalid.lookup=nullptr;CHECK(!nfs_hd_install_native_mark_provider(&invalid));CHECK(!nfs_hd_install_native_mark_provider(nullptr));CHECK(nfs_hd_install_native_mark_provider(&p));CHECK(nfs_hd_native_mark_provider_available());p.schema=0;p.lookup=failed;p.user=reinterpret_cast<void*>(uintptr_t(1));
  // Registry copies the startup descriptor; callers retain immutable callback data.
  std::atomic<bool>okay{true};std::atomic<unsigned>calls{0};std::vector<std::thread>threads;
  for(unsigned i=0;i<8;++i)threads.emplace_back([&]{for(unsigned k=0;k<1000;++k){NfsNativeMarkArt a{};if(!nfs_hd_native_mark_art(1,128,key.data(),&a)||a.pixels!=pixels.data()||a.bytes!=pixels.size()||a.identity!=123||a.schema!=1)okay=false;std::array<uint8_t,32>unknown{};NfsNativeMarkArt sentinel;std::memset(&sentinel,0x6c,sizeof sentinel);auto saved=sentinel;if(nfs_hd_native_mark_art(1,128,unknown.data(),&sentinel)||std::memcmp(&sentinel,&saved,sizeof saved))okay=false;calls+=2;}});
  for(auto&t:threads)t.join();CHECK(okay&&calls==16000);CHECK(!nfs_hd_install_native_mark_provider(&invalid));CHECK(!nfs_hd_install_native_mark_provider(&p));CHECK(!nfs_hd_native_mark_art(2,128,key.data(),&value)&&!std::memcmp(&value,&before,sizeof value));
 }
 std::printf("PASS %s native mark immutable provider, no artwork data\n",argv[1]);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
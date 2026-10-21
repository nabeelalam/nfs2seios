// SPDX-License-Identifier: MIT
#include "HdArtworkPackFixture.h"
#include "hd/NativeMarkProvider.h"
#include <limits>
static uint64_t read64(const Bytes&b,size_t p){uint64_t v=0;for(unsigned i=0;i<8;++i)v|=uint64_t(b.at(p+i))<<(8*i);return v;}
struct NativeFixture {
 Bytes bytes;std::vector<size_t>at;
 NativeFixture(unsigned textures=2,unsigned bindings=3){
  Fixture original;std::vector<Bytes>parts;
  for(unsigned i=0;i<24;++i){const auto off=read64(original.bytes,72+32*i),n=read64(original.bytes,80+32*i);parts.emplace_back(original.bytes.begin()+off,original.bytes.begin()+off+n);}
  Bytes keys(16+64*bindings);p32(keys,0,1);p32(keys,4,bindings);p32(keys,8,64);
  for(unsigned i=0;i<bindings;++i){const size_t q=16+64*i;p32(keys,q,1);p32(keys,q+4,NFS_NATIVE_MARK_SOURCE_RGBA5551_LE);p32(keys,q+8,128);p32(keys,q+12,32768);p32(keys,q+16,i%textures);p32(keys,q+20,128);p32(keys,q+24,128);fill(keys,q+32,uint8_t(0x31+i));}parts.push_back(std::move(keys));
  for(unsigned i=0;i<textures;++i){Bytes t(112+512*512*4);p32(t,0,512);p32(t,4,512);p32(t,8,1);p32(t,12,i);p32(t,16,1);p64(t,32,0xabc+i);p64(t,40,512*512*4);
   // Synthetic PMA: distinct opaque pixel plus transparent padding.
   t[112]=uint8_t(20+i);t[113]=30;t[114]=40;t[115]=255;CHECK(nfs_texture_sha256(t.data()+112,t.size()-112,t.data()+48));parts.push_back(std::move(t));}
  bytes.resize(64+32*parts.size());std::memcpy(bytes.data(),"NFSHDPK2",8);p32(bytes,8,2);p32(bytes,12,64);p32(bytes,24,unsigned(parts.size()));p32(bytes,28,32);p64(bytes,32,64);
  for(unsigned i=0;i<parts.size();++i){const unsigned tag=i==0?1:i<=5?2:i==6?3:i<=18?4:i<=23?5:i==24?6:7;const unsigned role=i==0?0:i<=5?i-1:i==6?0:i<=18?i-6:i<=23?i-19:i==24?0:i-25;at.push_back(bytes.size());p32(bytes,64+32*i,tag);p32(bytes,68+32*i,role);p64(bytes,72+32*i,bytes.size());p64(bytes,80+32*i,parts[i].size());bytes.insert(bytes.end(),parts[i].begin(),parts[i].end());}p64(bytes,16,bytes.size());
 }
};
int main(){try{
 Fixture old;auto v1=ArtworkPack::loadOwned(old.bytes,sha(old.bytes));CHECK(v1);auto absent=v1->nativeMarkProvider();CHECK(absent.schema==0&&!absent.lookup&&!absent.user);
 NativeFixture f;std::string error;auto pack=ArtworkPack::loadOwned(f.bytes,sha(f.bytes),&error);CHECK(pack&&error.empty());auto p=pack->nativeMarkProvider();CHECK(p.schema==1&&p.lookup&&p.user);
 // All legacy semantic outputs remain byte-identical with the same synthetic catalogue/art.
 for(unsigned i=0;i<5;++i){NfsBackgroundSceneTexture a{},b{};auto q=v1->provider();CHECK(q.texture(q.user,i,&a));auto z=pack->provider();CHECK(z.texture(z.user,i,&b));CHECK(a.width==b.width&&a.height==b.height&&a.bytes==b.bytes&&!std::memcmp(a.pixels,b.pixels,a.bytes));}
 for(unsigned i=0;i<24;++i){const auto off=read64(old.bytes,72+32*i),n=read64(old.bytes,80+32*i);CHECK(n==read64(f.bytes,80+32*i)&&!std::memcmp(old.bytes.data()+off,f.bytes.data()+f.at[i],size_t(n)));}
 for(unsigned i=0;i<3;++i){uint8_t key[32];std::memset(key,0x31+i,sizeof key);NfsNativeMarkArt a{};CHECK(p.lookup(p.user,1,128,key,&a));CHECK(a.width==512&&a.height==512&&a.logicalWidth==128&&a.logicalHeight==128&&a.bytes==1048576&&a.schema==1&&a.premultiplied==1&&!a.opacityBaked&&a.identity==0xabc+i%2&&a.pixels[0]==20+i%2);CHECK(nfs_texture_sha256(a.pixels,a.bytes,key)&&!std::memcmp(key,a.sha256,32));}
 NfsNativeMarkArt out;std::memset(&out,0x5a,sizeof out);auto saved=out;uint8_t unknown[32]{};CHECK(!p.lookup(p.user,1,128,unknown,&out)&&!std::memcmp(&out,&saved,sizeof out));std::memset(unknown,0x31,32);CHECK(!p.lookup(p.user,2,128,unknown,&out)&&!p.lookup(p.user,1,64,unknown,&out)&&!p.lookup(p.user,1,128,nullptr,&out)&&!p.lookup(p.user,1,128,unknown,nullptr)&&!p.lookup(nullptr,1,128,unknown,&out));CHECK(!std::memcmp(&out,&saved,sizeof out));
 std::atomic<bool>okay{true};std::vector<std::thread>threads;for(unsigned k=0;k<4;++k)threads.emplace_back([&]{for(unsigned z=0;z<200;++z){uint8_t key[32];std::memset(key,0x31+z%3,32);NfsNativeMarkArt a{};if(!p.lookup(p.user,1,128,key,&a)||a.identity!=0xabc+(z%3)%2||a.pixels[0]!=20+(z%3)%2)okay=false;}});for(auto&t:threads)t.join();CHECK(okay);
 unsigned rejected=0;auto reject=[&](Bytes b){std::string why;CHECK(!ArtworkPack::loadOwned(b,sha(b),&why)&&!why.empty());++rejected;};auto mutate=[&](auto change){auto b=f.bytes;change(b);reject(std::move(b));};const size_t binding=f.at[24]+16,tex=f.at[25],tex2=f.at[26];
 mutate([](Bytes&b){b[7]="1"[0];});mutate([](Bytes&b){p32(b,8,3);});mutate([](Bytes&b){p32(b,24,24);});mutate([](Bytes&b){p32(b,24,34);});mutate([](Bytes&b){p64(b,72+32*25,UINT64_MAX);});mutate([](Bytes&b){p64(b,80+32*25,UINT64_MAX);});mutate([](Bytes&b){p32(b,64+32*25,6);});mutate([](Bytes&b){p32(b,68+32*26,0);});mutate([](Bytes&b){b.push_back(0);p64(b,16,b.size());});mutate([](Bytes&b){b.pop_back();p64(b,16,b.size());});
 mutate([&](Bytes&b){p32(b,f.at[24],2);});mutate([&](Bytes&b){p32(b,f.at[24]+4,0);});mutate([&](Bytes&b){p32(b,f.at[24]+4,65);});mutate([&](Bytes&b){p32(b,f.at[24]+8,63);});mutate([&](Bytes&b){p32(b,f.at[24]+12,1);});
 mutate([&](Bytes&b){p32(b,binding,0);});mutate([&](Bytes&b){p32(b,binding+4,2);});mutate([&](Bytes&b){p32(b,binding+8,64);});mutate([&](Bytes&b){p32(b,binding+12,UINT32_MAX);});mutate([&](Bytes&b){p32(b,binding+16,2);});mutate([&](Bytes&b){p32(b,binding+20,127);});mutate([&](Bytes&b){p32(b,binding+24,127);});mutate([&](Bytes&b){p32(b,binding+28,1);});mutate([&](Bytes&b){fill(b,binding+32,0);});mutate([&](Bytes&b){std::copy_n(b.begin()+binding+32,32,b.begin()+binding+64+32);});
 mutate([&](Bytes&b){p32(b,tex,UINT32_MAX);});mutate([&](Bytes&b){p32(b,tex+4,0);});mutate([&](Bytes&b){p32(b,tex+8,0);});mutate([&](Bytes&b){p32(b,tex+12,1);});mutate([&](Bytes&b){p32(b,tex+16,0);});mutate([&](Bytes&b){p32(b,tex+20,1);});mutate([&](Bytes&b){p32(b,tex+24,1);});mutate([&](Bytes&b){p32(b,tex+28,1);});mutate([&](Bytes&b){p64(b,tex+32,0);});mutate([&](Bytes&b){p64(b,tex2+32,0xabc);});mutate([&](Bytes&b){p64(b,tex+40,UINT64_MAX);});mutate([&](Bytes&b){b[tex+48]^=1;});mutate([&](Bytes&b){b[tex+80]=1;});mutate([&](Bytes&b){b[tex+112]^=1;});mutate([&](Bytes&b){b[tex+112]=255;b[tex+115]=1;CHECK(nfs_texture_sha256(b.data()+tex+112,1048576,b.data()+tex+48));});
 Digest wrong=sha(f.bytes);wrong[0]^=1;CHECK(!ArtworkPack::loadOwned(f.bytes,wrong,&error));Digest zero{};CHECK(!ArtworkPack::loadOwned(f.bytes,zero,&error));CHECK(pack->nativeMarkProvider().user==p.user);std::printf("PASS syntheticv1unchanged/v2twoassets/multiplekeys/concurrency/%u structuralnegatives/pins\n",rejected);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
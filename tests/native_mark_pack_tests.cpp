// SPDX-License-Identifier: MIT
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
int main(int argc,char**argv){try{
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
 // Version 3 adds exactly the cursor profile; legacy sections stay untouched.
 NativeFixture mixed(2,3,true);auto both=ArtworkPack::loadOwned(mixed.bytes,sha(mixed.bytes),&error);CHECK(both&&error.empty());auto mp=both->nativeMarkProvider();
 for(unsigned i=0;i<3;++i){uint8_t key[32];std::memset(key,0x31+i,32);const bool cursor=i==1;NfsNativeMarkArt a{};CHECK(mp.lookup(mp.user,cursor?2:1,cursor?32:128,key,&a));CHECK(a.width==(cursor?128:512)&&a.height==a.width&&a.bytes==(cursor?65536:1048576)&&a.logicalWidth==(cursor?32:128)&&a.identity==0xabc+i%2);CHECK(!mp.lookup(mp.user,cursor?1:2,cursor?128:32,key,&out));CHECK(!std::memcmp(&out,&saved,sizeof out));}
 for(unsigned i=0;i<24;++i){const auto n=read64(f.bytes,80+32*i);CHECK(n==read64(mixed.bytes,80+32*i)&&!std::memcmp(f.bytes.data()+f.at[i],mixed.bytes.data()+mixed.at[i],size_t(n)));}
 const size_t mk=mixed.at[24]+16,ct=mixed.at[26];auto mutateMixed=[&](auto change){auto b=mixed.bytes;change(b);reject(std::move(b));};
 // A digest alone cannot alias profiles: the complete tuple selects art.
 auto shared=mixed.bytes;std::copy_n(shared.begin()+mk+32,32,shared.begin()+mk+64+32);auto sp=ArtworkPack::loadOwned(shared,sha(shared));CHECK(sp);auto spl=sp->nativeMarkProvider();uint8_t sk[32];std::memset(sk,0x31,32);NfsNativeMarkArt large{},small{};CHECK(spl.lookup(spl.user,1,128,sk,&large)&&spl.lookup(spl.user,2,32,sk,&small)&&large.width==512&&small.width==128&&large.identity!=small.identity);
 mutateMixed([&](Bytes&b){b[7]='2';p32(b,8,2);}); // v2 never silently accepts cursors.
 mutateMixed([&](Bytes&b){p32(b,mk+64+4,1);});
 mutateMixed([&](Bytes&b){p32(b,mk+64+8,128);});
 mutateMixed([&](Bytes&b){p32(b,mk+64+12,32768);});
 mutateMixed([&](Bytes&b){p32(b,mk+64+16,0);}); // cross-profile reference
 mutateMixed([&](Bytes&b){p32(b,mk+16,1);});
 mutateMixed([&](Bytes&b){p32(b,mk+64+20,128);});
 mutateMixed([&](Bytes&b){std::copy_n(b.begin()+mk+32,32,b.begin()+mk+128+32);}); // same tuple duplicate
 mutateMixed([&](Bytes&b){p32(b,ct,512);});
 mutateMixed([&](Bytes&b){p32(b,ct+4,512);});
 mutateMixed([&](Bytes&b){p64(b,ct+40,1048576);});
 mutateMixed([&](Bytes&b){b[ct+112]^=1;});
 mutateMixed([&](Bytes&b){b[ct+80]=1;});
 mutateMixed([&](Bytes&b){p64(b,ct+32,0xabc);});
 mutateMixed([&](Bytes&b){b[ct+112]=255;b[ct+115]=1;CHECK(nfs_texture_sha256(b.data()+ct+112,65536,b.data()+ct+48));});
 NfsNativeMarkProfile profile{},unchanged{};CHECK(nfs_native_mark_profile(2,32,&profile)&&profile.preparedBytes==2048&&profile.artBytes==65536&&profile.sourceLod==3);unchanged=profile;CHECK(!nfs_native_mark_profile(2,128,&profile)&&!std::memcmp(&profile,&unchanged,sizeof profile));CHECK(!nfs_native_mark_profile(1,32,&profile)&&!nfs_native_mark_profile(3,32,&profile)&&!nfs_native_mark_profile(2,32,nullptr));
 // Capacity expansion is version-specific; exercise every resource and binding at the bound.
 for(unsigned count:{9u,40u}){NativeFixture capacity(count,64,true);auto loaded=ArtworkPack::loadOwned(capacity.bytes,sha(capacity.bytes));CHECK(loaded);auto cp=loaded->nativeMarkProvider();for(unsigned i=0;i<64;++i){uint8_t key[32];std::memset(key,0x31+i,32);const auto role=i%count;const bool cursor=role==1;NfsNativeMarkArt a{},again{};CHECK(cp.lookup(cp.user,cursor?2:1,cursor?32:128,key,&a));CHECK(a.identity==0xabc+role&&a.width==(cursor?128:512)&&a.pixels[0]==20+role);CHECK(cp.lookup(cp.user,cursor?2:1,cursor?32:128,key,&again)&&again.pixels==a.pixels&&again.identity==a.identity);}}
 {NativeFixture tooMany(41,41,true);reject(std::move(tooMany.bytes));}
 {NativeFixture tooManyV2(9,9,false);reject(std::move(tooManyV2.bytes));}
 {NativeFixture boundaryV2(8,64,false);CHECK(ArtworkPack::loadOwned(boundaryV2.bytes,sha(boundaryV2.bytes)));}
 // A structurally valid full v3 pack crosses 96 MiB without padding or relaxed schemas.
 {NativeFixture roomy(40,64,true,4096);CHECK(roomy.bytes.size()>ArtworkPack::LegacyMaxBytes&&roomy.bytes.size()<ArtworkPack::MaxBytes);auto pin=sha(roomy.bytes);auto loaded=ArtworkPack::loadOwned(roomy.bytes,pin);CHECK(loaded&&loaded->size()==roomy.bytes.size());
  if(argc>1){std::ofstream file(argv[1],std::ios::binary);CHECK(bool(file.write(reinterpret_cast<const char*>(roomy.bytes.data()),roomy.bytes.size())));file.close();CHECK(ArtworkPack::loadFile(argv[1],pin));}
  roomy.bytes[7]='2';p32(roomy.bytes,8,2);CHECK(!ArtworkPack::loadOwned(roomy.bytes,sha(roomy.bytes),&error)&&error=="HD artwork pack does not match its trusted content pin");
  if(argc>1){std::ofstream file(argv[1],std::ios::binary);CHECK(bool(file.write(reinterpret_cast<const char*>(roomy.bytes.data()),roomy.bytes.size())));file.close();CHECK(!ArtworkPack::loadFile(argv[1],sha(roomy.bytes),&error)&&error=="HD artwork pack size is unsupported");CHECK(std::remove(argv[1])==0);}}
 {NativeFixture oversized(40,64,true,4096,3072);CHECK(oversized.bytes.size()>ArtworkPack::MaxBytes);reject(std::move(oversized.bytes));}
 // Original v2 and v1 byte caps reject even before parsing otherwise untrusted input.
 {NativeFixture oldSize(8,8,false,4096,3072);CHECK(oldSize.bytes.size()>ArtworkPack::LegacyMaxBytes&&oldSize.bytes.size()<ArtworkPack::MaxBytes);CHECK(!ArtworkPack::loadOwned(oldSize.bytes,sha(oldSize.bytes),&error)&&error=="HD artwork pack does not match its trusted content pin");oldSize.bytes[7]='1';p32(oldSize.bytes,8,1);CHECK(!ArtworkPack::loadOwned(oldSize.bytes,sha(oldSize.bytes),&error));}
 Digest wrong=sha(f.bytes);wrong[0]^=1;CHECK(!ArtworkPack::loadOwned(f.bytes,wrong,&error));Digest zero{};CHECK(!ArtworkPack::loadOwned(f.bytes,zero,&error));CHECK(pack->nativeMarkProvider().user==p.user);std::printf("PASS syntheticv1unchanged/v2twoassets/v3mixedprofiles/tuplekeys/concurrency/%u structuralnegatives/pins\n",rejected);return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}
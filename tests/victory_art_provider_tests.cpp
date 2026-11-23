// SPDX-License-Identifier: MIT
// Explicit private real-preview and synthetic two-aspect fixtures only.
#include "hd/ClassicScreens/VictoryArtProvider.h"
#include "hd/ClassicScreens/VictoryArtworkArchive.h"
#include "hd/ClassicScreens/VictoryOriginalArtwork.h"
#include "hd/ClassicScreens/VictoryCanvasLayout.h"
#include <cmath>
#include <cassert>
#include <atomic>
#include <thread>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <iterator>
using nfs_victory_art::Archive;
using nfs_victory_art::ArchivePin;
ArchivePin pin(const char*bytes,const char*hex){
 ArchivePin p;p.fileBytes=std::strtoull(bytes,nullptr,10);assert(std::strlen(hex)==64);
 for(unsigned i=0;i<32;++i){char text[3]={hex[2*i],hex[2*i+1],0};p.metadataSha[i]=static_cast<unsigned char>(std::strtoul(text,nullptr,16));}
 return p;
}
void valid(const NfsVictoryArtView&v){
 assert(v.schema==1&&v.owner&&v.retain&&v.release&&v.generation&&v.identity&&v.aspect<2&&v.image<16);
 assert(v.width&&v.height&&v.bytes==size_t(v.width)*v.height*4&&v.pixels);
 assert(!std::memcmp(v.sourceSha256,nfs_victory_original::sources[v.image].sha,32));
 unsigned char sha[32];assert(nfs_texture_sha256(v.pixels,v.bytes,sha)&&!std::memcmp(sha,v.rgbaSha256,32));
}
void release(NfsVictoryArtView&v){v.release(v.owner);v={};}
int load(const char*path,const ArchivePin&p,int preview,char*error,size_t size){return nfs_victory_art_load_file(path,p.fileBytes,p.metadataSha.data(),preview,error,size);}
int main(int argc,char**argv){
 const int sizes[][2]={{640,480},{708,492},{800,450},{2360,1640},{1920,1080}};
 for(const auto&size:sizes){int viewport[4]={17,9,size[0],size[1]};NfsVictoryCanvasLayout layout{};assert(nfs_victory_canvas_layout(viewport,&layout));
  assert(layout.textViewport[1]==9&&layout.textViewport[3]==size[1]);
  if(size[0]*3==size[1]*4){assert(!layout.aspect&&layout.leftU==0&&layout.rightU==1&&layout.textViewport[0]==17&&layout.textViewport[2]==size[0]);}
  else{assert(layout.aspect==1);float left=17+(.125f-layout.leftU)/(layout.rightU-layout.leftU)*size[0],right=17+(.875f-layout.leftU)/(layout.rightU-layout.leftU)*size[0];assert(std::fabs(left-layout.textViewport[0])<=1&&std::fabs(right-layout.textViewport[0]-layout.textViewport[2])<=1);}
 }
 int invalid[4]={0,0,0,480};NfsVictoryCanvasLayout untouchedLayout{};untouchedLayout.aspect=99;assert(!nfs_victory_canvas_layout(invalid,&untouchedLayout)&&untouchedLayout.aspect==99);
 assert(argc==11);const auto preview=pin(argv[2],argv[3]),complete=pin(argv[5],argv[6]),wrong=pin(argv[8],argv[9]);
 nfs_victory_art_clear();assert(!nfs_victory_art_generation());char error[128]={};
 assert(nfs_victory_art_source_identity_valid(0,nfs_victory_original::sources[0].sha));assert(!nfs_victory_art_source_identity_valid(16,nfs_victory_original::sources[0].sha));assert(!nfs_victory_art_source_identity_valid(0,nullptr));assert(!nfs_victory_art_source_identity_valid(1,nfs_victory_original::sources[0].sha));
 assert(!load(argv[1],preview,0,error,sizeof error)&&error[0]);
 assert(!load(argv[7],wrong,0,error,sizeof error)); // Valid archive; incorrect original source associations.
 auto bad=preview;bad.metadataSha[0]^=1;assert(!load(argv[1],bad,1,error,sizeof error));
 assert(!load(argv[1],preview,2,error,sizeof error));char one='x';assert(!load(argv[1],preview,0,&one,1)&&one==0);
 assert(load(argv[1],preview,1,error,sizeof error)&&!error[0]);const auto generation=nfs_victory_art_generation();assert(generation);
 assert(!load(argv[1],preview,1,error,sizeof error)&&nfs_victory_art_generation()==generation);
 NfsVictoryArtView first{},same{},last{};const auto*back=nfs_victory_original::sources[0].sha;
 assert(nfs_victory_art_acquire(0,0,back,&first));valid(first);
 assert(nfs_victory_art_acquire(0,0,back,&same)&&first.pixels==same.pixels&&first.identity==same.identity);release(same);
 first.retain(first.owner);first.release(first.owner);valid(first);
 NfsVictoryArtView untouched{};untouched.schema=77;untouched.bytes=123;unsigned char before[sizeof untouched];std::memcpy(before,&untouched,sizeof untouched);
 unsigned char different[32];std::memcpy(different,back,32);different[0]^=1;
 assert(!nfs_victory_art_acquire(0,0,different,&untouched));assert(!std::memcmp(before,&untouched,sizeof untouched));
 assert(!nfs_victory_art_acquire(1,0,back,&untouched));assert(!nfs_victory_art_acquire(2,0,back,&untouched));assert(!nfs_victory_art_acquire(0,16,back,&untouched));assert(!nfs_victory_art_acquire(0,0,nullptr,&untouched));assert(!nfs_victory_art_acquire(0,0,back,nullptr));assert(!std::memcmp(before,&untouched,sizeof untouched));
 auto real=Archive::open(argv[1],preview,true);assert(real);
 for(unsigned image=0;image<16;++image){
  NfsVictoryArtView v{};assert(nfs_victory_art_acquire(0,image,nfs_victory_original::sources[image].sha,&v));valid(v);
  const auto*entry=real->entry(0,image);assert(entry&&entry->width==v.width&&entry->height==v.height&&!std::memcmp(v.rgbaSha256,entry->rgbaSha.data(),32));
  if(image==15)last=v;else release(v);
 }
 nfs_victory_art_clear();assert(!nfs_victory_art_generation());valid(first);valid(last);
 assert(!nfs_victory_art_acquire(0,0,back,&untouched));assert(!std::memcmp(before,&untouched,sizeof untouched));
 assert(load(argv[4],complete,0,error,sizeof error));assert(nfs_victory_art_generation()>generation);
 NfsVictoryArtView replacement{};assert(nfs_victory_art_acquire(0,0,back,&replacement));assert(replacement.generation!=first.generation&&replacement.identity==first.identity);valid(first);release(replacement);
 auto full=Archive::open(argv[4],complete);assert(full&&full->count()==32);
 for(unsigned aspect=0;aspect<2;++aspect)for(unsigned image=0;image<16;++image){
  NfsVictoryArtView v{};assert(nfs_victory_art_acquire(aspect,image,nfs_victory_original::sources[image].sha,&v));valid(v);
  const auto*entry=full->entry(aspect,image);assert(entry&&entry->width==v.width&&entry->height==v.height&&!std::memcmp(v.rgbaSha256,entry->rgbaSha.data(),32));release(v);
 }
 // Two independent acquired images must survive cache eviction and clear/reload.
 std::atomic<unsigned>ready{0};std::atomic<bool>go{false};
 auto worker=[&](unsigned image){NfsVictoryArtView v{};assert(nfs_victory_art_acquire(1,image,nfs_victory_original::sources[image].sha,&v));valid(v);ready.fetch_add(1);while(!go.load())std::this_thread::yield();valid(v);release(v);};
 std::thread a(worker,1),b(worker,2);while(ready.load()!=2)std::this_thread::yield();nfs_victory_art_clear();assert(load(argv[4],complete,0,error,sizeof error));go.store(true);a.join();b.join();
 valid(first);valid(last);release(first);release(last);nfs_victory_art_clear();assert(!nfs_victory_art_generation());
 // Corrupt one private synthetic chunk: registration still reads only metadata.
 // Its first decode must fail without poisoning the other image or cached view.
 {std::ifstream existing(argv[10],std::ios::binary);assert(!existing);}
 std::ifstream input(argv[4],std::ios::binary);assert(input);
 std::vector<unsigned char>damaged(std::istreambuf_iterator<char>(input),{});input.close();
 const auto*firstEntry=full->entry(0,0);assert(firstEntry&&firstEntry->offset<damaged.size());damaged[size_t(firstEntry->offset)]^=1;
 {std::ofstream output(argv[10],std::ios::binary);assert(output);output.write(reinterpret_cast<const char*>(damaged.data()),damaged.size());assert(output);}
 assert(load(argv[10],complete,0,error,sizeof error));
 assert(!nfs_victory_art_acquire(0,0,back,&untouched));assert(!std::memcmp(before,&untouched,sizeof untouched));
 NfsVictoryArtView intact{},again{};assert(nfs_victory_art_acquire(0,1,nfs_victory_original::sources[1].sha,&intact));valid(intact);
 assert(!nfs_victory_art_acquire(0,0,back,&untouched));assert(!std::memcmp(before,&untouched,sizeof untouched));
 assert(nfs_victory_art_acquire(0,1,nfs_victory_original::sources[1].sha,&again)&&again.pixels==intact.pixels);release(again);release(intact);
 nfs_victory_art_clear();assert(std::remove(argv[10])==0);
 std::cout<<"Verified16real and32synthetic views, exact source binding, cache reuse, output preservation and concurrent retained-view lifetime.\n";
}

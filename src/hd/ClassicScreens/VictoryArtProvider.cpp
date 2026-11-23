// SPDX-License-Identifier: MIT
#include "VictoryArtProvider.h"
#include "VictoryArtworkArchive.h"
#include "VictoryOriginalArtwork.h"
#include <atomic>
#include <mutex>
#include <memory>
#include <cstring>
namespace {
using Archive=nfs_victory_art::Archive;
struct Image {uint64_t generation=0;unsigned aspect=0,image=0,width=0,height=0;uint64_t identity=0;nfs_victory_art::Digest rgba{},source{};std::vector<unsigned char>pixels;};
std::mutex mutex;std::shared_ptr<const Archive>active;std::shared_ptr<const Image>cached;
uint64_t serial=0;std::atomic<uint64_t>published{0};
struct Lease {
 std::atomic<unsigned>refs{1};std::shared_ptr<const Image>image;
 static void retain(void*p){static_cast<Lease*>(p)->refs.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p){auto*l=static_cast<Lease*>(p);if(l->refs.fetch_sub(1,std::memory_order_acq_rel)==1)delete l;}
};
void errorText(char*out,size_t capacity,const char*text){if(out&&capacity){const size_t n=std::strlen(text)<capacity-1?std::strlen(text):capacity-1;std::memcpy(out,text,n);out[n]=0;}}
bool sourceCatalog(const Archive&archive){
 for(unsigned aspect=0;aspect<archive.count()/16;++aspect)for(unsigned image=0;image<16;++image){const auto*entry=archive.entry(aspect,image);if(!entry||std::memcmp(entry->sourceSha.data(),nfs_victory_original::sources[image].sha,32))return false;}
 return true;
}
}
extern "C" int nfs_victory_art_load_file(const char*path,uint64_t fileBytes,const unsigned char metadata[32],int allowPreview,char*error,size_t capacity){
 if(error&&capacity)error[0]=0;
 try{
  if(!metadata||(allowPreview!=0&&allowPreview!=1)){errorText(error,capacity,"Victory artwork configuration is invalid");return 0;}
  nfs_victory_art::ArchivePin pin;pin.fileBytes=fileBytes;std::memcpy(pin.metadataSha.data(),metadata,32);
  auto archive=Archive::open(path,pin,allowPreview!=0);
  if(!archive||!sourceCatalog(*archive)){errorText(error,capacity,"Victory artwork metadata or source identities differ from their pins");return 0;}
  std::lock_guard<std::mutex>lock(mutex);
  if(active||serial==UINT64_MAX){errorText(error,capacity,"Victory artwork is already registered or generation is exhausted");return 0;}
  active=std::move(archive);cached.reset();published.store(++serial,std::memory_order_release);return 1;
 }catch(...){errorText(error,capacity,"Victory artwork could not be registered");return 0;}
}
extern "C" int nfs_victory_art_source_identity_valid(unsigned image,const unsigned char source[32]){return source&&image<16&&!std::memcmp(source,nfs_victory_original::sources[image].sha,32);}
extern "C" void nfs_victory_art_clear(void){try{std::lock_guard<std::mutex>lock(mutex);published.store(0,std::memory_order_release);active.reset();cached.reset();}catch(...){}}
extern "C" uint64_t nfs_victory_art_generation(void){return published.load(std::memory_order_acquire);}
extern "C" int nfs_victory_art_acquire(unsigned aspect,unsigned image,const unsigned char original[32],NfsVictoryArtView*out){
 if(!out||!original||aspect>1||image>=16)return 0;
 try{
  std::shared_ptr<const Archive>archive;std::shared_ptr<const Image>selected;uint64_t generation=0;
  {std::lock_guard<std::mutex>lock(mutex);archive=active;generation=published.load(std::memory_order_relaxed);
   if(!archive||!generation)return 0;const auto*entry=archive->entry(aspect,image);if(!entry||std::memcmp(entry->sourceSha.data(),original,32))return 0;
   if(cached&&cached->generation==generation&&cached->aspect==aspect&&cached->image==image)selected=cached;
  }
  if(!selected){
   const auto*entry=archive->entry(aspect,image);if(!entry)return 0;
   auto decoded=std::make_shared<Image>();decoded->generation=generation;decoded->aspect=aspect;decoded->image=image;decoded->width=entry->width;decoded->height=entry->height;decoded->rgba=entry->rgbaSha;decoded->source=entry->sourceSha;decoded->identity=UINT64_C(0x5649430000000000)|(uint64_t(aspect)<<8)|image;
   if(!archive->load(aspect,image,decoded->source,decoded->pixels))return 0;
   std::lock_guard<std::mutex>lock(mutex);if(active!=archive||published.load(std::memory_order_relaxed)!=generation)return 0;
   if(cached&&cached->generation==generation&&cached->aspect==aspect&&cached->image==image)selected=cached;
   else{cached=std::move(decoded);selected=cached;}
  }
  auto lease=std::unique_ptr<Lease>(new Lease);lease->image=selected;NfsVictoryArtView view{};
  view.schema=1;view.aspect=aspect;view.image=image;view.width=selected->width;view.height=selected->height;view.generation=generation;view.identity=selected->identity;view.bytes=selected->pixels.size();view.pixels=selected->pixels.data();std::memcpy(view.rgbaSha256,selected->rgba.data(),32);std::memcpy(view.sourceSha256,selected->source.data(),32);
  view.owner=lease.get();view.retain=Lease::retain;view.release=Lease::release;*out=view;lease.release();return 1;
 }catch(...){return 0;}
}

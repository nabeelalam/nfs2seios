// SPDX-License-Identifier: MIT
#include "CreditsArtProvider.h"
#include "CreditsSourceCatalog.h"
#include "TextureDigest.h"
#include <array>
#include <atomic>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>
namespace {
constexpr size_t Limit=96u*1024u*1024u;
struct Image {NfsCreditsArtImage view{};std::vector<unsigned char>bytes;};
struct Art {uint64_t generation=0;std::array<Image,NFS_CREDITS_ART_COUNT>images;};
std::mutex mutex;std::shared_ptr<const Art>active;std::atomic<uint64_t>published{0};uint64_t serial=0;
bool shape(unsigned role,uint32_t w,uint32_t h){
 if(!w||!h||w>8192||h>8192)return false;
 if(role==NFS_CREDITS_ART_BACKDROP){const uint64_t a=uint64_t(w)*9,b=uint64_t(h)*16;return w>=1280&&h>=720&&a+16>=b&&b+16>=a;}
 if(role==NFS_CREDITS_ART_LOGO)return w==868&&h==368;
 if(role==NFS_CREDITS_ART_WATERMARK)return w==200&&h==1680;
 if(role>=NFS_CREDITS_ART_HEADING_EN&&role<=NFS_CREDITS_ART_HEADING_SV){const auto&s=nfs_credits_sources[nfs_credits_title_source_indices[role-NFS_CREDITS_ART_HEADING_EN]];return w==4*s.width&&h==4*s.height;}
 if(role==NFS_CREDITS_ART_KENWOOD)return w==784&&h==152;
 const uint64_t a=uint64_t(w)*130,b=uint64_t(h)*243;return role==NFS_CREDITS_ART_Y2&&w>=972&&h>=520&&a+243>=b&&b+243>=a;
}
struct Lease {std::atomic<unsigned>refs{1};std::shared_ptr<const Art>art;static void retain(void*p){static_cast<Lease*>(p)->refs.fetch_add(1,std::memory_order_relaxed);}static void release(void*p){auto*l=static_cast<Lease*>(p);if(l->refs.fetch_sub(1,std::memory_order_acq_rel)==1)delete l;}};
}
extern "C" int nfs_credits_art_install(const NfsCreditsArtInput*in){
 if(!in||in->schema!=1)return 0;
 try{auto art=std::make_shared<Art>();size_t total=0;
  for(unsigned role=0;role<NFS_CREDITS_ART_COUNT;++role){const auto&s=in->images[role];
   if(!shape(role,s.width,s.height)||!s.pixels||size_t(s.width)>Limit/4/s.height)return 0;
   const size_t count=size_t(s.width)*s.height*4;if(s.bytes!=count||count>Limit-total)return 0;
   auto&d=art->images[role];d.bytes.assign(s.pixels,s.pixels+count);unsigned char sha[32];
   if(!nfs_texture_sha256(d.bytes.data(),count,sha)||std::memcmp(sha,s.sha256,32))return 0;
   for(size_t k=0;k<count;k+=4){const unsigned a=d.bytes[k+3];if(d.bytes[k]>a||d.bytes[k+1]>a||d.bytes[k+2]>a||(role==NFS_CREDITS_ART_BACKDROP&&a!=255))return 0;}
   d.view=s;d.view.pixels=d.bytes.data();total+=count;
  }
  std::lock_guard<std::mutex>lock(mutex);if(active||serial==UINT64_MAX)return 0;art->generation=++serial;active=std::move(art);published.store(active->generation,std::memory_order_release);return 1;
 }catch(...){return 0;}
}
extern "C" void nfs_credits_art_clear(){std::lock_guard<std::mutex>lock(mutex);published.store(0,std::memory_order_release);active.reset();}
extern "C" uint64_t nfs_credits_art_generation(){return published.load(std::memory_order_acquire);}
extern "C" int nfs_credits_art_acquire(NfsCreditsArtView*out){
 if(!out)return 0;try{std::shared_ptr<const Art>art;{std::lock_guard<std::mutex>lock(mutex);art=active;}if(!art)return 0;
  auto lease=std::unique_ptr<Lease>(new Lease);lease->art=art;NfsCreditsArtView v{};v.schema=1;v.generation=art->generation;
  for(unsigned i=0;i<NFS_CREDITS_ART_COUNT;++i)v.images[i]=art->images[i].view;
  v.owner=lease.get();v.retain=Lease::retain;v.release=Lease::release;*out=v;lease.release();return 1;
 }catch(...){return 0;}
}

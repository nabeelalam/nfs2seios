// SPDX-License-Identifier: MIT
#pragma once
#include "HistorySourceCatalog.h"
#include "TextureDigest.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
namespace nfs_history_wide {
// Separate optional panorama. The existing History page remains the center.
struct Input {
 uint32_t sourceIndex=0,width=0,height=0;
 uint64_t baseGeneration=0;
 size_t bytes=0;const unsigned char*pixels=nullptr;unsigned char sha[32]{};
};
class Artwork final {
 uint32_t source_=0,width_=0,height_=0;uint64_t base_=0;
 std::vector<unsigned char>pixels_;unsigned char sha_[32]{};
 Artwork()=default;
public:
 struct View {uint32_t width,height;size_t bytes;const unsigned char*pixels;unsigned char sha[32];};
 static std::shared_ptr<const Artwork>create(const Input&i,uint64_t liveBase)noexcept{
  try{
   constexpr size_t limit=32u*1024u*1024u;
   if(!liveBase||i.baseGeneration!=liveBase||i.sourceIndex>=NFS_HISTORY_SOURCE_COUNT||nfs_history_sources[i.sourceIndex].role||!i.pixels||!i.width||!i.height||i.width>4096||i.height>4096||size_t(i.width)>limit/4/i.height)return {};
   // Permit integer rounding at the two outer edges, not stretched panoramas.
   const int64_t aspect=int64_t(i.width)*9-int64_t(i.height)*16;
   if(aspect < -18 || aspect > 18)return {};
   const size_t bytes=size_t(i.width)*i.height*4;
   if(i.bytes!=bytes)return {};
   auto a=std::shared_ptr<Artwork>(new Artwork);
   a->pixels_.assign(i.pixels,i.pixels+bytes);
   unsigned char sha[32];if(!nfs_texture_sha256(a->pixels_.data(),bytes,sha)||std::memcmp(sha,i.sha,32))return {};
   for(size_t p=3;p<bytes;p+=4)if(a->pixels_[p]!=255)return {};
   std::memcpy(a->sha_,sha,32);a->source_=i.sourceIndex;a->base_=liveBase;a->width_=i.width;a->height_=i.height;
   return a;
  }catch(...){return {};}
 }
 // Caller retains this owner through FIFO and revalidates the real base
 // generation/source before each draw. A retained pointer is not authority.
 bool view(uint64_t liveBase,uint32_t source,View&out)const noexcept{
  if(!liveBase||liveBase!=base_||source!=source_)return false;
  out={width_,height_,pixels_.size(),pixels_.data(),{}};std::memcpy(out.sha,sha_,32);return true;
 }
};
}

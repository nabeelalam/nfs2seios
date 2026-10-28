// SPDX-License-Identifier: MIT
#pragma once
#include "ArtworkPack.h"
#include "SetupBadgeData.h"
#include <array>
#include <cstring>
namespace nfs_hd {
// Retain alongside the immutable pack for process lifetime; install once BEFORE
// the first provider getter. Existing pack lookups keep their exact precedence.
class SetupEmblemProvider final {
 std::shared_ptr<const ArtworkPack> pack_; NfsNativeMarkProvider base_{};
 std::array<std::vector<unsigned char>,4> bytes_;
 static int lookup(void*p,uint32_t enc,uint32_t edge,const uint8_t sha[32],NfsNativeMarkArt*out){
  if(!p||!sha||!out)return 0;auto&s=*static_cast<SetupEmblemProvider*>(p);
  if(s.base_.lookup&&s.base_.lookup(s.base_.user,enc,edge,sha,out))return 1;
  if(enc!=1||edge!=128)return 0;
  for(size_t k=0;k<4;++k)if(!std::memcmp(sha,nfs_setup_badges[k].source,32)){
   NfsNativeMarkArt a{};a.pixels=s.bytes_[k].data();a.bytes=s.bytes_[k].size();a.width=a.height=512;a.logicalWidth=a.logicalHeight=128;a.schema=1;a.premultiplied=1;a.identity=UINT64_C(0x4d41524b00000100)+k;std::memcpy(a.sha256,nfs_setup_badges[k].art,32);*out=a;return 1;
  }return 0;
 }
public:
 static std::shared_ptr<const SetupEmblemProvider>create(std::shared_ptr<const ArtworkPack>p)noexcept{
  try{if(!p)return {};auto s=std::shared_ptr<SetupEmblemProvider>(new SetupEmblemProvider);s->pack_=std::move(p);s->base_=s->pack_->nativeMarkProvider();
   for(size_t k=0;k<4;++k){const auto&a=nfs_setup_badges[k];auto&v=s->bytes_[k];v.reserve(512*512*4);
    for(size_t at=0;at<a.bytes;at+=6){if(a.bytes-at<6)return {};unsigned n=a.rle[at]|unsigned(a.rle[at+1])<<8;if(!n||n>(512*512*4-v.size())/4)return {};const auto*c=a.rle+at+2;if(c[0]>c[3]||c[1]>c[3]||c[2]>c[3])return {};for(unsigned j=0;j<n;++j)v.insert(v.end(),c,c+4);}
    if(v.size()!=512*512*4)return {};
   }return s;
  }catch(...){return {};}
 }
 NfsNativeMarkProvider provider()const noexcept{return {1,const_cast<SetupEmblemProvider*>(this),&lookup};}
};
}

// SPDX-License-Identifier: MIT
#pragma once
#include "ControlsBindingPhraseIdentity.h"
#include "ControlsBindingPhraseData.h"
#include "../TextureDigest.h"
#include <array>
#include <memory>
#include <vector>
#include <cstring>
#include <algorithm>
namespace nfs_controls_binding_phrase {
struct Artwork {unsigned image=0,width=0,height=0;std::vector<unsigned char>rgba;std::array<unsigned char,32>sha{};};
inline std::shared_ptr<const Artwork>decode(unsigned image,const unsigned short*runs,size_t count)noexcept{
 const auto*id=nfs_controls_binding_phrase_identity(image);if(!id||!runs||!count||count%2||!id->width||id->width>4096||id->height!=128)return {};
 const size_t pixels=size_t(id->width)*128;if(count>pixels*2)return {};size_t total=0;
 for(size_t i=0;i<count;i+=2){if(!runs[i]||runs[i+1]>255||runs[i]>pixels-total)return {};total+=runs[i];}if(total!=pixels)return {};
 try{auto a=std::shared_ptr<Artwork>(new Artwork);a->image=image;a->width=id->width;a->height=128;a->rgba.assign(pixels*4,255);size_t at=0;
  for(size_t i=0;i<count;i+=2)for(unsigned j=0;j<runs[i];++j)a->rgba[(at++)*4+3]=static_cast<unsigned char>(runs[i+1]);
  if(!nfs_texture_sha256(a->rgba.data(),a->rgba.size(),a->sha.data())||std::memcmp(a->sha.data(),id->sha256,32))return {};
  for(unsigned x=0;x<a->width;++x)if(a->rgba[size_t(x)*4+3]||a->rgba[(size_t(127)*a->width+x)*4+3])return {};
  for(unsigned y=0;y<128;++y)if(a->rgba[size_t(y)*a->width*4+3]||a->rgba[(size_t(y)*a->width+a->width-1)*4+3])return {};return a;
 }catch(...){return {};}
}
// A bounded strong cache prevents per-frame decode/hash work. Old frame leases
// retain their own immutable owners independently of presentation-cache eviction.
inline std::shared_ptr<const Artwork>artwork(unsigned image)noexcept{
 if(!nfs_controls_binding_phrase_identity(image))return {};
 struct Slot {std::shared_ptr<const Artwork>owner;uint64_t age=0;};
 static thread_local std::array<Slot,40>cache;static thread_local uint64_t clock=0;
 if(clock==UINT64_MAX){for(auto&s:cache)s.age=0;clock=0;}++clock;
 for(auto&s:cache)if(s.owner&&s.owner->image==image){s.age=clock;return s.owner;}
 const auto&r=nfs_controls_binding_phrase_runs[image-1];auto a=decode(image,r.data,r.count);if(!a)return {};
 auto slot=std::min_element(cache.begin(),cache.end(),[](const Slot&a,const Slot&b){return a.age<b.age;});slot->owner=a;slot->age=clock;return a;
}
} // namespace nfs_controls_binding_phrase

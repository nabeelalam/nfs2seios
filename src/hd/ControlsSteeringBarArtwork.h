// SPDX-License-Identifier: MIT
#pragma once
#include "ControlsSteeringBarRuntimeAbi.h"
#include "ControlsSteeringBarIdentity.h"
#include "ControlsSteeringBarData.h"
#include "../TextureDigest.h"
#include <array>
#include <cstring>
#include <memory>
#include <vector>
namespace nfs_controls_steering_bar {
struct Artwork {
 unsigned level=0;std::vector<unsigned char>rgba;std::array<unsigned char,32>sha{};
 NfsControlsSteeringBarView view()const noexcept{NfsControlsSteeringBarView v{};v.schema=NFS_CONTROLS_STEERING_BAR_SCHEMA;v.level=level;v.width=1424;v.height=196;v.rgba=rgba.data();v.bytes=rgba.size();std::memcpy(v.sha256,sha.data(),32);v.left=280;v.top=280;v.right=636;v.bottom=329;return v;}
};
inline std::shared_ptr<const Artwork>decode(unsigned level,const uint32_t*runs,size_t count)noexcept{
 const size_t pixels=size_t(1424)*196;if(level>=4||!runs||!count||(count&1)||count>pixels*2)return {};
 size_t total=0;for(size_t i=0;i<count;i+=2){const uint32_t color=runs[i+1];const unsigned a=color>>24;if(!runs[i]||runs[i]>pixels-total||(color&255)>a||((color>>8)&255)>a||((color>>16)&255)>a)return {};total+=runs[i];}if(total!=pixels)return {};
 try{std::shared_ptr<Artwork>p(new Artwork);p->level=level;p->rgba.resize(pixels*4);size_t at=0;for(size_t i=0;i<count;i+=2)for(uint32_t n=0;n<runs[i];++n){for(unsigned c=0;c<4;++c)p->rgba[at*4+c]=static_cast<unsigned char>(runs[i+1]>>(8*c));++at;}
  for(unsigned y=0;y<196;++y)for(unsigned x=0;x<1424;++x)if((x<4||x>=1420||y<4||y>=192)&&p->rgba[(size_t(y)*1424+x)*4+3])return {};
  if(!nfs_texture_sha256(p->rgba.data(),p->rgba.size(),p->sha.data())||std::memcmp(p->sha.data(),nfs_controls_steering_bar_sha256[level],32))return {};return p;
 }catch(...){return {};}
}
inline std::shared_ptr<const Artwork>artwork(unsigned level)noexcept{
 if(level>=4)return {};static const std::shared_ptr<const Artwork>images[4]={decode(0,nfs_controls_steering_bar_rle[0],nfs_controls_steering_bar_rle_count[0]),decode(1,nfs_controls_steering_bar_rle[1],nfs_controls_steering_bar_rle_count[1]),decode(2,nfs_controls_steering_bar_rle[2],nfs_controls_steering_bar_rle_count[2]),decode(3,nfs_controls_steering_bar_rle[3],nfs_controls_steering_bar_rle_count[3])};return images[level];
}
} // namespace nfs_controls_steering_bar

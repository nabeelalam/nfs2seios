// SPDX-License-Identifier: MIT
#pragma once
#include "hd/HdArtProvider.h"
#include <vector>
#include <array>
#include <cstring>
struct AudioSceneAssets{
 std::array<std::vector<unsigned char>,5>bytes;std::array<NfsBackgroundSceneTexture,5>tex{};
 std::array<NfsBackgroundSceneWord,12>words{};std::vector<unsigned char>logoBytes;NfsSmallLogoArt logo{};bool badRegion=false;
 AudioSceneAssets(){const unsigned widths[5]={2048,2048,1372,2012,1824},heights[5]={1536,1536,1920,1924,1924};
  for(unsigned i=0;i<5;++i){bytes[i].resize(size_t(widths[i])*heights[i]*4);if(i<3){const unsigned char colors[3][4]={{20,40,60,255},{100,80,40,255},{80,40,0,128}};for(size_t k=0;k<bytes[i].size();k+=4)std::memcpy(bytes[i].data()+k,colors[i],4);}tex[i].pixels=bytes[i].data();tex[i].bytes=bytes[i].size();tex[i].width=widths[i];tex[i].height=heights[i];tex[i].schema=1;tex[i].role=i;tex[i].premultiplied=1;tex[i].opaque=i<2;tex[i].identity=i+1;}
  for(unsigned i=0;i<12;++i){auto&w=words[i];w.schema=1;w.sourceRole=i+1;w.textureRole=i<6?3:4;w.x=2+(i%6)*68;w.y=2;w.width=64;w.height=48;w.scale=4;w.logicalWidth=16;w.logicalHeight=12;
   unsigned char color[4]={static_cast<unsigned char>(i%2?64:0),static_cast<unsigned char>(i%2?0:64),0,128};for(unsigned y=0;y<52;++y)for(unsigned x=0;x<68;++x)std::memcpy(bytes[w.textureRole].data()+((w.y-2+y)*widths[w.textureRole]+w.x-2+x)*4,color,4);}
  for(unsigned i=0;i<5;++i)ck(nfs_texture_sha256(bytes[i].data(),bytes[i].size(),tex[i].sha256),"synthetic texture sha");
  logoBytes.resize(NFS_SMALL_LOGO_ART_BYTES);for(const auto&r:nfs_logo_regions){unsigned char color[4]={static_cast<unsigned char>(r.role==0?30:r.role==2?60:0),static_cast<unsigned char>(r.role==0?30:r.role==1?60:0),static_cast<unsigned char>(r.role==0?30:0),128};for(unsigned y=0;y<r.height;++y)for(unsigned x=0;x<r.width;++x)std::memcpy(logoBytes.data()+((r.y+y)*2048+r.x+x)*4,color,4);}
  logo.pixels=logoBytes.data();logo.bytes=logoBytes.size();logo.width=2048;logo.height=768;logo.schema=1;logo.premultiplied=1;logo.identity=100;ck(nfs_texture_sha256(logoBytes.data(),logoBytes.size(),logo.sha256),"synthetic logo sha");
 }
 static int texture(void*p,uint32_t role,NfsBackgroundSceneTexture*out){if(role>=5)return 0;*out=static_cast<AudioSceneAssets*>(p)->tex[role];return 1;}
 static int word(void*p,uint32_t role,NfsBackgroundSceneWord*out){if(!role||role>12)return 0;auto&a=*static_cast<AudioSceneAssets*>(p);*out=a.words[role-1];if(a.badRegion){out->logicalWidth=0x40000001;out->width=4;}return 1;}
 static int art(void*p,NfsSmallLogoArt*out){*out=static_cast<AudioSceneAssets*>(p)->logo;return 1;}
 static int region(void*,uint32_t role,NfsSmallLogoRegion*out){if(role>=5)return 0;*out=nfs_logo_regions[role];return 1;}
};

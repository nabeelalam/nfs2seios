// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFontProvider.h"
#include "SoftwareFontData.h"
#include "../TextureDigest.h"
#include <algorithm>
#include <cstring>
namespace nfs_soft {
// Decode only an exact bounded R8 atlas. Validate the complete stream before
// allocation/output changes; zero runs, overrun and incomplete totals fail.
inline bool decodeFontCoverage(const uint8_t*source,size_t bytes,std::vector<uint8_t>&output)noexcept{
 constexpr size_t required=size_t(FontCatalog::width)*FontCatalog::height;
 if(!source||!bytes||bytes%3||bytes>required*3)return false;
 size_t total=0;
 for(size_t i=0;i<bytes;i+=3){const size_t n=source[i]|size_t(source[i+1])<<8;if(!n||n>required-total)return false;total+=n;}
 if(total!=required)return false;
 try{std::vector<uint8_t>next(required);total=0;for(size_t i=0;i<bytes;i+=3){const size_t n=source[i]|size_t(source[i+1])<<8;std::fill_n(next.begin()+total,n,source[i+2]);total+=n;}output.swap(next);return true;}catch(...){return false;}
}
inline FontDescription builtinFontDescription()noexcept{
 FontDescription d;
 for(unsigned role=0;role<2;++role){d.baseline[role]=nfs_soft_authored_data::baseline[role];
  for(unsigned k=0;k<96;++k){const auto&g=nfs_soft_authored_data::glyphs[role][k];d.glyphs[role][k]={g.x,g.y,g.w,g.h,g.left,g.top,g.advance};}
  std::copy_n(nfs_soft_authored_data::kern[role],96*96,d.kern[role].begin());
 }
 return d;
}
inline bool authenticatedFontMetadata(const FontDescription&d)noexcept{
 // Canonical little-endian serialization excludes host padding and endianness.
 try{std::vector<uint8_t>bytes;bytes.reserve(2*96*16+2*96*96*4+8);
  auto u16=[&](uint16_t v){bytes.push_back(uint8_t(v));bytes.push_back(uint8_t(v>>8));};
  auto u32=[&](uint32_t v){for(unsigned k=0;k<4;++k)bytes.push_back(uint8_t(v>>(8*k)));};
  for(const auto&role:d.glyphs)for(const auto&g:role){u16(uint16_t(g.x));u16(uint16_t(g.y));u16(uint16_t(g.w));u16(uint16_t(g.h));u16(uint16_t(g.left));u16(uint16_t(g.top));u32(uint32_t(g.advance));}
  for(const auto&role:d.kern)for(auto k:role)u32(uint32_t(k));for(auto b:d.baseline)u32(uint32_t(b));
  uint8_t digest[32];return nfs_texture_sha256(bytes.data(),bytes.size(),digest)&&!std::memcmp(digest,nfs_soft_authored_data::metadataSha256,32);
 }catch(...){return false;}
}
// Build-time source/recipe/font/license pins are recorded in software-fonts.json.
// Runtime content validation authenticates both metadata and decoded coverage;
// the catalog's numeric identity is only a correlation value.
inline std::shared_ptr<const FontCatalog> createAuthenticatedSoftwareFont(const FontDescription&d,const uint8_t*source,size_t bytes)noexcept{
 if(!authenticatedFontMetadata(d))return {};
 std::vector<uint8_t>alpha;if(!decodeFontCoverage(source,bytes,alpha))return {};
 uint8_t digest[32];if(!nfs_texture_sha256(alpha.data(),alpha.size(),digest)||std::memcmp(digest,nfs_soft_authored_data::coverageSha256,32))return {};
 return FontCatalog::create(d,alpha.data(),alpha.size(),nfs_soft_authored_data::identity);
}
inline std::shared_ptr<const FontCatalog> createBuiltinFontCatalog()noexcept{
 return createAuthenticatedSoftwareFont(builtinFontDescription(),nfs_soft_authored_data::coverageRle,sizeof nfs_soft_authored_data::coverageRle);
}
}

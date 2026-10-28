// PRIVATE immutable classic-condensed UI font. Does not touch the global font registry.
#pragma once
#include "hd/ClassicScreens/private/ResultsFontData.h"
#include "hd/SoftwareFontCatalog.h"
#include "TextureDigest.h"
#include <cstring>
namespace nfs_results_text {
inline std::shared_ptr<const nfs_soft::FontCatalog>resultsFont()noexcept{
 static const auto value=[]()->std::shared_ptr<const nfs_soft::FontCatalog>{try{
  std::vector<uint8_t>alpha;alpha.reserve(1024*512);
  const auto&r=nfs_results_font_data::runs;
  for(size_t i=0;i<sizeof(r);i+=3){if(i+2>=sizeof(r))return {};const size_t count=r[i]|size_t(r[i+1])<<8;if(!count||count>524288-alpha.size())return {};alpha.insert(alpha.end(),count,r[i+2]);}
  unsigned char sha[32];if(alpha.size()!=524288||!nfs_texture_sha256(alpha.data(),alpha.size(),sha)||std::memcmp(sha,nfs_results_font_data::sha,32))return {};
  nfs_soft::FontDescription description;
  for(unsigned role=0;role<2;++role)for(unsigned i=0;i<96;++i){const auto&g=role?nfs_results_font_data::timing[i]:nfs_results_font_data::glyphs[i];description.glyphs[role][i]={g.x,g.y,g.w,g.h,g.left,g.top,g.advance};}
  return nfs_soft::FontCatalog::create(description,alpha.data(),alpha.size(),nfs_results_font_data::identity);
 }catch(...){return {};}}();return value;
}
}

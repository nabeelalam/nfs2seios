// SPDX-License-Identifier: MIT
// History-local U+00E0 coverage; source text, source metrics and global fonts stay immutable.
#pragma once
#include "ResultsFont.h"
#include "HistoryAccentData.h"
#include <algorithm>
namespace nfs_history_text {
inline std::shared_ptr<const nfs_soft::FontCatalog>historyFont()noexcept{
 static const auto value=[]()->std::shared_ptr<const nfs_soft::FontCatalog>{try{
  const auto base=nfs_results_text::resultsFont();if(!base)return {};
  auto description=base->description();auto alpha=base->alpha();
  constexpr unsigned x=nfs_history_accent_data::x,y=nfs_history_accent_data::y;
  constexpr unsigned w=nfs_history_accent_data::width,h=nfs_history_accent_data::height;
  static_assert(sizeof(nfs_history_accent_data::pixels)==w*h,"History agrave coverage size");
  if(!w||!h||x<1||y<1||x+w>=1024||y+h>=512||alpha.size()!=524288)return {};
  // This region is an unused atlas allocation, never another glyph's ink.
  for(unsigned row=y-1;row<=y+h;++row)for(unsigned col=x-1;col<=x+w;++col)
   if(alpha[size_t(row)*1024+col])return {};
  for(unsigned row=0;row<h;++row)
   std::copy_n(nfs_history_accent_data::pixels+size_t(row)*w,w,alpha.begin()+size_t(y+row)*1024+x);
  // The existing decoder already maps the genuine Latin-1 byte0xE0 toslot95.
  // Retain its former advance, allother advances/kerns and everyASCII glyph.
  auto&g=description.glyphs[0][95];g.x=x;g.y=y;g.w=w;g.h=h;
  g.left=nfs_history_accent_data::left;g.top=nfs_history_accent_data::top;
  unsigned char sha[32];if(!nfs_texture_sha256(alpha.data(),alpha.size(),sha)||
   std::memcmp(sha,nfs_history_accent_data::alphaSha,32))return {};
  return nfs_soft::FontCatalog::create(description,alpha.data(),alpha.size(),nfs_history_accent_data::identity);
 }catch(...){return {};}}();return value;
}
}

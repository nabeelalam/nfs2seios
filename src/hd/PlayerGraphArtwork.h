// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerGraphSourceProof.h"
#include "PlayerStatMaterial.h"
#include <utility>

// Display derivative of an authenticated graph source. Reuse the same authored
// PMA plate/bars as the steady Player preview; preserve original translation
// and clipping, not a percentage-width approximation of the rating bars.
namespace nfs_player_graph_art {
constexpr unsigned Width=384*4,Height=160*4;
namespace detail {
struct Bounds {unsigned left=211,top=316,width=307,height=147;};
struct Source {std::array<uint8_t,4>name{};std::array<unsigned char,32>sha{};};
struct Selection {uint32_t graphEnabled=1;std::array<uint32_t,4>graphValues{};};
struct Preview {
 std::array<Source,5>source;Selection selected;
 const std::array<Source,5>&sources()const noexcept{return source;}
 const Selection&selection()const noexcept{return selected;}
 Bounds bounds()const noexcept{return {};}
};
struct Overlay {
 int left=211,top=316;unsigned width=384,height=160;
 std::vector<uint16_t>pixels;std::vector<uint8_t>coverage;
 std::vector<unsigned char>authored;std::array<unsigned char,32>authoredSha{};
};
// Compile-time adapter for the decorator's optional swatch interface. Animation
// canvases have no swatch operation; the original later swatch stays separate.
struct NoSwatch {
 Bounds bounds()const noexcept{return {};}
 const std::vector<uint16_t>&pixels()const noexcept{static const std::vector<uint16_t>v;return v;}
 const std::vector<uint8_t>&coverage()const noexcept{static const std::vector<uint8_t>v;return v;}
};
}
inline std::vector<unsigned char>canvas(const nfs_player_graph_source::Packet&packet)noexcept{
 try{
  const auto&original=packet.canvases()[1];
  if(original.bits!=16||original.pixels.size()!=384u*160u*2u)return {};
  detail::Preview preview;preview.selected.graphValues=packet.domain().statistics;
  for(unsigned k=0;k<5;++k){const auto&id=nfs_player_graph_source::catalog[k];std::copy_n(id.name,4,preview.source[k].name.begin());preview.source[k].sha=id.sha;}
  detail::Overlay out;out.pixels.resize(384*160);out.coverage.assign(384*160,0);
  for(size_t k=0;k<out.pixels.size();++k)out.pixels[k]=uint16_t(original.pixels[2*k]|(uint16_t(original.pixels[2*k+1])<<8));
  nfs_player_stat_material::decorate(preview,static_cast<const detail::NoSwatch*>(nullptr),out);
  if(out.authored.size()!=size_t(Width)*Height*4)return {};
  for(size_t k=0;k<out.authored.size();k+=4)for(unsigned c=0;c<3;++c)if(out.authored[k+c]>out.authored[k+3])return {};
  return std::move(out.authored);
 }catch(...){return {};}
}
} // namespace nfs_player_graph_art

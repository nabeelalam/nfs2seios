// SPDX-License-Identifier: MIT
#pragma once
#include "AudioBarReplay.h"
#include <mutex>
namespace nfs_audio_runtime {
using Digest=std::array<unsigned char,32>;
// Construct only from the independently authenticated startup pack. No source
// pixels are present; the live owner must authenticate actual archive copies.
class TrustedAssets final {
 std::array<nfs_audio_frame::SourceIdentity,6> sources_{};
 std::array<Digest,2> maps_{};
 std::shared_ptr<const nfs_audio_frame::ArtBinding> art_;
 TrustedAssets()=default;
public:
 static std::shared_ptr<const TrustedAssets>create(const std::array<nfs_audio_frame::SourceIdentity,6>&sources,const std::array<Digest,2>&maps,const NfsAudioArt&art,const Digest&expectedArt)noexcept{
  using namespace nfs_audio_frame;
  if(!nonzero(maps[0])||!nonzero(maps[1])||maps[0]==maps[1])return {};
  for(unsigned k=0;k<6;++k){const auto&i=sources[k];if(!nonzero(i.sha)||i.span<32||i.span>MaxSource||i.width!=Width[k]||i.height!=Height[k]||i.hotX!=HotX[k]||i.hotY!=HotY[k]||!i.paletteCount||i.paletteCount>256||uint64_t(16)+i.width*i.height>i.paletteOffset||uint64_t(i.paletteOffset)+16+4*i.paletteCount>i.span||(i.header[0]&255)!=0x7b||(i.header[0]>>8)!=i.paletteOffset||i.header[1]!=(i.width|(i.height<<16))||int16_t(uint16_t(i.header[2]))!=i.hotX||int16_t(uint16_t(i.header[2]>>16))!=i.hotY)return {};}
  auto owned=nfs_audio_frame::ArtBinding::create(art,expectedArt);if(!owned)return {};
  try{auto p=std::shared_ptr<TrustedAssets>(new TrustedAssets);p->sources_=sources;p->maps_=maps;p->art_=std::move(owned);return p;}catch(...){return {};}
 }
 const std::array<nfs_audio_frame::SourceIdentity,6>&sources()const noexcept{return sources_;}
 const std::array<Digest,2>&maps()const noexcept{return maps_;}
 const std::shared_ptr<const nfs_audio_frame::ArtBinding>&art()const noexcept{return art_;}
};
namespace detail {inline std::mutex&mutex(){static std::mutex m;return m;}inline std::shared_ptr<const TrustedAssets>&slot(){static std::shared_ptr<const TrustedAssets>p;return p;}}
// Install once at startup before workers; getters return a strong immutable owner.
inline bool installTrustedAssets(std::shared_ptr<const TrustedAssets>p)noexcept{if(!p)return false;try{std::lock_guard<std::mutex>guard(detail::mutex());if(detail::slot())return false;detail::slot()=std::move(p);return true;}catch(...){return false;}}
inline std::shared_ptr<const TrustedAssets>acquireTrustedAssets()noexcept{try{std::lock_guard<std::mutex>guard(detail::mutex());return detail::slot();}catch(...){return {};}}
}

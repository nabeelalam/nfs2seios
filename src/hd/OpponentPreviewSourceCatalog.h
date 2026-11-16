// SPDX-License-Identifier: MIT
#pragma once
#include "TextureDigest.h"
#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <cstring>
namespace nfs_opponent_preview_source {
using Digest=std::array<unsigned char,32>;
struct Entry {std::array<uint8_t,4> name{};uint32_t width=0,height=0,bytes=0,archiveOffset=0;Digest sha{};};
inline const char*nameFor(unsigned i)noexcept{static const char*const names[15]={"car1","car9","carc","car7","car8","cara","car4","car6","car3","carb","car5","car2","clsa","clsb","clsc"};return i<15?names[i]:nullptr;}
inline bool nonzero(const Digest&d)noexcept{for(auto b:d)if(b)return true;return false;}
// Independent startup pins. Never construct these from a live preview observation.
class Catalog final {
 Digest archive_,decoded_;std::array<Entry,15>entries_;
 Catalog(const Digest&a,const Digest&d,const std::array<Entry,15>&e):archive_(a),decoded_(d),entries_(e){}
public:
 Catalog(const Catalog&)=delete;Catalog&operator=(const Catalog&)=delete;
 static std::shared_ptr<const Catalog>create(const Digest&archive,const Digest&decoded,const std::array<Entry,15>&entries)noexcept{
  if(!nonzero(archive)||!nonzero(decoded))return {};
  for(unsigned i=0;i<15;++i){const auto&e=entries[i];if(std::memcmp(e.name.data(),nameFor(i),4)||e.width!=(i<12?307u:306u)||e.height!=146||e.bytes!=(i<12?45880u:45732u)||!e.archiveOffset||uint64_t(e.archiveOffset)+e.bytes>64u*1024u*1024u||!nonzero(e.sha))return {};}
  try{return std::shared_ptr<const Catalog>(new Catalog(archive,decoded,entries));}catch(...){return {};}
 }
 const Digest&archiveSha()const noexcept{return archive_;}const Digest&decodedSha()const noexcept{return decoded_;}
 bool matchesDecoded(const unsigned char*source,size_t bytes)const noexcept{Digest digest{};return source&&bytes>=16&&bytes<=64u*1024u*1024u&&nfs_texture_sha256(source,bytes,digest.data())&&digest==decoded_;}
 const Entry*entry(unsigned index)const noexcept{return index<15?&entries_[index]:nullptr;}
 bool matches(unsigned index,const std::array<uint8_t,4>&name,uint32_t width,uint32_t height,const unsigned char*source,size_t bytes)const noexcept{
  const auto*e=entry(index);if(!e||e->name!=name||e->width!=width||e->height!=height||e->bytes!=bytes||!source)return false;
  Digest digest{};return nfs_texture_sha256(source,bytes,digest.data())&&digest==e->sha;
 }
};
// One immutable startup install per process, no replacement while owners are live.
inline std::mutex&catalogMutex(){static std::mutex m;return m;}
inline std::shared_ptr<const Catalog>&catalogSlot(){static std::shared_ptr<const Catalog>p;return p;}
inline bool install(std::shared_ptr<const Catalog>catalog,const Digest&verifiedOriginalArchiveSha)noexcept{if(!catalog||catalog->archiveSha()!=verifiedOriginalArchiveSha)return false;try{std::lock_guard<std::mutex>lock(catalogMutex());if(catalogSlot())return false;catalogSlot()=std::move(catalog);return true;}catch(...){return false;}}
inline std::shared_ptr<const Catalog>acquire()noexcept{try{std::lock_guard<std::mutex>lock(catalogMutex());return catalogSlot();}catch(...){return {};}}
} // namespace nfs_opponent_preview_source

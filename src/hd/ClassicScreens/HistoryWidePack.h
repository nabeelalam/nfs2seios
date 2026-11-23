// SPDX-License-Identifier: MIT
#pragma once
#include "HistoryWideArtwork.h"
#include <istream>
#include <zlib.h>
namespace nfs_history_wide {
constexpr size_t PackLimit=34u*1024u*1024u;
inline uint32_t wideLe32(const unsigned char*p)noexcept{return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
// Trust is supplied by the caller's pinned whole-file hash, never the file.
// Header: magic[8], version, background source, width, height, RGBA bytes, zero.
inline std::shared_ptr<const Artwork>decode(const unsigned char*file,size_t bytes,const unsigned char*trusted,uint64_t liveBase)noexcept{
 try{
  if(!file||!trusted||bytes<32||bytes>PackLimit||!liveBase)return {};
  unsigned nonzero=0;for(unsigned k=0;k<32;++k)nonzero|=trusted[k];if(!nonzero)return {};
  unsigned char sha[32];if(!nfs_texture_sha256(file,bytes,sha)||std::memcmp(sha,trusted,32))return {};
  const bool compressed=!std::memcmp(file,"NFSHWID2",8)&&wideLe32(file+8)==2;
  if((!compressed&&(std::memcmp(file,"NFSHWID1",8)||wideLe32(file+8)!=1))||wideLe32(file+28))return {};
  Input i{};i.sourceIndex=wideLe32(file+12);i.width=wideLe32(file+16);i.height=wideLe32(file+20);i.bytes=wideLe32(file+24);
  constexpr size_t decodedLimit=32u*1024u*1024u;
  if(!i.width||!i.height||i.width>4096||i.height>4096||size_t(i.width)>decodedLimit/4/i.height||i.bytes!=size_t(i.width)*i.height*4)return {};
  std::vector<unsigned char>decoded;
  if(compressed){
   if(bytes==32)return {};
   decoded.resize(i.bytes+1);z_stream z{};
   z.next_in=const_cast<Bytef*>(file+32);z.avail_in=static_cast<uInt>(bytes-32);
   z.next_out=decoded.data();z.avail_out=static_cast<uInt>(decoded.size());
   if(inflateInit(&z)!=Z_OK)return {};
   const int result=inflate(&z,Z_FINISH);
   const bool exact=result==Z_STREAM_END&&z.total_in==bytes-32&&z.total_out==i.bytes;
   inflateEnd(&z);if(!exact)return {};
   i.pixels=decoded.data();
  }else{if(i.bytes!=bytes-32)return {};i.pixels=file+32;}
  i.baseGeneration=liveBase;if(!nfs_texture_sha256(i.pixels,i.bytes,i.sha))return {};
  return Artwork::create(i,liveBase);
 }catch(...){return {};}
}
inline std::shared_ptr<const Artwork>load(std::istream&stream,const unsigned char*trusted,uint64_t liveBase)noexcept{
 try{
  if(!trusted||!liveBase||!stream)return {};
  stream.seekg(0,std::ios::end);const auto count=stream.tellg();
  if(count<32||uint64_t(count)>PackLimit)return {};
  stream.seekg(0,std::ios::beg);std::vector<unsigned char>file(static_cast<size_t>(count));
  if(!stream.read(reinterpret_cast<char*>(file.data()),std::streamsize(file.size()))||stream.peek()!=std::char_traits<char>::eof())return {};
  return decode(file.data(),file.size(),trusted,liveBase);
 }catch(...){return {};}
}
}

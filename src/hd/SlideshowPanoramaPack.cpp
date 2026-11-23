// SPDX-License-Identifier: MIT
#include "SlideshowPanoramaPack.h"
#include <cstring>
#include <fstream>
#include <istream>
#include <streambuf>
#include "TextureDigest.h"
namespace nfs_slideshow_panorama {
namespace {
bool read(std::istream&s,void*p,size_t n){return bool(s.read(static_cast<char*>(p),static_cast<std::streamsize>(n)));}
uint32_t u32(const unsigned char*p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
uint64_t u64(const unsigned char*p){return uint64_t(u32(p))|(uint64_t(u32(p+4))<<32);}
}
bool loadPack(std::istream&s,Registry&registry) noexcept {
 try {
  unsigned char header[32];
  if(!read(s,header,sizeof header)||std::memcmp(header,"NFSSPAN\0",8)||u32(header+8)!=1||
     !u32(header+12)||u32(header+12)>Registry::MaximumEntries||u32(header+16)!=216||u32(header+20))return false;
  const uint64_t total=u64(header+24);if(!total||total>Registry::MaximumCompressedBytes)return false;
  const size_t count=u32(header+12);std::vector<Entry>entries(count);std::vector<size_t>lengths(count);uint64_t sum=0;
  for(size_t i=0;i<count;++i){
   unsigned char record[216];if(!read(s,record,sizeof record))return false;
   auto&e=entries[i];Digest*digests[]={&e.key.archiveSha,&e.key.entrySha,&e.key.centerRgbaSha,&e.key.footerRgbaSha,&e.sideSha,&e.composedSha};
   for(size_t k=0;k<6;++k)std::memcpy(digests[k]->data(),record+k*32,32);
   e.width=u32(record+192);e.height=u32(record+196);e.centerWidth=u32(record+200);
   if(u32(record+204)!=1)return false;
   e.reviewed=true;
   const uint64_t bytes=u64(record+208);
   if(!bytes||bytes>Registry::MaximumCompressedEntryBytes||bytes>total-sum)return false;
   sum+=bytes;lengths[i]=static_cast<size_t>(bytes);
  }
  if(sum!=total)return false;
  for(size_t i=0;i<count;++i){auto&v=entries[i].compressedSides;v.resize(lengths[i]);if(!read(s,v.data(),v.size()))return false;}
  // EOF must be real: an I/O error is not a valid terminator.
  if(s.peek()!=std::char_traits<char>::eof()||!s.eof()||s.bad())return false;
  return registry.install(std::move(entries));
 }catch(...){return false;}
}
bool loadPack(const std::string&path,Registry&registry) noexcept {
 try {std::ifstream file(path,std::ios::binary);return file.is_open()&&loadPack(file,registry);}catch(...){return false;}
}
bool loadPack(const std::string&path,const Digest&expectedSha,uint64_t expectedBytes,Registry&registry) noexcept {
 try {
  bool nonzero=false;for(auto b:expectedSha)nonzero=nonzero||b;
  if(!nonzero||expectedBytes<32+216||expectedBytes>32+216*Registry::MaximumEntries+Registry::MaximumCompressedBytes)return false;
  std::ifstream file(path,std::ios::binary|std::ios::ate);
  if(!file.is_open()||file.tellg()<0||uint64_t(file.tellg())!=expectedBytes)return false;
  file.seekg(0);std::vector<unsigned char>bytes(static_cast<size_t>(expectedBytes));
  if(!read(file,bytes.data(),bytes.size())||file.peek()!=std::char_traits<char>::eof()||!file.eof()||file.bad())return false;
  Digest actual{};if(!nfs_texture_sha256(bytes.data(),bytes.size(),actual.data())||actual!=expectedSha)return false;
  // Parse the same immutable verified bytes, without reopening a mutable file.
  struct Buffer:std::streambuf {Buffer(unsigned char*p,size_t n){auto*c=reinterpret_cast<char*>(p);setg(c,c,c+n);}}buffer(bytes.data(),bytes.size());
  std::istream input(&buffer);return loadPack(input,registry);
 }catch(...){return false;}
}

}

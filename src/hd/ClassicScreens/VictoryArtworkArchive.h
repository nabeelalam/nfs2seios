// SPDX-License-Identifier: MIT
#pragma once
#include "hd/SlideshowPixels.h"
#include <fstream>
#include <memory>
#include <string>
namespace nfs_victory_art {
using Digest=std::array<unsigned char,32>;
struct ArchivePin {uint64_t fileBytes=0;Digest metadataSha{};};
struct Entry {
 uint32_t aspect=0,image=0,width=0,height=0,bytes=0;uint64_t offset=0;
 Digest rgbaSha{},chunkSha{},sourceSha{};
};
// File metadata names artwork, not live engine authority. The caller must
// separately prove the original backdrop/photo owner and draw chain.
class Archive final {
 std::string path_;ArchivePin pin_;std::vector<Entry>entries_;bool draft_=false;
 static uint32_t u32(const unsigned char*p){return uint32_t(p[0])|uint32_t(p[1])<<8|uint32_t(p[2])<<16|uint32_t(p[3])<<24;}
 static uint64_t u64(const unsigned char*p){return uint64_t(u32(p))|uint64_t(u32(p+4))<<32;}
 static bool hash(const void*bytes,size_t count,const Digest&expected){unsigned char actual[32];return expected!=Digest{}&&nfs_texture_sha256(bytes,count,actual)&&!std::memcmp(actual,expected.data(),32);}
 Archive()=default;
public:
 static std::shared_ptr<const Archive>open(const char*path,const ArchivePin&pin,bool preview=false)noexcept{
  try{
   if(!path||!*path||pin.fileBytes<24||pin.fileBytes>512ull*1024*1024||pin.metadataSha==Digest{})return {};
   std::ifstream file(path,std::ios::binary);if(!file)return {};file.seekg(0,std::ios::end);if(file.tellg()!=std::streamoff(pin.fileBytes))return {};file.seekg(0);
   unsigned char header[24];if(!file.read(reinterpret_cast<char*>(header),24)||std::memcmp(header,"NFSVIC01",8)||u32(header+8)!=1||u32(header+20)!=128)return {};
   const uint32_t count=u32(header+12),flags=u32(header+16);
   if((count!=16&&count!=32)||flags>1||(flags==0&&count!=32)||(flags==1&&!preview))return {};
   const size_t metadataBytes=24+size_t(count)*128;if(metadataBytes>=pin.fileBytes)return {};
   std::vector<unsigned char>metadata(metadataBytes);std::memcpy(metadata.data(),header,24);
   if(!file.read(reinterpret_cast<char*>(metadata.data()+24),metadataBytes-24)||!hash(metadata.data(),metadata.size(),pin.metadataSha))return {};
   auto result=std::shared_ptr<Archive>(new Archive);result->path_=path;result->pin_=pin;result->draft_=flags!=0;result->entries_.reserve(count);uint64_t next=metadataBytes;
   for(uint32_t index=0;index<count;++index){
    const auto*p=metadata.data()+24+size_t(index)*128;Entry e;e.aspect=u32(p);e.image=u32(p+4);e.width=u32(p+8);e.height=u32(p+12);e.bytes=u32(p+20);e.offset=u64(p+24);
    std::memcpy(e.rgbaSha.data(),p+32,32);std::memcpy(e.chunkSha.data(),p+64,32);std::memcpy(e.sourceSha.data(),p+96,32);
    if(e.aspect!=index/16||e.image!=index%16||u32(p+16)!=2||!e.width||!e.height||e.width>4096||e.height>4096||uint64_t(e.width)*e.height*4>64ull*1024*1024||
       (e.aspect==0?e.width*3!=e.height*4:!nfs_authored_wide_canvas(e.width,e.height))||!e.bytes||e.bytes>32u*1024*1024||e.offset!=next||e.offset>pin.fileBytes||e.bytes>pin.fileBytes-e.offset||e.rgbaSha==Digest{}||e.chunkSha==Digest{}||e.sourceSha==Digest{})return {};
    next+=e.bytes;result->entries_.push_back(e);
   }
   if(next!=pin.fileBytes)return {};
   if(count==32)for(unsigned i=0;i<16;++i)if(result->entries_[i].sourceSha!=result->entries_[i+16].sourceSha)return {};
   return result;
  }catch(...){return {};}
 }
 bool draft()const noexcept{return draft_;}
 size_t count()const noexcept{return entries_.size();}
 const Entry*entry(unsigned aspect,unsigned image)const noexcept{if(aspect>1||image>=16)return nullptr;const size_t at=aspect*16+image;return at<entries_.size()?&entries_[at]:nullptr;}
 // Read/decode just the requested image. No all-images decoded cache is kept.
 // Failure leaves output unchanged, including source mismatch or file damage.
 bool load(unsigned aspect,unsigned image,const Digest&originalSource,std::vector<unsigned char>&output)const noexcept{
  try{
   const auto*e=entry(aspect,image);if(!e||e->sourceSha!=originalSource)return false;
   std::ifstream file(path_,std::ios::binary);if(!file)return false;file.seekg(0,std::ios::end);if(file.tellg()!=std::streamoff(pin_.fileBytes))return false;
   file.seekg(std::streamoff(e->offset));std::vector<unsigned char>chunk(e->bytes);if(!file.read(reinterpret_cast<char*>(chunk.data()),chunk.size())||!hash(chunk.data(),chunk.size(),e->chunkSha))return false;
   nfs_slideshow::PixelPin pixels{e->width,e->height,e->rgbaSha,2};return nfs_slideshow::inflateCanvasPixels(chunk.data(),chunk.size(),pixels,output);
  }catch(...){return false;}
 }
};
}

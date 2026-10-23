// SPDX-License-Identifier: MIT
#pragma once
#include "CompactCaptionWordIdentity.h"
#include "CompactCaptionWordData.h"
#include "../TextureDigest.h"
#include <array>
#include <cstring>
#include <memory>
#include <vector>
namespace nfs_compact_caption_display {
struct Artwork {
 unsigned word=0,width=0,height=0;
 std::vector<unsigned char>rgba;
 std::array<unsigned char,32>sha{};
};
// Only three authenticated complete-word images are admitted. Caller-provided
// RLE is copied; no source span or mutable image is retained after publication.
inline std::shared_ptr<const Artwork>decode(unsigned word,const unsigned short*runs,size_t count)noexcept{
 if(word<1||word>3||!runs||!count||count%2)return {};
 const unsigned width=word==1?300:word==2?208:192,height=128;
 const size_t pixels=size_t(width)*height;
 if(count>pixels*2)return {};
 size_t total=0;
 for(size_t i=0;i<count;i+=2){
  if(!runs[i]||runs[i+1]>255||runs[i]>pixels-total)return {};
  total+=runs[i];
 }
 if(total!=pixels)return {};
 try{
  std::shared_ptr<Artwork>image(new Artwork);
  image->word=word;image->width=width;image->height=height;
  image->rgba.assign(pixels*4,255);
  size_t at=0;
  for(size_t i=0;i<count;i+=2)for(unsigned j=0;j<runs[i];++j)
   image->rgba[(at++)*4+3]=static_cast<unsigned char>(runs[i+1]);
  if(!nfs_texture_sha256(image->rgba.data(),image->rgba.size(),image->sha.data())
     ||std::memcmp(image->sha.data(),nfs_compact_caption_word_sha256[word-1],32))return {};
  // Preserve transparent padding on every side of the fixed original text box.
  for(unsigned x=0;x<width;++x)
   if(image->rgba[size_t(x)*4+3]||image->rgba[(size_t(height-1)*width+x)*4+3])return {};
  for(unsigned y=0;y<height;++y)
   if(image->rgba[size_t(y)*width*4+3]||image->rgba[(size_t(y)*width+width-1)*4+3])return {};
  return image;
 }catch(...){return {};}
}
inline std::shared_ptr<const Artwork>artwork(unsigned word)noexcept{
 if(word<1||word>3)return {};
 static const std::shared_ptr<const Artwork>images[3]={
  decode(1,nfs_compact_caption_word_alpha_rle_1,sizeof(nfs_compact_caption_word_alpha_rle_1)/sizeof(unsigned short)),
  decode(2,nfs_compact_caption_word_alpha_rle_2,sizeof(nfs_compact_caption_word_alpha_rle_2)/sizeof(unsigned short)),
  decode(3,nfs_compact_caption_word_alpha_rle_3,sizeof(nfs_compact_caption_word_alpha_rle_3)/sizeof(unsigned short))};
 return images[word-1];
}
} // namespace nfs_compact_caption_display

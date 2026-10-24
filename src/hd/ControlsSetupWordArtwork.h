// SPDX-License-Identifier: MIT
#pragma once
#include "ControlsSetupWordRuntimeAbi.h"
#include "ControlsSetupWordIdentity.h"
#include "ControlsSetupWordData.h"
#include "../TextureDigest.h"
#include <array>
#include <cstring>
#include <memory>
#include <vector>
namespace nfs_controls_setup_word_display {
struct Artwork {
 unsigned word=0,width=0,height=0;
 std::vector<unsigned char>rgba;
 std::array<unsigned char,32>sha{};
};
// Only 28 authenticated complete-word images are admitted. Caller-provided
// RLE is copied; no source span or mutable image is retained after publication.
inline std::shared_ptr<const Artwork>decode(unsigned word,const unsigned short*runs,size_t count)noexcept{
 if(word<1||word>NFS_CONTROLS_SETUP_WORD_IMAGES||!runs||!count||count%2)return {};
 const unsigned width=nfs_controls_setup_word_width[word-1],height=128;
 if(!width||width>4096)return {};
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
     ||std::memcmp(image->sha.data(),nfs_controls_setup_word_sha256[word-1],32))return {};
  // Preserve transparent padding on every side of the fixed original text box.
  for(unsigned x=0;x<width;++x)
   if(image->rgba[size_t(x)*4+3]||image->rgba[(size_t(height-1)*width+x)*4+3])return {};
  for(unsigned y=0;y<height;++y)
   if(image->rgba[size_t(y)*width*4+3]||image->rgba[(size_t(y)*width+width-1)*4+3])return {};
  return image;
 }catch(...){return {};}
}
inline std::shared_ptr<const Artwork>artwork(unsigned word)noexcept{
 if(word<1||word>NFS_CONTROLS_SETUP_WORD_IMAGES)return {};
 static const std::shared_ptr<const Artwork>images[NFS_CONTROLS_SETUP_WORD_IMAGES]={
  decode(1,nfs_controls_setup_word_alpha_rle_1,sizeof(nfs_controls_setup_word_alpha_rle_1)/sizeof(unsigned short)),
  decode(2,nfs_controls_setup_word_alpha_rle_2,sizeof(nfs_controls_setup_word_alpha_rle_2)/sizeof(unsigned short)),
  decode(3,nfs_controls_setup_word_alpha_rle_3,sizeof(nfs_controls_setup_word_alpha_rle_3)/sizeof(unsigned short)),
  decode(4,nfs_controls_setup_word_alpha_rle_4,sizeof(nfs_controls_setup_word_alpha_rle_4)/sizeof(unsigned short)),
  decode(5,nfs_controls_setup_word_alpha_rle_5,sizeof(nfs_controls_setup_word_alpha_rle_5)/sizeof(unsigned short)),
  decode(6,nfs_controls_setup_word_alpha_rle_6,sizeof(nfs_controls_setup_word_alpha_rle_6)/sizeof(unsigned short)),
  decode(7,nfs_controls_setup_word_alpha_rle_7,sizeof(nfs_controls_setup_word_alpha_rle_7)/sizeof(unsigned short)),
  decode(8,nfs_controls_setup_word_alpha_rle_8,sizeof(nfs_controls_setup_word_alpha_rle_8)/sizeof(unsigned short)),
  decode(9,nfs_controls_setup_word_alpha_rle_9,sizeof(nfs_controls_setup_word_alpha_rle_9)/sizeof(unsigned short)),
  decode(10,nfs_controls_setup_word_alpha_rle_10,sizeof(nfs_controls_setup_word_alpha_rle_10)/sizeof(unsigned short)),
  decode(11,nfs_controls_setup_word_alpha_rle_11,sizeof(nfs_controls_setup_word_alpha_rle_11)/sizeof(unsigned short)),
  decode(12,nfs_controls_setup_word_alpha_rle_12,sizeof(nfs_controls_setup_word_alpha_rle_12)/sizeof(unsigned short)),
  decode(13,nfs_controls_setup_word_alpha_rle_13,sizeof(nfs_controls_setup_word_alpha_rle_13)/sizeof(unsigned short)),
  decode(14,nfs_controls_setup_word_alpha_rle_14,sizeof(nfs_controls_setup_word_alpha_rle_14)/sizeof(unsigned short)),
  decode(15,nfs_controls_setup_word_alpha_rle_15,sizeof(nfs_controls_setup_word_alpha_rle_15)/sizeof(unsigned short)),
  decode(16,nfs_controls_setup_word_alpha_rle_16,sizeof(nfs_controls_setup_word_alpha_rle_16)/sizeof(unsigned short)),
  decode(17,nfs_controls_setup_word_alpha_rle_17,sizeof(nfs_controls_setup_word_alpha_rle_17)/sizeof(unsigned short)),
  decode(18,nfs_controls_setup_word_alpha_rle_18,sizeof(nfs_controls_setup_word_alpha_rle_18)/sizeof(unsigned short)),
  decode(19,nfs_controls_setup_word_alpha_rle_19,sizeof(nfs_controls_setup_word_alpha_rle_19)/sizeof(unsigned short)),
  decode(20,nfs_controls_setup_word_alpha_rle_20,sizeof(nfs_controls_setup_word_alpha_rle_20)/sizeof(unsigned short)),
  decode(21,nfs_controls_setup_word_alpha_rle_21,sizeof(nfs_controls_setup_word_alpha_rle_21)/sizeof(unsigned short)),
  decode(22,nfs_controls_setup_word_alpha_rle_22,sizeof(nfs_controls_setup_word_alpha_rle_22)/sizeof(unsigned short)),
  decode(23,nfs_controls_setup_word_alpha_rle_23,sizeof(nfs_controls_setup_word_alpha_rle_23)/sizeof(unsigned short)),
  decode(24,nfs_controls_setup_word_alpha_rle_24,sizeof(nfs_controls_setup_word_alpha_rle_24)/sizeof(unsigned short)),
  decode(25,nfs_controls_setup_word_alpha_rle_25,sizeof(nfs_controls_setup_word_alpha_rle_25)/sizeof(unsigned short)),
  decode(26,nfs_controls_setup_word_alpha_rle_26,sizeof(nfs_controls_setup_word_alpha_rle_26)/sizeof(unsigned short)),
  decode(27,nfs_controls_setup_word_alpha_rle_27,sizeof(nfs_controls_setup_word_alpha_rle_27)/sizeof(unsigned short)),
  decode(28,nfs_controls_setup_word_alpha_rle_28,sizeof(nfs_controls_setup_word_alpha_rle_28)/sizeof(unsigned short))};
 return images[word-1];
}
} // namespace nfs_controls_setup_word_display

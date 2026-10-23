// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerCaptionWordIdentity.h"
#include "PlayerCaptionWordData.h"
#include "../TextureDigest.h"
#include <array>
#include <memory>
#include <vector>
namespace nfs_player_caption_display {
struct Artwork {unsigned word=0,width=0,height=0;std::vector<unsigned char>rgba;std::array<unsigned char,32>sha{};};
inline std::shared_ptr<const Artwork>decode(unsigned word,const unsigned short*runs,size_t count)noexcept{
 if(word<1||word>2||!runs||!count||count%2)return {};const size_t pixels=size_t(word==1?272:280)*128;if(count>pixels*2)return {};size_t total=0;
 for(size_t i=0;i<count;i+=2){if(!runs[i]||runs[i+1]>255||runs[i]>pixels-total)return {};total+=runs[i];}if(total!=pixels)return {};
 try{std::shared_ptr<Artwork>a(new Artwork);a->word=word;a->width=word==1?272:280;a->height=128;a->rgba.assign(pixels*4,255);size_t at=0;for(size_t i=0;i<count;i+=2)for(unsigned j=0;j<runs[i];++j)a->rgba[(at++)*4+3]=static_cast<unsigned char>(runs[i+1]);if(!nfs_texture_sha256(a->rgba.data(),a->rgba.size(),a->sha.data())||memcmp(a->sha.data(),nfs_player_caption_word_sha256[word-1],32))return {};return a;}catch(...){return {};}
}
inline std::shared_ptr<const Artwork>artwork(unsigned word)noexcept{
 static const std::shared_ptr<const Artwork>images[2]={decode(1,nfs_player_caption_word_alpha_rle_1,sizeof(nfs_player_caption_word_alpha_rle_1)/sizeof(unsigned short)),decode(2,nfs_player_caption_word_alpha_rle_2,sizeof(nfs_player_caption_word_alpha_rle_2)/sizeof(unsigned short))};return word>=1&&word<=2?images[word-1]:std::shared_ptr<const Artwork>{};
}
} // namespace nfs_player_caption_display

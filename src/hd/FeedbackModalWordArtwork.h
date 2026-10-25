// SPDX-License-Identifier: MIT
#pragma once
#include "ControlsDialogWordArtwork.h"
#include "FeedbackModalWordIdentity.h"
#include "FeedbackModalWordData.h"
namespace nfs_feedback_modal_word {
namespace controls=nfs_controls_dialog_display;
class Artwork;
inline std::shared_ptr<const Artwork>sharedTitle()noexcept;
inline std::shared_ptr<const Artwork>decodeMessage(const unsigned short*,size_t)noexcept;
class Artwork final {
 unsigned word_=0,width_=0,height_=0;
 std::shared_ptr<const controls::Artwork>shared_;
 std::vector<unsigned char>message_;
 std::array<unsigned char,32>sha_{};
 Artwork()=default;
 friend std::shared_ptr<const Artwork>sharedTitle()noexcept;
 friend std::shared_ptr<const Artwork>decodeMessage(const unsigned short*,size_t)noexcept;
public:
 unsigned word()const noexcept{return word_;}unsigned width()const noexcept{return width_;}
 unsigned height()const noexcept{return height_;}
 const std::vector<unsigned char>&rgba()const noexcept{return shared_?shared_->rgba:message_;}
 const std::array<unsigned char,32>&sha()const noexcept{return sha_;}
};
inline std::shared_ptr<const Artwork>sharedTitle()noexcept{
 try{auto original=controls::artwork(4);std::array<unsigned char,32>actual{};
  if(!original||original->word!=4||original->width!=384||original->height!=128||
     original->rgba.size()!=size_t(384)*128*4||
     !nfs_texture_sha256(original->rgba.data(),original->rgba.size(),actual.data())||
     std::memcmp(actual.data(),nfs_feedback_modal_word_sha256[0],32)||original->sha!=actual)return {};
  auto out=std::shared_ptr<Artwork>(new Artwork);out->word_=1;out->width_=384;out->height_=128;
  out->sha_=actual;out->shared_=std::move(original);return out;
 }catch(...){return {};}
}
inline std::shared_ptr<const Artwork>decodeMessage(const unsigned short*runs,size_t count)noexcept{
 const size_t pixels=size_t(432)*128;
 if(!runs||!count||count%2||count>pixels*2)return {};
 size_t total=0;for(size_t i=0;i<count;i+=2){
  if(!runs[i]||runs[i+1]>255||runs[i]>pixels-total)return {};total+=runs[i];
 }
 if(total!=pixels)return {};
 try{auto out=std::shared_ptr<Artwork>(new Artwork);out->word_=2;out->width_=432;out->height_=128;
  out->message_.assign(pixels*4,255);size_t at=0;
  for(size_t i=0;i<count;i+=2)for(unsigned n=0;n<runs[i];++n)out->message_[(at++)*4+3]=static_cast<unsigned char>(runs[i+1]);
  if(!nfs_texture_sha256(out->message_.data(),out->message_.size(),out->sha_.data())||
     std::memcmp(out->sha_.data(),nfs_feedback_modal_word_sha256[1],32))return {};
  for(unsigned x=0;x<432;++x)if(out->message_[size_t(x)*4+3]||out->message_[(size_t(127)*432+x)*4+3])return {};
  for(unsigned y=0;y<128;++y)if(out->message_[size_t(y)*432*4+3]||out->message_[(size_t(y)*432+431)*4+3])return {};
  return out;
 }catch(...){return {};}
}
inline std::shared_ptr<const Artwork>artwork(unsigned word)noexcept{
 if(word<1||word>2)return {};
 static const std::shared_ptr<const Artwork>images[2]={sharedTitle(),decodeMessage(
  nfs_feedback_modal_word_message_alpha_rle,sizeof(nfs_feedback_modal_word_message_alpha_rle)/sizeof(unsigned short))};
 return images[word-1];
}
} // namespace nfs_feedback_modal_word

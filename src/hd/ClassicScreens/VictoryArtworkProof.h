// SPDX-License-Identifier: MIT
#pragma once
#include "VictoryOriginalArtwork.h"
#include "TextureDigest.h"
#include <memory>
#include <vector>
#include <utility>
namespace nfs_victory_art_proof {
// These receipts prove exact source pixels and observed composition only.
// Engine owner/target/lifetime remain the caller's responsibility.
class Image final {
 const nfs_victory_original::Source*source_=nullptr;
 std::vector<unsigned char>bytes_;
 Image()=default;
public:
 static std::shared_ptr<const Image>create(const unsigned char name[4],const std::vector<unsigned char>&bytes)noexcept{
  try{
   const auto*s=nfs_victory_original::find(name);
   if(!s||bytes.size()!=s->span)return {};
   unsigned char digest[32];
   if(!nfs_texture_sha256(bytes.data(),bytes.size(),digest)||std::memcmp(digest,s->sha,32))return {};
   auto image=std::shared_ptr<Image>(new Image);image->source_=s;image->bytes_=bytes;return image;
  }catch(...){return {};}
 }
 const nfs_victory_original::Source&source()const noexcept{return *source_;}
 const std::vector<unsigned char>&bytes()const noexcept{return bytes_;}
private:
 friend class Scene;
 uint16_t pixel(unsigned x,unsigned y)const noexcept{
  const size_t at=16+2*(size_t(y)*source_->width+x);
  return uint16_t(bytes_[at])|uint16_t(bytes_[at+1])<<8;
 }
};
class Scene final {
 std::shared_ptr<const Image>backdrop_,photo_;
 std::vector<uint16_t>pixels_;
 Scene()=default;
 static bool compose(const Image&image,std::vector<uint16_t>&pixels){
  const auto&s=image.source();
  if(pixels.size()!=640*480||s.left<0||s.top<0||s.left+int(s.width)>640||s.top+int(s.height)>480)return false;
  for(unsigned y=0;y<s.height;++y)for(unsigned x=0;x<s.width;++x)
   pixels[size_t(s.top+y)*640+s.left+x]=image.pixel(x,y);
  return true;
 }
public:
 static std::shared_ptr<const Scene>background(std::shared_ptr<const Image>image,const std::vector<uint16_t>&observed)noexcept{
  try{
   if(!image||image->source().image!=0||observed.size()!=640*480)return {};
   std::vector<uint16_t>expected(640*480);
   if(!compose(*image,expected)||expected!=observed)return {};
   auto scene=std::shared_ptr<Scene>(new Scene);scene->backdrop_=std::move(image);scene->pixels_=std::move(expected);return scene;
  }catch(...){return {};}
 }
 static std::shared_ptr<const Scene>winner(std::shared_ptr<const Scene>background,std::shared_ptr<const Image>image,const std::vector<uint16_t>&observed)noexcept{
  try{
   if(!background||background->photo_||!image||!image->source().image||image->source().image>15||observed.size()!=640*480)return {};
   auto expected=background->pixels_;
   if(!compose(*image,expected)||expected!=observed)return {};
   auto scene=std::shared_ptr<Scene>(new Scene);scene->backdrop_=background->backdrop_;scene->photo_=std::move(image);scene->pixels_=std::move(expected);return scene;
  }catch(...){return {};}
 }
 const std::vector<uint16_t>&pixels()const noexcept{return pixels_;}
 const Image&backdrop()const noexcept{return *backdrop_;}
 const Image*photo()const noexcept{return photo_.get();}
 unsigned image()const noexcept{return photo_?photo_->source().image:0;}
};
}

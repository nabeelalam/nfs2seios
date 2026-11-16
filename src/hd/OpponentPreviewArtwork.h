// SPDX-License-Identifier: MIT
#pragma once
#include "OpponentPreviewPacket.h"
#include "PlayerPhotoArtProvider.h"
#include "ClassPictureArtProvider.h"
#include <cstdio>
#include <cstdlib>
namespace nfs_opponent_preview_art {
class Image final {
 NfsPlayerPhotoArtView photo_{};NfsClassPictureArtView klass_{};bool class_=false;unsigned selection_=15;
 std::vector<unsigned char>classComposite_;std::array<unsigned char,32>classCompositeSha_{};
 std::vector<uint16_t>canvas_;std::vector<unsigned char>coverage_;std::array<unsigned char,32>canvasSha_{},coverageSha_{};NfsOriginalPreviewLayerView footprint_{};uint32_t left_=0,top_=0,logicalWidth_=0,logicalHeight_=0;
 Image()=default;
public:
 Image(const Image&)=delete;Image&operator=(const Image&)=delete;
 ~Image(){if(photo_.owner&&photo_.release)photo_.release(photo_.owner);if(klass_.owner&&klass_.release)klass_.release(klass_.owner);}
 static void diagnose(const char*reason,const nfs_opponent_preview::Packet&p)noexcept{const char*flag=std::getenv("NFS_PRIVATE_OPPONENT_DIAG");if(!flag||flag[0]!='1'||flag[1])return;static thread_local unsigned seen[16]={};const unsigned key=p.selection()<16?p.selection():15;if(seen[key]>=2)return;++seen[key];std::fprintf(stderr,"OPPONENT ART IMAGE reason=%s selection=%u sourceAuth=%u classGeneration=%llu photoGeneration=%llu bounds=%u,%u,%u,%u\n",reason,p.selection(),unsigned(p.sourceAuthenticated()),(unsigned long long)nfs_class_picture_art_generation(),(unsigned long long)nfs_player_photo_art_generation(),p.bounds().left,p.bounds().top,p.bounds().width,p.bounds().height);}
 static std::shared_ptr<const Image>acquire(const nfs_opponent_preview::Packet&p)noexcept{
  if(!p.sourceAuthenticated()||p.selection()>=15||p.sources().size()!=1||p.logicalLeft()!=211||p.logicalTop()!=316||p.sources()[0].width!=(p.selection()<12?307u:306u)||p.sources()[0].height!=146||p.bounds().left<p.logicalLeft()||p.bounds().top<p.logicalTop()||uint64_t(p.bounds().left)+p.bounds().width>uint64_t(p.logicalLeft())+p.sources()[0].width||uint64_t(p.bounds().top)+p.bounds().height>uint64_t(p.logicalTop())+146){diagnose("source-or-geometry",p);return {};}
  try{std::shared_ptr<Image>out(new Image);out->selection_=p.selection();out->class_=p.selection()>=12;
   out->footprint_=p.view();out->footprint_.pixels565=nullptr;out->footprint_.coverage=nullptr;out->left_=p.logicalLeft();out->top_=p.logicalTop();out->logicalWidth_=p.sources()[0].width;out->logicalHeight_=p.sources()[0].height;
   out->canvas_.assign(size_t(out->logicalWidth_)*out->logicalHeight_,0);out->coverage_.assign(out->canvas_.size(),0);
   const auto&r=p.bounds();for(unsigned y=0;y<r.height;++y){const size_t destination=size_t(y+r.top-out->top_)*out->logicalWidth_+r.left-out->left_,source=size_t(y)*r.width;std::copy_n(p.pixels().data()+source,r.width,out->canvas_.data()+destination);std::copy_n(p.coverage().data()+source,r.width,out->coverage_.data()+destination);}
   if(!nfs_opponent_preview::wordsSha(out->canvas_.data(),out->canvas_.size(),out->canvasSha_)||!nfs_texture_sha256(out->coverage_.data(),out->coverage_.size(),out->coverageSha_.data()))return {};
   const unsigned index=out->class_?p.selection()-12:p.selection();
   if(out->class_){if(!nfs_class_picture_art_acquire(index,&out->klass_)){diagnose("class-provider-unavailable",p);return {};}}else if(!nfs_player_photo_art_acquire(index,&out->photo_)){diagnose("photo-provider-unavailable",p);return {};}
   // Source replay proves this exact original fill before the indexed class draw.
   // Rebuild only that black base, never composite over the old class photograph.
   if(out->class_){const auto&v=out->klass_.images[0];if(v.width!=1224||v.height!=584||!v.pixels||v.bytes!=size_t(1224)*584*4)return {};out->classComposite_.assign(v.pixels,v.pixels+v.bytes);for(unsigned y=0;y<512;++y)for(unsigned x=0;x<1120;++x)out->classComposite_[(size_t(y)*1224+x)*4+3]=255;if(!nfs_texture_sha256(out->classComposite_.data(),out->classComposite_.size(),out->classCompositeSha_.data()))return {};}
   // Provider input validation is not replaced by a borrowed numeric identity.
   NfsOriginalPreviewLayerView v{};if(!out->apply(p.view(),v)){diagnose("apply-rejected",p);return {};}diagnose("selected",p);return out;
  }catch(...){return {};}
 }
 uint64_t generation()const noexcept{return class_?klass_.generation:photo_.generation;}
 uintptr_t identity()const noexcept{return reinterpret_cast<uintptr_t>(class_?klass_.owner:photo_.owner);}
 // acquire() seals the complete pixel/PMA proof before publishing const Image.
 // The retained provider owner keeps those immutable bytes alive; hot-path
 // liveness needs only its nonzero identity and matching provider generation.
 bool current()const noexcept{return identity()&&generation()&&generation()==(class_?nfs_class_picture_art_generation():nfs_player_photo_art_generation());}
 bool apply(const NfsOriginalPreviewLayerView&original,NfsOriginalPreviewLayerView&out)const noexcept{
  out=original;if(selection_>=15||original.schema!=footprint_.schema||original.left!=footprint_.left||original.top!=footprint_.top||original.width!=footprint_.width||original.height!=footprint_.height||original.pixelCount!=footprint_.pixelCount||original.coverageCount!=footprint_.coverageCount||std::memcmp(original.sha256,footprint_.sha256,32)||std::memcmp(original.coverageSha256,footprint_.coverageSha256,32))return false;const auto*bytes=class_?klass_.images[0].pixels:photo_.images[0].pixels;const auto width=class_?klass_.images[0].width:photo_.images[0].width,height=class_?klass_.images[0].height:photo_.images[0].height;const auto count=class_?klass_.images[0].bytes:photo_.images[0].bytes;const auto*sha=class_?klass_.images[0].sha:photo_.images[0].sha;
  const auto generation=class_?klass_.generation:photo_.generation;const auto current=class_?nfs_class_picture_art_generation():nfs_player_photo_art_generation();
  const auto index=class_?klass_.images[0].sourceIndex:photo_.images[0].sourceIndex;const auto schema=class_?klass_.schema:photo_.schema,n=class_?klass_.count:photo_.count;
  if(schema!=1||n!=1||!generation||generation!=current||index!=(class_?selection_-12:selection_)||!bytes||width!=logicalWidth_*4||height!=logicalHeight_*4||count!=size_t(width)*height*4)return false;
  unsigned char actual[32];if(!nfs_texture_sha256(bytes,count,actual)||std::memcmp(actual,sha,32))return false;
  for(size_t at=0;at<count;at+=4)if((!class_&&bytes[at+3]!=255)||bytes[at]>bytes[at+3]||bytes[at+1]>bytes[at+3]||bytes[at+2]>bytes[at+3])return false;
  if(class_&&(classComposite_.size()!=count))return false;
  out.left=int32_t(left_);out.top=int32_t(top_);out.width=logicalWidth_;out.height=logicalHeight_;out.pixels565=canvas_.data();out.pixelCount=canvas_.size();out.coverage=coverage_.data();out.coverageCount=coverage_.size();std::copy(canvasSha_.begin(),canvasSha_.end(),out.sha256);std::copy(coverageSha_.begin(),coverageSha_.end(),out.coverageSha256);
  out.schema=NFS_AUTHORED_PREVIEW_LAYER_SCHEMA;out.authoredRGBA=class_?classComposite_.data():bytes;out.authoredBytes=count;out.authoredWidth=width;out.authoredHeight=height;std::memcpy(out.authoredSha256,class_?classCompositeSha_.data():sha,32);return true;
 }
};
}

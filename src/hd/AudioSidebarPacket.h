// SPDX-License-Identifier: MIT
// Source-neutral Audio cache verification. Runtime supplies independently trusted
// source catalogs and an actual Audio owner/cache sequencer; no upload is implied.
// Guest descriptor/LUT refcounts and source event order remain runtime obligations.
#pragma once
#include "BackgroundScenePrepare.h"
#include "SoftwareFramePacket.h"
#include "NestedSidebarCaptionDisplay.h"
#include <memory>
#include <vector>
namespace nfs_audio_sidebar {
constexpr size_t Pixels=343*480,Bytes=16+Pixels*2;
using Target=nfs_logo_capture::Target;
using Catalog=nfs_background_scene_cpu::SourceCatalog;
struct Binding {
 uint32_t form=0,sourceOwner=0,bitmap=0,archive=0;
 uint64_t ownerEpoch=0,cacheRevision=0,thread=0,lifetime=0;
 bool valid()const noexcept{return form&&sourceOwner&&bitmap&&archive&&ownerEpoch&&cacheRevision&&thread&&lifetime;}
 bool operator==(const Binding&b)const noexcept{return form==b.form&&sourceOwner==b.sourceOwner&&bitmap==b.bitmap&&archive==b.archive&&ownerEpoch==b.ownerEpoch&&cacheRevision==b.cacheRevision&&thread==b.thread&&lifetime==b.lifetime;}
};
struct Input {
 Binding binding;
 const nfs_background_scene_cpu::Scene&scene;
 const nfs_logo_capture::Group&logos;
 std::shared_ptr<const nfs_soft::FramePacket>caption;
 uint64_t acceptedCaptionSerial=0,acceptedCaptionLifetime=0;
 const std::vector<uint8_t>&sourceBefore;
 const std::vector<uint8_t>&sourceAfter;
 std::shared_ptr<const nfs_nested_sidebar_caption::Proof>captionProof{};nfs_nested_sidebar_caption::Binding captionBinding{};
};
inline nfs_soft::SurfaceIdentity surface(const Target&t,uint64_t lifetime)noexcept{
 nfs_soft::SurfaceIdentity s;s.lifetime=lifetime;std::copy(t.w.begin(),t.w.end(),s.geometry.begin());auto&a=s.geometry;a[14]=t.x[0];a[15]=t.x[1];a[16]=t.x[342];a[17]=t.y[0];a[18]=t.y[1];a[19]=t.y[479];return s;
}
// 40BBE0 flips only the selected cache word after the343 tail. That ambient
// ownerState[0] selects a handle; it is not the retained resource generation.
inline bool retainedTarget(const Target&a,const Target&b)noexcept{
 if(a.w!=b.w||a.header!=b.header||a.x!=b.x||a.y!=b.y||a.owner!=b.owner||a.ownerWords!=b.ownerWords)return false;
 for(unsigned k=1;k<5;++k)if(a.ownerState[k]!=b.ownerState[k])return false;return true;
}
class Packet {
 Binding binding_;Target target_;std::vector<uint8_t>raw_;
 std::shared_ptr<const Catalog>catalog_;
 std::shared_ptr<const nfs_soft::FramePacket>caption_;
 std::shared_ptr<const nfs_nested_sidebar_caption::Display>captionWord_;
 std::array<NfsBackgroundSceneWordLayer,4>words_{};
 std::array<NfsSmallLogoLayer,3>logos_{};unsigned wordCount_=0;
 Packet()=default;
 public:
 const std::shared_ptr<const nfs_nested_sidebar_caption::Display>&captionWord()const noexcept{return captionWord_;}
 const Binding&binding()const noexcept{return binding_;}
 const Target&target()const noexcept{return target_;}
 const std::vector<uint8_t>&raw()const noexcept{return raw_;}
 const std::shared_ptr<const Catalog>&catalog()const noexcept{return catalog_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&caption()const noexcept{return caption_;}
 const std::array<NfsBackgroundSceneWordLayer,4>&words()const noexcept{return words_;}
 const std::array<NfsSmallLogoLayer,3>&logos()const noexcept{return logos_;}
 unsigned wordCount()const noexcept{return wordCount_;}
 bool matches(const Binding&current,const Target&t,const uint8_t*raw,size_t count)const noexcept{return current.valid()&&current==binding_&&retainedTarget(t,target_)&&raw&&count==raw_.size()&&!std::memcmp(raw,raw_.data(),count);}
 static std::shared_ptr<const Packet>create(const Input&i,const Binding&current,const Target&currentTarget,std::shared_ptr<const Catalog>catalog)noexcept{
  namespace bg=nfs_background_scene_cpu;namespace sl=nfs_small_logo_live;
  const auto&s=i.scene;const auto&g=i.logos;const auto&t=s.preSide.target;
  if(!catalog||!bg::validCatalog(*catalog)||!i.binding.valid()||!(i.binding==current)||s.form!=i.binding.form||!s.complete||s.error||s.draws.empty()||s.draws.size()>5||!bg::plane(s.preSide,343)||t.owner!=i.binding.sourceOwner||t.w[8]!=i.binding.bitmap||!nfs_logo_capture::same(t,currentTarget)||g.thread!=i.binding.thread||!i.caption||!i.acceptedCaptionSerial||!i.acceptedCaptionLifetime)return {};
  if(!bg::plane(s.postSide,343)||!bg::plane(s.postWords,343)||!nfs_logo_capture::same(t,s.postSide.target)||!nfs_logo_capture::same(t,s.postWords.target)||i.sourceBefore.size()!=Bytes||i.sourceBefore!=i.sourceAfter)return {};
  for(unsigned k=0;k<4;++k)if(sl::le32(i.sourceBefore.data()+4*k)!=t.header[k])return {};
  const auto identity=surface(t,i.acceptedCaptionLifetime);
  if(!identity.valid()||i.caption->pixels()!=Pixels||!(i.caption->surface()==identity)||i.caption->serial()!=i.acceptedCaptionSerial)return {};
  try{
   auto work=std::unique_ptr<bg::Workspace>(new bg::Workspace{});
   if(bg::side(s.draws[0],t,*work,*catalog)!=bg::Failure::None||!std::equal(work->side.begin(),work->side.end(),s.postSide.pixels.begin()))return {};
   std::array<NfsBackgroundSceneWordLayer,4>words{};
   for(unsigned k=1;k<s.draws.size();++k)if(bg::word(s.draws[k],t,*work,words[k-1],*catalog)!=bg::Failure::None)return {};
   if(!std::equal(work->side.begin(),work->side.end(),s.postWords.pixels.begin())||!bg::samePlane(s.postWords,g.pre,343)||sl::verifyGroup(g,work->logo,catalog->logos)!=sl::Failure::None||!std::equal(i.caption->clean().begin(),i.caption->clean().end(),g.post.pixels.begin()))return {};
   for(unsigned k=0;k<Pixels;++k)work->side[k]=sl::le16(i.sourceBefore.data()+16+2*k);
   if(!i.caption->matches(identity,i.acceptedCaptionSerial,work->side.data(),Pixels))return {};
   std::shared_ptr<const nfs_nested_sidebar_caption::Display>captionWord;
   if(i.captionProof||!i.captionBinding.empty()){
    if(!i.captionProof||i.captionBinding.form!=i.binding.form||i.captionBinding.archive!=i.binding.archive||i.captionBinding.serial!=i.acceptedCaptionSerial||i.captionBinding.lifetime!=i.acceptedCaptionLifetime||!retainedTarget(i.captionProof->evidence().post.target,t))return {};
    captionWord=nfs_nested_sidebar_caption::Display::create(i.captionProof,i.caption,i.captionBinding);if(!captionWord)return {};
   }
   auto out=std::shared_ptr<Packet>(new Packet);out->binding_=i.binding;out->target_=t;out->raw_=i.sourceBefore;out->catalog_=std::make_shared<const Catalog>(*catalog);out->caption_=i.caption;out->captionWord_=std::move(captionWord);out->words_=words;out->wordCount_=unsigned(s.draws.size()-1);
   for(unsigned k=0;k<3;++k){const auto&d=g.draw[k];const auto&a=catalog->logos.identities[d.role];auto&l=out->logos_[k];l.role=d.role;l.scalar256=k==1?(d.args[1]&511):256;l.left=float(sl::signed12(a.header[3]));l.top=float(sl::signed12(a.header[3]>>16));l.right=l.left+float(a.width);l.bottom=l.top+float(a.height);l.clip[0]=float(t.w[3]);l.clip[1]=float(t.w[4]);l.clip[2]=float(t.w[5]);l.clip[3]=float(t.w[6]);}
   return out;
  }catch(...){return {};}
 }
};
} // namespace nfs_audio_sidebar

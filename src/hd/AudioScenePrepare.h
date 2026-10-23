// SPDX-License-Identifier: MIT
#pragma once
#include "AudioFramePacket.h"
#include "AudioSidebarPacket.h"
#include "AudioSceneRuntimeAbi.h"
namespace nfs_audio_scene_cpu {
// Immutable CPU join only. Independent live owners supply current bindings,
// source bytes and serials; the renderer still proves its own actual pass.
class Prepared final {
 std::shared_ptr<const nfs_audio_frame::AudioFramePacket>frame_;
 std::shared_ptr<const nfs_audio_sidebar::Packet>sidebar_;unsigned mapRole_=0;
 Prepared()=default;
 static bool captionWordValid(const std::shared_ptr<const nfs_audio_sidebar::Packet>&sidebar)noexcept{
  if(!sidebar)return false;const auto&word=sidebar->captionWord();
  return !word||(word->current()&&word->packet()==sidebar->caption()&&word->proof()&&word->proof()->evidence().policy==nfs_nested_sidebar_caption::Policy::Audio);
 }

 static bool textView(const std::shared_ptr<const nfs_soft::FramePacket>&p,unsigned width,NfsSoftwareTextLayerView&v)noexcept{
  if(!p||!p->font())return false;const auto&owner=p->font();const auto&f=*owner;
  struct Digest {std::shared_ptr<const nfs_soft::FontCatalog>owner;std::array<unsigned char,32>sha{};};
  static thread_local std::array<Digest,2>cache;unsigned at=width==343?0:1;auto&c=cache[at];
  if(c.owner.get()!=owner.get()||c.owner.owner_before(owner)||owner.owner_before(c.owner)){
   std::array<unsigned char,32>sha{};if(!nfs_texture_sha256(f.alpha().data(),f.alpha().size(),sha.data()))return false;c.owner=owner;c.sha=sha;
  }
  v.logicalWidth=width;v.logicalHeight=480;v.font.schema=1;v.font.width=f.width;v.font.height=f.height;v.font.identity=f.identity();v.font.alpha=f.alpha().data();v.font.bytes=f.alpha().size();std::copy(c.sha.begin(),c.sha.end(),v.font.sha256);v.vertices=p->vertices().data();v.vertexCount=p->vertices().size();return true;
 }
public:
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const nfs_audio_frame::AudioFramePacket>frame,std::shared_ptr<const nfs_audio_sidebar::Packet>sidebar,const nfs_audio_frame::Binding&current,uint64_t acceptedTextSerial,const uint16_t*post,size_t count,const nfs_audio_sidebar::Binding&currentCache,const nfs_logo_capture::Target&cacheTarget,const uint8_t*cacheBytes,size_t cacheCount)noexcept{
  if(!frame||!sidebar||!captionWordValid(sidebar)||!frame->matches(current,acceptedTextSerial,post,count)||!sidebar->matches(currentCache,cacheTarget,cacheBytes,cacheCount))return {};
  const auto&b=frame->background();if(!b||!b->catalog()||!sidebar->catalog()||current.form!=currentCache.form||current.archive!=currentCache.archive||current.ownerEpoch!=currentCache.ownerEpoch||b->sidebarHandle()!=currentCache.sourceOwner||b->sidebarBitmap()!=currentCache.bitmap||b->sidebarSource()!=sidebar->raw())return {};
  unsigned role=2;for(unsigned k=0;k<2;++k)if(b->catalog()->sha()==sidebar->catalog()->mapSha[k]){if(role!=2)return {};role=k;}if(role==2)return {};
  try{auto p=std::shared_ptr<Prepared>(new Prepared);p->frame_=std::move(frame);p->sidebar_=std::move(sidebar);p->mapRole_=role;return p;}catch(...){return {};}
 }
 const std::shared_ptr<const nfs_audio_frame::AudioFramePacket>&frame()const noexcept{return frame_;}
 const std::shared_ptr<const nfs_audio_sidebar::Packet>&sidebar()const noexcept{return sidebar_;}
 NfsAudioSceneFrameView view(uint64_t serial)const noexcept{
  NfsAudioSceneFrameView v{};if(!serial||!frame_||!sidebar_||!captionWordValid(sidebar_))return v;
  if(sidebar_->captionWord()){v.layers.sidebarWord=sidebar_->captionWord()->view();if(!v.layers.sidebarWord.schema)return {};}
  v.schema=NFS_AUDIO_SCENE_RUNTIME_SCHEMA;v.width=640;v.height=480;v.serial=serial;v.contentRevision=frame_->binding().pass;v.planeWords=nfs_audio_frame::Pixels;v.original565=frame_->text()->original().data();
  auto&b=v.layers.background;b.mapRole=mapRole_;b.wordCount=sidebar_->wordCount();b.panelClip[2]=343;b.panelClip[3]=480;std::copy(sidebar_->words().begin(),sidebar_->words().end(),b.words);std::copy(sidebar_->logos().begin(),sidebar_->logos().end(),b.logos);
  if(!textView(sidebar_->caption(),343,v.layers.sidebarCaption)||!textView(frame_->text(),640,v.layers.text))return {};
  v.layers.bars=frame_->layout().vertices.data();v.layers.barVertices=frame_->layout().vertexCount;v.art=frame_->art()->view();return v;
 }
};
} // namespace nfs_audio_scene_cpu

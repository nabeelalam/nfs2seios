// SPDX-License-Identifier: MIT
#pragma once
#include "GraphicsFramePacket.h"
#include "AudioSidebarPacket.h"
#include "ControlsSetupSceneRuntimeAbi.h"
#include "ControlsSetupSidebarCaptionDisplay.h"
#include "ControlsSetupWordDisplay.h"
#include "ControlsBindingPhraseDisplay.h"
#include <atomic>
namespace nfs_controls_setup_scene_cpu {
// Immutable CPU join only. Independent live owners supply current bindings,
// source bytes and serials; the renderer still proves its own actual pass.
class Prepared final {
 std::shared_ptr<const nfs_graphics_frame::GraphicsFramePacket>frame_;
 std::shared_ptr<const nfs_audio_sidebar::Packet>sidebar_;unsigned mapRole_=0;
 std::shared_ptr<const nfs_controls_setup_sidebar_caption::Display>caption_;
 std::shared_ptr<const nfs_controls_setup_word_display::Prepared>words_;
 std::shared_ptr<const nfs_controls_binding_phrase::Prepared>bindings_;
 Prepared()=default;
 static bool captionWordValid(const std::shared_ptr<const nfs_audio_sidebar::Packet>&sidebar,const std::shared_ptr<const nfs_controls_setup_sidebar_caption::Display>&caption)noexcept{
  if(!sidebar||sidebar->captionWord()||!caption||!caption->current()||caption->packet()!=sidebar->caption()||!caption->proof())return false;
  const auto&e=caption->proof()->evidence();const auto&b=sidebar->binding();const auto&c=caption->binding();
  return e.form==b.form&&e.incomingEbp==b.form&&e.post.archive==b.archive&&e.thread==b.thread&&e.serial==sidebar->caption()->serial()&&e.lifetime==sidebar->caption()->surface().lifetime&&e.ownerEpoch==b.ownerEpoch&&c.form==b.form&&c.archive==b.archive&&c.ownerEpoch==b.ownerEpoch;
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
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const nfs_graphics_frame::GraphicsFramePacket>frame,std::shared_ptr<const nfs_audio_sidebar::Packet>sidebar,const nfs_audio_frame::Binding&current,uint64_t acceptedTextSerial,const uint16_t*post,size_t count,const nfs_graphics_frame::SidebarBinding&currentSidebar,const nfs_audio_sidebar::Binding&currentCache,const nfs_logo_capture::Target&cacheTarget,const uint8_t*cacheBytes,size_t cacheCount,std::shared_ptr<const nfs_controls_setup_sidebar_caption::Display>caption)noexcept{
  if(!frame||!sidebar||!captionWordValid(sidebar,caption)||!currentSidebar.valid()||!frame->matches(current,acceptedTextSerial,post,count)||!sidebar->matches(currentCache,cacheTarget,cacheBytes,cacheCount))return {};
  auto words=nfs_controls_setup_word_display::Prepared::create(frame->text());if(!words)return {};
  const auto&b=frame->background();
  // Current selected table/selector, handle, raw source and complete LUT owner
  // are observed independently of the two candidate packets. Cache and caption
  // lifetimes are intentionally distinct from this current background lifetime.
  if(!b||!b->catalog()||!sidebar->catalog()||current.form!=currentCache.form||current.archive!=currentCache.archive||current.ownerEpoch!=currentCache.ownerEpoch||
     !(b->sidebar()==currentSidebar)||currentSidebar.handle!=currentCache.sourceOwner||currentSidebar.target.w[8]!=currentCache.bitmap||
     currentSidebar.target.ownerState[3]!=current.owner||!nfs_audio_sidebar::retainedTarget(currentSidebar.target,cacheTarget)||
     currentSidebar.source!=sidebar->raw())return {};
  const auto&e=caption->proof()->evidence();
  if(!e.game||!e.thread||e.thread!=currentCache.thread||!nfs_audio_sidebar::retainedTarget(e.post.target,cacheTarget))return {};
  unsigned role=2;for(unsigned k=0;k<2;++k)if(b->catalog()->sha()==sidebar->catalog()->mapSha[k]){if(role!=2)return {};role=k;}if(role==2)return {};
  auto bindings=nfs_controls_binding_phrase::Prepared::create(frame->text());if(!bindings)return {};
  try{auto p=std::shared_ptr<Prepared>(new Prepared);p->frame_=std::move(frame);p->sidebar_=std::move(sidebar);p->caption_=std::move(caption);p->words_=std::move(words);p->bindings_=std::move(bindings);p->mapRole_=role;return p;}catch(...){return {};}
 }
 const std::shared_ptr<const nfs_graphics_frame::GraphicsFramePacket>&frame()const noexcept{return frame_;}
 const std::shared_ptr<const nfs_audio_sidebar::Packet>&sidebar()const noexcept{return sidebar_;}
 NfsControlsSetupSceneFrameView view(uint64_t serial)const noexcept{
  NfsControlsSetupSceneFrameView v{};if(!serial||!frame_||!sidebar_||!captionWordValid(sidebar_,caption_)||!words_||!words_->current()||words_->packet()!=frame_->text()||!bindings_||bindings_->packet()!=frame_->text())return v;
  v.layers.sidebarWord=caption_->view();if(!v.layers.sidebarWord.schema||v.layers.sidebarWord.word!=3)return {};
  v.layers.bindingPhrases=bindings_->view();
  v.layers.childWords=words_->view();if(v.layers.childWords.schema!=NFS_CONTROLS_SETUP_WORD_SCHEMA||!v.layers.childWords.count)return {};
  v.schema=NFS_CONTROLS_SETUP_SCENE_RUNTIME_SCHEMA;v.width=640;v.height=480;v.serial=serial;v.contentRevision=frame_->binding().pass;v.planeWords=nfs_graphics_frame::Pixels;v.original565=frame_->text()->original().data();
  auto&b=v.layers.background;b.mapRole=mapRole_;b.wordCount=sidebar_->wordCount();b.panelClip[2]=343;b.panelClip[3]=480;std::copy(sidebar_->words().begin(),sidebar_->words().end(),b.words);std::copy(sidebar_->logos().begin(),sidebar_->logos().end(),b.logos);
  // Fixed labels and exact supported complete binding groups are authored.
  // Unsupported values and every unrelated source span retain their catalog.
  if(!textView(sidebar_->caption(),343,v.layers.sidebarCaption)||!textView(frame_->text(),640,v.layers.text))return {};
  return v;
 }
};
// The CPU join retains historical immutable resources. Live publication must
// still reacquire genuine current Controls text, cache and native pass owners.
// C callbacks hold a separate reference if queue cancellation removes its owner.
class Lease final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const Prepared>prepared_;Lease*const owner_;
 explicit Lease(std::shared_ptr<const Prepared>p):prepared_(std::move(p)),owner_(this){}
 ~Lease()=default;
 static void retain(void*p)noexcept{static_cast<Lease*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p)noexcept{auto*self=static_cast<Lease*>(p);if(self->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete self;}
public:
 static std::shared_ptr<const Lease>create(std::shared_ptr<const Prepared>p)noexcept{
  if(!p)return {};try{auto*owner=new Lease(std::move(p));return std::shared_ptr<const Lease>(owner,[owner](const Lease*){release(owner);});}catch(...){return {};}
 }
 NfsControlsSetupSceneFrameView view(uint64_t serial)const noexcept{auto v=prepared_->view(serial);if(v.schema){v.lifetimeOwner=owner_;v.retainOwner=retain;v.releaseOwner=release;}return v;}
 const std::shared_ptr<const Prepared>&prepared()const noexcept{return prepared_;}
};
} // namespace nfs_controls_setup_scene_cpu

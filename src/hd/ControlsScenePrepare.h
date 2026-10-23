// SPDX-License-Identifier: MIT
#pragma once
#include "GraphicsFramePacket.h"
#include "AudioSidebarPacket.h"
#include "ControlsSceneRuntimeAbi.h"
#include "ControlsDialogWordDisplay.h"
#include <atomic>
namespace nfs_controls_scene_cpu {
// Immutable CPU join only. Independent live owners supply current bindings,
// source bytes and serials; the renderer still proves its own actual pass.
class Prepared final {
 std::shared_ptr<const nfs_graphics_frame::GraphicsFramePacket>frame_;
 std::shared_ptr<const nfs_audio_sidebar::Packet>sidebar_;unsigned mapRole_=0;
 std::shared_ptr<const nfs_controls_dialog_display::Prepared>dialog_;
 Prepared()=default;
 static bool captionWordValid(const std::shared_ptr<const nfs_audio_sidebar::Packet>&sidebar)noexcept{
  if(!sidebar)return false;const auto&word=sidebar->captionWord();
  if(!word||!word->current()||word->packet()!=sidebar->caption()||!word->proof())return false;
  const auto&e=word->proof()->evidence();const auto&b=sidebar->binding();
  return e.policy==nfs_nested_sidebar_caption::Policy::Controls&&e.form==b.form&&e.incomingEbp==b.form&&e.post.archive==b.archive&&e.thread==b.thread&&e.serial==sidebar->caption()->serial()&&e.lifetime==sidebar->caption()->surface().lifetime;
 }
 static bool dialogWordValid(const std::shared_ptr<const nfs_graphics_frame::GraphicsFramePacket>&frame,const std::shared_ptr<const nfs_controls_dialog_display::Prepared>&dialog)noexcept{
  if(!frame||!dialog||!dialog->proof()||!dialog->current()||dialog->packet()!=frame->text())return false;
  const auto&b=dialog->binding();const auto&current=frame->binding();const auto&e=dialog->proof()->evidence();
  return b.game&&b.form==current.form&&b.archive==current.archive&&b.serial==frame->text()->serial()&&b.lifetime==current.lifetime&&e.post.identity==current.surface&&nfs_audio_frame::targetValid(e.post.target,current)&&e.post.target.w[8]==current.surface.geometry[8];
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
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const nfs_graphics_frame::GraphicsFramePacket>frame,std::shared_ptr<const nfs_audio_sidebar::Packet>sidebar,const nfs_audio_frame::Binding&current,uint64_t acceptedTextSerial,const uint16_t*post,size_t count,const nfs_graphics_frame::SidebarBinding&currentSidebar,const nfs_audio_sidebar::Binding&currentCache,const nfs_logo_capture::Target&cacheTarget,const uint8_t*cacheBytes,size_t cacheCount,std::shared_ptr<const nfs_controls_dialog_display::Prepared>dialog)noexcept{
  if(!frame||!sidebar||!captionWordValid(sidebar)||!dialogWordValid(frame,dialog)||!currentSidebar.valid()||!frame->matches(current,acceptedTextSerial,post,count)||!sidebar->matches(currentCache,cacheTarget,cacheBytes,cacheCount))return {};
  const auto&b=frame->background();
  // Current selected table/selector, handle, raw source and complete LUT owner
  // are observed independently of the two candidate packets. Cache and caption
  // lifetimes are intentionally distinct from this current background lifetime.
  if(!b||!b->catalog()||!sidebar->catalog()||current.form!=currentCache.form||current.archive!=currentCache.archive||current.ownerEpoch!=currentCache.ownerEpoch||
     !(b->sidebar()==currentSidebar)||currentSidebar.handle!=currentCache.sourceOwner||currentSidebar.target.w[8]!=currentCache.bitmap||
     currentSidebar.target.ownerState[3]!=current.owner||!nfs_audio_sidebar::retainedTarget(currentSidebar.target,cacheTarget)||
     currentSidebar.source!=sidebar->raw())return {};
  const auto&caption=sidebar->captionWord()->proof()->evidence();const auto&dialogEvidence=dialog->proof()->evidence();
  if(caption.game!=dialog->binding().game||caption.thread!=dialogEvidence.thread||caption.thread!=currentCache.thread||!caption.thread)return {};
  // Both collectors see the same descriptor table and full640 owner. Only the
  // ambient selected cache word may flip after the retained sidebar was drawn.
  for(unsigned k=1;k<5;++k)if(currentSidebar.target.ownerState[k]!=dialogEvidence.post.target.ownerState[k])return {};
  unsigned role=2;for(unsigned k=0;k<2;++k)if(b->catalog()->sha()==sidebar->catalog()->mapSha[k]){if(role!=2)return {};role=k;}if(role==2)return {};
  try{auto p=std::shared_ptr<Prepared>(new Prepared);p->frame_=std::move(frame);p->sidebar_=std::move(sidebar);p->dialog_=std::move(dialog);p->mapRole_=role;return p;}catch(...){return {};}
 }
 const std::shared_ptr<const nfs_graphics_frame::GraphicsFramePacket>&frame()const noexcept{return frame_;}
 const std::shared_ptr<const nfs_audio_sidebar::Packet>&sidebar()const noexcept{return sidebar_;}
 NfsControlsSceneFrameView view(uint64_t serial)const noexcept{
  NfsControlsSceneFrameView v{};if(!serial||!frame_||!sidebar_||!captionWordValid(sidebar_)||!dialogWordValid(frame_,dialog_))return v;
  v.layers.sidebarWord=sidebar_->captionWord()->view();if(!v.layers.sidebarWord.schema||v.layers.sidebarWord.word!=3)return {};
  v.layers.dialogWords=dialog_->view();if(v.layers.dialogWords.schema!=NFS_CONTROLS_DIALOG_WORD_SCHEMA||v.layers.dialogWords.count!=NFS_CONTROLS_DIALOG_WORD_PLACEMENTS)return {};
  v.schema=NFS_CONTROLS_SCENE_RUNTIME_SCHEMA;v.width=640;v.height=480;v.serial=serial;v.contentRevision=frame_->binding().pass;v.planeWords=nfs_graphics_frame::Pixels;v.original565=frame_->text()->original().data();
  auto&b=v.layers.background;b.mapRole=mapRole_;b.wordCount=sidebar_->wordCount();b.panelClip[2]=343;b.panelClip[3]=480;std::copy(sidebar_->words().begin(),sidebar_->words().end(),b.words);std::copy(sidebar_->logos().begin(),sidebar_->logos().end(),b.logos);
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
 NfsControlsSceneFrameView view(uint64_t serial)const noexcept{auto v=prepared_->view(serial);if(v.schema){v.lifetimeOwner=owner_;v.retainOwner=retain;v.releaseOwner=release;}return v;}
 const std::shared_ptr<const Prepared>&prepared()const noexcept{return prepared_;}
};
} // namespace nfs_controls_scene_cpu

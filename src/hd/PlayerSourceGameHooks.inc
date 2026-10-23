// SPDX-License-Identifier: MIT
#pragma once
#ifndef NFS_PLAYER_SOURCE_RUNTIME_TEST
#include "PlayerPreviewGameHooks.inc"
#include "PlayerCopyGameHooks.inc"
#endif
#include "PlayerNativeUploadWitness.h"
// Joins owned observations at the ACTUAL cached upload entry. This module does
// not change guest state, supply renderer keys, or authorize GPU presentation.
namespace nfs_player_source_live {
namespace live=nfs_player_scene_live;namespace cpu=nfs_player_scene_cpu;namespace native=nfs_player_native;
struct State {
 std::shared_ptr<const live::SidebarCapture>rawSidebar;std::shared_ptr<const cpu::SidebarProof>sidebar;
 std::shared_ptr<const cpu::VerifiedSource>source;std::shared_ptr<const nfs_player_initial_sidebar::Proof>initialSidebar;live::State*owner=nullptr;
 uint64_t epoch=0,event=0,content=0;cpu::SourceKind kind=cpu::SourceKind::Initial;
 nfs_logo_capture::Plane upload;bool preparing=false;
};
inline State*&slot()noexcept{struct Storage{State*p=nullptr;~Storage(){delete p;}};static thread_local Storage s;return s.p;}
inline bool equalEvent(const cpu::Event&a,const cpu::Event&b)noexcept{return cpu::sameEvent(a,b);}
inline live::State*current(Game&g)noexcept{auto*l=live::current(g);return l&&!l->invalid&&l->baseVerified&&l->catalogue&&l->formGuest==live::relocatedForm(l->form)&&l->sourceOwner==l->formGuest&&l->ownerEpoch&&l->currentCopy&&l->currentCopy->complete&&l->currentCopyEvent.ownerEpoch==l->ownerEpoch&&l->currentCopyEvent.sequence&&l->currentCopy->serial==l->copySerial&&equalEvent(l->copyEvent,l->currentCopyEvent)&&live::archiveCurrent(*l)?l:nullptr;}
inline bool copyKind(Game&g,live::State&l,cpu::SourceKind&kind)noexcept{auto*c=nfs_player_copy_live::slot();if(!c||c->invalid||c->depth||c->owner!=&l||c->game!=reinterpret_cast<uintptr_t>(&g)||c->thread!=l.thread||c->epoch!=l.ownerEpoch||c->serial!=l.currentCopy->serial||c->form!=l.form||c->formGuest!=l.formGuest||c->sourceOwner!=l.sourceOwner||c->archive!=l.archive||!equalEvent(c->baseEvent,l.base.event))return false;switch(c->site){case nfs_player_copy_live::Site::Initial:kind=cpu::SourceKind::Initial;break;case nfs_player_copy_live::Site::Refresh:kind=cpu::SourceKind::Refresh;break;case nfs_player_copy_live::Site::Lazy:kind=cpu::SourceKind::Lazy;break;default:return false;}return c->iteration==l.currentCopyEvent.iteration;}
inline bool anchoredPlane(nfs_logo_capture::Plane&p,const nfs_logo_capture::Target&reference)noexcept{p.valid=0;try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,p.target,reference.owner);if(!cpu::retainedBaseTarget(reference,p.target))return false;p.width=640;p.height=480;nfs_logo_publisher::checked(p.target.w[8],16+cpu::Full*2);std::memcpy(p.pixels.data(),nfs_host_address(p.target.w[8]+16,cpu::Full*2),cpu::Full*2);nfs_logo_capture::Target after;nfs_logo_publisher::target(tx,after,reference.owner);if(!nfs_logo_capture::same(after,p.target))return false;p.valid=1;return true;}catch(...){return false;}}
inline bool selectionCurrent(live::State&l)noexcept{nfs_player_preview::Selection actual;return l.preview&&l.previewSerial&&l.previewLifetime==l.ownerEpoch&&l.preview->form()==l.form&&l.preview->selection()==l.previewSelection&&nfs_player_preview_live::selection(l,actual)&&actual==l.previewSelection;}
inline bool swatchCurrent(live::State&l)noexcept{if(!l.swatch)return !l.swatchCurrent&&!l.acceptedSwatchCopySerial;const auto&b=l.swatchAuthority.binding;try{if(!l.swatchCurrent||b.form!=l.form||b.owner!=l.formGuest||b.epoch!=l.ownerEpoch||b.archive!=l.archive||b.car!=l.previewSelection.selectedCar)return false;nfs::GuestCopyTransaction tx;const uint32_t carAt=uint32_t(guest_address(b.player==1?byte_512DBF:byte_512DDA)),colors=uint32_t(guest_address(b.player==1?byte_512DC1:byte_512DDC));if(tx.load<uint8_t>(carAt)!=b.car||tx.load<uint8_t>(colors+b.car)!=b.color)return false;const uint32_t table=uint32_t(guest_address(dword_4D62C8))+16*b.color;for(unsigned k=0;k<4;++k)if(tx.load<uint32_t>(table+4*k)!=b.table[k])return false;return true;}catch(...){return false;}}
inline void reset()noexcept{if(auto*s=slot()){s->source.reset();s->rawSidebar.reset();s->sidebar.reset();s->initialSidebar.reset();s->owner=nullptr;s->epoch=s->event=s->content=0;s->preparing=false;s->upload.valid=0;}}
inline std::shared_ptr<const cpu::VerifiedSource>prepareSource(Game&g,uint64_t observedEvent)noexcept{
 auto*l=current(g);if(!l||!observedEvent||observedEvent==UINT64_MAX||nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)!=observedEvent||l->cachedUploadDepth!=1||l->cachedUploadSite==live::CachedUploadSite::None||uint32_t(g.edx)!=1||uint32_t(g.eax)!=l->base.post.target.w[8]||!selectionCurrent(*l)||!swatchCurrent(*l))return {};
 cpu::SourceKind kind;if(!copyKind(g,*l,kind))return {};const auto expected=kind==cpu::SourceKind::Initial?live::CachedUploadSite::Initial:kind==cpu::SourceKind::Refresh?live::CachedUploadSite::Refresh:live::CachedUploadSite::Lazy;if(l->cachedUploadSite!=expected)return {};
 auto raw=l->retainedSidebar;auto*copy=nfs_player_copy_live::slot();auto initial=(kind==cpu::SourceKind::Initial||kind==cpu::SourceKind::Refresh)&&copy?copy->initialSidebar:std::shared_ptr<const nfs_player_initial_sidebar::Proof>{};if(!l->actualSidebarPost.valid||!l->sidebarValidationEvent.sequence)return {};if(initial){if(initial!=l->pendingInitialSidebar||!live::initialSidebarCurrent(*l))return {};}else if(!raw||raw.get()!=live::retainedSidebarCapture(*l))return {};
 auto*&storage=slot();if(!storage)storage=new(std::nothrow)State;if(!storage)return {};auto&s=*storage;if(s.preparing){s.source.reset();return {};}s.preparing=true;struct Finish{State&s;~Finish(){s.preparing=false;}}finish{s};s.source.reset();
 if(s.owner!=l||s.epoch!=l->ownerEpoch){s.rawSidebar.reset();s.sidebar.reset();s.initialSidebar.reset();}s.initialSidebar=initial;s.owner=l;s.epoch=l->ownerEpoch;s.event=observedEvent;s.kind=kind;
 try{
  if(!initial&&(s.rawSidebar!=raw||!s.sidebar)){cpu::SidebarInput i{raw->scene,raw->group,raw->originalCopy,raw->caption,raw->copySerial,raw->captionSerial,raw->captionLifetime,raw->groupEvent,raw->tailEvent};i.compactCaption=raw->compactCaption;i.captionGame=raw->captionGame;i.captionFormGuest=raw->captionFormGuest;i.captionArchive=raw->archive;auto verified=cpu::verifySidebar(i,l->workspace,*l->catalogue);if(!verified)return {};s.rawSidebar=raw;s.sidebar=std::move(verified);}
  if(!anchoredPlane(s.upload,l->base.post.target)||uint32_t(g.eax)!=s.upload.target.w[8]||l->nextContent==UINT64_MAX)return {};
  // Fresh source event from the owner sequencer at this lexical upload entry.
  auto upload=l->event(l->currentCopyEvent.iteration);if(!upload.sequence||l->invalid)return {};const uint64_t content=++l->nextContent;
  cpu::JoinedInput i(l->base,*l->currentCopy);i.binding={l->form,l->formGuest,l->sourceOwner,l->archive,l->game,l->thread,l->ownerEpoch,l->ownerEpoch,observedEvent,content,l->currentCopy->serial,kind};if(initial){i.initialSidebar=initial;}else{i.sidebar=s.sidebar;i.acceptedSidebarSerial=raw->copySerial;i.sidebarLifetime=raw->captionLifetime;}i.actualSidebarPost=&l->actualSidebarPost;i.sidebarValidationEvent=l->sidebarValidationEvent;i.preview=l->preview;i.acceptedPreviewSerial=l->previewSerial;i.previewLifetime=l->previewLifetime;i.previewArchive=l->archive;i.previewBaseEvent=l->previewBaseEvent;i.previewEvent=l->previewEvent;i.actualPreviewPost=&l->actualPreviewPost;i.copyEvent=l->currentCopyEvent;i.uploadEvent=upload;i.actualUploadPost=&s.upload;
  if(l->swatch){i.swatch=l->swatch;i.swatchAuthority=l->swatchAuthority;i.acceptedSwatchCopySerial=l->acceptedSwatchCopySerial;i.swatchCopyEvent=l->swatchCopyEvent;i.swatchEvent=l->swatchEvent;i.swatchCopyPost=&l->swatchCopyPost;i.actualSwatchPost=&l->actualSwatchPost;}
  auto source=cpu::verifySource(i,l->workspace,*l->catalogue);if(!source||!source->current()||current(g)!=l||(initial&&(!live::initialSidebarCurrent(*l)||l->pendingInitialSidebar!=initial))||!selectionCurrent(*l)||!swatchCurrent(*l)||nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)!=observedEvent)return {};
  s.content=content;s.source=source;l->nativeUploadEvent=observedEvent;l->uploadEvent=upload;l->retainedSidebarVerified=true;l->retainedSidebarValidationEvent=l->sidebarValidationEvent;return source;
 }catch(...){return {};}
}
inline bool currentBinding(Game&g,const cpu::SourceBinding&occurrence,native::CurrentBinding&out)noexcept{
 out={};auto*l=current(g);auto*s=slot();if(!l||!s||s->preparing||s->owner!=l||s->epoch!=l->ownerEpoch||!s->source||!s->source->current()||s->content!=l->nextContent||s->event!=l->nativeUploadEvent||nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)!=s->event||!selectionCurrent(*l)||!swatchCurrent(*l))return false;
 if(s->initialSidebar){if((s->kind!=cpu::SourceKind::Initial&&s->kind!=cpu::SourceKind::Refresh)||s->initialSidebar!=l->pendingInitialSidebar||!live::initialSidebarCurrent(*l)||!s->initialSidebar->catalogMatches(*l->catalogue))return false;}else if(!s->sidebar||!s->sidebar->catalogMatches(*l->catalogue)||s->rawSidebar!=l->retainedSidebar)return false;
 cpu::SourceKind kind;if(!copyKind(g,*l,kind)||kind!=s->kind)return false;
 native::CurrentBinding now;now.source={l->form,l->formGuest,l->sourceOwner,l->archive,l->game,l->thread,l->ownerEpoch,l->ownerEpoch,s->event,l->nextContent,l->currentCopy->serial,kind};now.sourceOwner=l->formGuest;
 try{nfs::GuestCopyTransaction tx;now.fullTargetOwner=tx.load<uint32_t>(dword_4D4D24);if(!now.fullTargetOwner||now.fullTargetOwner!=l->base.post.target.owner)return false;now.sourceBitmap=tx.load<uint32_t>(now.fullTargetOwner+32);if(now.sourceBitmap!=l->base.post.target.w[8])return false;}catch(...){return false;}
 if(!cpu::sameBinding(occurrence,now.source)||!native::bindingMatches(*s->source,now))return false;out=now;return true;
}
inline bool relatedRetire(uint32_t at)noexcept{auto*s=slot();if(!at||!s||!s->owner)return false;const auto*l=s->owner;auto target=[&](const nfs_logo_capture::Target&t){return at==t.owner||at==t.w[8]||at==t.w[11]||at==t.w[12];};return at==l->formGuest||target(l->base.post.target)||target(l->actualSidebarPost.target)||(l->currentCopy&&target(l->currentCopy->sourceOwner))||(s->rawSidebar&&target(s->rawSidebar->scene.tailPost.target))||(s->initialSidebar&&target(s->initialSidebar->post().target));}
}
namespace nfs_player_native_live {
inline std::shared_ptr<const nfs_player_scene_cpu::VerifiedSource>prepareSource(Game&g,uint64_t event)noexcept{return nfs_player_source_live::prepareSource(g,event);}
inline bool currentBinding(Game&g,const nfs_player_scene_cpu::SourceBinding&occurrence,nfs_player_native::CurrentBinding&out)noexcept{return nfs_player_source_live::currentBinding(g,occurrence,out);}
}

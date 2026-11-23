// SPDX-License-Identifier: MIT
#pragma once
#ifndef NFS_PLAYER_QUEUE_RUNTIME_TEST
#include "PlayerSourceGameHooks.inc"
#include "PlayerNativeUploadGameHooks.inc"
#include "BackgroundSceneQueueAdapter.inc"
#include "ChildStaticSceneHistory.h"
#endif
#include "MainThreadBridge.h"
#include "WorkerControl.h"
#include <stdexcept>
namespace nfs_player_scene_live {
namespace pn=nfs_player_native;namespace pnu=nfs_player_native_live;
struct Accepted {State*owner=nullptr;uint64_t epoch=0;std::shared_ptr<const pn::Proof>proof;std::shared_ptr<const bg::Prepared>prepared;};
inline Accepted&accepted()noexcept{static thread_local Accepted a;return a;}
inline uint64_t&activeRequest()noexcept{return nfs_background_scene_live::activeRequest();}
#ifndef NFS_PLAYER_QUEUE_RUNTIME_TEST
inline bool rememberPreview(Game&,State&,const Accepted&,const std::shared_ptr<const pn::Proof>&,const pn::CurrentBinding&)noexcept;
#endif
inline void sealUpload(Game&g)noexcept{
 auto*l=current(g);if(!l)return;l->published.reset();accepted()={};l->stage=13;
 if(l->invalid||l->cachedUploadDepth!=1||l->cachedUploadSite==CachedUploadSite::None)return;auto*source=nfs_player_source_live::slot();if(!source)return;
#ifndef NFS_PLAYER_QUEUE_RUNTIME_TEST
 auto*native=pnu::slot();const unsigned nativeFailure=native?unsigned(native->failure):99,nativeTiles=native?native->witness.uploaded():0,nativeWitnessFailure=native?unsigned(native->witness.failure()):99;
#endif
 pn::CurrentBinding binding{};auto proof=pnu::proofForPresentation(g,binding,source->upload);if(!proof){
#ifndef NFS_PLAYER_QUEUE_RUNTIME_TEST
  nfs_player_source_live::sourceTrace(12,l,nativeWitnessFailure,nativeTiles,nativeFailure);
#endif
  return;}

 try{std::shared_ptr<bg::Prepared>prepared(new bg::Prepared);if(!proof->preparedForPresentation(binding,source->upload,*prepared)||!proof->current(binding)||current(g)!=l)return;
  accepted()={l,l->ownerEpoch,proof,prepared};l->published=std::move(prepared);
#ifndef NFS_PLAYER_QUEUE_RUNTIME_TEST
  // Retain authenticated upload semantics before subsequent menu input can
  // replace the binding, even if this completed frame is never presented.
  // Transition consumers still prove the actual native tiles and draw passes.
  (void)rememberPreview(g,*l,accepted(),proof,binding);
#endif
 }catch(...){l->published.reset();accepted()={};}
}
// A retained record supplies immutable source evidence only. Its renderer
// callback cannot access worker TLS or guest memory.
struct RetainedLease {std::shared_ptr<const void>owner;bool(*current)(const void*)=nullptr;bool(*workerCurrent)(const void*)=nullptr;};
struct Intent {RetainedLease retained;std::shared_ptr<const bg::Prepared>owned;std::shared_ptr<const pn::Proof>proof;pn::CurrentBinding binding{};uint64_t serial=0;bool beginConsumed=false,finishAttempted=false,finishConsumed=false;};
inline void cancel(Intent&i)noexcept{
 const auto serial=i.serial;const auto owned=i.owned;const auto proof=i.proof;const auto retained=i.retained;
 if(serial&&i.beginConsumed&&!i.finishConsumed)try{nfs::ScopedWorkerStopDeferral cleanup;(void)nfs::enqueue_owned_renderer([owned,proof,retained,serial](){(void)owned;(void)proof;(void)retained;return int32_t(nfs_private_player_scene_cancel(serial));});}catch(...){}
 if(activeRequest()==serial)activeRequest()=0;i.owned.reset();i.proof.reset();i.retained={};i.binding={};i.serial=0;i.beginConsumed=false;
}
inline bool lexicalNative(Game&g)noexcept{if(uint32_t(g.eax)!=1||g.edx||g.ebx||uint32_t(g.ecx)!=32||uint64_t(g.esp)+4>UINT64_C(0x100000000))return false;try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(uint32_t(g.esp))==UINT32_MAX;}catch(...){return false;}}
#ifndef NFS_PLAYER_QUEUE_RUNTIME_TEST
// Player Refresh may add its independently verified colour swatch after the
// original343 copy. Authenticate the final uploaded plane through the Player
// source/native proof, never by pretending Copy.POST equals that final plane.
struct PreviewSnapshot {
 std::shared_ptr<const bg::Prepared>prepared;
 std::shared_ptr<const pn::Proof>proof;
 std::shared_ptr<const bg::VerifiedSource>source;
};
inline bool rememberPreview(Game&g,State&l,const Accepted&a,const std::shared_ptr<const pn::Proof>&proof,const pn::CurrentBinding&binding)noexcept{
 unsigned stage=0;bool success=false;struct Trace{unsigned&stage;bool&success;State&l;~Trace(){nfs_child_static_scene::diagnostic(success?8:9,stage);const char*on=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(!on||on[0]!='1'||on[1])return;const auto event=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire);struct Key{uint64_t event;unsigned stage,success;};static thread_local Key keys[128];static thread_local unsigned used=0;for(unsigned k=0;k<used;++k)if(keys[k].event==event&&keys[k].stage==stage&&keys[k].success==unsigned(success))return;if(used==128)return;keys[used++]={event,stage,unsigned(success)};auto*s=nfs_child_static_scene::active();std::fprintf(stderr,"PLAYER HISTORY seal success=%u stage=%u event=%llu acceptedEvent=%llu savedEvent=%llu sidebarContent=%llu copy=%llu invalid=%u\n",unsigned(success),stage,(unsigned long long)event,(unsigned long long)l.nativeUploadEvent,(unsigned long long)(s&&s->record()?s->record()->domain.uploadEvent:0),(unsigned long long)(s&&s->sidebar()?s->sidebar()->content:0),(unsigned long long)l.copySerial,unsigned(l.invalid));}}trace{stage,success,l};
 try{
  stage=1;auto*scope=nfs_child_static_scene::active();const auto p=a.prepared;const auto copy=l.currentCopy;
  const uint64_t event=proof?proof->uploadEvent(binding):0;
  if(!scope||!scope->token()||!p||!p->valid||!copy||!proof||!event||!l.catalogue||!l.ownerEpoch||!l.archive||current(g)!=&l||l.invalid||a.owner!=&l||a.epoch!=l.ownerEpoch||a.proof!=proof||l.published!=p||!nfs_child_static_scene::matching(*scope->token(),l.game,l.thread,l.formGuest)||l.game!=reinterpret_cast<uintptr_t>(&g)||l.thread!=nfs_logo_publisher::threadToken()||relocatedForm(l.form)!=l.formGuest)return false;
  stage=2;auto source=proof->source(binding);std::array<uint32_t,300>keys{};
  if(!source||!source->current()||source->form()!=l.form||p->form!=l.form||binding.source.formGuest!=l.formGuest||binding.source.gameOwner!=l.game||binding.source.thread!=l.thread||binding.source.epoch!=l.ownerEpoch||binding.source.archive!=l.archive||binding.source.copySerial!=copy->serial||copy->serial!=l.copySerial||!proof->keys(binding,keys)||keys!=p->keys||source->original().size()!=nfs_background_scene_cpu::Full||!std::equal(source->original().begin(),source->original().end(),p->original.begin())||nfs_background_scene_runtime::acquireCatalog()!=l.catalogue)return false;
  stage=3;if(!copy->complete||copy->error||!copy->rows||copy->span!=nfs_logo_capture::MaxCopySource||!copy->linkedGroup||copy->thread!=l.thread||!copy->args[0]||copy->sourceOwner.w[8]!=copy->args[0]||copy->sourceBefore!=copy->sourceAfter||copy->shaBefore!=copy->shaAfter)return false;
  for(unsigned k=0;k<4;++k)if(nfs_background_scene_cpu::le32(copy->sourceAfter.data()+4*k)!=copy->sourceOwner.header[k])return false;
  const auto side=scope->sidebar();stage=31;
  if(!side)return false;stage=32;
  if(side->token!=scope->token()||!nfs_child_static_scene::matching(*side->token,l.game,l.thread,l.formGuest)||side->game!=l.game||side->thread!=l.thread||side->form!=l.form||side->formGuest!=l.formGuest||side->archive!=l.archive||side->epoch!=l.ownerEpoch||side->catalog!=l.catalogue||!side->gameLife||!side->gameLife->current()||!side->typed||!side->raw||side->raw->size()!=16+nfs_background_scene_cpu::Side*2||!side->uploadEvent||side->uploadEvent>event)return false;
  stage=33;{nfs::GuestCopyTransaction tx;for(unsigned k=0;k<300;++k){const auto descriptor=tx.load<uint32_t>(uint32_t(guest_address(dword_4EB74C))+4*k);std::array<uint32_t,7>w{};nfs_logo_publisher::descriptor(tx,descriptor,w);if(!descriptor||w[0]!=3||w[1]!=3||w[2]!=3||w[3]!=11||w[6]!=3||w[5]!=side->keys[k])return false;}}
  stage=4;NfsChildExitPreludeView v{};v.family=l.form==0x4d3050?NFS_CHILD_EXIT_PLAYER1:NFS_CHILD_EXIT_PLAYER2;v.form=l.form;v.player=p->view();
  if((l.form!=0x4d3050&&l.form!=0x4d3100)||!nfs_child_static_scene::validFamily(*p,v))return false;
  auto life=nfs_main_sidebar_origin::acquireGameLife(l.game);if(!life||!life->current())return false;
  stage=5;auto owned=std::make_shared<PreviewSnapshot>();owned->prepared=p;owned->proof=proof;owned->source=source;
  auto r=std::make_shared<nfs_child_static_scene::Record>();r->typed=owned;r->full=p;r->gameLife=life;r->token=scope->token();r->domain={l.game,l.thread,l.formGuest,l.ownerEpoch,l.archive,event,copy->linkedGroup,copy->serial,l.catalogue};r->sidebarOrigin=side;r->sidebarTarget=side->target;r->sidebarSha=side->sha;r->sidebarRaw=side->raw;
  v.schema=1;v.ownerEpoch=l.ownerEpoch;v.uploadEvent=event;v.sourceGame=l.game;v.sourceCatalog=reinterpret_cast<uintptr_t>(l.catalogue.get());v.sourceArchive=l.archive;v.sourceLife=life->serial();r->view=v;
  stage=6;if(scope->sidebar()!=side||current(g)!=&l||l.currentCopy!=copy||l.published!=p||!proof->current(binding)||proof->uploadEvent(binding)!=event||nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)!=event||!nfs_child_static_scene::matching(*scope->token(),l.game,l.thread,l.formGuest)||!life->current())return false;
  scope->store(r);success=true;return true;
 }catch(...){return false;}
}
// Reacquire the last authenticated full plane only at the original cached draw.
// A clear still revokes presentation. The ordinary renderer must prove the
// current target, every native tile payload and all300/600 native draws anew.
struct RetainedFrame {
 std::shared_ptr<const nfs_child_static_scene::Record>record;
 std::shared_ptr<const nfs_child_static_scene::SidebarRecord>sidebar;
 nfs_child_static_scene::Domain domain;uint32_t form=0;NfsMenuCachedSnapshot responsive{};
 static bool current(const void*at)noexcept{
  const auto*o=static_cast<const RetainedFrame*>(at);if(!o||!o->record||!o->sidebar)return false;
  const auto&r=*o->record;const auto&q=*o->sidebar;const auto&d=o->domain;uint64_t epoch=0;float x=0,y=0;
  return r.full&&r.full->valid&&r.typed&&r.token&&q.token==r.token&&
   nfs_child_static_scene::matching(*r.token,d.game,d.thread,d.formGuest)&&
   q.alive.load(std::memory_order_acquire)&&r.gameLife&&q.gameLife==r.gameLife&&r.gameLife->current()&&
   d.catalog&&nfs_background_scene_runtime::acquireCatalog()==d.catalog&&
   d.uploadEvent&&nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)==d.uploadEvent&&
   nfs_menu_cached_current(&o->responsive,&epoch,&x,&y)&&epoch==o->responsive.sourceEpoch;
 }
 static bool workerCurrent(const void*at)noexcept{
  const auto*o=static_cast<const RetainedFrame*>(at);if(!current(at))return false;
  auto*l=slot();const auto&d=o->domain;
  if(!l||!l->on||l->invalid||l->mainDepth!=1||l->game!=d.game||l->thread!=d.thread||
   l->thread!=nfs_logo_publisher::threadToken()||l->form!=o->form||l->formGuest!=d.formGuest||
   l->ownerEpoch!=d.ownerEpoch||l->archive!=d.archive||l->catalogue!=d.catalog||!archiveCurrent(*l))return false;
  std::shared_ptr<const nfs_child_static_scene::SidebarRecord>side;
  return nfs_child_static_scene::acquirePlayerHistoryForNativeRevalidation(d,o->form,side)==o->record&&side==o->sidebar;
 }
};
inline void beginRetained(Game&g,State&l,Intent&i){
 if(l.invalid||!l.baseVerified||l.formGuest!=relocatedForm(l.form)||!archiveCurrent(l))return;
 const uint64_t event=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire);
 const nfs_child_static_scene::Domain domain{l.game,l.thread,l.formGuest,l.ownerEpoch,l.archive,event,0,0,l.catalogue};
 std::shared_ptr<const nfs_child_static_scene::SidebarRecord>side;
 auto record=nfs_child_static_scene::acquirePlayerHistoryForNativeRevalidation(domain,l.form,side);
 if(!record||!record->view.player.schema||current(g)!=&l)return;
 const uint64_t serial=nfs_background_scene_live::nextRequest();if(!serial)return;
 auto owner=std::make_shared<RetainedFrame>();owner->record=record;owner->sidebar=side;owner->domain=domain;owner->form=l.form;
 const int savedErrno=errno;try{nfs_menu_cached_snapshot(l.form,l.formGuest,l.ownerEpoch,l.game,serial,record->full->contentRevision,&owner->responsive);}catch(...){nfs_menu_pointer_metadata_failure();}errno=savedErrno;
 if(!RetainedFrame::workerCurrent(owner.get()))return;
 i.retained={owner,RetainedFrame::current,RetainedFrame::workerCurrent};i.serial=serial;
 nfs::check_worker_stop();
 const auto receipt=nfs::enqueue_owned_renderer([owner,serial](){
  if(!RetainedFrame::current(owner.get()))return int32_t(0);
  auto view=owner->record->view.player;view.scene.serial=serial;view.responsive=owner->responsive;
  if(!RetainedFrame::current(owner.get()))return int32_t(0);
  return int32_t(nfs_private_player_scene_begin(&view));
 });
 if(receipt==nfs::OwnedRendererAcceptance::UseExistingOwnerPath){i={};return;}
 i.beginConsumed=true;activeRequest()=serial;nfs::check_worker_stop();
}

#endif
inline void begin(Game&g,Intent&i){
 auto*l=current(g);const auto a=accepted();if(!l||l->invalid||activeRequest()||i.serial||i.beginConsumed)return;
 nfs::check_worker_stop();if(!lexicalNative(g))return;
 auto*source=nfs_player_source_live::slot();pn::CurrentBinding binding{};std::shared_ptr<const pn::Proof>proof;
 if(l->published&&a.owner==l&&a.epoch==l->ownerEpoch&&a.proof&&a.prepared==l->published&&source)
  proof=pnu::proofForPresentation(g,binding,source->upload);
 if(proof!=a.proof||!proof||!proof->current(binding)||!proof->source(binding)){
#ifndef NFS_PLAYER_QUEUE_RUNTIME_TEST
  beginRetained(g,*l,i);
#endif
  return;
 }
#ifndef NFS_PLAYER_QUEUE_RUNTIME_TEST
 (void)rememberPreview(g,*l,a,proof,binding);
#endif
 const uint64_t serial=nfs_background_scene_live::nextRequest();if(!serial)return;
 NfsMenuCachedSnapshot responsive{};const int savedErrno=errno;try{nfs_menu_cached_snapshot(l->form,l->formGuest,l->ownerEpoch,l->game,serial,a.prepared->contentRevision,&responsive);}catch(...){nfs_menu_pointer_metadata_failure();}errno=savedErrno;
 i.owned=a.prepared;i.proof=proof;i.binding=binding;i.serial=serial;nfs::check_worker_stop();const auto owned=i.owned;
 // Only immutable native-owned buffers cross the FIFO. Guest pointers are not
 // read on the renderer owner. GPU context/target/tiles are checked there anew.
 const auto receipt=nfs::enqueue_owned_renderer([owned,proof,binding,serial,responsive](){if(!proof->current(binding))return int32_t(0);auto view=owned->view();view.scene.serial=serial;view.responsive=responsive;if(!proof->current(binding))return int32_t(0);return int32_t(nfs_private_player_scene_begin(&view));});
 if(receipt==nfs::OwnedRendererAcceptance::UseExistingOwnerPath){i={};return;}i.beginConsumed=true;activeRequest()=serial;nfs::check_worker_stop();
}
inline void finish(Intent&i){
 if(!i.beginConsumed||i.finishAttempted)return;if(!i.serial||activeRequest()!=i.serial){cancel(i);return;}
 nfs::check_worker_stop();i.finishAttempted=true;const auto owned=i.owned;const auto proof=i.proof;const auto binding=i.binding;const auto serial=i.serial;const auto retained=i.retained;
 if(retained.owner&&(!retained.workerCurrent||!retained.workerCurrent(retained.owner.get()))){cancel(i);return;}
 const auto receipt=nfs::enqueue_owned_renderer([owned,proof,binding,serial,retained](){(void)owned;if(retained.owner?(!retained.current||!retained.current(retained.owner.get())):(!proof||!proof->current(binding)))return int32_t(nfs_private_player_scene_cancel(serial));return int32_t(nfs_private_player_scene_finish(serial));});
 if(receipt!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("Player scene finish changed renderer owner");i.finishConsumed=true;activeRequest()=0;nfs::check_worker_stop();
}
struct PresentationScope {
 Intent intent;int unwind=0;
 explicit PresentationScope(Game&g):unwind(nfs_logo_publisher::exceptions()){try{begin(g,intent);}catch(...){cancel(intent);throw;}}
 ~PresentationScope()noexcept(false){if(nfs_logo_publisher::exceptions()>unwind){cancel(intent);return;}try{finish(intent);}catch(...){cancel(intent);throw;}}
};
inline void resetSources()noexcept{if(!enabled())return;pnu::reset();nfs_player_source_live::reset();accepted()={};if(auto*l=slot()){if(l->on&&l->thread==nfs_logo_publisher::threadToken()){l->published.reset();l->nativeUploadEvent=0;}}}
inline void retireSource(Game&g)noexcept{auto*l=current(g);if(!l)return;const uint32_t at=uint32_t(g.eax);if(!nfs_player_source_live::relatedRetire(at))return;pnu::retire(at);nfs_player_source_live::reset();accepted()={};l->fail(Reason::Retire,true);}
struct SourceLifetimeScope {bool active=false;explicit SourceLifetimeScope(Game&g)noexcept{active=current(g)!=nullptr;if(active)resetSources();}~SourceLifetimeScope(){if(active)resetSources();}};
} // namespace nfs_player_scene_live

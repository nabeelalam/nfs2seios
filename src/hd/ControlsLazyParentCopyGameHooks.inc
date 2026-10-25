// SPDX-License-Identifier: MIT
#pragma once
// Actual original Controls40BBE0 copy producer. A retained-plane update is
// issued only after exact copy replay, current ownership and HD recipe equality.
// No final modal/native admission is issued here.
#include "ControlsRetainedParentGameHooks.inc"
#include "ControlsLazyParentCopyReplay.h"
#include "ControlsSidebarRecipeEquivalent.h"
#include "ControlsLazyParentTextProtection.h"
#include "ControlsLazyParentRecomposition.h"
#include <fstream>
namespace nfs_controls_lazy_parent_live {
namespace copy=nfs_controls_lazy_parent_copy;
namespace parent=nfs_controls_retained_parent;
namespace cache=nfs_controls_sidebar_live;
class Snapshot {
 friend struct CopyScope;friend struct ResolveScope;friend struct TransformScope;friend struct DispatchScope;friend struct ByteScope;
 friend bool packetCurrent(Snapshot&,cache::State*&,nfs_logo_capture::Target&,std::vector<uint8_t>&)noexcept;
 friend bool bindingNow(Snapshot&,const cache::State&,const nfs_logo_capture::Target&,copy::Binding&)noexcept;
 friend void observeIndirect(Game&)noexcept;friend void checkpoint(Game&)noexcept;
 copy::Record record;copy::Current current;
 std::shared_ptr<const parent::detail::Retained>retained;
 std::shared_ptr<const parent::detail::LazyUpdate>previousUpdate;
 std::shared_ptr<const nfs_audio_sidebar::Packet>sidebar;
 nfs_audio_sidebar::Binding cacheBinding{};
 std::unique_ptr<const copy::VerifiedUpdate>verified;
 struct ActualDispatch { uint32_t callee;std::array<uint32_t,4> arguments; };
 struct TransformRow { uint32_t transform=0,forward=0,keyBefore=0,keyAfter=0;std::array<uint32_t,4> entry{},transformArgs{},forwardArgs{};std::vector<uint8_t> source,temporary,destinationBefore,destinationAfter; };
 std::vector<TransformRow> transforms;
 std::vector<ActualDispatch> actualDispatches;
 Game*game=nullptr;uint64_t sequence=0;unsigned resolveDepth=0,dispatchDepth=0,byteDepth=0,transformDepth=0;
 bool failed=false;uint32_t replayFailure=UINT32_MAX;const char*reason="none";
 void reject(const char*why)noexcept{failed=true;reason=why;record.error=1;}
 Snapshot()=default;
public:
 ~Snapshot()=default;
 Snapshot(const Snapshot&)=delete;Snapshot&operator=(const Snapshot&)=delete;
};
inline Snapshot*&active()noexcept{static thread_local Snapshot*s=nullptr;return s;}
inline std::shared_ptr<const Snapshot>&latest()noexcept{static thread_local std::shared_ptr<const Snapshot>s;return s;}
inline bool read(uint32_t at,void*out,size_t n)noexcept{return nfs_feedback_modal_live::read(at,out,n);}
inline bool raw(uint32_t at,size_t n,std::vector<uint8_t>&v)noexcept{try{v.resize(n);return read(at,v.data(),n);}catch(...){return false;}}
inline bool target(nfs_logo_capture::Target&t,uint32_t at)noexcept{try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t,at);return true;}catch(...){return false;}}
inline bool mainPlane(nfs_logo_capture::Target&t,std::vector<uint8_t>&pixels)noexcept{
 try{{nfs::GuestCopyTransaction tx;const uint32_t main=tx.load<uint32_t>(dword_4D4D24);nfs_logo_publisher::target(tx,t);if(t.owner!=main||!nfs_small_logo_live::target(t,640))return false;}return raw(t.w[8]+16,copy::PlaneBytes,pixels);}catch(...){return false;}
}
inline bool packetCurrent(Snapshot&s,cache::State*&c,nfs_logo_capture::Target&t,std::vector<uint8_t>&bytes)noexcept{
 c=cache::current(*s.game);if(!c||c->invalid||!cache::ownerCurrent(*c,*s.game)||c->phase!=cache::Phase::complete||!c->published||
    c->binding.form!=uint32_t(guest_address(off_4D37C8))||c->epoch!=s.retained->binding.ownerEpoch||c->thread!=s.retained->thread)return false;
 if(s.sidebar&&(s.sidebar!=c->published||!(c->binding==s.cacheBinding)))return false;
 if(!target(t,c->binding.sourceOwner)||!raw(c->binding.bitmap,copy::SourceBytes,bytes)||!c->published->matches(c->binding,t,bytes.data(),bytes.size()))return false;
 auto word=c->published->captionWord();if(!word||!word->current()||!word->proof()||word->packet()!=c->published->caption()||word->proof()->evidence().policy!=nfs_nested_sidebar_caption::Policy::Controls)return false;
 return cache::current(*s.game)==c&&cache::ownerCurrent(*c,*s.game)&&c->published->binding()==c->binding;
}
inline bool bindingNow(Snapshot&s,const cache::State&c,const nfs_logo_capture::Target&m,copy::Binding&out)noexcept{
 auto full=nfs_controls_live::current(*s.game);auto text=nfs_controls_text::current(*s.game);auto retained=parent::detail::slot();
 if(retained!=s.retained||!full||!text||!nfs_controls_live::bound(*full,*s.game,true)||nfs_controls_live::lastPacket(*s.game)!=retained->prepared->frame()||text->packet!=retained->dialog->packet()||text->dialogProof!=retained->dialog||text->serial!=full->acceptedTextSerial)return false;
 nfs_controls_dialog_word::Authority authority;if(!nfs_controls_text::readDialogAuthority(authority)||!(authority==retained->dialog->evidence().authority))return false;
 out.game=reinterpret_cast<uintptr_t>(s.game);out.thread=nfs_logo_publisher::threadToken();out.epoch=full->epoch;out.lifetime=text->lifetime;out.parentRevision=full->binding.pass;out.packetSerial=text->serial;out.cacheRevision=c.binding.cacheRevision;out.form=c.binding.form;out.archive=c.binding.archive;out.sourceOwner=c.binding.sourceOwner;out.sourceBitmap=c.binding.bitmap;out.mainOwner=m.owner;
 return out.valid()&&out.game==retained->game&&out.thread==retained->thread&&out.epoch==retained->binding.ownerEpoch&&out.lifetime==retained->binding.lifetime&&out.parentRevision==retained->binding.pass&&out.packetSerial==retained->prepared->frame()->text()->serial()&&out.mainOwner==retained->binding.owner;
}
inline std::array<uint32_t,4>args(Game&g)noexcept{return {{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};}
struct CopyScope {
private:
 std::shared_ptr<Snapshot>s;int unwind=0;
public:
 CopyScope(const CopyScope&)=delete;CopyScope&operator=(const CopyScope&)=delete;
 CopyScope(CopyScope&&)=delete;CopyScope&operator=(CopyScope&&)=delete;
 explicit CopyScope(Game&g)noexcept{
  if(active()||uint32_t(g.edx)!=0x2800||uint32_t(g.ebx)||uint32_t(g.ecx)||uint32_t(g.ebp)!=uint32_t(guest_address(off_4D37C8)))return;
  try{auto retained=parent::detail::slot();if(!retained||retained->dialog->evidence().authority.selectedIndex!=2)return;
   auto candidate=std::shared_ptr<Snapshot>(new Snapshot);candidate->game=&g;candidate->retained=retained;candidate->previousUpdate=parent::detail::updateSlot();if(candidate->previousUpdate&&candidate->previousUpdate->retained()!=retained)return;
   cache::State*c=nullptr;auto&r=candidate->record;
   if(!packetCurrent(*candidate,c,r.sourceBefore,r.sidebarBefore)||uint32_t(g.eax)!=c->binding.bitmap||!mainPlane(r.mainBefore,r.mainPlaneBefore))return;
   candidate->sidebar=c->published;candidate->cacheBinding=c->binding;
   auto&b=r.bindingBefore;if(!bindingNow(*candidate,*c,r.mainBefore,b))return;
   if(!b.valid()||b.game!=retained->game||b.thread!=retained->thread||b.mainOwner!=retained->binding.owner)return;
   nfs_controls_dialog_word::Authority authority;if(!nfs_controls_text::readDialogAuthority(authority)||!(authority==retained->dialog->evidence().authority))return;
   if(!read(uint32_t(guest_address(dword_4DD03C)),&r.keyBefore,4))return;
   r.site=0x40bbe0;r.mode=uint32_t(g.edx);r.originalArgs=args(g);r.packetAccepted=++candidate->sequence;r.copyBegin=++candidate->sequence;
   if(!bindingNow(*candidate,*c,r.mainBefore,candidate->current.binding)||!target(candidate->current.sidebar,c->binding.sourceOwner)||!target(candidate->current.main,uint32_t(guest_address(dword_4DB240))))return;candidate->current.sidebarBytes=candidate->sidebar->raw();
   unwind=nfs_logo_publisher::exceptions();s=std::move(candidate);active()=s.get();
  }catch(...){s.reset();}
 }
private:
 static bool samePlane(const std::vector<uint16_t>&words,const std::vector<uint8_t>&bytes)noexcept{
  if(bytes.size()!=words.size()*2)return false;for(size_t i=0;i<words.size();++i)if(words[i]!=uint16_t(uint16_t(bytes[2*i])|(uint16_t(bytes[2*i+1])<<8)))return false;return true;
 }
 void retainVerified()noexcept{
  if(!s||!s->verified||s->failed||s->record.rowKind!=copy::RowKind::Keyed)return;
  auto skip=[](const char*why){std::fprintf(stderr,"CONTROLS LAZY PARENT promoted=0 reason=%s\n",why);};
  try{
   if(parent::detail::slot()!=s->retained||parent::detail::updateSlot()!=s->previousUpdate){skip("retained-changed");return;}
   const auto&before=s->previousUpdate?s->previousUpdate->after():s->retained->prepared->frame()->text()->original();
   if(!samePlane(before,s->verified->beforePlane())){skip("before-chain");return;}
   if(samePlane(before,s->verified->afterPlane())){skip("unchanged-plane");return;}
   if(!parent::detail::sameMain(s->record.mainBefore,s->retained->main)||!parent::detail::sameMain(s->record.mainAfter,s->retained->main)){skip("main-metadata");return;}
   const auto&old=s->retained->prepared->sidebar();namespace image=nfs_controls_sidebar_recipe::detail;
   if(!old||!old->catalog()||!s->sidebar->catalog()||!old->caption()||!s->sidebar->caption()||!old->caption()->font()||!s->sidebar->caption()->font()||!old->captionWord()||!s->sidebar->captionWord()||!old->captionWord()->current()||!s->sidebar->captionWord()->current()||!nfs_background_scene_cpu::validCatalog(*s->sidebar->catalog())||!image::catalog(*old->catalog(),*s->sidebar->catalog())||!image::font(*old->caption()->font(),*s->sidebar->caption()->font())||!image::captionWord(old->captionWord()->view(),s->sidebar->captionWord()->view())){skip("sidebar-resource-identities");return;}
   auto base=s->retained->prepared->frame()->background();auto view=s->retained->prepared->view(1);unsigned role=2;
   if(!base||!base->catalog()||!s->sidebar->catalog()||!view.schema){skip("prepared-view");return;}
   for(unsigned k=0;k<2;++k)if(base->catalog()->sha()==s->sidebar->catalog()->mapSha[k]){if(role!=2){skip("map-ambiguous");return;}role=k;}
   if(role==2||role!=view.layers.background.mapRole){skip("map-role");return;}
   if(!nfs_controls_lazy_parent_text_protection::preserved(s->verified->geometry(),s->sidebar->raw(),s->record.keyAfter,view)){skip("text-occlusion");return;}
   const auto&text=s->retained->prepared->frame()->text();
   auto composition=nfs_controls_lazy_parent_recomposition::verify(base->catalog()->source(),s->sidebar->raw(),text->clean(),text->original(),s->verified->afterPlane(),view);
   if(!composition.recipe){std::fprintf(stderr,"CONTROLS LAZY RECOMPOSITION accepted=0 failure=%u\n",unsigned(composition.failure));skip("canonical-recomposition");return;}
   auto freshView=s->retained->prepared->viewWithSidebar(1,s->sidebar,role);if(!freshView.schema){skip("fresh-sidebar-view");return;}
   cache::State*c=nullptr;nfs_logo_capture::Target sidebarNow,mainNow;std::vector<uint8_t>sourceNow,planeNow;copy::Binding binding;
   if(!packetCurrent(*s,c,sidebarNow,sourceNow)||!mainPlane(mainNow,planeNow)||!nfs_logo_capture::same(mainNow,s->record.mainAfter)||planeNow!=s->verified->afterPlane()||!c||!bindingNow(*s,*c,mainNow,binding)||!(binding==s->record.bindingAfter)||parent::detail::slot()!=s->retained||parent::detail::updateSlot()!=s->previousUpdate){skip("repeat-current");return;}
   std::vector<uint16_t>after(planeNow.size()/2);for(size_t i=0;i<after.size();++i)after[i]=uint16_t(uint16_t(planeNow[2*i])|(uint16_t(planeNow[2*i+1])<<8));
   auto update=std::shared_ptr<const parent::detail::LazyUpdate>(new parent::detail::LazyUpdate(s->retained,s->sidebar,std::move(after),mainNow,s->record.keyAfter));
   parent::detail::updateSlot()=std::move(update);
   std::fprintf(stderr,"CONTROLS LAZY PARENT promoted=1 reason=exact-chain-and-canonical-fresh-recipe cache=%llu parent=%llu\n",(unsigned long long)s->record.bindingAfter.cacheRevision,(unsigned long long)s->record.bindingAfter.parentRevision);
  }catch(...){skip("allocation-or-read");}
 }
public:
 ~CopyScope(){if(!s)return;auto&r=s->record;if(active()==s.get())active()=nullptr;
  try{cache::State*c=nullptr;if(nfs_logo_publisher::exceptions()>unwind||s->resolveDepth||s->dispatchDepth||s->byteDepth||s->transformDepth||!packetCurrent(*s,c,r.sourceAfter,r.sidebarAfter)||!mainPlane(r.mainAfter,r.mainPlaneAfter))s->reject("copy-return-owner");
   if(!c||!bindingNow(*s,*c,r.mainAfter,r.bindingAfter))s->reject("copy-binding-after");if(!read(uint32_t(guest_address(dword_4DD03C)),&r.keyAfter,4))s->reject("copy-key-after");r.copyEnd=++s->sequence;r.complete=1;
   if(!r.rows.empty()){
    const auto&first=r.rows.front();const uint32_t sa=first.copyArgs[0],da=first.copyArgs[1];
    if(sa<r.bindingBefore.sourceBitmap+16||da<r.mainBefore.w[8]+16)s->reject("geometry-address");
    else{uint32_t so=(sa-r.bindingBefore.sourceBitmap-16)/2,po=(da-r.mainBefore.w[8]-16)/2;
     r.resolved.x=int(po%640)-int(so%343);r.resolved.y=int(po/640)-int(so/343);r.resolved.left=int(po%640);r.resolved.top=int(po/640);r.resolved.right=r.resolved.left+int(first.copyArgs[2]/2);r.resolved.bottom=r.resolved.top+int(r.rows.size());}
   }
   if(!r.keyedRows.empty()){
    const auto&first=r.keyedRows.front();const uint32_t sa=first.entry[0],da=first.entry[1];
    if(sa<r.bindingBefore.sourceBitmap+16||da<r.mainBefore.w[8]+16)s->reject("keyed-geometry-address");
    else{uint32_t so=(sa-r.bindingBefore.sourceBitmap-16)/2,po=(da-r.mainBefore.w[8]-16)/2;
     r.resolved.x=int(po%640)-int(so%343);r.resolved.y=int(po/640)-int(so/343);r.resolved.left=int(po%640);r.resolved.top=int(po/640);r.resolved.right=r.resolved.left+int(first.entry[2]);r.resolved.bottom=r.resolved.top+int(r.keyedRows.size());}
   }
   if(!s->failed){auto result=copy::ReplayVerifier::verify(r,s->current);s->replayFailure=uint32_t(result.failure);if(result.update)s->verified=std::move(result.update);else{s->reason="numeric-replay";s->failed=true;}}
   std::fprintf(stderr,"CONTROLS LAZY COPY observed=%u verified=%u reason=%s rows=%u x=%d y=%d clip=%d,%d,%d,%d cache=%llu parent=%llu header3=%u verifyCode=%u\n",unsigned(!s->failed),unsigned(bool(s->verified)),s->reason,unsigned(r.rowKind==copy::RowKind::Keyed?r.keyedRows.size():r.rows.size()),r.resolved.x,r.resolved.y,r.resolved.left,r.resolved.top,r.resolved.right,r.resolved.bottom,(unsigned long long)r.bindingBefore.cacheRevision,(unsigned long long)r.bindingBefore.parentRevision,r.sourceBefore.header[3],s->replayFailure);
   retainVerified();
   latest()=s;
  }catch(...){s->reject("copy-close-allocation");latest()=s;}
 }
};
struct ResolveScope {
 Snapshot*s=nullptr;int unwind=0;
 explicit ResolveScope(Game&g)noexcept{s=active();if(!s||s->game!=&g||s->failed){s=nullptr;return;}if(s->resolveDepth++||s->record.resolveBegin){s->reject("nested-resolve");return;}s->record.resolveBegin=++s->sequence;unwind=nfs_logo_publisher::exceptions();}
 ~ResolveScope(){if(!s)return;if(s->resolveDepth)--s->resolveDepth;if(nfs_logo_publisher::exceptions()>unwind)s->reject("resolve-unwind");s->record.resolveEnd=++s->sequence;}
};
// Diagnostic only: record the actual indirect choice, without assuming its identity
// or promoting it into a verified row/source record.
inline void observeIndirect(Game&g)noexcept{
 auto*s=active();if(!s||s->game!=&g||s->failed)return;
 try{if(s->actualDispatches.size()>=512){s->reject("indirect-capacity");return;}
  uint32_t callee=0;if(!read(uint32_t(g.esp)+0x38,&callee,sizeof(callee))){s->reject("indirect-read");return;}
  s->actualDispatches.push_back({callee,args(g)});
 }catch(...){s->reject("indirect-allocation");}
}
struct TransformScope {
 Snapshot*s=nullptr;Game&g;size_t index=0;int unwind=0;
 explicit TransformScope(Game&game)noexcept:g(game){s=active();if(!s||s->game!=&g||s->failed){s=nullptr;return;}
  if(s->resolveDepth!=1||s->transformDepth||!s->record.rows.empty()||s->record.keyedRows.size()>=480){s->reject("transform-order");s=nullptr;return;}
  try{s->transforms.emplace_back();s->record.keyedRows.emplace_back();index=s->record.keyedRows.size()-1;
   auto&k=s->record.keyedRows[index];k.wrapper=uint32_t(game_id_sub_4A1090);k.wrapperEsp=uint32_t(g.esp);k.entry=args(g);k.rowBegin=++s->sequence;s->record.rowKind=copy::RowKind::Keyed;
   if(!k.entry[2]||k.entry[2]>343||!raw(k.entry[0],k.entry[2]*2,k.sourceBefore)||!raw(k.entry[1],k.entry[2]*2,k.destinationBefore)){s->reject("transform-source");s=nullptr;return;}
   s->transformDepth=1;unwind=nfs_logo_publisher::exceptions();
  }catch(...){s->reject("transform-allocation");s=nullptr;}
 }
 void beforeTransform()noexcept{if(!s||s->failed)return;auto&k=s->record.keyedRows[index];k.transformArgs=args(g);k.temporaryAddress=uint32_t(g.edx);k.allocationBase=uint32_t(g.esp);k.allocationBytes=0x1400;k.transformBegin=++s->sequence;
  if(!read(uint32_t(guest_address(dword_4DD03C)),&k.keyBefore,4)||!read(uint32_t(guest_address(dword_571264)),&k.transform,4))s->reject("transform-id-key");
 }
 void afterTransform()noexcept{if(s&&!s->failed)s->record.keyedRows[index].transformEnd=++s->sequence;}
 void beforeForward()noexcept{if(!s||s->failed)return;auto&k=s->record.keyedRows[index];k.forwardArgs=args(g);k.forwardBegin=++s->sequence;
  if(!read(uint32_t(guest_address(dword_571260)),&k.forward,4)||!read(uint32_t(guest_address(dword_4DD03C)),&k.keyAfter,4)||!raw(uint32_t(g.eax),k.entry[2]*4,k.temporaryBeforeForward))s->reject("transform-intermediate");
 }
 void afterForward()noexcept{if(!s||s->failed)return;auto&k=s->record.keyedRows[index];k.forwardEnd=++s->sequence;
  if(!raw(k.entry[0],k.entry[2]*2,k.sourceAfter)||!raw(k.temporaryAddress,k.entry[2]*4,k.temporaryAfterForward)||!raw(k.entry[1],k.entry[2]*2,k.destinationAfter))s->reject("transform-forward-return");
 }
 ~TransformScope(){if(!s)return;auto&k=s->record.keyedRows[index];if(nfs_logo_publisher::exceptions()>unwind)s->reject("transform-unwind");k.rowEnd=++s->sequence;s->transformDepth=0;
  try{auto&d=s->transforms[index];d.transform=k.transform;d.forward=k.forward;d.keyBefore=k.keyBefore;d.keyAfter=k.keyAfter;d.entry=k.entry;d.transformArgs=k.transformArgs;d.forwardArgs=k.forwardArgs;d.source=k.sourceBefore;d.temporary=k.temporaryBeforeForward;d.destinationBefore=k.destinationBefore;d.destinationAfter=k.destinationAfter;}catch(...){s->reject("transform-diagnostic-allocation");}
 }
};
struct DispatchScope {
 Snapshot*s=nullptr;Game&g;size_t index=0;int unwind=0;
 explicit DispatchScope(Game&game)noexcept:g(game){s=active();if(!s||s->game!=&g||s->failed){s=nullptr;return;}
  if(s->resolveDepth!=1||s->dispatchDepth||s->record.rows.size()>=480){s->reject("dispatch-order");s=nullptr;return;}
  try{s->record.rows.emplace_back();index=s->record.rows.size()-1;auto&r=s->record.rows[index];r.dispatch=uint32_t(game_id_sub_483438);r.dispatchArgs=args(g);r.dispatchBegin=++s->sequence;s->dispatchDepth=1;unwind=nfs_logo_publisher::exceptions();}catch(...){s->reject("dispatch-allocation");s=nullptr;}
 }
 ~DispatchScope(){if(!s)return;auto&r=s->record.rows[index];if(s->byteDepth||nfs_logo_publisher::exceptions()>unwind)s->reject("dispatch-unwind");r.dispatchReturn=args(g);r.dispatchEnd=++s->sequence;s->dispatchDepth=0;}
};
struct ByteScope {
 Snapshot*s=nullptr;Game&g;size_t index=0;int unwind=0;
 explicit ByteScope(Game&game)noexcept:g(game){s=active();if(!s||s->game!=&g||s->failed){s=nullptr;return;}
  if(s->resolveDepth!=1||s->dispatchDepth!=1||s->byteDepth||s->record.rows.empty()){s->reject("byte-order");s=nullptr;return;}index=s->record.rows.size()-1;auto&r=s->record.rows[index];r.callback=uint32_t(game_id_sub_4A14D4);r.copyArgs=args(g);r.copyBegin=++s->sequence;
  const uint32_t n=r.copyArgs[2];if(!n||n>686||n%2||!raw(r.copyArgs[0],n,r.sourceBefore)||!raw(r.copyArgs[1],n,r.destinationBefore)){s->reject("byte-span");s=nullptr;return;}s->byteDepth=1;unwind=nfs_logo_publisher::exceptions();
 }
 ~ByteScope(){if(!s)return;auto&r=s->record.rows[index];if(nfs_logo_publisher::exceptions()>unwind||!raw(r.copyArgs[0],r.copyArgs[2],r.sourceAfter)||!raw(r.copyArgs[1],r.copyArgs[2],r.destinationAfter))s->reject("byte-return");r.copyReturn=args(g);r.copyEnd=++s->sequence;s->byteDepth=0;}
};
inline void recipeDiagnostic(std::ostream&f,const std::shared_ptr<const nfs_audio_sidebar::Packet>&a,const std::shared_ptr<const nfs_audio_sidebar::Packet>&b,uint32_t key){
 namespace d=nfs_controls_sidebar_recipe::detail;
 if(!a||!b||!a->catalog()||!b->catalog()||!a->caption()||!b->caption()||!a->captionWord()||!b->captionWord()){f<<"{\"valid\":false}";return;}
 size_t changed=0,keySupport=0;if(a->raw().size()==b->raw().size())for(size_t i=16;i+1<a->raw().size();i+=2){uint32_t x=uint32_t(a->raw()[i])|(uint32_t(a->raw()[i+1])<<8),y=uint32_t(b->raw()[i])|(uint32_t(b->raw()[i+1])<<8);changed+=x!=y;keySupport+=(x==key)!=(y==key);}
 f<<"{\"valid\":true,\"rawEqual\":"<<unsigned(a->raw()==b->raw())<<",\"changedPixelWords\":"<<changed<<",\"keySupportDifferences\":"<<keySupport<<",\"catalogEqual\":"<<unsigned(d::catalog(*a->catalog(),*b->catalog()))<<",\"captionEqual\":"<<unsigned(d::caption(*a->caption(),*b->caption()))<<",\"captionCleanEqual\":"<<unsigned(a->caption()->clean()==b->caption()->clean())<<",\"captionOriginalEqual\":"<<unsigned(a->caption()->original()==b->caption()->original())<<",\"fontEqual\":"<<unsigned(a->caption()->font()&&b->caption()->font()&&d::font(*a->caption()->font(),*b->caption()->font()))<<",\"captionWordEqual\":"<<unsigned(d::captionWord(a->captionWord()->view(),b->captionWord()->view()))<<",\"wordCounts\":["<<a->wordCount()<<","<<b->wordCount()<<"],\"words\":[";
 for(unsigned i=0;i<a->wordCount()&&i<b->wordCount()&&i<a->words().size();++i){const auto&x=a->words()[i];const auto&y=b->words()[i];f<<(i?",":"")<<"{\"equal\":"<<unsigned(d::word(x,y))<<",\"old\":["<<x.sourceRole<<","<<x.scalar256<<","<<x.left<<","<<x.top<<"],\"fresh\":["<<y.sourceRole<<","<<y.scalar256<<","<<y.left<<","<<y.top<<"]}";}
 f<<"],\"logos\":[";for(size_t i=0;i<a->logos().size();++i){const auto&x=a->logos()[i];const auto&y=b->logos()[i];f<<(i?",":"")<<"{\"equal\":"<<unsigned(d::logo(x,y))<<",\"old\":["<<x.role<<","<<x.scalar256<<","<<x.left<<","<<x.top<<","<<x.right<<","<<x.bottom<<"],\"fresh\":["<<y.role<<","<<y.scalar256<<","<<y.left<<","<<y.top<<","<<y.right<<","<<y.bottom<<"]}";}f<<"]}";
}
inline void checkpoint(Game&g)noexcept{
 auto s=latest();if(!s||s->game!=&g)return;const char*root=std::getenv("NFS_PRIVATE_FEEDBACK_OUTPUT");if(!root)return;
 try{auto put=[&](const char*name,const std::vector<uint8_t>&data){std::ofstream f(std::string(root)+"/"+name,std::ios::binary);f.write(reinterpret_cast<const char*>(data.data()),data.size());};put("lazy-copy-source.bin",s->record.sidebarBefore);put("lazy-copy-main-before.bin",s->record.mainPlaneBefore);put("lazy-copy-main-after.bin",s->record.mainPlaneAfter);
  auto current=parent::detail::slot();std::fprintf(stderr,"CONTROLS LAZY CHECKPOINT verified=%u sameRetained=%u sameParentRevision=%u rows=%u\n",unsigned(bool(s->verified)),unsigned(current==s->retained),unsigned(current&&current->binding.pass==s->record.bindingBefore.parentRevision),unsigned(s->record.rowKind==copy::RowKind::Keyed?s->record.keyedRows.size():s->record.rows.size()));
  if(s->retained&&s->retained->prepared&&s->sidebar){put("lazy-parent-snapshot-sidebar.bin",s->retained->prepared->sidebar()->raw());put("lazy-parent-source-sidebar.bin",s->sidebar->raw());if(current&&current->prepared)put("lazy-parent-current-sidebar.bin",current->prepared->sidebar()->raw());
   std::ofstream recipe(std::string(root)+"/lazy-parent-recipe.json");recipe<<"{\"snapshotPair\":";recipeDiagnostic(recipe,s->retained->prepared->sidebar(),s->sidebar,s->record.keyAfter);recipe<<",\"currentRetainedPair\":";recipeDiagnostic(recipe,current&&current->prepared?current->prepared->sidebar():nullptr,s->sidebar,s->record.keyAfter);recipe<<"}";
  }
  std::ofstream f(std::string(root)+"/lazy-copy-summary.json");const auto&r=s->record;
  std::vector<uint8_t> transformedBytes;std::ofstream rows(std::string(root)+"/lazy-transform-rows.json");rows<<"[";
  for(size_t i=0;i<s->transforms.size();++i){const auto&t=s->transforms[i];rows<<(i?",":"")<<"{\"transform\":"<<t.transform<<",\"forward\":"<<t.forward<<",\"keyBefore\":"<<t.keyBefore<<",\"keyAfter\":"<<t.keyAfter<<",\"entry\":[";for(unsigned k=0;k<4;++k)rows<<(k?",":"")<<t.entry[k];rows<<"],\"transformArgs\":[";for(unsigned k=0;k<4;++k)rows<<(k?",":"")<<t.transformArgs[k];rows<<"],\"forwardArgs\":[";for(unsigned k=0;k<4;++k)rows<<(k?",":"")<<t.forwardArgs[k];rows<<"]}";
   for(const auto*v:{&t.source,&t.temporary,&t.destinationBefore,&t.destinationAfter})transformedBytes.insert(transformedBytes.end(),v->begin(),v->end());
  }rows<<"]";put("lazy-transform-row-bytes.bin",transformedBytes);
  f<<"{\"resolveBegin\":"<<r.resolveBegin<<",\"resolveEnd\":"<<r.resolveEnd<<",\"actualDispatches\":[";
  for(size_t i=0;i<s->actualDispatches.size();++i){const auto&d=s->actualDispatches[i];f<<(i?",":"")<<"["<<d.callee;for(auto v:d.arguments)f<<","<<v;f<<"]";}f<<"],\"capture\":";
  f<<"{\"verified\":"<<unsigned(bool(s->verified))<<",\"reason\":\""<<s->reason<<"\",\"rows\":"<<(r.rowKind==copy::RowKind::Keyed?r.keyedRows.size():r.rows.size())<<",\"geometry\":["<<r.resolved.x<<","<<r.resolved.y<<","<<r.resolved.left<<","<<r.resolved.top<<","<<r.resolved.right<<","<<r.resolved.bottom<<"],\"header\":[";for(unsigned k=0;k<4;++k)f<<(k?",":"")<<r.sourceBefore.header[k];f<<"],\"cacheRevision\":"<<r.bindingBefore.cacheRevision<<",\"parentRevision\":"<<r.bindingBefore.parentRevision<<"}}";
 }catch(...){}
}
} // namespace nfs_controls_lazy_parent_live

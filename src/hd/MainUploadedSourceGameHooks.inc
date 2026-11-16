// SPDX-License-Identifier: MIT
#pragma once
#include "MainUploadedSourceLease.h"
namespace nfs_main_uploaded_source {
struct Errno{int value=errno;~Errno(){errno=value;}};
struct Store{std::shared_ptr<Owner>sealed;uint64_t attemptedEvent=0;std::shared_ptr<Owner>attempted;std::weak_ptr<OwnerLife>phase;};
inline Store&store(){static thread_local Store s;return s;}
inline void trace(unsigned event,const Owner&o,uint64_t iteration=0)noexcept{Errno preserve;static thread_local unsigned lines=0,retainedLines=0;const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(p&&p[0]=='1'&&!p[1]&&(event==5?retainedLines<8:lines<64)){if(event==5)++retainedLines;else ++lines;std::fprintf(stderr,"MAIN UPLOADED SOURCE event=%u ownerEpoch=%llu upload=%llu iteration=%llu status=%u sealed=%u token=%llu\n",event,(unsigned long long)o.domain.ownerEpoch,(unsigned long long)o.domain.uploadEvent,(unsigned long long)iteration,o.nativeStatus.load(std::memory_order_acquire),unsigned(o.completion.load(std::memory_order_acquire)&&o.sealed),(unsigned long long)(o.completion.load(std::memory_order_acquire)?o.seal.token:0));}}

// Main redraw and collector revisions are separate counters. Bind them at
// the exact lazy-start point while both lexical scopes are active; this is
// source-route identity only, never permission to adopt CPU work as GPU content.
struct IterationBinding {
 const nfs_background_scene_live::State*state=nullptr;uintptr_t game=0,thread=0;
 uint32_t form=0,archive=0;uint64_t ownerEpoch=0,iteration=0,collectorRevision=0;
 std::shared_ptr<OwnerLife>phase;std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>catalog;
 std::shared_ptr<const nfs_main_sidebar_origin::GameLife>gameLife;
};
inline IterationBinding&iterationBinding()noexcept{static thread_local IterationBinding b;return b;}
inline void bindCurrentIteration(Game&g)noexcept{Errno preserve;
 auto&b=iterationBinding();b={};
 auto*s=nfs_background_scene_live::current(g);auto*c=nfs_logo_publisher::store();const uint32_t form=uint32_t(guest_address(off_4D3EC4));
 if(!s||!s->on||s->invalid||s->mainDepth!=1||s->invocationDepth!=1||!s->archiveBound||!s->baseVerified||!s->ownerEpoch||!s->iteration||uint32_t(g.ebp)!=form||!s->uploadedLife||!s->uploadedLife->current()||!s->catalogue||nfs_background_scene_runtime::acquireCatalog()!=s->catalogue||s->groupSerial||s->copySerial||s->nativeUploadEvent||s->scene.complete||s->scene.event.ownerEpoch!=s->ownerEpoch||s->scene.event.iteration!=s->iteration||!s->scene.event.sequence||!c||!c->inMenu||c->invalid||c->gameOwner!=s->game||c->thread!=s->thread||c->depth!=1||c->invocationDepth!=1||c->ownerKind!=1||c->ownerArg!=form||!c->iterationRevision||c->group.serial!=c->iterationRevision||c->group.thread!=s->thread||c->group.kind!=1||c->group.ownerArg!=form||c->groupActive||c->copyActive||c->group.complete||c->copy.complete||c->group.draws||c->copy.serial||c->group.error||c->copy.error)return;
 auto life=nfs_main_sidebar_origin::acquireGameLife(s->game);if(!life||!life->current())return;
 b.state=s;b.game=s->game;b.thread=s->thread;b.form=form;b.archive=s->archive;b.ownerEpoch=s->ownerEpoch;b.iteration=s->iteration;b.collectorRevision=c->iterationRevision;b.phase=s->uploadedLife;b.catalog=s->catalogue;b.gameLife=std::move(life);
}
inline bool iterationCurrent(const nfs_background_scene_live::State&s,const nfs_logo_publisher::Collector&c)noexcept{
 const auto&b=iterationBinding();
 if(b.state!=&s||b.game!=s.game||b.thread!=s.thread||b.form!=uint32_t(guest_address(off_4D3EC4))||b.archive!=s.archive||b.ownerEpoch!=s.ownerEpoch||!b.iteration||b.iteration!=s.iteration||!b.collectorRevision||b.collectorRevision!=c.iterationRevision||c.group.serial!=b.collectorRevision||c.group.thread!=b.thread||c.group.kind!=1||c.group.ownerArg!=b.form||b.phase!=s.uploadedLife||!b.phase||!b.phase->current()||b.catalog!=s.catalogue||nfs_background_scene_runtime::acquireCatalog()!=b.catalog||!b.gameLife||!b.gameLife->current()||s.scene.event.ownerEpoch!=s.ownerEpoch||s.scene.event.iteration!=s.iteration||!s.scene.event.sequence)return false;
 if(s.groupSerial)return s.groupSerial==b.collectorRevision&&s.groupEvent.ownerEpoch==s.ownerEpoch&&s.groupEvent.iteration==s.iteration&&s.groupEvent.sequence;
 // Empty capture and later partial consumption are distinct states. Even
 // error-free partial records never become the retained GPU comparison plane.
 if(c.group.draws>3)return false;
 for(unsigned k=0;k<c.group.draws;++k)if(!c.group.draw[k].complete||c.group.draw[k].error)return false;
 return !s.groupEvent.sequence&&!s.copySerial&&!s.copyEvent.sequence&&!s.scene.complete&&!c.group.complete&&!c.copy.complete&&!c.copy.serial;
}

// Exact original40BBE0 early return after complete CPU copy, before slot1
// upload. This receipt never prepares or publishes that newer CPU plane.
struct OriginalAbortReceipt {
 const nfs_background_scene_live::State*state=nullptr;uintptr_t game=0,thread=0;
 uint32_t form=0,archive=0;uint64_t ownerEpoch=0,iteration=0,collectorRevision=0,group=0,copy=0,uploadEvent=0;
 std::shared_ptr<OwnerLife>phase;std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog>catalog;
 std::shared_ptr<const nfs_main_sidebar_origin::GameLife>gameLife;
};
inline OriginalAbortReceipt&originalAbortReceipt()noexcept{static thread_local OriginalAbortReceipt r;return r;}
// Bounded scalar refusal evidence; no source pixels, paths or new authority.
inline void abortRefusal(Game&g,const nfs_background_scene_live::State*s,const nfs_logo_publisher::Collector*c,unsigned stage,unsigned extra)noexcept{
 const uint32_t form=uint32_t(guest_address(off_4D3EC4));if(!s||!c||c->ownerArg!=form)return;
 const char*on=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");static thread_local unsigned count[5]{};if(!on||on[0]!='1'||on[1]||stage>4||count[stage]++>=3)return;
 unsigned f=0;
 if(!uint32_t(g.eax)||uint32_t(g.esp)>UINT32_MAX-0x1du||uint32_t(g.ebp)!=form)f|=1u;
 if(!s->on||s->mainDepth!=1||s->invocationDepth!=1||s->invalid)f|=2u;
 if(!s->archiveBound||!s->baseVerified||!s->iteration||s->stage!=4)f|=4u;
 if(!s->scene.complete||s->scene.error||s->published||s->nativeUploadEvent||s->uploadEvent.sequence)f|=8u;
 if(!s->uploadedLife||!s->uploadedLife->current()||!s->catalogue||nfs_background_scene_runtime::acquireCatalog()!=s->catalogue)f|=16u;
 if(!c->inMenu||c->invalid||c->gameOwner!=s->game||c->thread!=s->thread)f|=32u;
 if(c->depth!=1||c->invocationDepth!=1||c->ownerKind!=1||c->ownerArg!=form)f|=64u;
 if(c->iterationRevision!=s->groupSerial||c->group.serial!=c->iterationRevision)f|=128u;
 if(!iterationCurrent(*s,*c))f|=2048u;
 if(c->groupActive||c->copyActive||c->uploadActive||!c->group.complete||!c->copy.complete||c->group.error||c->copy.error)f|=256u;
 if(!s->groupSerial||!s->copySerial||c->group.serial!=s->groupSerial||c->copy.serial!=s->copySerial||c->copy.linkedGroup!=s->groupSerial||c->group.thread!=s->thread||c->copy.thread!=s->thread)f|=512u;
 if(s->groupEvent.ownerEpoch!=s->ownerEpoch||s->tailEvent.ownerEpoch!=s->ownerEpoch||s->copyEvent.ownerEpoch!=s->ownerEpoch||s->groupEvent.iteration!=s->iteration||s->tailEvent.iteration!=s->iteration||s->copyEvent.iteration!=s->iteration||!s->groupEvent.sequence||s->tailEvent.sequence<=s->groupEvent.sequence||s->copyEvent.sequence<=s->tailEvent.sequence)f|=1024u;
 std::fprintf(stderr,"MAIN ABORT REFUSED stage=%u flags=%u extra=%u epoch=%llu iteration=%llu collectorRevision=%llu group=%llu copy=%llu stateInvocation=%u collectorDepth=%u collectorInvocation=%u\n",stage,f,extra,(unsigned long long)s->ownerEpoch,(unsigned long long)s->iteration,(unsigned long long)c->iterationRevision,(unsigned long long)s->groupSerial,(unsigned long long)s->copySerial,s->invocationDepth,c->depth,c->invocationDepth);
}
inline void observeOriginalAbort(Game&g)noexcept{Errno preserve;
 auto&r=originalAbortReceipt();r={};auto*s=nfs_background_scene_live::current(g);auto*c=nfs_logo_publisher::store();
 unsigned refusedStage=1,refusedExtra=0;bool captured=false;struct Refusal{Game&g;const nfs_background_scene_live::State*s;const nfs_logo_publisher::Collector*c;unsigned&stage;unsigned&extra;bool&captured;~Refusal(){if(!captured)abortRefusal(g,s,c,stage,extra);}}refusal{g,s,c,refusedStage,refusedExtra,captured};
 const uint32_t form=uint32_t(guest_address(off_4D3EC4));
 if(!uint32_t(g.eax)||uint32_t(g.esp)>UINT32_MAX-0x1du||!s||!s->on||s->mainDepth!=1||s->invocationDepth!=1||s->invalid||!s->archiveBound||!s->baseVerified||!s->iteration||s->stage!=4||!s->scene.complete||s->scene.error||s->published||s->nativeUploadEvent||s->uploadEvent.sequence||uint32_t(g.ebp)!=form||!s->uploadedLife||!s->uploadedLife->current()||!s->catalogue||nfs_background_scene_runtime::acquireCatalog()!=s->catalogue||!c||!c->inMenu||c->invalid||c->gameOwner!=s->game||c->thread!=s->thread||c->depth!=1||c->invocationDepth!=1||c->ownerKind!=1||c->ownerArg!=form||!iterationCurrent(*s,*c)||c->iterationRevision!=s->groupSerial||c->group.serial!=c->iterationRevision||c->groupActive||c->copyActive||c->uploadActive||!c->group.complete||!c->copy.complete||c->group.error||c->copy.error||!s->groupSerial||!s->copySerial||c->group.serial!=s->groupSerial||c->copy.serial!=s->copySerial||c->copy.linkedGroup!=s->groupSerial||c->group.thread!=s->thread||c->copy.thread!=s->thread||s->groupEvent.ownerEpoch!=s->ownerEpoch||s->tailEvent.ownerEpoch!=s->ownerEpoch||s->copyEvent.ownerEpoch!=s->ownerEpoch||s->groupEvent.iteration!=s->iteration||s->tailEvent.iteration!=s->iteration||s->copyEvent.iteration!=s->iteration||!s->groupEvent.sequence||s->tailEvent.sequence<=s->groupEvent.sequence||s->copyEvent.sequence<=s->tailEvent.sequence)return;
 refusedStage=3;try{nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=s->archive)refusedExtra|=1u;if(tx.load<uint16_t>(uint32_t(g.esp)+0x1c)!=0)refusedExtra|=2u;if(refusedExtra)return;}catch(...){refusedExtra|=4u;return;}
 refusedStage=4;const uint64_t event=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire);auto life=nfs_main_sidebar_origin::acquireGameLife(s->game);if(!event||!life||!life->current())return;
 r.state=s;r.game=s->game;r.thread=s->thread;r.form=form;r.archive=s->archive;r.ownerEpoch=s->ownerEpoch;r.iteration=s->iteration;r.collectorRevision=c->iterationRevision;r.group=s->groupSerial;r.copy=s->copySerial;r.uploadEvent=event;r.phase=s->uploadedLife;r.catalog=s->catalogue;r.gameLife=std::move(life);captured=true;
 const char*flag=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");static thread_local unsigned lines=0;if(flag&&flag[0]=='1'&&!flag[1]&&lines++<8)std::fprintf(stderr,"MAIN ORIGINAL ABORT phase=2 epoch=%llu iteration=%llu event=%llu group=%llu copy=%llu\n",(unsigned long long)r.ownerEpoch,(unsigned long long)r.iteration,(unsigned long long)r.uploadEvent,(unsigned long long)r.group,(unsigned long long)r.copy);
}

inline bool args(Game&g,nfs_background_scene_live::State&s)noexcept{Errno preserve;
 if(!s.on||s.mainDepth!=1||s.invalid||!s.archiveBound||s.game!=reinterpret_cast<uintptr_t>(&g)||s.thread!=nfs_logo_publisher::threadToken()||!s.uploadedLife||!s.uploadedLife->current()||uint32_t(g.esi)!=uint32_t(guest_address(off_4D3EC4))||uint32_t(g.eax)!=1||g.edx||g.ebx||uint32_t(g.ecx)!=32)return false;
 try{nfs::GuestCopyTransaction tx;const bool archive=tx.load<uint32_t>(dword_4D40B8)==s.archive;if(!archive)s.uploadedLife->revoke();return archive&&tx.load<uint32_t>(uint32_t(g.esp))==UINT32_MAX;}catch(...){return false;}
}
inline bool descriptors(Game&,Owner&o,bool mint)noexcept{Errno preserve;
 try{nfs::GuestCopyTransaction tx;
  for(unsigned i=0;i<300;++i){const uint32_t d=tx.load<uint32_t>(dword_4EB74C+1200+i*4);if(!d||(d&3)||d>UINT32_MAX-27u||(!mint&&d!=o.descriptors[i]))return false;std::array<uint32_t,7>p{};nfs_logo_publisher::descriptor(tx,d,p);
   if(p[0]!=3||p[1]!=3||p[2]!=3||p[3]!=11||p[6]!=3||p[5]!=o.frame->keys[i])return false;
   if(mint){o.descriptors[i]=d;o.profiles[i]=p;o.profiles[i][4]=0;}else for(unsigned k=0;k<7;++k)if(k!=4&&p[k]!=o.profiles[i][k])return false;
  }
 }catch(...){return false;}return true;
}
inline std::shared_ptr<Owner>candidate(Game&g,nfs_background_scene_live::State&s,nfs_logo_publisher::Collector&c,const std::shared_ptr<const nfs_background_scene_cpu::Prepared>&frame)noexcept{Errno preserve;
 auto&st=store();if(st.phase.lock()!=s.uploadedLife){st.sealed.reset();st.attemptedEvent=0;st.phase=s.uploadedLife;}
 if(!args(g,s)||!frame||!frame->valid||s.published!=frame||!c.bound||!c.prepared||!c.copySourceVerified||!c.copy.complete||!c.group.complete||c.uploaded!=300||!s.nativeUploadEvent||s.nativeUploadEvent!=c.nativeUploadEvent )return {};
 if(st.attemptedEvent==s.nativeUploadEvent){auto prior=st.attempted;if(prior&&prior->nativeStatus.load(std::memory_order_acquire)!=3)return {};}
 st.attemptedEvent=s.nativeUploadEvent;
 try{auto o=adopt(new Owner);o->frame=frame;o->phase=s.uploadedLife;o->catalog=s.catalogue;o->gameLife=nfs_main_sidebar_origin::acquireGameLife(s.game);if(!o->gameLife)return {};o->domain.game=s.game;o->domain.catalog=reinterpret_cast<uintptr_t>(s.catalogue.get());o->domain.life=o->gameLife->serial();o->domain.ownerEpoch=s.ownerEpoch;o->domain.archive=s.archive;o->domain.form=0x401910u;o->domain.uploadEvent=s.nativeUploadEvent;o->ledger=&nfs_logo_publisher::uploadEvents();o->completedIteration=s.iteration;if(!Owner::current(o.get())||!descriptors(g,*o,true))return {};for(unsigned i=0;i<300;++i)if(o->descriptors[i]!=c.descriptors[i]||frame->keys[i]!=c.rendererAddresses[i])return {};st.attempted=o;trace(1,*o,s.iteration);return o;}catch(...){return {};}
}
// A completed343 group is still pending FULL640 when copyEnd has not occurred.
// This does not prepare/adopt the mutable collector; only the older sealed
// upload remains authoritative, and its normal native300/600 proof still runs.
inline bool pendingGroup(const nfs_background_scene_live::State&s,const nfs_logo_publisher::Collector&c)noexcept{
 if(!c.group.complete)return true;
 return s.stage==4&&s.groupSerial==c.iterationRevision&&c.group.serial==s.groupSerial&&c.group.thread==s.thread&&s.groupEvent.ownerEpoch==s.ownerEpoch&&s.groupEvent.iteration==s.iteration&&s.groupEvent.sequence&&!s.scene.complete&&!s.copySerial&&!s.copyEvent.sequence&&!s.nativeUploadEvent;
}
inline bool postCopyAbortCurrent(const nfs_background_scene_live::State&s,const nfs_logo_publisher::Collector&c,const Owner&o)noexcept{
 const auto&r=originalAbortReceipt();
 return c.stop==nfs_logo_publisher::Collector::Stop::Owner&&!c.prepared&&!c.bound&&!c.copySourceVerified&&!c.uploadActive&&!c.presentationArmed&&r.state==&s&&r.game==s.game&&r.thread==s.thread&&r.form==uint32_t(guest_address(off_4D3EC4))&&r.archive==s.archive&&r.ownerEpoch==s.ownerEpoch&&r.iteration==s.iteration&&r.collectorRevision==c.iterationRevision&&r.group&&r.group==r.collectorRevision&&r.group==s.groupSerial&&r.group==c.group.serial&&r.copy&&r.copy==s.copySerial&&r.copy==c.copy.serial&&c.copy.linkedGroup==r.group&&c.group.complete&&c.copy.complete&&!c.group.error&&!c.copy.error&&c.group.thread==r.thread&&c.copy.thread==r.thread&&s.scene.complete&&!s.scene.error&&!s.invocationDepth&&!s.nativeUploadEvent&&!s.uploadEvent.sequence&&r.phase==s.uploadedLife&&r.phase==o.phase&&r.catalog==s.catalogue&&r.catalog==o.catalog&&r.gameLife==o.gameLife&&r.gameLife&&r.gameLife->current()&&r.uploadEvent==o.domain.uploadEvent&&r.uploadEvent==nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire);
}
inline std::shared_ptr<Owner>pending(Game&g,nfs_background_scene_live::State&s,nfs_logo_publisher::Collector*c)noexcept{Errno preserve;
 auto &st=store();auto done=st.attempted;if(done&&done->completion.load(std::memory_order_acquire)){if(done->sealed&&Owner::current(done.get())){if(st.sealed!=done)trace(3,*done,s.iteration);st.sealed=done;}else if(st.sealed==done)st.sealed.reset();}auto o=st.sealed;
 if(!o||!o->sealed||o->nativeStatus.load(std::memory_order_acquire)!=2||!o->seal.token||!Owner::current(o.get())||!args(g,s)||s.published||!s.iteration||s.iteration<=o->completedIteration||s.invocationDepth||s.nativeUploadEvent||!s.baseVerified||!c||!c->inMenu||c->ownerKind!=1||c->ownerArg!=uint32_t(guest_address(off_4D3EC4))||!iterationCurrent(s,*c)||c->invalid||c->depth||c->invocationDepth||c->groupActive||c->copyActive||c->copy.error||c->group.error||c->gameOwner!=s.game||c->thread!=s.thread||o->phase!=s.uploadedLife||o->catalog!=s.catalogue||nfs_background_scene_runtime::acquireCatalog()!=o->catalog||o->domain.game!=s.game||o->domain.ownerEpoch!=s.ownerEpoch||o->domain.archive!=s.archive)return {};
 const bool postCopy=postCopyAbortCurrent(s,*c,*o);
 if(!postCopy&&(s.scene.complete||s.copySerial||c->copy.complete||!pendingGroup(s,*c)))return {};
 if(!descriptors(g,*o,false)||!Owner::current(o.get()))return {};trace(postCopy?5u:2u,*o,s.iteration);return o;
}
inline bool seed(Game&g,nfs_background_scene_live::State&s,const std::shared_ptr<Owner>&o)noexcept{Errno preserve;
 if(!o||!Owner::current(o.get())||!args(g,s))return false;uint32_t x=0,y=0,slot5=0,valid=0;
 try{{nfs::GuestCopyTransaction tx;x=tx.load<uint32_t>(dword_4D4C7C);y=tx.load<uint32_t>(dword_4D4C80);const uint32_t d=tx.load<uint32_t>(dword_4EB5A4+20);if(d&&!(d&3)&&d<=UINT32_MAX-27u){slot5=tx.load<uint32_t>(d+20);valid=!(slot5&3u)&&slot5<=0x200000u-32768u;}}
  if(!Owner::current(o.get())||nfs_background_scene_live::current(g)!=&s)return false;nfs_menu_responsive_source_observation(0x401910u,x,y,slot5,valid);return true;
 }catch(...){return false;}
}
inline void completed(const std::shared_ptr<Owner>&o)noexcept{Errno preserve;if(!o||!o->completion.load(std::memory_order_acquire))return;trace(o->sealed?3u:4u,*o);if(o->sealed&&Owner::current(o.get()))store().sealed=o;else if(store().sealed==o)store().sealed.reset();}
} // namespace

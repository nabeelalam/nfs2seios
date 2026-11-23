// SPDX-License-Identifier: MIT
#pragma once
#include "OptionsUploadedSourceAbi.h"
#include "Main343OriginLifetime.h"
namespace nfs_options_uploaded_source {
namespace live=nfs_options_scene_live;
struct Owner {
 std::atomic<unsigned>refs{1},nativeStatus{0};
 std::shared_ptr<const nfs_options_scene_cpu::Prepared>frame;
 std::shared_ptr<const std::vector<uint8_t>>sidebarHistory;
 nfs_logo_capture::Target sidebarHistoryOwner{};
 std::shared_ptr<std::atomic<bool>>life;
 std::shared_ptr<const nfs_main_sidebar_origin::GameLife>gameLife;
 std::shared_ptr<const nfs_options_scene_cpu::SourceCatalog>catalog;
 NfsMainUploadedDomain domain{};NfsMainUploadedSeal seal{};
 std::array<uint32_t,300>descriptors{};
 std::array<std::array<uint32_t,7>,300>profiles{};
 uint64_t completedIteration=0;std::atomic<bool>completion{false};bool sealed=false;
 static void retain(void*p){if(p)static_cast<Owner*>(p)->refs.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p){if(p&&static_cast<Owner*>(p)->refs.fetch_sub(1,std::memory_order_acq_rel)==1)delete static_cast<Owner*>(p);}
 static int current(void*p){auto*o=static_cast<Owner*>(p);return o&&o->frame&&o->frame->valid&&o->frame->captionWord&&o->frame->captionWord->current()&&o->life&&o->life->load(std::memory_order_acquire)&&o->gameLife&&o->gameLife->current()&&o->domain.uploadEvent&&nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)==o->domain.uploadEvent;}
 static void status(void*p,unsigned n){if(p)static_cast<Owner*>(p)->nativeStatus.store(n,std::memory_order_release);}
 NfsOptionsUploadedView view(uint64_t serial,const NfsMenuCachedSnapshot&snapshot){NfsOptionsUploadedView v{};v.schema=1;v.domain=domain;v.frame=frame->view();v.frame.scene.serial=serial;v.frame.responsive=snapshot;v.owner=this;v.retain=retain;v.release=release;v.current=current;v.status=status;return v;}
};
struct Store{std::shared_ptr<Owner>attempted,sealed;};
inline Store&store(){static thread_local Store s;return s;}
inline void trace(unsigned event,const Owner&o,uint64_t iteration)noexcept {
 const int saved=errno;static thread_local unsigned lines[6]{};const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");
 if(p&&p[0]=='1'&&!p[1]&&event<6&&lines[event]<16){++lines[event];std::fprintf(stderr,"OPTIONS UPLOADED SOURCE event=%u epoch=%llu upload=%llu iteration=%llu status=%u token=%llu liveUpload=%llu\n",event,(unsigned long long)o.domain.ownerEpoch,(unsigned long long)o.domain.uploadEvent,(unsigned long long)iteration,o.nativeStatus.load(std::memory_order_acquire),(unsigned long long)(o.completion.load(std::memory_order_acquire)?o.seal.token:0),(unsigned long long)nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire));}errno=saved;
}
inline bool sourceCurrent(Game&g,live::State&s)noexcept {
 return live::current(g)==&s&&!s.invalid&&s.baseVerified&&s.uploadedOwnerLife&&s.uploadedOwnerLife->load(std::memory_order_acquire)&&live::archiveCurrent(s);
}
inline bool args(Game&g,live::State&s)noexcept {
 if(!sourceCurrent(g,s)||uint32_t(g.esi)!=uint32_t(guest_address(off_4D3A88))||uint32_t(g.eax)!=1||g.edx||g.ebx||uint32_t(g.ecx)!=32)return false;
 try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(uint32_t(g.esp))==UINT32_MAX;}catch(...){return false;}
}
inline bool descriptors(Owner&o,bool mint)noexcept {
 try{nfs::GuestCopyTransaction tx;for(unsigned i=0;i<300;++i){const uint32_t d=tx.load<uint32_t>(dword_4EB74C+1200+i*4);if(!d||(d&3)||d>UINT32_MAX-27u||(!mint&&d!=o.descriptors[i]))return false;
 std::array<uint32_t,7>p{};nfs_logo_publisher::descriptor(tx,d,p);if(p[0]!=3||p[1]!=3||p[2]!=3||p[3]!=11||p[6]!=3||p[5]!=o.frame->keys[i])return false;
 if(mint){o.descriptors[i]=d;o.profiles[i]=p;o.profiles[i][4]=0;}else for(unsigned k=0;k<7;++k)if(k!=4&&p[k]!=o.profiles[i][k])return false;}}catch(...){return false;}return true;
}
inline std::shared_ptr<Owner>candidate(Game&g,live::State&s,nfs_logo_publisher::Collector&c,const std::shared_ptr<const nfs_options_scene_cpu::Prepared>&frame)noexcept {
 if(!args(g,s)||!frame||s.published!=frame||!frame->valid||!c.bound||!c.prepared||!c.copySourceVerified||!c.copy.complete||!c.group.complete||c.uploaded!=300||!s.nativeUploadEvent||s.nativeUploadEvent!=c.nativeUploadEvent)return {};
 if(c.copy.error||c.copy.span!=16+nfs_options_scene_cpu::Side*2||c.copy.span>c.copy.sourceBefore.size()||c.copy.sourceBefore!=c.copy.sourceAfter||c.copy.serial!=s.copySerial||c.copy.linkedGroup!=s.groupSerial||c.copy.thread!=s.thread||c.copy.sourceOwner.w[8]!=c.copy.args[0])return {};
 auto&st=store();if(st.attempted&&st.attempted->domain.uploadEvent==s.nativeUploadEvent&&st.attempted->nativeStatus.load(std::memory_order_acquire)!=3&&Owner::current(st.attempted.get()))return {};
 try{auto o=std::shared_ptr<Owner>(new Owner,[](Owner*p){Owner::release(p);});o->frame=frame;o->sidebarHistory=std::make_shared<const std::vector<uint8_t>>(c.copy.sourceBefore.begin(),c.copy.sourceBefore.begin()+c.copy.span);o->sidebarHistoryOwner=c.copy.sourceOwner;
 for(unsigned k=0;k<4;++k)if(nfs_options_scene_cpu::le32(o->sidebarHistory->data()+4*k)!=o->sidebarHistoryOwner.header[k])return {};
 o->life=s.uploadedOwnerLife;o->catalog=s.catalogue;o->gameLife=nfs_main_sidebar_origin::acquireGameLife(s.game);if(!o->gameLife)return {};
 o->domain={s.game,reinterpret_cast<uintptr_t>(s.catalogue.get()),o->gameLife->serial(),s.ownerEpoch,s.nativeUploadEvent,s.archive,0x4d3a88u};o->completedIteration=s.iteration;
 if(!Owner::current(o.get())||!descriptors(*o,true))return {};for(unsigned k=0;k<300;++k)if(o->descriptors[k]!=c.descriptors[k]||frame->keys[k]!=c.rendererAddresses[k])return {};st.attempted=o;trace(1,*o,s.iteration);return o;}catch(...){return {};}
}
inline void pendingTrace(Game&g,live::State&s,nfs_logo_publisher::Collector*c,const std::shared_ptr<Owner>&o,const std::shared_ptr<Owner>&attempted,const live::OriginalAbortReceipt&a)noexcept {
 const int saved=errno;const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(!p||p[0]!='1'||p[1]){errno=saved;return;}
 const bool receipt=a.ownerEpoch==s.ownerEpoch&&a.iteration==s.iteration;
 unsigned flags=unsigned(bool(o));if(o){flags|=unsigned(o->sealed)<<1;flags|=unsigned(o->completion.load(std::memory_order_acquire))<<2;flags|=unsigned(o->nativeStatus.load(std::memory_order_acquire)==2)<<3;flags|=unsigned(bool(o->seal.token))<<4;flags|=unsigned(Owner::current(o.get()))<<5;}
 flags|=unsigned(sourceCurrent(g,s))<<6;flags|=unsigned(bool(s.published))<<7;flags|=unsigned(bool(c))<<8;flags|=unsigned(receipt)<<9;flags|=unsigned(!s.nativeUploadEvent)<<10;
 const unsigned phase=receipt?a.phase:4,status=attempted?attempted->nativeStatus.load(std::memory_order_acquire):4;
 struct Seen{unsigned flags,phase,status;};static thread_local Seen seen[64]{};static thread_local unsigned used;
 for(unsigned k=0;k<used;++k)if(seen[k].flags==flags&&seen[k].phase==phase&&seen[k].status==status){errno=saved;return;}
 if(used==64){errno=saved;return;}seen[used++]={flags,phase,status};
 std::fprintf(stderr,"OPTIONS LEASE PENDING flags=%u phase=%u attemptStatus=%u attemptComplete=%u epoch=%llu iteration=%llu expectedUpload=%llu liveUpload=%llu stateUpload=%llu sourceCopy=%u copyComplete=%u groupComplete=%u groupDraws=%u\n",flags,phase,status,unsigned(attempted&&attempted->completion.load(std::memory_order_acquire)),(unsigned long long)s.ownerEpoch,(unsigned long long)s.iteration,(unsigned long long)(o?o->domain.uploadEvent:0),(unsigned long long)nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire),(unsigned long long)s.nativeUploadEvent,unsigned(c&&c->copySourceVerified),unsigned(c&&c->copy.complete),unsigned(c&&c->group.complete),c?c->group.draws:0);errno=saved;
}
inline std::shared_ptr<Owner>pendingSource(Game&g,live::State&s,nfs_logo_publisher::Collector*c)noexcept {
 auto&st=store();auto done=st.attempted;
 if(done&&done->completion.load(std::memory_order_acquire)){
  if(done->sealed&&Owner::current(done.get()))st.sealed=done;
  else if(st.sealed==done)st.sealed.reset();
 }
 auto o=st.sealed;const auto&a=live::originalAbortReceipt();pendingTrace(g,s,c,o,st.attempted,a);
 if(!o||!o->sealed||!o->completion.load(std::memory_order_acquire)||o->nativeStatus.load(std::memory_order_acquire)!=2||!o->seal.token||!Owner::current(o.get())||!sourceCurrent(g,s)||s.published||!c||!c->inMenu||c->ownerKind!=1||c->ownerArg!=uint32_t(guest_address(off_4D3A88))||c->invalid||c->depth||c->invocationDepth||c->groupActive||c->copyActive||c->group.error||c->copy.error||c->gameOwner!=s.game||c->thread!=s.thread||c->iterationRevision!=a.collectorRevision||!s.iteration||s.iteration<=o->completedIteration||s.nativeUploadEvent||a.ownerEpoch!=s.ownerEpoch||a.iteration!=s.iteration||a.uploadEvent!=o->domain.uploadEvent||a.phase>3||o->life!=s.uploadedOwnerLife||o->catalog!=s.catalogue||nfs_background_scene_runtime::acquireCatalog()!=o->catalog||o->domain.game!=s.game||o->domain.ownerEpoch!=s.ownerEpoch||o->domain.archive!=s.archive)return {};
 if(a.phase==0&&(a.group||s.groupSerial||a.copy||s.copySerial||s.scene.complete||c->group.complete||c->copy.complete||c->group.draws||c->copy.serial||c->group.serial!=a.collectorRevision))return {};
 if(a.phase!=0&&(!a.group||a.group!=s.groupSerial||a.group!=c->group.serial))return {};
 if((a.phase==1||a.phase==2)&&!c->group.complete)return {};
 if(a.phase==3){
  if((a.groupDraws!=1&&a.groupDraws!=2)||c->group.draws!=a.groupDraws||c->group.complete||a.copy||s.copySerial||s.scene.complete||c->copy.serial||c->copy.complete)return {};
  for(unsigned k=0;k<a.groupDraws;++k)if(!c->group.draw[k].complete||c->group.draw[k].error)return {};
 }
 if(a.phase==1&&(a.copy||s.copySerial||s.scene.complete||c->copy.complete))return {};
 if(a.phase==2&&(!a.copy||a.copy!=s.copySerial||a.copy!=c->copy.serial||!s.scene.complete||!c->copy.complete||c->copy.linkedGroup!=a.group))return {};
 if(!descriptors(*o,false)||!Owner::current(o.get()))return {};trace(2,*o,s.iteration);return o;
}
// The draw route retains its exact427B30 register/stack guard. The return
// route acquires the same immutable source seal at its independently hooked
// original Escape boundary; no draw arguments are fabricated there.
inline std::shared_ptr<Owner>pending(Game&g,live::State&s,nfs_logo_publisher::Collector*c)noexcept {
 if(!args(g,s))return {};return pendingSource(g,s,c);
}
inline std::shared_ptr<Owner>pendingForReturn(Game&g,live::State&s)noexcept {
 if(uint32_t(g.esi)!=uint32_t(guest_address(off_4D3A88)))return {};
 return pendingSource(g,s,nfs_logo_publisher::store());
}
inline void completed(const std::shared_ptr<Owner>&o)noexcept {if(!o||!o->completion.load(std::memory_order_acquire))return;if(o->sealed&&Owner::current(o.get()))store().sealed=o;else if(store().sealed==o)store().sealed.reset();}
}

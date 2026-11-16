// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundSceneLiveGameHooks.inc"
#include "MainSourceFrameGameHooks.inc"
#include "MainThreadBridge.h"
#include "MainUploadedSourceGameHooks.inc"
#include <stdexcept>
namespace nfs_background_scene_live {
struct Intent{std::shared_ptr<nfs_main_uploaded_source::Owner>uploaded;bool retained=false;std::shared_ptr<const bg::Prepared>owned;uint64_t serial=0;bool beginConsumed=false,finishAttempted=false,finishConsumed=false;};
inline uint64_t nextRequest()noexcept{static std::atomic<uint64_t>n{1};uint64_t old=n.load(std::memory_order_acquire);while(old&&old!=UINT64_MAX){if(n.compare_exchange_weak(old,old+1,std::memory_order_acq_rel))return old;}return 0;}
inline uint64_t&activeRequest()noexcept{static thread_local uint64_t v=0;return v;}
struct Builder{State*s=nullptr;nfs_logo_publisher::Collector*c=nullptr;std::shared_ptr<const bg::Prepared>owned;};
inline bool sourceSnapshot(void*context,const nfs_logo_publisher::OwnedPacket&packet)noexcept{auto&b=*static_cast<Builder*>(context);if(!b.s||!b.c||b.s->invalid||!b.s->published||&packet!=&b.c->packet||!packet.prepared.valid||b.s->copySerial!=b.c->copy.serial||b.s->groupSerial!=b.c->group.serial||b.s->nativeUploadEvent!=b.c->nativeUploadEvent||b.s->published->original!=packet.originalCopyPost565||b.s->published->keys!=b.c->rendererAddresses)return false;b.owned=b.s->published;return true;}
inline void cancel(Intent&i)noexcept{if(activeRequest()==i.serial)activeRequest()=0;i.owned.reset();i.uploaded.reset();i.serial=0;i.beginConsumed=false;}
inline void enqueueBegin(const std::shared_ptr<const bg::Prepared>&owned,Intent&i,const std::shared_ptr<nfs_main_uploaded_source::Owner>&uploaded={},bool retained=false){if(!enabled()||!owned||!owned->valid||activeRequest()||i.serial||i.beginConsumed)return;const uint64_t serial=nextRequest();if(!serial)return;i.owned=owned;i.uploaded=uploaded;i.retained=retained;i.serial=serial;nfs::check_worker_stop();const auto receipt=nfs::enqueue_owned_renderer([owned,uploaded,retained,serial](){if(uploaded){if(!nfs_main_uploaded_source::Owner::current(uploaded.get()))return int32_t(0);auto v=uploaded->view(serial);return int32_t(nfs_private_main_uploaded_begin(&v,retained?&uploaded->seal:nullptr));}auto view=owned->view();view.serial=serial;return int32_t(nfs_private_background_scene_begin(&view));});if(receipt==nfs::OwnedRendererAcceptance::UseExistingOwnerPath){i.owned.reset();i.uploaded.reset();i.serial=0;return;}i.beginConsumed=true;activeRequest()=serial;nfs::check_worker_stop();}
// Diagnostic snapshots contain only owned scalar state, never guest bytes/paths.
struct MainBeginFacts {
 uint64_t epoch=0,iteration=0,copy=0,group=0,event=0,currentEvent=0;
 unsigned state=0,invalid=0,reason=0,stage=0,published=0,collector=0,stop=0,prepared=0,cpu=0,bound=0,packet=0,depth=0,uploaded=0,copyComplete=0,groupComplete=0,copyError=0,groupError=0;
};
inline bool mainBeginTraceEnabled()noexcept{const int saved=errno;const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");const bool on=p&&p[0]=='1'&&!p[1];errno=saved;return on;}
inline MainBeginFacts mainBeginFacts(const State*s,const nfs_logo_publisher::Collector*c)noexcept{
 MainBeginFacts f;if(s){f.state=1;f.epoch=s->ownerEpoch;f.iteration=s->iteration;f.invalid=s->invalid;f.reason=unsigned(s->reason);f.stage=s->stage;f.published=bool(s->published);f.copy=s->copySerial;f.group=s->groupSerial;f.event=s->nativeUploadEvent;}
 if(c){f.collector=1;f.stop=unsigned(c->stop);f.prepared=c->prepared;f.cpu=c->copySourceVerified;f.bound=c->bound;f.packet=c->packet.prepared.valid;f.depth=c->depth;f.uploaded=c->uploaded;f.copyComplete=c->copy.complete;f.groupComplete=c->group.complete;f.copyError=c->copy.error;f.groupError=c->group.error;}
 f.currentEvent=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire);return f;
}
inline void mainBeginTrace(unsigned route,const MainBeginFacts&before,const MainBeginFacts&after)noexcept{
 const int saved=errno;
 struct Seen{uint64_t epoch;unsigned route,reason,stage,stop,hits;};static thread_local Seen seen[32]{};static thread_local unsigned count=0;
 unsigned at=0;for(;at<count;++at)if(seen[at].epoch==before.epoch&&seen[at].route==route&&seen[at].reason==before.reason&&seen[at].stage==before.stage&&seen[at].stop==before.stop)break;
 if(at==count){if(count==32){errno=saved;return;}seen[count++]={before.epoch,route,before.reason,before.stage,before.stop,0};}
 if(seen[at].hits>=2){errno=saved;return;}++seen[at].hits;std::fprintf(stderr,"MAIN BEGIN REJECT route=%u epoch=%llu iteration=%llu state=%u invalid=%u reason=%u stage=%u published=%u collector=%u stop=%u afterStop=%u prepared=%u afterPrepared=%u cpu=%u bound=%u packet=%u depth=%u uploaded=%u copyComplete=%u groupComplete=%u copyError=%u groupError=%u copy=%llu group=%llu event=%llu currentEvent=%llu afterCurrentEvent=%llu\n",route,(unsigned long long)before.epoch,(unsigned long long)before.iteration,before.state,before.invalid,before.reason,before.stage,before.published,before.collector,before.stop,after.stop,before.prepared,after.prepared,before.cpu,before.bound,before.packet,before.depth,before.uploaded,before.copyComplete,before.groupComplete,before.copyError,before.groupError,(unsigned long long)before.copy,(unsigned long long)before.group,(unsigned long long)before.event,(unsigned long long)before.currentEvent,(unsigned long long)after.currentEvent);
 errno=saved;
}
inline void begin(Game&g,Intent&i){
 auto*s=current(g);auto*c=nfs_logo_publisher::store();
 if(s&&!activeRequest()&&!i.serial&&!i.beginConsumed){auto uploaded=nfs_main_uploaded_source::pending(g,*s,c);if(uploaded&&nfs_main_uploaded_source::seed(g,*s,uploaded)){enqueueBegin(uploaded->frame,i,uploaded,true);return;}}
 if(!s||s->invalid||!s->published||!c||activeRequest()||i.serial||i.beginConsumed){if(mainBeginTraceEnabled()){const auto f=mainBeginFacts(s,c);mainBeginTrace(1,f,f);}return;}
 if(!archiveCurrent(*s)){if(mainBeginTraceEnabled()){const auto f=mainBeginFacts(s,c);mainBeginTrace(2,f,f);}s->fail(Reason::Source,true);return;}
 Builder b{s,c,{}};const bool trace=mainBeginTraceEnabled();MainBeginFacts before;if(trace)before=mainBeginFacts(s,c);
 if(!nfs_logo_publisher::publishNativeEntry(g,sourceSnapshot,&b,0)){if(trace)mainBeginTrace(3,before,mainBeginFacts(s,c));return;}
 (void)nfs_main_source_frame::seed(g,b.owned);auto uploaded=nfs_main_uploaded_source::candidate(g,*s,*c,b.owned);enqueueBegin(b.owned,i,uploaded);
 if(trace&&!i.beginConsumed)mainBeginTrace(4,before,mainBeginFacts(s,c));
}
inline void finish(Intent&i){if(!i.beginConsumed||i.finishAttempted)return;if(!i.serial||activeRequest()!=i.serial){cancel(i);return;}nfs::check_worker_stop();i.finishAttempted=true;const auto owned=i.owned;const auto uploaded=i.uploaded;const bool retained=i.retained;const uint64_t serial=i.serial;const auto receipt=nfs::enqueue_owned_renderer([owned,uploaded,retained,serial](){(void)owned;if(uploaded){NfsMainUploadedSeal seal{};const int ok=nfs_private_main_uploaded_finish(serial,&seal);if(!retained){uploaded->seal=seal;uploaded->sealed=ok!=0;uploaded->completion.store(true,std::memory_order_release);}return int32_t(ok);}return int32_t(nfs_private_background_scene_finish(serial));});if(receipt!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("background scene finish changed renderer owner");if(uploaded)nfs_main_uploaded_source::completed(uploaded);i.finishConsumed=true;activeRequest()=0;nfs::check_worker_stop();}
} // namespace nfs_background_scene_live

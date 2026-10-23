// SPDX-License-Identifier: MIT
// Real Controls queue/Prepared/Lease/MainThreadBridge transport. All current
// source authorities below are explicit SYNTHETIC stubs, never OriginalGame.
#include "MainThreadHarness.h"
#include <atomic>
#include <cstring>
#include <thread>
#include <functional>
#include <stdexcept>
#include <vector>
static thread_local unsigned leaseDepth=0;static unsigned guestReads=0;
struct Game{uint32_t eax=0;};
static constexpr uint32_t off_4D37C8=0x410000,dword_4D4D24=0x2020;
static uint32_t guest_address(uint32_t at){return at;}
static thread_local uint32_t syntheticCurrentOwner=0;
static thread_local unsigned ownerReadCalls=0,ownerRefuseAt=0;
namespace nfs {struct GuestCopyTransaction{GuestCopyTransaction(){++leaseDepth;}~GuestCopyTransaction(){--leaseDepth;}template<class T>T load(uint32_t at){++guestReads;if(at!=dword_4D4D24||sizeof(T)!=4)throw std::out_of_range("synthetic native owner only");++ownerReadCalls;const uint32_t v=ownerReadCalls==ownerRefuseAt?0:syntheticCurrentOwner;T out{};std::memcpy(&out,&v,sizeof out);return out;}};}
static uintptr_t syntheticThread(){static thread_local unsigned char token;return reinterpret_cast<uintptr_t>(&token);}// SPDX-License-Identifier: MIT
#include "AudioSidebarFixture.h"
#define main controlsSyntheticProofUnusedMain
#undef CHECK
#include "controls_dialog_word_proof_tests.cpp"
#undef main
#include "hd/ControlsScenePrepare.h"

#include <thread>
#include <type_traits>
namespace gc=nfs_controls_scene_cpu;namespace gf=nfs_graphics_frame;namespace af=nfs_audio_frame;
// EXPLICIT SYNTHETIC sealers only. This TU never includes or executes either
// Controls collector. Neutral replay verifiers below are real production code.
namespace nfs_controls_text {struct TailScope {static std::shared_ptr<const word::Proof>synthetic(Packet p,const word::Evidence&e){return std::shared_ptr<const word::Proof>(new word::Proof(std::move(p),e));}};}
namespace nfs_controls_sidebar_text {struct TailScope {static std::shared_ptr<const nfs_nested_sidebar_caption::Proof>synthetic(Packet p,const nfs_nested_sidebar_caption::Evidence&e){return std::shared_ptr<const nfs_nested_sidebar_caption::Proof>(new nfs_nested_sidebar_caption::Proof(std::move(p),e));}};}
namespace cd=nfs_controls_dialog_display;namespace caption=nfs_nested_sidebar_caption;
struct JoinFixture {
 SidebarFixture s;af::Binding current{};gf::SidebarBinding selected{};gf::BackgroundRecord record{};
 std::vector<uint8_t>map;std::vector<uint16_t>post;std::array<unsigned char,32>mapSha{};
 std::shared_ptr<const af::BackgroundCatalog>mapCatalog;
 std::shared_ptr<const gf::GraphicsFramePacket>frame;
 std::shared_ptr<const as::Packet>cache;word::Evidence dialogEvidence{};caption::Evidence captionEvidence{};std::shared_ptr<const cd::Prepared>dialog;
 JoinFixture(){
  s.binding.form=0x410000;s.f->scene.form=s.binding.form;rebuildCaption();
  auto full=target(640,0x100000,0x41000);
  current.form=s.binding.form;current.owner=full.owner;current.archive=s.binding.archive;
  current.ownerEpoch=s.binding.ownerEpoch;current.pass=7;current.lifetime=current.surface.lifetime=31;
  std::copy(full.w.begin(),full.w.end(),current.surface.geometry.begin());
  auto&g=current.surface.geometry;g[14]=full.x[0];g[15]=full.x[1];g[16]=full.x[639];g[17]=full.y[0];g[18]=full.y[1];g[19]=full.y[479];CHECK(current.valid());
  map.resize(614416);put32(map.data(),0x78);put32(map.data()+4,640u|(480u<<16));
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)put16(map.data()+16+2*(y*640+x),uint16_t(1+(x*13+y*31)%65535));
  CHECK(nfs_texture_sha256(map.data(),map.size(),mapSha.data()));
  mapCatalog=af::BackgroundCatalog::create(mapSha,map);CHECK(mapCatalog);
  s.catalog->mapSha[0]=mapSha;CHECK(s.catalog->mapSha[1]!=mapSha);cache=makeCache();CHECK(cache);
  selected.target=s.f->scene.preSide.target;selected.target.ownerState[0]=0;selected.word=0;
  selected.handle=selected.tableEntry=s.binding.sourceOwner;selected.lifetime=17;selected.source=s.before;CHECK(selected.valid());
  record.binding=current;record.begin=1;record.end=6;record.complete=1;
  record.sidebarBefore=record.sidebarAfter=selected;
  record.preReset.resize(gf::Pixels);record.postReset.resize(gf::Pixels);record.postSidebar.resize(gf::Pixels);
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x){size_t at=y*640+x;uint16_t m=logo::le16(map.data()+16+2*at);record.preReset[at]=uint16_t(m^0xffff);record.postReset[at]=record.postSidebar[at]=m;if(x<343){uint16_t v=logo::le16(s.before.data()+16+2*(y*343+x));if(v)record.postSidebar[at]=v;}}
  auto draw=[&](gf::ScopedDraw&d,uint64_t begin,uint32_t source,uint32_t mode,const std::vector<uint8_t>&bytes){d.entry=d.exit=current;d.before=d.after=full;d.begin=begin;d.end=begin+1;d.complete=1;d.args={{source,mode,0,0}};d.sourceBefore=d.sourceAfter=bytes;};
  draw(record.reset,2,0x800000,0x400,map);draw(record.sidebar,4,s.binding.bitmap,0x1000,s.before);
  auto background=gf::BackgroundVerifier::verify(record,current,selected,mapCatalog);CHECK(background);
  nfs_soft::FramePacketBuilder builder(font());CHECK(builder.begin(current.surface,41,background->pixels().data(),gf::Pixels));
  post=background->pixels();auto strings=commands(0);
  for(unsigned i=0;i<strings.size();++i){CHECK(builder.beginString(strings[i]));const uint16_t value=uint16_t(0x8000+i);CHECK(builder.word(i,post[i],value,value));post[i]=value;CHECK(builder.endString(strings[i]));}
  auto text=builder.finish(current.surface,post.data(),post.size());CHECK(text&&text->commands()==16&&text->vertices().size()==696);
  dialogEvidence=evidence(0,41);dialogEvidence.form=dialogEvidence.source.form=current.form;dialogEvidence.authority.parentWords[1]=current.form;dialogEvidence.authority.archive=current.archive;dialogEvidence.thread=s.binding.thread;dialogEvidence.lifetime=current.lifetime;
  auto&snap=dialogEvidence.post;snap.identity=current.surface;snap.target=full;snap.archive=current.archive;
  for(auto&r:dialogEvidence.resources){const auto color=r.after.color;r.before=r.resolvedState=r.measure=r.after=snap;r.after.color=color;}
  dialog=makeDialog(dialogEvidence,text);CHECK(dialog);
  frame=gf::GraphicsFramePacket::create(background,text,current,41,post.data(),post.size());CHECK(frame);
 }
 static std::shared_ptr<const cd::Prepared>makeDialog(const word::Evidence&e,Packet p){auto proof=nfs_controls_text::TailScope::synthetic(p,e);return cd::Prepared::create(proof,p,{e.form,e.authority.archive,e.game,e.serial,e.lifetime});}
 void rebuildCaption(){
  const auto id=as::surface(s.f->group.post.target,s.captionLifetime);nfs_soft::FramePacketBuilder b(font());CHECK(b.begin(id,s.captionSerial,s.f->group.post.pixels.data(),as::Pixels));
  nfs_soft::TextCommand c{};std::memcpy(c.text.data(),"options",7);c.length=7;c.role=1;c.alignment=2;c.sourceSize=18;c.x=120;c.y=364;c.advance=48;c.color=65535;c.clip={0,0,343,480};
  auto pixels=s.f->group.post.pixels;const unsigned at=369*343+123;CHECK(b.beginString(c)&&b.word(at,pixels[at],65535,65535)&&b.endString(c));pixels[at]=65535;s.caption=b.finish(id,pixels.data(),as::Pixels);CHECK(s.caption);
  s.before.resize(as::Bytes);for(unsigned k=0;k<4;++k)put32(s.before.data()+4*k,s.f->group.post.target.header[k]);for(unsigned k=0;k<as::Pixels;++k)put16(s.before.data()+16+2*k,pixels[k]);s.after=s.before;
  auto&e=captionEvidence;e.policy=caption::Policy::Controls;e.form=e.incomingEbp=s.binding.form;e.textGuest=0x12300;e.game=0x900001;e.thread=s.binding.thread;e.serial=s.captionSerial;e.lifetime=s.captionLifetime;e.measureWidth=48;e.measureAnchor=168;e.measureKind=2;e.command=c;
  caption::Snapshot snap;snap.target=s.f->group.post.target;snap.identity=id;snap.archive=s.binding.archive;snap.selector=6;snap.entry=0x12400;snap.entryWord=0x70002;e.pre=e.measure=e.before=e.after=e.post=snap;
 }
 std::shared_ptr<const as::Packet>makeCache(){auto proof=nfs_controls_sidebar_text::TailScope::synthetic(s.caption,captionEvidence);as::Input input{s.binding,s.f->scene,s.f->group,s.caption,s.captionSerial,s.captionLifetime,s.before,s.after,proof,{captionEvidence.policy,s.binding.form,s.binding.archive,captionEvidence.game,s.captionSerial,s.captionLifetime}};return as::Packet::create(input,s.binding,s.f->scene.preSide.target,s.catalog);}
 std::shared_ptr<const gc::Prepared>verify(){return gc::Prepared::create(frame,cache,current,41,post.data(),post.size(),selected,s.binding,s.f->scene.preSide.target,s.before.data(),s.before.size(),dialog);}
};

namespace nfs_background_scene_live {
inline bool&on(){static bool v=true;return v;}inline bool enabled()noexcept{return on();}
inline uint64_t&activeRequest()noexcept{static thread_local uint64_t v=0;return v;}
inline uint64_t nextRequest()noexcept{static std::atomic<uint64_t>v{1};return v.fetch_add(1);}
}
// These stubs own independent current values, not authority borrowed from the
// candidate Prepared. Their before/after observations are deliberately mutable.
namespace nfs_controls_text {
struct CurrentStub{uintptr_t game=0,thread=0;bool active=true;unsigned packetCalls=0,proofCalls=0,packetRefuseAt=0,proofRefuseAt=0;std::shared_ptr<const nfs_soft::FramePacket>packet;std::shared_ptr<const word::Proof>proof;};
inline CurrentStub*&slot(){static thread_local CurrentStub*s=nullptr;return s;}
inline std::shared_ptr<const nfs_soft::FramePacket>acquireCurrentPacket(Game&g){auto*s=slot();if(!s||!s->active||s->game!=reinterpret_cast<uintptr_t>(&g)||s->thread!=syntheticThread()||++s->packetCalls==s->packetRefuseAt)return {};return s->packet;}
inline std::shared_ptr<const word::Proof>acquireCurrentDialogProof(Game&g){auto*s=slot();if(!s||!s->active||s->game!=reinterpret_cast<uintptr_t>(&g)||s->thread!=syntheticThread()||++s->proofCalls==s->proofRefuseAt)return {};return s->proof;}
}
namespace nfs_controls_live {
struct State{uintptr_t game=0,thread=0;bool active=true,boundOkay=true;uint64_t epoch=0,acceptedTextSerial=41;af::Binding binding{};rec::Target target{},observedTarget{};gf::BackgroundRecord background{};std::shared_ptr<const gf::GraphicsFramePacket>published;gf::SidebarBinding sidebar{};std::vector<uint16_t>post;unsigned frameCalls=0,sidebarCalls=0,planeCalls=0,frameRefuseAt=0,sidebarRefuseAt=0,planeRefuseAt=0;};
inline State*&slot(){static thread_local State*s=nullptr;return s;}
inline State*current(Game&g){auto*s=slot();return s&&s->active&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==syntheticThread()?s:nullptr;}
inline bool bound(State&s,Game&g,bool){return current(g)==&s&&s.boundOkay;}
inline std::shared_ptr<const gf::GraphicsFramePacket>acquireCurrentPacket(Game&g){auto*s=current(g);return s&&++s->frameCalls!=s->frameRefuseAt?s->published:std::shared_ptr<const gf::GraphicsFramePacket>{};}
inline bool acquireCurrentSidebarBinding(Game&g,gf::SidebarBinding&out){auto*s=current(g);if(!s||++s->sidebarCalls==s->sidebarRefuseAt)return false;out=s->sidebar;return true;}
inline bool plane(std::vector<uint16_t>&out,rec::Target&t){auto*s=slot();if(!s||!s->active||s->thread!=syntheticThread()||++s->planeCalls==s->planeRefuseAt)return false;out=s->post;t=s->observedTarget;return true;}
}
namespace nfs_controls_sidebar_live {
struct State{uintptr_t game=0,thread=0;bool active=true;uint64_t epoch=0;as::Binding binding{};rec::Target target{};std::shared_ptr<const as::Packet>published;unsigned calls=0,refuseAt=0;};
inline State*&slot(){static thread_local State*s=nullptr;return s;}
inline State*current(Game&g){auto*s=slot();return s&&s->active&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==syntheticThread()?s:nullptr;}
inline std::shared_ptr<const as::Packet>acquireForSidebar(Game&g,const gf::ScopedDraw&draw,const gf::SidebarBinding&actual,rec::Target*out){auto*s=current(g);if(!s||++s->calls==s->refuseAt||!s->published||!actual.valid()||!as::retainedTarget(actual.target,s->target)||actual.handle!=s->binding.sourceOwner||actual.source!=draw.sourceBefore||draw.sourceAfter!=draw.sourceBefore||!s->published->matches(s->binding,s->target,draw.sourceBefore.data(),draw.sourceBefore.size()))return {};if(out)*out=s->target;return s->published;}
}
#define NFS_CONTROLS_QUEUE_RUNTIME_TEST 1
#include "hd/ControlsSceneQueueAdapter.inc"
#undef NFS_CONTROLS_QUEUE_RUNTIME_TEST
namespace queue=nfs_controls_scene_live;
struct Event{unsigned kind;uint64_t serial;};static std::vector<Event>queueEvents;static std::thread::id rendererThread;static uint64_t pendingSerial=0;static NfsControlsSceneFrameView retainedView{};static bool retainCallbackOwner=false;static unsigned retainCalls=0,releaseCalls=0;
static void releaseCallback(){if(retainedView.lifetimeOwner){retainedView.releaseOwner(retainedView.lifetimeOwner);++releaseCalls;retainedView={};}}
extern "C" int nfs_private_controls_scene_begin(const NfsControlsSceneFrameView*v){CHECK(std::this_thread::get_id()==rendererThread&&!leaseDepth&&v&&v->schema==NFS_CONTROLS_SCENE_RUNTIME_SCHEMA&&v->serial&&v->layers.sidebarWord.word==3&&v->layers.dialogWords.count==10&&v->lifetimeOwner&&v->retainOwner&&v->releaseOwner);queueEvents.push_back({1,v->serial});pendingSerial=v->serial;if(retainCallbackOwner){CHECK(!retainedView.lifetimeOwner);v->retainOwner(v->lifetimeOwner);++retainCalls;retainedView=*v;}return 1;}
extern "C" int nfs_private_controls_scene_finish(uint64_t serial){CHECK(std::this_thread::get_id()==rendererThread&&!leaseDepth&&pendingSerial==serial);queueEvents.push_back({3,serial});pendingSerial=0;releaseCallback();return 1;}
struct QueueContext {
 Game game;std::unique_ptr<JoinFixture>fixture{new JoinFixture};nfs_controls_live::State full;nfs_controls_sidebar_live::State cache;nfs_controls_text::CurrentStub text;
 QueueContext(){auto&x=*fixture;const auto g=reinterpret_cast<uintptr_t>(&game),thread=syntheticThread();x.s.binding.thread=thread;x.s.f->group.thread=thread;x.rebuildCaption();x.captionEvidence.game=g;x.captionEvidence.thread=thread;x.cache=x.makeCache();CHECK(x.cache);x.dialogEvidence.game=g;x.dialogEvidence.thread=thread;x.dialog=x.makeDialog(x.dialogEvidence,x.frame->text());CHECK(x.dialog&&x.verify());
  full.game=cache.game=text.game=g;full.thread=cache.thread=text.thread=thread;full.epoch=cache.epoch=x.current.ownerEpoch;full.binding=x.current;full.target=full.observedTarget=x.dialogEvidence.post.target;full.background=x.record;full.published=x.frame;full.sidebar=x.selected;full.post=x.post;cache.binding=x.s.binding;cache.target=x.s.f->scene.preSide.target;cache.published=x.cache;text.packet=x.frame->text();text.proof=x.dialog->proof();game.eax=x.current.surface.geometry[8];syntheticCurrentOwner=x.current.owner;ownerReadCalls=ownerRefuseAt=0;nfs_controls_live::slot()=&full;nfs_controls_sidebar_live::slot()=&cache;nfs_controls_text::slot()=&text;queue::activeRequest()=0;
 }
 void resetObservations(){full.frameCalls=full.sidebarCalls=full.planeCalls=cache.calls=text.packetCalls=text.proofCalls=ownerReadCalls=0;full.frameRefuseAt=full.sidebarRefuseAt=full.planeRefuseAt=cache.refuseAt=text.packetRefuseAt=text.proofRefuseAt=ownerRefuseAt=0;}
 void dropCandidates(){full.published.reset();cache.published.reset();text.packet.reset();text.proof.reset();fixture.reset();}
 ~QueueContext(){nfs_controls_live::slot()=nullptr;nfs_controls_sidebar_live::slot()=nullptr;nfs_controls_text::slot()=nullptr;queue::activeRequest()=0;}
};
static void drainQueue(){const auto before=guestReads;nfs::flush_renderer_commands();CHECK(guestReads==before&&!leaseDepth);}
static void originalNative(uint64_t serial){CHECK(nfs::enqueue_owned_renderer([serial](){CHECK(std::this_thread::get_id()==rendererThread&&!leaseDepth);queueEvents.push_back({2,serial});return int32_t(0);})==nfs::OwnedRendererAcceptance::Consumed);}
static void ordered(){QueueContext c;queueEvents.clear();retainCallbackOwner=true;queue::Intent i;queue::begin(c.game,i);CHECK(i.beginConsumed&&i.serial&&i.owned&&queue::activeRequest()==i.serial);const auto serial=i.serial;std::weak_ptr<const gc::Prepared>weak=i.owned->prepared();originalNative(serial);queue::finish(i);CHECK(i.finishConsumed&&!queue::activeRequest()&&queueEvents.empty());c.dropCandidates();queue::cancel(i);CHECK(!weak.expired());drainQueue();CHECK(weak.expired()&&queueEvents.size()==3&&queueEvents[0].kind==1&&queueEvents[1].kind==2&&queueEvents[2].kind==3&&retainCalls==releaseCalls&&!retainedView.lifetimeOwner);for(auto event:queueEvents)CHECK(event.serial==serial);retainCallbackOwner=false;}
static void separatelyFlushedPhases(){QueueContext c;queueEvents.clear();queue::Intent i;queue::begin(c.game,i);CHECK(i.beginConsumed);const auto serial=i.serial;drainQueue();CHECK(queueEvents.size()==1&&queueEvents[0].kind==1&&pendingSerial==serial);originalNative(serial);queue::finish(i);queue::cancel(i);drainQueue();CHECK(queueEvents.size()==3&&queueEvents[1].kind==2&&queueEvents[2].kind==3&&!pendingSerial);}
static void admission(){QueueContext c;const auto full=c.full;const auto cache=c.cache;const auto text=c.text;const auto originalOwner=syntheticCurrentOwner;for(unsigned fault=0;fault<25;++fault){c.full=full;c.cache=cache;c.text=text;c.resetObservations();syntheticCurrentOwner=originalOwner;c.game.eax=full.binding.surface.geometry[8];switch(fault){case 0:c.full.active=false;break;case 1:c.cache.active=false;break;case 2:c.text.active=false;break;case 3:++c.full.game;break;case 4:++c.cache.thread;break;case 5:++c.text.game;break;case 6:c.full.boundOkay=false;break;case 7:++c.full.binding.form;break;case 8:++c.full.epoch;break;case 9:++c.game.eax;break;case 10:syntheticCurrentOwner++;break;case 11:c.full.post[12345]^=1;break;case 12:c.full.target.x[123]^=2;break;case 13:++c.cache.binding.cacheRevision;break;case 14:++c.full.sidebar.tableEntry;break;case 15:c.full.published.reset();break;case 16:c.text.proof.reset();break;case 17:c.text.packet.reset();break;case 18:++c.full.acceptedTextSerial;break;case 19:++c.cache.epoch;break;case 20:c.cache.published=c.fixture->s.verify();CHECK(c.cache.published&&!c.cache.published->captionWord());break;case 21:case 22:case 23:{auto bad=c.fixture->dialogEvidence;if(fault==21)++bad.game;if(fault==22)++bad.thread;if(fault==23)++bad.lifetime;c.text.proof=nfs_controls_text::TailScope::synthetic(c.text.packet,bad);break;}case 24:c.full.observedTarget.y[123]^=2;break;}
 queue::Intent i;queue::begin(c.game,i);CHECK(!i.serial&&!i.beginConsumed&&!i.owned&&!queue::activeRequest());}
 c.full=full;c.cache=cache;c.text=text;syntheticCurrentOwner=originalOwner;c.game.eax=full.binding.surface.geometry[8];for(unsigned fault=0;fault<7;++fault){c.resetObservations();if(fault==0)c.full.frameRefuseAt=2;if(fault==1)c.text.packetRefuseAt=2;if(fault==2)c.text.proofRefuseAt=2;if(fault==3)c.full.sidebarRefuseAt=2;if(fault==4)c.cache.refuseAt=2;if(fault==5)c.full.planeRefuseAt=2;if(fault==6)ownerRefuseAt=2;queue::Intent i;queue::begin(c.game,i);CHECK(!i.serial&&!i.beginConsumed&&!i.owned&&!queue::activeRequest());}
 c.resetObservations();queue::activeRequest()=999;queue::Intent blocked;queue::begin(c.game,blocked);CHECK(!blocked.serial&&queue::activeRequest()==999);queue::activeRequest()=0;auto reads=guestReads;std::thread wrong([&]{queue::Intent denied;queue::begin(c.game,denied);CHECK(!denied.serial);});wrong.join();CHECK(guestReads==reads);queue::Intent valid;queue::begin(c.game,valid);CHECK(valid.beginConsumed);queue::finish(valid);queue::cancel(valid);drainQueue();
}
static void cancellation(){QueueContext c;queueEvents.clear();retainCallbackOwner=true;queue::Intent i;queue::begin(c.game,i);CHECK(i.beginConsumed);std::weak_ptr<const gc::Prepared>weak=i.owned->prepared();const auto serial=i.serial;queue::cancel(i);c.dropCandidates();CHECK(!weak.expired()&&!queue::activeRequest());drainQueue();CHECK(!weak.expired()&&pendingSerial==serial&&retainedView.lifetimeOwner&&queueEvents.size()==1);std::thread release([]{releaseCallback();});release.join();CHECK(weak.expired());pendingSerial=0;retainCallbackOwner=false;
 QueueContext fresh;queue::Intent stopped;nfs::WorkerControl control;nfs::current_worker_control()=&control;control.stop=true;bool thrown=false;try{queue::begin(fresh.game,stopped);}catch(const nfs::WorkerStopped&){thrown=true;}control.stop=false;nfs::current_worker_control()=nullptr;queue::cancel(stopped);CHECK(thrown&&!queue::activeRequest());drainQueue();
 queue::Intent mismatch;queue::begin(fresh.game,mismatch);CHECK(mismatch.beginConsumed);const auto request=mismatch.serial;queue::activeRequest()=request+1;queue::finish(mismatch);CHECK(!mismatch.serial&&queue::activeRequest()==request+1);queue::activeRequest()=0;drainQueue();pendingSerial=0;
}
int main(){try{MainThreadHarness harness;rendererThread=harness.owner_id();ordered();separatelyFlushedPhases();admission();cancellation();queueEvents.clear();nfs::run_on_main([](void*){QueueContext owner;queue::Intent i;queue::begin(owner.game,i);CHECK(!i.serial&&!i.beginConsumed&&!i.owned&&!queue::activeRequest());},nullptr);drainQueue();CHECK(queueEvents.empty()&&!leaseDepth);std::puts("PASS real Controls queue/Prepared/Lease/worker FIFO, ordered native callbacks, repeat-current gates, cancellation and owner fallback; all source/current authorities EXPLICITLY SYNTHETIC, no OriginalGame/GPU/native-witness execution claim");return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL Controls queue %s\n",e.what());return 1;}}
// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerIntroPreparedCases.h"
#include "MainThreadHarness.h"
#include <string>
// Explicit transport/request stubs only. Source and native proof constructors
// remain private and all source/native records are genuinely sealed above.
namespace nfs_background_scene_live {
inline uint64_t&activeRequest()noexcept{static thread_local uint64_t value=0;return value;}
inline uint64_t nextRequest()noexcept{static std::atomic<uint64_t>value{0};return ++value;}
}
namespace nfs_player_scene_live {inline uint64_t&activeRequest()noexcept{return nfs_background_scene_live::activeRequest();}}
#define NFS_PLAYER_INTRO_QUEUE_RUNTIME_TEST 1
#include "hd/PlayerIntroQueueAdapter.inc"
#undef NFS_PLAYER_INTRO_QUEUE_RUNTIME_TEST
static std::vector<std::string>introCalls;
static std::thread::id introOwner;
static uint64_t introPending=0,introMainSerial=0,introMainContent=0;
static bool introOverwriteInBegin=false;
static void introMock(const char*name){assert(std::this_thread::get_id()==introOwner&&!leaseDepth);introCalls.emplace_back(name);}
extern "C" int nfs_private_player_intro_begin(const NfsPlayerIntroFrameView*v){
 introMock("begin");const auto before=reads;assert(v&&v->schema==1&&v->requestSerial&&v->uploadEvent&&(v->form==0x4d3050||v->form==0x4d3100));assert(!introPending);introPending=v->requestSerial;assert(v->inheritedMain.serial==introMainSerial&&v->inheritedMain.contentRevision==introMainContent&&v->inheritedMain.tailKind==NFS_BACKGROUND_TAIL_MAIN_SKIP);assert(v->wordsCount==bg::Full&&v->paddedOriginal565&&v->paddedOriginal565[343]==0&&v->paddedOriginal565[bg::Full-1]==0);assert(v->caption.logicalWidth==343&&v->caption.vertexCount&&v->caption.font.alpha&&v->caption.font.bytes);assert(v->keys[0]==0x01000000&&v->keys[299]==0x01000000+299*2048);assert(reads==before);if(introOverwriteInBegin)nfs_player_initial_native::overwriteEvent();return 1;
}
extern "C" int nfs_private_player_intro_slot0_finish(uint64_t serial){introMock("slot0");assert(serial==introPending);return 1;}
extern "C" int nfs_private_player_intro_tail_finish(uint64_t serial){introMock("tail");assert(serial==introPending);return 1;}
extern "C" int nfs_private_player_intro_end(uint64_t serial){introMock("end");assert(serial==introPending);introPending=0;return 1;}
extern "C" int nfs_private_player_intro_cancel(uint64_t serial){introMock("cancel");if(serial==introPending)introPending=0;return 1;}
static void playerIntroQueueCases(const std::shared_ptr<const nfs_player_initial_sidebar::Proof>&source,Game&g){
 namespace owner=nfs_player_scene_live;namespace live=nfs_player_initial_native_live;namespace native=nfs_player_initial_native;namespace intro=nfs_player_intro_live;
 PlayerIntroFixture fixture(source);const auto savedSelected=selected;auto saved=std::unique_ptr<decltype(memory)>(new decltype(memory)(memory));const auto registers=g;auto*oldOwner=owner::slot();const bool oldAnchors=initialNativeAnchors;owner::State state;const auto&b=source->binding();state.on=true;state.game=b.game;state.thread=b.thread;state.ownerEpoch=b.epoch;state.form=b.form;state.formGuest=state.sourceOwner=b.formGuest;state.archive=b.archive;state.pendingInitialSidebar=source;state.catalogue=fixture.catalog;owner::slot()=&state;initialNativeAnchors=true;introMainSerial=source->origin()->frame()->serial;introMainContent=source->origin()->frame()->contentRevision;
 MainThreadHarness harness;introOwner=harness.owner_id();
 auto install=[&](){live::reset();memory=*saved;selected=source->post().target;feature=true;deniedRead=0;state.on=true;state.invalid=false;state.ownerEpoch=b.epoch;state.form=b.form;state.formGuest=state.sourceOwner=b.formGuest;state.archive=b.archive;state.pendingInitialSidebar=source;state.catalogue=fixture.catalog;owner::activeRequest()=0;introCalls.clear();introPending=0;introOverwriteInBegin=false;
  for(unsigned k=0;k<14;++k)put<uint32_t>(selected.owner+4*k,selected.ownerWords[k]);for(unsigned k=0;k<4;++k)put<uint32_t>(selected.w[8]+4*k,selected.header[k]);for(unsigned k=0;k<343;++k)put<uint32_t>(selected.w[12]+4*k,selected.x[k]);for(unsigned k=0;k<480;++k)put<uint32_t>(selected.w[11]+4*k,selected.y[k]);std::memcpy(memory.data()+selected.w[8]+16,source->post().pixels.data(),bg::Side*2);put<uint32_t>(dword_4D2314+2,selected.ownerState[0]);put<uint32_t>(dword_4D231C,selected.ownerState[1]);put<uint32_t>(dword_4D2320,selected.ownerState[2]);put<uint32_t>(dword_4D4D24,selected.ownerState[3]);put<uint32_t>(dword_4DABE4,selected.ownerState[4]);put<uint32_t>(dword_4D40B8,b.archive);
  // Configure explicit completed live transport metadata around a real CPU
  // Witness seal; actual live Upload/Tile scopes have separate focused tests.
  auto*&p=live::slot();if(!p)p=new live::State;auto proof=fixture.seal(p->witness);p->source=source;p->game=b.game;p->thread=b.thread;p->event=proof->uploadEvent(fixture.binding,fixture.catalog);p->depth=0;p->stageOwner=fixture.stage->target.owner;p->phase=live::Phase::Finished;p->invalid=p->tileActive=false;p->returned=true;p->failure=live::Failure::None;g=registers;g.esi=int32_t(b.formGuest);g.eax=g.edx=g.ebx=0;g.ecx=32;g.esp=0xa100;put<uint32_t>(g.esp,UINT32_MAX);native::Binding current;auto actual=std::unique_ptr<bg::Plane>(new bg::Plane);assert(live::proofForPresentation(g,current,*actual)==proof);
 };
 auto drain=[&](){nfs::flush_renderer_commands();assert(!leaseDepth&&allocationsInLease==0);};
 // Callback borrows remain valid after the stack scope and caller snapshots
 // disappear. Actual swap marker stays before end; no native buffer is changed.
 install();{intro::IntroFrameScope frame(g);assert(owner::activeRequest());frame.slot0Finished();frame.tailFinished();nfs::enqueue_owned_renderer([](){introMock("swap");return int32_t(1);});}assert(!owner::activeRequest());drain();assert((introCalls==std::vector<std::string>{"begin","slot0","tail","swap","end"})&&!introPending);
 for(unsigned mode=0;mode<5;++mode){install();{intro::IntroFrameScope frame(g);if(mode==0){}if(mode==1)frame.slot0Finished();if(mode==2)frame.tailFinished();if(mode==3){frame.slot0Finished();frame.slot0Finished();}if(mode==4){frame.slot0Finished();frame.tailFinished();frame.tailFinished();}}drain();assert(!owner::activeRequest()&&!introPending&&std::find(introCalls.begin(),introCalls.end(),"end")==introCalls.end()&&std::find(introCalls.begin(),introCalls.end(),"cancel")!=introCalls.end());}
 // Stale worker state cancels before enqueueing a later phase; different
 // shared active requests must never be erased by this request's cleanup.
 for(unsigned mode=0;mode<5;++mode){install();uint64_t other=0;{intro::IntroFrameScope frame(g);if(mode==0)++g.esi;if(mode==1)state.invalid=true;if(mode==2)put<uint16_t>(selected.w[8]+16,uint16_t(source->post().pixels[0]^1));if(mode==3)native::overwriteEvent();if(mode==4){other=owner::activeRequest()+10000;owner::activeRequest()=other;}frame.slot0Finished();if(mode==4)assert(owner::activeRequest()==other);}drain();assert(!introPending&&std::find(introCalls.begin(),introCalls.end(),"end")==introCalls.end());if(mode==4){assert(owner::activeRequest()==other);owner::activeRequest()=0;}}
 install();{intro::IntroFrameScope frame(g);frame.slot0Finished();frame.tailFinished();}live::reset();drain();assert(std::find(introCalls.begin(),introCalls.end(),"begin")==introCalls.end()&&std::find(introCalls.begin(),introCalls.end(),"end")==introCalls.end()&&!introPending);
 install();introOverwriteInBegin=true;{intro::IntroFrameScope frame(g);frame.slot0Finished();frame.tailFinished();}drain();assert(introCalls.front()=="begin"&&std::find(introCalls.begin(),introCalls.end(),"slot0")==introCalls.end()&&std::find(introCalls.begin(),introCalls.end(),"end")==introCalls.end()&&!introPending);
 install();try{intro::IntroFrameScope frame(g);throw 7;}catch(int v){assert(v==7);}drain();assert(!owner::activeRequest()&&!introPending&&introCalls.back()=="cancel");
 install();nfs::WorkerControl control;nfs::current_worker_control()=&control;{intro::IntroFrameScope frame(g);control.stop.store(true);bool stopped=false;try{frame.slot0Finished();}catch(const nfs::WorkerStopped&){stopped=true;}assert(stopped);control.stop.store(false);}nfs::current_worker_control()=nullptr;drain();assert(!owner::activeRequest()&&!introPending&&introCalls.back()=="cancel");
 // Exact ordinary slot0 argument and form guards, with no callback borrowing.
 for(unsigned mode=0;mode<6;++mode){install();if(mode==0)g.eax=1;if(mode==1)g.edx=1;if(mode==2)g.ebx=1;if(mode==3)g.ecx=31;if(mode==4)put<uint32_t>(g.esp,0);if(mode==5)++g.esi;{intro::IntroFrameScope frame(g);frame.slot0Finished();frame.tailFinished();}drain();assert(introCalls.empty()&&!owner::activeRequest());}
 live::reset();owner::slot()=oldOwner;initialNativeAnchors=oldAnchors;memory=*saved;selected=savedSelected;g=registers;feature=true;deniedRead=0;assert(!leaseDepth&&allocationsInLease==0);std::puts("PASS Player Intro owned FIFO: real source/native seals, immutable post-scope borrows, exact phase order, stale-source callback rejection, distinct-request preservation, unwind/WorkerStop cancellation; explicit live-owner/transport stubs and C renderer mocks only");
}

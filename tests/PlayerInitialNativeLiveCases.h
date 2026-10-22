// SPDX-License-Identifier: MIT
#pragma once
// Mutable independent original-owner stub; the source below is still created
// by actual caption scopes and the full Main verifier in the including test.
namespace nfs_player_scene_live {
struct State{bool on=false,invalid=false;uintptr_t game=0,thread=0;uint64_t ownerEpoch=0;uint32_t form=0,formGuest=0,sourceOwner=0,archive=0;std::shared_ptr<const nfs_player_initial_sidebar::Proof>pendingInitialSidebar;std::shared_ptr<const bg::SourceCatalog>catalogue;};
inline State*&slot(){static thread_local State*s=nullptr;return s;}
inline State*current(Game&g){auto*s=slot();return feature&&s&&s->on&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==29?s:nullptr;}
inline uint32_t relocatedForm(uint32_t form){return form==0x4d3050?off_4D3050:form==0x4d3100?off_4D3100:0;}
inline bool initialSidebarCurrent(State&s){return s.pendingInitialSidebar&&s.pendingInitialSidebar->current()&&s.pendingInitialSidebar->captionProof()==cap::lastProof(*reinterpret_cast<Game*>(s.game));}
inline bool archiveCurrent(State&s)noexcept{try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(dword_4D40B8)==s.archive;}catch(...){return false;}}
}
#define NFS_PLAYER_INITIAL_NATIVE_RUNTIME_TEST 1
#include "hd/PlayerInitialNativeUploadGameHooks.inc"
#undef NFS_PLAYER_INITIAL_NATIVE_RUNTIME_TEST
static void initialNativeLiveCases(const std::shared_ptr<const nfs_player_initial_sidebar::Proof>&source,Game&g){
 namespace live=nfs_player_initial_native_live;namespace native=nfs_player_initial_native;namespace owner=nfs_player_scene_live;
 assert(source&&source->current());const auto savedSelected=selected;auto saved=std::unique_ptr<decltype(memory)>(new decltype(memory)(memory));const auto oldRegisters=g;
 owner::State state;const auto&b=source->binding();state.on=true;state.game=b.game;state.thread=b.thread;state.ownerEpoch=b.epoch;state.form=b.form;state.formGuest=state.sourceOwner=b.formGuest;state.archive=b.archive;state.pendingInitialSidebar=source;state.catalogue=source->origin()->catalog();owner::slot()=&state;initialNativeAnchors=true;
 auto stage=std::unique_ptr<bg::Plane>(new bg::Plane(source->origin()->base().post));stage->target.owner=0xb000;stage->target.w[8]=0x100000;stage->target.w[12]=0x8000;stage->target.w[11]=0x9000;stage->target.ownerWords=stage->target.w;stage->target.ownerState=source->post().target.ownerState;stage->pixels.fill(0);for(unsigned y=0;y<480;++y)std::copy_n(source->post().pixels.begin()+y*343,343,stage->pixels.begin()+y*640);
 auto materialize=[&](const bg::Plane&p){for(unsigned k=0;k<14;++k)put<uint32_t>(p.target.owner+4*k,p.target.ownerWords[k]);for(unsigned k=0;k<4;++k)put<uint32_t>(p.target.w[8]+4*k,p.target.header[k]);for(unsigned k=0;k<p.width;++k)put<uint32_t>(p.target.w[12]+4*k,p.target.x[k]);for(unsigned k=0;k<480;++k)put<uint32_t>(p.target.w[11]+4*k,p.target.y[k]);std::memcpy(memory.data()+p.target.w[8]+16,p.pixels.data(),p.width*480*2);};
 auto select=[&](const bg::Plane&p){selected=p.target;for(unsigned k=0;k<14;++k)put<uint32_t>(dword_4DB240+4*k,p.target.w[k]);};
 auto install=[&](){memory=*saved;deniedRead=0;g=oldRegisters;state.on=true;state.invalid=false;state.ownerEpoch=b.epoch;state.archive=b.archive;feature=true;materialize(source->post());const auto&handles=source->post().target.ownerState;put<uint32_t>(dword_4D2314+2,handles[0]);put<uint32_t>(dword_4D231C,handles[1]);put<uint32_t>(dword_4D2320,handles[2]);put<uint32_t>(dword_4D4D24,handles[3]);put<uint32_t>(dword_4DABE4,handles[4]);select(source->post());g.eax=int32_t(source->post().target.w[8]);g.edx=0;g.esp=0xa100;};
 auto tile=[&](unsigned n,unsigned variant){const uint32_t descriptor=0x1a0000+64*n,payload=0x1b0000;put<uint32_t>(dword_4EB74C+4*n,descriptor);const uint32_t words[7]={3,3,3,11,0,0x01000000+2048*n,3};for(unsigned k=0;k<7;++k)put<uint32_t>(descriptor+4*k,words[k]);for(unsigned yy=0;yy<32;++yy)for(unsigned xx=0;xx<32;++xx){uint16_t p=stage->pixels[((n%15)*32+yy)*640+(n/15)*32+xx];uint16_t q=uint16_t((p/2048)*1024+((p/64)%32)*32+p%32);put<uint16_t>(payload+2*(yy*32+xx),q?uint16_t(q+32768):0);}put<uint32_t>(g.esp,descriptor);put<uint32_t>(g.esp+4,payload);put<uint32_t>(g.esp+8,0);
  if(n==1&&variant==8)memory[payload+2047]^=1;if(n==1&&variant==10)put<uint32_t>(dword_4EB74C+4*n,descriptor+4);if(n==1&&variant==24)put<uint32_t>(descriptor+12,12);
  {live::UploadTileScope observed(g);put<uint32_t>(descriptor+16,payload);if(n==1&&variant==9)memory[payload+2047]^=1;if(n==1&&variant==11){live::UploadTileScope nested(g);}if(n==1&&variant==12){g.edx=0;live::UploadScope nested(g);}if(n==1&&variant==21)throw 7;}
 };
 auto record=[&](unsigned variant){install();try{live::UploadScope upload(g);assert(upload.entered);materialize(*stage);select(*stage);put<uint32_t>(g.esp+8,stage->target.owner);if(variant==4)put<uint32_t>(g.esp+8,stage->target.owner+4);if(variant==5)put<uint32_t>(dword_4DB240+32,stage->target.w[8]+2);if(variant==6)put<uint16_t>(stage->target.w[8]+16+(bg::Full-1)*2,1);live::stageBegin(g);if(variant==3)live::stageBegin(g);for(unsigned n=0;n<(variant==7?299u:300u);++n)tile(n,variant);
  if(variant==15)put<uint16_t>(source->post().target.w[8]+16+(bg::Side-1)*2,uint16_t(source->post().pixels[bg::Side-1]^1));if(variant==16)put<uint16_t>(stage->target.w[8]+16+(bg::Full-1)*2,1);if(variant==17)live::retireAllocation(source->post().target.w[11]-4);if(variant==20)++state.ownerEpoch;
  g.eax=int32_t(stage->target.owner+(variant==14?4:0));if(variant!=22)live::stageFinish(g);
  // Actual expected cleanup: no registered handle points at this temporary.
  // Poison it and deny future owner reads before UploadScope's destructor.
  live::retireAllocation(stage->target.owner);live::retireAllocation(stage->target.w[8]);live::retireAllocation(stage->target.w[11]-4);std::memset(memory.data()+stage->target.owner,0xcc,56);std::memset(memory.data()+stage->target.w[8],0xcc,614416);select(source->post());deniedRead=stage->target.owner;
  if(variant==13)throw 7;
 }catch(int){assert(variant==13||variant==21);}deniedRead=0;auto actual=std::unique_ptr<bg::Plane>(new bg::Plane);native::Binding binding;return live::proofForPresentation(g,binding,*actual);};
 auto proof=record(0);assert(proof&&live::uploaded()==300);native::Binding binding;auto actual=std::unique_ptr<bg::Plane>(new bg::Plane);assert(live::proofForPresentation(g,binding,*actual)==proof);assert(proof->source(binding,state.catalogue)==source);std::array<uint32_t,300>keys{};assert(proof->keys(binding,state.catalogue,keys));for(unsigned n=0;n<300;++n)assert(keys[n]==0x01000000+2048*n);
 const auto event=native::uploadEvents().load();g.edx=1;{live::UploadScope unrelated(g);assert(!unrelated.entered);}assert(native::uploadEvents().load()==event&&proof->current(binding,state.catalogue));g.edx=0;g.eax=123;{live::UploadScope wrongSource(g);assert(!wrongSource.entered);}assert(native::uploadEvents().load()==event+1&&!proof->current(binding,state.catalogue));
 for(unsigned variant:{3u,4u,5u,6u,7u,8u,9u,10u,11u,12u,13u,14u,15u,16u,17u,20u,21u,22u,24u}){assert(!record(variant));assert(!leaseDepth&&allocationsInLease==0);}
 proof=record(0);assert(proof);assert(live::currentBinding(g,binding));feature=false;g.edx=0;const auto before=reads;const auto oldEvent=native::uploadEvents().load();{live::UploadScope disabled(g);assert(!disabled.entered);}assert(reads==before&&native::uploadEvents().load()==oldEvent+1&&!proof->current(binding,state.catalogue));feature=true;
 proof=record(0);assert(proof&&live::currentBinding(g,binding));g.edx=0;const auto beforeForeign=reads;std::thread foreign([&](){foreignPath=true;{live::UploadScope noOwner(g);assert(!noOwner.entered);}foreignPath=false;});foreign.join();assert(reads==beforeForeign&&foreignAllocations==0&&!proof->current(binding,state.catalogue));
 proof=record(0);assert(proof&&live::currentBinding(g,binding));{live::SourceLifetimeScope lifetime(g);assert(!proof->current(binding,state.catalogue));}
 live::reset();owner::slot()=nullptr;initialNativeAnchors=false;memory=*saved;selected=savedSelected;g=oldRegisters;deniedRead=0;assert(!leaseDepth&&allocationsInLease==0);std::puts("PASS Initial slot0 live upload: actual descriptor/LUT/temporary capture, all300 real scope calls, prefree seal/postfree survival, wrong context/nesting/overwrite/unwind rejection; no original-data or GPU claim");
}

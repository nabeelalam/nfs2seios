#pragma once
// SPDX-License-Identifier: MIT
// Actual maintained Setup-control source collector against public synthetic guest
// memory/target/font/words. No original assets, sealed-proof bypass, original
// glyph execution, native renderer, device or Controls child activation claimed.
#include <array>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <new>
#include <thread>
#include <functional>
#include <type_traits>
#include <vector>
#include "hd/OriginalUiRecords.h"
static std::array<uint8_t,4194304>memory{};static unsigned leaseDepth=0,reads=0,checks=0;static uint32_t deniedRead=0;static std::atomic<unsigned>allocationsInLease{0};
#define CHECK(v) do{++checks;if(!(v)){std::fprintf(stderr,"FAIL Controls Setup runtime line%d: %s\n",__LINE__,#v);std::abort();}}while(0)
void*operator new(size_t n){if(leaseDepth)++allocationsInLease;if(void*p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void*p)noexcept{std::free(p);}void operator delete(void*p,size_t)noexcept{std::free(p);}
void*operator new(size_t n,const std::nothrow_t&)noexcept{try{return ::operator new(n);}catch(...){return nullptr;}}void operator delete(void*p,const std::nothrow_t&)noexcept{std::free(p);}
struct Game{int32_t eax=0,edx=0,ebx=0,ecx=0,ebp=0,esi=0,esp=0x1f0000;};
static constexpr uint32_t dword_4D40B8=0x200,dword_4DD770=0x208,dword_4DD77C=0x210,dword_4DD774=0x218,dword_4DD714=0x220,dword_4D20A0=0x228,byte_4DB25E=0x230,byte_4DB25C=0x238,dword_4DD6F4=0x240,dword_4DD6F8=0x248,dword_4E54AC=0x250,dword_4DD6EC=0x258,dword_4DB240=0x1000,dword_4D243C=0x260,byte_4D3ECC=0x268,off_4D3ED0=0x280,dword_4D3E9C=0x2c0,dword_4D4AE0=0x300,byte_512DA9=0x310,byte_512DA8=0x318,inControlAssignMode=0x320;
static constexpr uint32_t off_4D37C8=0x3000,off_4D349C=0x4000,off_4D35E8=0x5000,off_4D36A8=0x5400,off_4D372C=0x5800,off_4D3A88=0x6000,dword_4D3A08=0x6800,dword_4D24E8=0x7000,aMain=0x7100,aBlk4_0=0x7200;
static constexpr uint32_t dword_4D3764=0x9000,dword_4D3778=0x9020,dword_4D378C=0x9040,dword_4D37A0=0x9060,dword_4D3750=0x9080;
static constexpr uint32_t dword_4D3384=0x10000,dword_4D3398=0x10020,dword_4D33AC=0x10040,dword_4D33C0=0x10060,dword_4D33FC=0x10080,dword_4D3424=0x100a0,dword_4D3438=0x100c0,dword_4D33E8=0x100e0,dword_4D33D4=0x10100,dword_4D3410=0x10120,dword_4D344C=0x10140,dword_4D3460=0x10160,dword_4D3474=0x10180,dword_4D3488=0x101a0;
static constexpr uint32_t dword_4D34E4=0x11000,dword_4D34F8=0x11020,dword_4D350C=0x11040,dword_4D3520=0x11060,dword_4D355C=0x11080,dword_4D3584=0x110a0,dword_4D3598=0x110c0,dword_4D3548=0x110e0,dword_4D3534=0x11100,dword_4D3570=0x11120,dword_4D35AC=0x11140,dword_4D35C0=0x11160,dword_4D35D4=0x11180,dword_512E0C=0x14000,dword_512E34=0x14100;
static constexpr uint32_t game_id_sub_4A5830=0xface0010,Plane=0x20000,Font=0x180000,Palette=0x190000,Text=0x1a0000,Table=0x18000;
inline uint32_t guest_address(uint32_t p){return p;}
inline void*nfs_host_address(uint32_t p,size_t n){if(!leaseDepth||uint64_t(p)+n>memory.size()||(deniedRead&&uint64_t(p)<=deniedRead&&uint64_t(p)+n>deniedRead))throw 1;++reads;return memory.data()+p;}
namespace nfs{struct GuestCopyTransaction{GuestCopyTransaction(){++leaseDepth;}~GuestCopyTransaction(){--leaseDepth;}template<class T>T load(uint32_t p){T v;std::memcpy(&v,nfs_host_address(p,sizeof v),sizeof v);return v;}};}
static nfs_logo_capture::Target selected{};static bool feature=true;
namespace nfs_logo_publisher{inline uintptr_t threadToken(){static thread_local unsigned char t;return reinterpret_cast<uintptr_t>(&t);}inline bool enabled(){return feature;}inline int exceptions(){return std::uncaught_exceptions();}inline void checked(uint32_t p,size_t n){(void)nfs_host_address(p,n);}inline void target(nfs::GuestCopyTransaction&,nfs_logo_capture::Target&t){++reads;t=selected;}}
#define NFS_CONTROLS_TEXT_TEST 1
#include "hd/ControlsSetupTextGameHooks.inc"
namespace setup=nfs_controls_setup_text;
#undef CHECK
// Reuse only the public synthetic authority description, never seal a Proof.
namespace parent_fixture {
#define main controlsParentPureFixtureUnusedMain
#include "controls_dialog_word_proof_tests.cpp"
#undef main
}
#undef CHECK
#define CHECK(v) do{++checks;if(!(v)){std::fprintf(stderr,"FAIL Controls Setup runtime line%d: %s\n",__LINE__,#v);std::abort();}}while(0)
template<class T>static void put(uint32_t p,T v){CHECK(uint64_t(p)+sizeof v<=memory.size());std::memcpy(memory.data()+p,&v,sizeof v);}
static void seedParent(){const auto source=nfs_controls_text::dialogSource();auto a=parent_fixture::evidence(0).authority;a.archive=0x1234;a.table=Table;a.entry=source.parentEntry;a.parentMenu=a.parentMenuChild=source.parentMenu;a.parentDescriptor=source.parentDescriptor;a.parentWords={{0x810002,source.form,2,0,source.defaultList}};a.descriptors=source.descriptors;
 put<uint32_t>(dword_4D40B8,a.archive);put<uint32_t>(dword_4D4AE0,Table);put<uint32_t>(byte_4D3ECC,6u<<16);put<uint32_t>(off_4D3ED0+24,source.parentEntry);put<uint32_t>(source.parentEntry,0x70002);put<uint32_t>(source.parentEntry+4,source.parentMenu);put<uint32_t>(source.parentEntry+16,source.defaultList);put<uint32_t>(source.parentMenu+20,source.parentDescriptor);for(unsigned k=0;k<5;++k)put<uint32_t>(source.parentDescriptor+4*k,a.parentWords[k]);
 for(unsigned i=0;i<5;++i){put<uint32_t>(source.form+12+4*i,source.descriptors[i]);a.descriptorWords[i][1]=i<3?source.children[i]:i==3?UINT32_MAX:0;a.descriptorWords[i][4]=i==4?0:source.defaultList;for(unsigned k=0;k<5;++k)put<uint32_t>(source.descriptors[i]+4*k,a.descriptorWords[i][k]);const uint32_t at=Table+12*nfs_controls_dialog_word::resource(i),text=0x1b0000+128*i;put<uint32_t>(at+8,text-Table);std::memcpy(memory.data()+at,a.headers[i].data(),8);const char*name=nfs_controls_dialog_word::text(i);std::memcpy(memory.data()+text,name,std::strlen(name)+1);}
 nfs_controls_dialog_word::Authority actual;CHECK(nfs_controls_text::readDialogAuthority(actual));CHECK(actual.selectedIndex==0);
}
static const std::array<uint32_t,14>single{{dword_4D3384,dword_4D3398,dword_4D33AC,dword_4D33C0,dword_4D33FC,dword_4D3424,dword_4D3438,dword_4D33E8,dword_4D33D4,dword_4D3410,dword_4D344C,dword_4D3460,dword_4D3474,dword_4D3488}};
static const std::array<uint32_t,16>dual{{dword_4D34E4,dword_4D34F8,dword_4D350C,dword_4D3520,dword_4D355C,dword_4D3584,dword_4D3598,dword_4D3548,dword_4D3534,dword_4D3570,dword_4D344C,dword_4D3460,dword_4D3474,dword_4D35AC,dword_4D35C0,dword_4D35D4}};
static void prepare(bool split){CHECK(!leaseDepth);deniedRead=0;memory.fill(0);seedParent();const uint32_t form=split?off_4D35E8:off_4D349C;put<uint8_t>(byte_512DA9,split?1:0);put<uint8_t>(byte_512DA8,0);put<uint32_t>(form,aMain);put<uint32_t>(form+4,aBlk4_0);put<uint32_t>(form+8,split?19:0);const unsigned resources[]={0x113,0x114,0x115,0x116,0x117,0x118,0x119,0x11a,0x11b,0x11c,0x11f,0x11d,0x11e,0x112},order[]={0,1,2,3,6,8,9,5,4,7};
 for(unsigned i=0;i<(split?16u:14u);++i){const uint32_t at=split?dual[i]:single[i];put<uint32_t>(form+12+4*i,at);const unsigned resource=split?(i<10?0x13b+i:i<13?resources[i]:0x138+i-13):resources[i];put<uint32_t>(at,(resource<<16)|(i<13?2u:0u));if(i<10){const uint32_t binding=(split?dword_512E34:dword_512E0C)+order[i]*(split?8:4);put<uint32_t>(at+8,split?0xd04:0xc04);put<uint32_t>(at+12,binding);put<uint32_t>(at+16,dword_4D24E8);put<uint32_t>(binding,0x120000+i);if(split)put<uint32_t>(binding+4,0x340000+i);}}
 selected={};auto&w=selected.w;w[1]=640;w[2]=480;w[5]=640;w[6]=480;w[7]=0x7810;w[8]=Plane;w[10]=1280;w[11]=0x15000;w[12]=0x16000;selected.owner=0x17000;selected.ownerWords=w;selected.header={{0x78,640|(480u<<16),0,0}};for(unsigned k=0;k<640;++k)selected.x[k]=2*k;for(unsigned k=0;k<480;++k)selected.y[k]=16+1280*k;for(unsigned k=0;k<4;++k)put<uint32_t>(Plane+4*k,selected.header[k]);put<uint32_t>(dword_4DD770,Font);put<uint32_t>(dword_4DD77C,game_id_sub_4A5830);put<uint32_t>(dword_4DD774,Palette);put<uint32_t>(dword_4DD714,65535);put<uint8_t>(byte_4DB25C,16);put<int32_t>(dword_4E54AC,split?16:18);for(unsigned k=0;k<16;++k)put<uint32_t>(Palette+4*k,0xff000000+k);std::memcpy(memory.data()+Text,"TEST",5);
}
static std::shared_ptr<const nfs_soft::FontCatalog>font(){nfs_soft::FontDescription d;for(auto&role:d.glyphs)for(auto&g:role){g.x=g.y=2;g.w=g.h=1;g.advance=65536;}std::array<uint8_t,1024*512>a{};a[2*1024+2]=255;return nfs_soft::FontCatalog::create(d,a.data(),a.size(),17);}
static void draw(Game&g,const char*mode){g.eax=0x13b;setup::PolicyScope policy(g,false);g.eax=0;setup::resourceAlignment(g);g.eax=Text;g.edx=20;g.ebx=20;setup::StringScope string(g);if(!std::strcmp(mode,"wrong-game")){Game other=g;setup::GlyphScope glyph(other);CHECK(!setup::current(other));}else if(!std::strcmp(mode,"nested-string")){setup::StringScope nested(g);}else{setup::GlyphScope glyph(g);const uint32_t at=Plane+16+21*1280+21*2;{setup::WordCapture word(g,at,65535);put<uint16_t>(at,!std::strcmp(mode,"bad-word")?123:65535);}if(!std::strcmp(mode,"unobserved-write"))put<uint16_t>(at+2,1);}
 if(!std::strcmp(mode,"text-mutate"))memory[Text]='X';if(!std::strcmp(mode,"text-terminate"))memory[Text+4]='!';if(!std::strcmp(mode,"font"))put<uint32_t>(dword_4DD770,Font+32);if(!std::strcmp(mode,"font-header"))memory[Font]^=1;if(!std::strcmp(mode,"palette"))put<uint32_t>(Palette+4,123);if(!std::strcmp(mode,"callback"))put<uint32_t>(dword_4DD77C,1);if(!std::strcmp(mode,"mode"))put<uint32_t>(dword_4D20A0,1);if(!std::strcmp(mode,"size"))put<int32_t>(dword_4E54AC,17);if(!std::strcmp(mode,"x-lut"))selected.x[100]^=2;if(!std::strcmp(mode,"y-lut"))selected.y[220]^=2;if(!std::strcmp(mode,"header"))selected.header[1]^=1;if(!std::strcmp(mode,"geometry"))selected.w[3]=1;if(!std::strcmp(mode,"binding")){const bool split=setup::current(g)->form==off_4D35E8;put<uint32_t>(split?dword_512E34:dword_512E0C,123);}if(!std::strcmp(mode,"authority"))put<uint16_t>(off_4D37C8+10,1);if(!std::strcmp(mode,"retire"))setup::beforeOwnerRetire(g,selected.owner);put<int32_t>(dword_4DD6EC,24);
}
static void frame(Game&g,const char*mode){auto*s=setup::current(g);if(!s){CHECK(!setup::acquireCurrentPacket(g));return;}const auto form=s->form;g.eax=form;setup::beginTail(g);draw(g,mode);setup::finishTail(g);}
static void positivesAndDrift(bool split){prepare(split);Game g;g.eax=split?off_4D35E8:off_4D349C;setup::OwnerScope owner(g,split?0x403970:0x4101c0);auto*s=setup::current(g);CHECK(s&&s->sourceSize==(split?16u:18u));setup::Authority a;CHECK(setup::readAuthority(*s,a));frame(g,"normal");const auto packet=setup::acquireCurrentPacket(g);CHECK(packet&&packet->commands()==1&&packet->writes()==1&&packet->records()[0].command.sourceSize==(split?16u:18u));CHECK(packet->clean()[21*640+21]==0&&packet->original()[21*640+21]==65535);
 const auto old=packet;const auto savedTarget=selected;const auto life=s->lifetime;
 put<uint32_t>(Plane+16,1);CHECK(!setup::acquireCurrentPacket(g));put<uint32_t>(Plane+16,0);CHECK(setup::acquireCurrentPacket(g)==packet);
 put<uint32_t>(split?dword_512E34:dword_512E0C,123);CHECK(!setup::acquireCurrentPacket(g));put<uint32_t>(split?dword_512E34:dword_512E0C,a.bindings[0][0]);CHECK(setup::acquireCurrentPacket(g)==packet);
 selected.x[300]^=2;CHECK(!setup::acquireCurrentPacket(g));selected=savedTarget;CHECK(setup::acquireCurrentPacket(g)==packet);
 Game other=g;CHECK(!setup::current(other)&&!setup::acquireCurrentPacket(other));
 setup::beforeDescriptorRetire(g,0);CHECK(setup::acquireCurrentPacket(g)==packet);setup::beforeOwnerRetire(g,selected.owner);CHECK(!setup::acquireCurrentPacket(g)&&s->lifetime>life);CHECK(old->original()[21*640+21]==65535);
 g.eax=s->form;setup::beginTail(g);draw(g,"normal");setup::finishTail(g);CHECK(setup::acquireCurrentPacket(g));
}
static void failedOwnerCannotResurrect(){
 prepare(false);Game g;g.eax=off_4D349C;put<uint32_t>(off_4D349C+12,0xbad);
 {setup::OwnerScope invalid(g,0x4101c0);CHECK(!setup::current(g)&&!setup::acquireCurrentPacket(g));
  // Repairing guest authority is not permission to enter a new lexical owner
  // through an invalid outer scope that has not returned yet.
  put<uint32_t>(off_4D349C+12,single[0]);g.eax=off_4D349C;
  {setup::OwnerScope nested(g,0x4101c0);CHECK(!setup::current(g)&&!setup::acquireCurrentPacket(g));frame(g,"normal");CHECK(!setup::acquireCurrentPacket(g));}
  CHECK(!setup::current(g)&&!setup::acquireCurrentPacket(g));
 }
 for(unsigned counter=0;counter<2;++counter){prepare(false);g.eax=off_4D349C;auto*s=setup::storage().single;CHECK(s);const auto lifetime=s->lifetime,epoch=s->ownerEpoch;
  if(counter==0)s->lifetime=UINT64_MAX;else s->ownerEpoch=UINT64_MAX;
  {setup::OwnerScope exhausted(g,0x4101c0);CHECK(!setup::current(g)&&!setup::acquireCurrentPacket(g));
   // Resetting a synthetic exhausted counter while the lexical owner is alive
   // must not let an inner owner evade the outer capacity refusal.
   s->lifetime=lifetime;s->ownerEpoch=epoch;g.eax=off_4D349C;
   {setup::OwnerScope nested(g,0x4101c0);CHECK(!setup::current(g)&&!setup::acquireCurrentPacket(g));}
   CHECK(!setup::current(g));
  }
  s->lifetime=lifetime;s->ownerEpoch=epoch;
 }
}
static void prompts(bool split){
 for(const char*mode:{"normal","no-assignment","wrong-descriptor","non-action-row","nested"}){prepare(split);Game g;g.eax=split?off_4D35E8:off_4D349C;setup::OwnerScope owner(g,split?0x403970:0x4101c0);auto*s=setup::current(g);CHECK(s);const auto form=s->form;
  put<uint8_t>(inControlAssignMode,!std::strcmp(mode,"no-assignment")?0:1);g.eax=form;g.edx=!std::strcmp(mode,"wrong-descriptor")?0xdead:int32_t(split?dual[0]:single[0]);if(!std::strcmp(mode,"non-action-row")){put<uint16_t>(form+10,10);g.edx=split?dual[10]:single[10];}
  {setup::PromptScope prompt(g);if(!std::strcmp(mode,"nested")){g.eax=form;g.edx=split?dual[0]:single[0];setup::PromptScope nested(g);frame(g,"normal");CHECK(!setup::acquireCurrentPacket(g));}else{frame(g,"normal");const auto packet=setup::acquireCurrentPacket(g);CHECK(bool(packet)==!std::strcmp(mode,"normal"));}}
  CHECK(!setup::acquireCurrentPacket(g));put<uint8_t>(inControlAssignMode,0);put<uint16_t>(form+10,0);g.eax=form;frame(g,"normal");CHECK(setup::acquireCurrentPacket(g));
 }
}
// The actual helper sees the original resource BEFORE PolicyScope construction.
// Raw binding text uses the stacked wrapper, regardless of any stale EAX value.
static void sizedDraw(Game&g,unsigned resource,unsigned size,unsigned index,bool raw=false,unsigned exitSize=0,const char*name="TEST"){
 put<int32_t>(dword_4E54AC,int32_t(size));g.eax=resource;if(raw)put<int32_t>(uint32_t(g.esp)+4,0);
 setup::PolicyScope policy(g,raw);if(!raw){g.eax=0;setup::resourceAlignment(g);}std::memcpy(memory.data()+Text,name,std::strlen(name)+1);g.eax=Text;g.edx=20;g.ebx=int32_t(20+30*index);
 {setup::StringScope string(g);{setup::GlyphScope glyph(g);const uint32_t at=Plane+16+(21+30*index)*1280+42;{setup::WordCapture word(g,at,65535);put<uint16_t>(at,65535);}}
  if(exitSize)put<int32_t>(dword_4E54AC,int32_t(exitSize));put<int32_t>(dword_4DD6EC,20+int32_t(std::strlen(name)));}
}
static void mixedSourceSizes(){
 prepare(true);Game g;g.eax=off_4D35E8;setup::OwnerScope owner(g,0x403970);auto*s=setup::current(g);CHECK(s);g.eax=s->form;setup::beginTail(g);
 sizedDraw(g,315,16,0);CHECK(!s->policyDepth&&s->policyAlignment==-1);
 sizedDraw(g,0x11f,18,1,false,0,"DONE");CHECK(!s->policyDepth&&s->policyAlignment==-1);
 sizedDraw(g,0x11f,16,2,true);CHECK(!s->policyDepth&&s->policyAlignment==-1);
 sizedDraw(g,0x137,18,3,false,0,"FOOTER");CHECK(!s->policyDepth&&s->policyAlignment==-1);setup::finishTail(g);
 auto p=setup::acquireCurrentPacket(g);CHECK(p&&p->commands()==4&&p->writes()==4);const unsigned sizes[]={16,18,16,18};for(unsigned i=0;i<4;++i)CHECK(p->records()[i].command.sourceSize==sizes[i]);CHECK(p->records()[1].command.length==4&&!std::memcmp(p->records()[1].command.text.data(),"DONE",4));CHECK(p->records()[3].command.length==6&&!std::memcmp(p->records()[3].command.text.data(),"FOOTER",6));
}
static void sizedPolicyContexts(){
 // Each case uses a fresh real helper owner, so failure cannot be hidden by
 // reusing a retained packet or an expectation left by an earlier resource.
 for(bool prompt:{false,true})for(unsigned resource:{0x11du,0x11eu,0x11fu,0x138u,7u,0x137u,0x135u,313u,314u,315u,316u,317u,318u,319u,320u,321u,322u,323u,324u,0u,0x999u})for(bool raw:{false,true})for(unsigned size:{16u,18u}){
  prepare(true);Game g;g.eax=off_4D35E8;setup::OwnerScope owner(g,0x403970);auto*s=setup::current(g);CHECK(s);const bool large=!raw&&(resource==0x11d||resource==0x11e||resource==0x11f||resource==0x138||resource==7||(!prompt&&resource==0x137)||(prompt&&resource==0x135));
  const bool allowed=raw||(!((resource==0x137&&prompt)||(resource==0x135&&!prompt))&&(large||resource==313||resource==314||(resource>=315&&resource<=324)));
  auto run=[&]{g.eax=s->form;setup::beginTail(g);sizedDraw(g,resource,size,0,raw);CHECK(!s->policyDepth&&s->policyAlignment==-1&&!s->policyResource&&!s->policyResourceCall);setup::finishTail(g);auto p=setup::acquireCurrentPacket(g);CHECK(bool(p)==(allowed&&size==(large?18u:16u)));if(p)CHECK(p->records()[0].command.sourceSize==size);};
  if(prompt){put<uint8_t>(inControlAssignMode,1);g.eax=s->form;g.edx=dual[0];setup::PromptScope scope(g);run();}else run();
 }
 for(unsigned entry:{16u,18u}){prepare(true);Game g;g.eax=off_4D35E8;setup::OwnerScope owner(g,0x403970);auto*s=setup::current(g);CHECK(s);g.eax=s->form;setup::beginTail(g);sizedDraw(g,entry==18?0x11f:315,entry,0,false,entry==18?16:18);CHECK(!s->policyDepth&&s->policyAlignment==-1);setup::finishTail(g);CHECK(!setup::acquireCurrentPacket(g));}
 // Single-player remains uniformly18 even for raw binding strings.
 prepare(false);Game g;g.eax=off_4D349C;setup::OwnerScope owner(g,0x4101c0);auto*s=setup::current(g);CHECK(s);g.eax=s->form;setup::beginTail(g);sizedDraw(g,315,18,0,true);setup::finishTail(g);CHECK(setup::acquireCurrentPacket(g));
}

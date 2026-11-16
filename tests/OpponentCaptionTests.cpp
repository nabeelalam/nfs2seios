// SPDX-License-Identifier: MIT
#include <array>
#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <new>
#include <thread>
#include <type_traits>
#include "hd/OriginalUiRecords.h"
#include "Main343OriginFixture.h"
static std::array<uint8_t,2097152> memory{};static unsigned leaseDepth=0,reads=0;static uint32_t deniedRead=0;static std::atomic<unsigned> allocationsInLease{0},foreignAllocations{0};static thread_local bool foreignPath=false;
void*operator new(size_t n){if(leaseDepth)++allocationsInLease;if(foreignPath)++foreignAllocations;if(void*p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void*p)noexcept{std::free(p);}
void operator delete(void*p,size_t)noexcept{std::free(p);}
void*operator new(size_t n,const std::nothrow_t&)noexcept{try{return ::operator new(n);}catch(...){return nullptr;}}
void operator delete(void*p,const std::nothrow_t&)noexcept{std::free(p);}
struct Game{int32_t eax=0,edx=0,ebx=0,ecx=0,ebp=0,esi=0,esp=0;};
static constexpr uint32_t off_4D3A88=0x100,dword_4D40B8=0x200,dword_4DD770=0x208,dword_4DD77C=0x210,dword_4DD774=0x218,dword_4DD714=0x220,dword_4D20A0=0x228,byte_4DB25E=0x230,byte_4DB25C=0x238,dword_4DD6F4=0x240,dword_4DD6F8=0x248,dword_4E54AC=0x250,dword_4DD6EC=0x258,dword_4DB240=0x1000,dword_4D243C=0x260,byte_4D3ECC=0x268,off_4D3ED0=0x280,dword_4D3E9C=0x2c0;
static constexpr uint32_t off_4D2790=0x110,dword_4D3E24=0x2d0;
static constexpr uint32_t off_4D3EC4=0x140,dword_4D2314=0x340,dword_4D231C=0x350,dword_4D2320=0x354;
static constexpr uint32_t dword_4EB74C=0x6000,dword_4D4D24=0x360,dword_4DABE4=0x364;
static constexpr uint32_t off_4D3050=0x120,off_4D3100=0x130,dword_4D3E4C=0x300,dword_4D3E60=0x320;
static constexpr uint32_t off_4D3204=0x150,dword_4D3E74=0x330;
static constexpr uint32_t testOwner=off_4D3204,testRecord=dword_4D3E74;
static constexpr unsigned testSelector=4,testResource=4,testWidth=68,testLeft=100,testY=271;
static constexpr char testText[]="opponents";

static constexpr uint32_t game_id_sub_4A5830=0xface0010;
inline uint32_t guest_address(uint32_t p){return p;}
inline void*nfs_host_address(uint32_t p,size_t n){if(!leaseDepth||uint64_t(p)+n>memory.size()||(deniedRead&&uint64_t(p)<=deniedRead&&uint64_t(p)+n>deniedRead))throw 1;++reads;return memory.data()+p;}
namespace nfs{struct GuestCopyTransaction{GuestCopyTransaction(){++leaseDepth;}~GuestCopyTransaction(){--leaseDepth;}template<class T>T load(uint32_t p){T v;std::memcpy(&v,nfs_host_address(p,sizeof(T)),sizeof(T));return v;}};}
static nfs_logo_capture::Target selected{};static bool feature=true;static bool initialNativeAnchors=false;
namespace nfs_logo_publisher{inline uintptr_t threadToken(){return 29;}inline bool enabled(){return feature;}inline int exceptions(){return std::uncaught_exception()?1:0;}inline void checked(uint32_t p,size_t n){(void)nfs_host_address(p,n);}inline void target(nfs::GuestCopyTransaction&tx,nfs_logo_capture::Target&t,uint32_t owner=0){++reads;if(!owner||!initialNativeAnchors){t=selected;return;}t={};for(unsigned i=0;i<14;++i)t.w[i]=t.ownerWords[i]=tx.load<uint32_t>(owner+4*i);t.owner=owner;assert(t.w[1]<=640&&t.w[2]==480);for(unsigned i=0;i<4;++i)t.header[i]=tx.load<uint32_t>(t.w[8]+4*i);for(unsigned i=0;i<t.w[1];++i)t.x[i]=tx.load<uint32_t>(t.w[12]+4*i);for(unsigned i=0;i<480;++i)t.y[i]=tx.load<uint32_t>(t.w[11]+4*i);t.ownerState={{tx.load<uint32_t>(dword_4D2314+2),tx.load<uint32_t>(dword_4D231C),tx.load<uint32_t>(dword_4D2320),tx.load<uint32_t>(dword_4D4D24),tx.load<uint32_t>(dword_4DABE4)}};}inline void descriptor(nfs::GuestCopyTransaction&tx,uint32_t at,std::array<uint32_t,7>&out){for(unsigned i=0;i<7;++i)out[i]=tx.load<uint32_t>(at+4*i);}}
#define NFS_OPPONENT_CAPTION_TEXT_TEST 1
#include "hd/OpponentCaptionGameHooks.inc"
#include "hd/OpponentCaptionDisplay.h"
#include "hd/OpponentInitialSidebarProof.h"
namespace cap=nfs_opponent_caption;
template<class T>static void put(uint32_t p,T v){assert(uint64_t(p)+sizeof v<=memory.size());std::memcpy(memory.data()+p,&v,sizeof v);}
static void prepare(){deniedRead=0;memory.fill(0);put<uint32_t>(dword_4D243C,0xdeadbeef);put<uint32_t>(byte_4D3ECC,testSelector<<16);put<uint32_t>(off_4D3ED0+testSelector*4,testRecord);put<uint32_t>(testRecord,(testResource<<16)|2);selected={};auto&w=selected.w;w[1]=343;w[2]=480;w[5]=343;w[6]=480;w[7]=0x7810;w[8]=0x40000;w[10]=686;w[11]=0x2000;w[12]=0x3000;selected.owner=0x1800;selected.ownerWords=w;selected.header={{0x78,343|(480u<<16),0,0}};for(unsigned k=0;k<343;++k)selected.x[k]=2*k;for(unsigned k=0;k<480;++k)selected.y[k]=16+686*k;for(unsigned k=0;k<4;++k)put(w[8]+4*k,selected.header[k]);put<uint32_t>(dword_4D40B8,0x1234);put<uint32_t>(dword_4DD770,0x98000);put<uint32_t>(dword_4DD77C,game_id_sub_4A5830);put<uint32_t>(dword_4DD774,0x99000);put<uint32_t>(dword_4DD714,0xffff);put<uint8_t>(byte_4DB25C,16);put<int32_t>(dword_4E54AC,18);put<int32_t>(dword_4DD6EC,116);std::memcpy(memory.data()+0x94000,testText,sizeof testText);for(unsigned k=0;k<16;++k)put<uint32_t>(0x99000+4*k,0xff000000u+k);}
static std::shared_ptr<const nfs_soft::FontCatalog>font(){nfs_soft::FontDescription d;for(auto&role:d.glyphs)for(auto&g:role){g.x=g.y=2;g.w=g.h=1;g.advance=65536;}std::array<uint8_t,1024*512>a{};a[2*1024+2]=255;return nfs_soft::FontCatalog::create(d,a.data(),a.size(),17);}
static void stringBody(Game&g,const char*mode){g.eax=0x94000;g.esi=!std::strcmp(mode,"measure-anchor")?169:168;g.edx=!std::strcmp(mode,"wrong-position")?testLeft+1:testLeft;g.ebx=testY;if(std::strcmp(mode,"missing-measure")){if(!std::strcmp(mode,"measure-color"))put<uint32_t>(dword_4DD714,0x1234);cap::measured(g,!std::strcmp(mode,"measure-pointer")?0x94001:0x94000,!std::strcmp(mode,"measure-zero")?0:(!std::strcmp(mode,"measure-width")?testWidth+1:(!std::strcmp(mode,"center")?testWidth*2:testWidth)),!std::strcmp(mode,"center")?cap::SourceMeasureAlignment::center:cap::SourceMeasureAlignment::right);if(!std::strcmp(mode,"measure-duplicate"))cap::measured(g,0x94000,testWidth,cap::SourceMeasureAlignment::right);if(!std::strcmp(mode,"measure-font"))put<uint8_t>(0x98000,1);if(!std::strcmp(mode,"measure-color"))put<uint32_t>(dword_4DD714,0xffff);}cap::StringScope text(g);{cap::GlyphScope glyph(g);const uint32_t at=selected.w[8]+16+(!std::strcmp(mode,"initial-logo-overlap")?0:369*686+123*2);{cap::WordCapture word(g,at,0xffff);put<uint16_t>(at,!std::strcmp(mode,"bad-word")?0x1234:0xffff);}if(!std::strcmp(mode,"extra-write"))put<uint16_t>(at+2,1);}
 if(!std::strcmp(mode,"palette"))put<uint32_t>(0x99004,123);if(!std::strcmp(mode,"font"))put<uint32_t>(dword_4DD770,0x97000);if(!std::strcmp(mode,"font-header"))put<uint8_t>(0x98000,1);if(!std::strcmp(mode,"proof-text-during"))memory[0x94000]^=1;put<int32_t>(dword_4DD6EC,g.edx+testWidth);}
static void trackedString(Game&g,const char*mode){
 g.eax=!std::strcmp(mode,"proof-resource")?testResource+1:testResource;g.edx=!std::strcmp(mode,"proof-anchor")?169:168;g.ebx=testY;g.ecx=!std::strcmp(mode,"proof-category")?0:1;
 cap::ResourceScope resource(g);
 if(!std::strcmp(mode,"proof-nested")){cap::ResourceScope nested(g);}
 if(std::strcmp(mode,"proof-no-resolved")){uint32_t text=0x94000;if(!std::strcmp(mode,"proof-resolved-pointer")){std::memcpy(memory.data()+0x94100,testText,sizeof testText);text=0x94100;}g.eax=int32_t(text);cap::resolvedResource(g,text);if(!std::strcmp(mode,"proof-resolved-duplicate"))cap::resolvedResource(g,text);}
 if(!std::strcmp(mode,"proof-resolved-font"))put<uint8_t>(0x98000,1);
 stringBody(g,mode);
 if(!std::strcmp(mode,"proof-text-after"))memory[0x94000]^=1;
 if(!std::strcmp(mode,"proof-unwind"))throw 7;
}

int main(){assert(nfs_soft::installFontCatalog(font()));unsigned cases=0;
 for(const char*mode:{"normal","proof-resource","proof-category","proof-anchor","proof-no-resolved","proof-resolved-pointer","proof-nested","proof-text-during","proof-text-after","wrong-position","measure-width","missing-measure"}){
  prepare();Game g;g.eax=testOwner;g.ebp=testOwner;cap::OwnerScope owner(g);{cap::TailScope tail(g);trackedString(g,mode);}auto proof=cap::acquireCurrentProof(g);const bool valid=!std::strcmp(mode,"normal");assert(bool(proof)==valid);if(proof){nfs_opponent_caption_display::Binding b{testOwner,0x1234,reinterpret_cast<uintptr_t>(&g),proof->evidence().serial,proof->evidence().lifetime};assert(nfs_opponent_caption_display::matches(*proof,b));++b.archive;assert(!nfs_opponent_caption_display::matches(*proof,b));}++cases;
 }
 for(const char*initialMode:{"normal","cold-zero","cold-zero-change"}){
 prepare();Game g;g.eax=g.esi=testOwner;g.ebp=off_4D3EC4;
 auto f=std::unique_ptr<main343_origin_fixture::Fixture>(new main343_origin_fixture::Fixture);f->binding.game=reinterpret_cast<uintptr_t>(&g);f->binding.publisherThread=29;f->group.thread=f->copy.thread=29;f->binding.formGuest=off_4D3EC4;f->binding.archive=0x1234;nfs_main_sidebar_origin::Store origins;auto origin=f->publish(origins);assert(origin&&origin->current());selected=origin->post().target;selected.ownerState[0]=0;std::memcpy(memory.data()+selected.w[8],selected.header.data(),16);std::memcpy(memory.data()+selected.w[8]+16,origin->post().pixels.data(),343*480*2);std::memcpy(memory.data()+selected.owner,selected.ownerWords.data(),56);put<uint32_t>(0x1c00+32,0xa0000);put<uint32_t>(dword_4D2314+2,0);put<uint32_t>(dword_4D231C,selected.ownerState[1]);put<uint32_t>(dword_4D2320,selected.ownerState[2]);put<uint32_t>(dword_4D243C,off_4D3EC4);
 if(std::strcmp(initialMode,"normal")){g.ebp=0;put<uint32_t>(dword_4D243C,0);}
 {cap::OwnerScope owner(g);{cap::InitialCallerScope caller(g,origin);if(!std::strcmp(initialMode,"cold-zero-change")){g.ebp=-1;put<uint32_t>(dword_4D243C,UINT32_MAX);}{cap::TailScope tail(g);trackedString(g,"normal");}g.esi=testOwner;}auto proof=cap::acquireCurrentProof(g);if(!std::strcmp(initialMode,"cold-zero-change")){assert(!proof);continue;}assert(proof&&proof->evidence().route==nfs_opponent_caption_source::Route::InitialMain);nfs_opponent_caption_display::Binding b{testOwner,0x1234,reinterpret_cast<uintptr_t>(&g),proof->evidence().serial,proof->evidence().lifetime};assert(nfs_opponent_caption_display::matches(*proof,b));nfs_opponent_initial_sidebar::Store initial;auto actual=origin->post();std::memcpy(actual.pixels.data(),memory.data()+selected.w[8]+16,343*480*2);nfs_opponent_initial_sidebar::Binding binding{0x4d3204,testOwner,0x1234,reinterpret_cast<uintptr_t>(&g),29,1,{1,1,0}};auto accepted=initial.accept(origin,proof,binding,actual,f->workspace,f->trusted());assert(accepted);auto wrong=binding;++wrong.epoch;assert(!initial.acquire(wrong,actual,f->trusted()));assert(!accepted->current());accepted=initial.accept(origin,proof,binding,actual,f->workspace,f->trusted());assert(accepted);origins.clear();assert(!accepted->current());assert(!proof->current());}
 }
 std::printf("PASS Opponents caption: %u steady source/negative cases, independent Main initial route and revocation; noGPUclaim\n",cases);
}

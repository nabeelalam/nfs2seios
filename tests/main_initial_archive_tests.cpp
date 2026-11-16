// SPDX-License-Identifier: MIT
#include "HdSceneFixture.h"
#include "MainThreadHarness.h"
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <thread>
static std::atomic<unsigned> allocationCount{0},allocationInLease{0};
static thread_local unsigned leaseDepth=0;
void*operator new(std::size_t n){++allocationCount;if(leaseDepth)++allocationInLease;if(void*p=std::malloc(n))return p;throw std::bad_alloc();}
void operator delete(void*p)noexcept{std::free(p);}
void operator delete(void*p,std::size_t)noexcept{std::free(p);}
void*operator new(std::size_t n,const std::nothrow_t&)noexcept{try{return ::operator new(n);}catch(...){return nullptr;}}
void operator delete(void*p,const std::nothrow_t&)noexcept{std::free(p);}
static std::vector<uint8_t>memory(16*1024*1024);static unsigned reads=0;
static void*nfs_host_address(uint32_t at,size_t n){++reads;if(at>memory.size()||n>memory.size()-at)throw std::out_of_range("synthetic guest span");return memory.data()+at;}
static uint32_t guest_address(uint32_t at){return at;}
namespace nfs {struct GuestCopyTransaction {GuestCopyTransaction(){check_worker_stop();++leaseDepth;}~GuestCopyTransaction(){--leaseDepth;}template<class T>T load(uint32_t at){check_worker_stop();T v;std::memcpy(&v,nfs_host_address(at,sizeof v),sizeof v);return v;}};}
struct Game{uint32_t eax=0,edx=0,ebx=0,ecx=0,esp=0x9000,ebp=0,esi=0;};
static constexpr uint32_t dword_4DB240=0x1000,dword_4D2314=0x2000,dword_4D231C=0x2010,dword_4D2320=0x2014,dword_4D4D24=0x2020,dword_4DABE4=0x2024,off_4DAD88=0x6000,off_4DAD64=0x6100,dword_4DADA0=0x6018,dword_4DADA8=0x6020,dword_4DAD7C=0x6118,dword_4DAD84=0x6120,dword_5636EC=0x7000,dword_5636E4=0x7004,dword_571260=0x7010,dword_571264=0x7014,dword_4DD03C=0x7018,dword_4EB74C=0xa000,dword_4D40B8=0x2028,word_56EF3C=0xb000,byte_4DB25C=0x7225,off_4D3EC4=0x405f61,off_4DADB4=0x6200;
constexpr uint32_t game_id_sub_482BD4 = 0xf0001818u;
constexpr uint32_t game_id_sub_482ED4 = 0xf000183cu;
constexpr uint32_t game_id_sub_482FD4 = 0xf0001840u;
constexpr uint32_t game_id_sub_483070 = 0xf0001844u;
constexpr uint32_t game_id_sub_483100 = 0xf0001848u;
constexpr uint32_t game_id_sub_4831A8 = 0xf000184cu;
constexpr uint32_t game_id_sub_483288 = 0xf0001850u;
constexpr uint32_t game_id_sub_483438 = 0xf0001878u;
constexpr uint32_t game_id_sub_4A14D4 = 0xf0002124u;
// Additional synthetic declarations needed by the real Location queue header.
static constexpr uint32_t aBlk4=0x7850;
static constexpr uint32_t off_4D2A20=0x8000,dword_4DD770=0xc000,dword_4DD77C=0xc004,dword_4DD774=0xc008,dword_4DD714=0xc00c,dword_4D20A0=0xc010,byte_4DB25E=0xc014,dword_4DD6F4=0xc018,dword_4DD6F8=0xc01c,dword_4E54AC=0xc020,dword_4DD6EC=0xc024;
static constexpr uint32_t game_id_sub_4A5830=0xfade0010;
#include "hd/BackgroundSceneQueueAdapter.inc"
static constexpr uint32_t word_4D232C=0xc100;
#include "hd/MenuInitialSidebarGameHooks.inc"


int main(){try{
 setenv("NFS_PRIVATE_BACKGROUND_SCENE_HD","1",1);auto f=std::unique_ptr<Fixture>(new Fixture);CHECK(nfs_background_scene_runtime::installCatalog(std::make_shared<const bg::SourceCatalog>(f->catalog)));
 auto writeArchive=[](uint32_t value){std::memcpy(memory.data()+dword_4D40B8,&value,4);};
 for(const char*mode:{"first-zero","replaced","same","wrong-form","wrong-game","order","bound-twice","missing","read-failure","unbound-iteration","unexpected-evidence","bound-retire"}){
  const uint32_t before=!std::strcmp(mode,"first-zero")||!std::strcmp(mode,"missing")||!std::strcmp(mode,"unbound-iteration")?0:0x80000;writeArchive(before);Game g;g.eax=g.esi=off_4D3EC4;
  nfs_logo_publisher::MenuOwnerScope publisher(g);nfs_background_scene_live::MainScope owner(g);auto*s=nfs_background_scene_live::lexicalCurrent(g);CHECK(!nfs_background_scene_live::current(g));CHECK(s&&!s->archiveBound&&s->archiveAtEntry==before);CHECK(!s->sequence&&!s->iteration);
  CHECK(!s->invalid);
  nfs_background_scene_live::invalidateCache();nfs_background_scene_live::retire(g);CHECK(!s->invalid&&!s->sequence&&!s->iteration);
  if(!std::strcmp(mode,"unbound-iteration")){nfs_background_scene_live::IterationScope iteration(g);iteration.start();CHECK(!s->invalid&&!s->archiveBound&&!s->iteration);continue;}
  const uint32_t actual=!std::strcmp(mode,"same")?before:!std::strcmp(mode,"missing")?0:0x90000;writeArchive(actual);
  if(!std::strcmp(mode,"wrong-form"))g.esi=0;
  if(!std::strcmp(mode,"order"))s->sequence=1;
  if(!std::strcmp(mode,"unexpected-evidence")){s->groupSerial=1;nfs_background_scene_live::invalidateCache();CHECK(s->invalid);}
  if(!std::strcmp(mode,"read-failure")){s->invalid=true;s->reason=nfs_background_scene_live::Reason::Read;}
  if(!std::strcmp(mode,"wrong-game")){Game other=g;nfs_background_scene_live::bindInitialArchive(other);CHECK(!s->archiveBound);continue;}
  nfs_background_scene_live::bindInitialArchive(g);
  const bool okay=!std::strcmp(mode,"first-zero")||!std::strcmp(mode,"replaced")||!std::strcmp(mode,"same")||!std::strcmp(mode,"bound-twice")||!std::strcmp(mode,"bound-retire");
  CHECK(s->archiveBound==okay);
  if(okay){CHECK(s->archive==actual&&!s->invalid&&nfs_background_scene_live::archiveCurrent(*s));CHECK(!s->sequence&&!s->iteration&&!s->published);if(!std::strcmp(mode,"bound-retire")){s->iteration=1;nfs_background_scene_live::invalidateCache();CHECK(s->invalid&&s->reason==nfs_background_scene_live::Reason::Retire);}
   if(!std::strcmp(mode,"bound-twice")){writeArchive(0xa0000);nfs_background_scene_live::bindInitialArchive(g);CHECK(s->archive==actual&&!nfs_background_scene_live::archiveCurrent(*s));}}
 }
 std::puts("PASS Main archive exact initialization return: first0/replaced/same and owner/order/read/missing/rebind negatives");return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"%s\n",e.what());return 1;}}

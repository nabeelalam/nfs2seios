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
#include "hd/BackgroundSceneQueueAdapter.inc"
namespace live=nfs_background_scene_live;
namespace pub=nfs_logo_publisher;
static unsigned gpuBegins=0,gpuEnds=0;static uint64_t lastBegin=0;
extern "C" int nfs_private_background_scene_begin(const NfsBackgroundSceneFrameView*v){CHECK(!leaseDepth&&v&&v->serial&&v->original565);++gpuBegins;lastBegin=v->serial;return 1;}
extern "C" int nfs_private_background_scene_finish(uint64_t serial){CHECK(serial==lastBegin);++gpuEnds;return 1;}
static void m32(uint32_t at,uint32_t v){put32(memory.data()+at,v);}
static void m16(uint32_t at,uint16_t v){put16(memory.data()+at,v);}
static void select(const rec::Target&t){for(unsigned k=0;k<14;++k){m32(t.owner+4*k,t.w[k]);m32(dword_4DB240+4*k,t.w[k]);}for(unsigned k=0;k<4;++k)m32(t.w[8]+4*k,t.header[k]);for(unsigned k=0;k<t.w[1];++k)m32(t.w[12]+4*k,t.x[k]);for(unsigned k=0;k<480;++k)m32(t.w[11]+4*k,t.y[k]);m32(dword_4D231C,0x51000);m32(dword_4D2320,0);m32(dword_4D4D24,0x41000);m32(dword_4DABE4,0);m32(dword_4D2314+2,0);}
static void prepareMemory(Fixture&f){select(f.scene.preSide.target);select(f.base.post.target);m32(dword_4D40B8,0x123456);m32(off_4D3EC4+4,0x7800);std::memcpy(memory.data()+0x7800,"blk4",5);std::memcpy(memory.data()+0x300000,f.base.sourceBefore.data(),614416);std::memcpy(memory.data()+0x500000,f.scene.draws[0].sourceBefore.data(),f.scene.draws[0].sourceBefore.size());std::memcpy(memory.data()+0x540000,f.scene.draws[1].sourceBefore.data(),f.scene.draws[1].sourceBefore.size());const uint32_t t[9]={0xf0001874,0xf0002124,0xf0001878,0xf0001878,0xf000187c,0xf0001880,0,2,0};for(unsigned k=0;k<9;++k)m32(off_4DADB4+4*k,t[k]);m32(dword_571264,0);memory[byte_4DB25C]=16;}
static void basePass(Game&g,Fixture&f){select(f.base.post.target);g.eax=0x7800;g.edx=0x400;g.ebx=g.ecx=0;live::RoleScope role(g,live::Phase::Base);g.eax=0x300000;{live::ResolvedScope resolved(g);g.eax=0x300010;g.edx=0x100010;g.ebx=307200;const auto saved=g;live::row(g,game_id_sub_483438);{live::CopyWrapperScope wrapper(g);g.ebx*=2;const auto before=reads;live::byteCopyRow(g);CHECK(before==reads);}g=saved;std::memcpy(memory.data()+0x100010,memory.data()+0x300010,614400);}CHECK(live::slot()->baseVerified&&!live::slot()->invalid);}
static void sidePass(Game&g,Fixture&f){select(f.scene.preSide.target);const auto&i=f.catalog.identities[0];for(unsigned k=0;k<256;++k)m16(word_56EF3C+2*k,opaque(bg::le32(f.scene.draws[0].sourceBefore.data()+i.paletteOffset+16+4*k)));g.eax=0x7804;g.edx=0x400;g.ebx=g.ecx=0;live::RoleScope role(g,live::Phase::Side);g.eax=0x500000;{live::ResolvedScope resolved(g);g.eax=0x500010;g.edx=0x200010;g.ebx=bg::Side;m32(g.esp+0x38,0xf000221c);const auto saved=g;{live::IndexedSideCallScope call(g,live::indexedSideCallee(g));CHECK(!std::memcmp(&g,&saved,sizeof g));for(unsigned k=0;k<bg::Side;++k)m16(0x200010+2*k,f.sideColor);}}}
static void wordPass(Game&g,Fixture&f){std::array<uint32_t,9>t{};table(t,false,0,0);for(unsigned k=0;k<9;++k)m32(off_4DAD64+4*k,t[k]);m32(dword_5636E4,0x85000);std::memset(memory.data()+0x85000,0,1024);m32(0x85004,0xffad6231);g.eax=0x7808;g.edx=0x1cff;g.ebx=g.ecx=0;live::RoleScope role(g,live::Phase::Word);g.eax=0x540000;{live::ResolvedScope resolved(g);m32(dword_4DAD7C,game_id_sub_482BD4);m32(dword_4DAD84,0x7b);g.eax=0x540010;g.edx=0x200010;g.ebx=1;live::row(g,game_id_sub_482BD4);m16(0x200010,f.wordColor);}}
static void originalGroupCopy(Fixture&f,Game&g){auto*c=pub::store();pub::preGroup(g);live::beginGroup(g);const auto serial=c->group.serial,thread=c->group.thread;c->group=f.group;c->group.serial=serial;c->group.thread=thread;m16(0x200010,f.frameColor);pub::postGroup(g);live::postGroup(g);g.ebp=off_4D3EC4;live::tailBegin(g);live::tailEnd(g);pub::armCopy(g);c->copy=f.copy;c->copy.serial=serial;c->copy.thread=thread;c->copy.linkedGroup=serial;CHECK(c->finishCopy());select(c->copy.post.target);std::memcpy(memory.data()+0x100010,c->copy.post.pixels.data(),614400);live::copyEnd(g);CHECK(!live::slot()->invalid);}
static void upload(Game&g){auto*c=pub::store();g.eax=0x100000;g.edx=1;{live::UploadEndScope after(g);pub::UploadScope original(g);CHECK(c->uploadActive);for(unsigned i=0;i<300;++i){const uint32_t desc=0x700000+64*i,payload=0xb00000;m32(dword_4EB74C+1200+4*i,desc);const uint32_t header[7]={3,3,3,11,0,i*2048,3};for(unsigned z=0;z<7;++z)m32(desc+4*z,header[z]);for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){const auto v=c->packet.originalCopyPost565[((i%15)*32+y)*640+(i/15)*32+x];const uint16_t reduced=uint16_t(((v&0xf800)>>1)|((v&0x7c0)>>1)|(v&31));m16(payload+2*(y*32+x),reduced?uint16_t(reduced|0x8000):0);}m32(g.esp,desc);m32(g.esp+4,payload);m32(g.esp+8,0);{pub::UploadTileScope tile(g);m32(desc+16,payload);}}}CHECK(c->bound&&c->uploaded==300&&live::slot()->published);}
static void present(Game&g){g.eax=1;g.edx=g.ebx=0;g.ecx=32;m32(g.esp,UINT32_MAX);pub::NativePresentationScope route(g);live::Intent intent;try{live::begin(g,intent);CHECK(intent.beginConsumed);live::finish(intent);CHECK(intent.finishConsumed);}catch(...){live::cancel(intent);throw;}}
int main(int argc,char**argv){try{CHECK(argc==2);std::string mode=argv[1];if(mode=="unset")unsetenv("NFS_PRIVATE_BACKGROUND_SCENE_HD");else setenv("NFS_PRIVATE_BACKGROUND_SCENE_HD",mode=="off"?"0":mode=="wrong"?"10":mode=="suffix"?"1x":"1",1);Game g;g.eax=off_4D3EC4;
 if(mode=="wrong"||mode=="suffix"){auto f=std::unique_ptr<Fixture>(new Fixture{});CHECK(nfs_background_scene_runtime::installCatalog(std::make_shared<const bg::SourceCatalog>(f->catalog)));}
 if(mode=="off"||mode=="unset"||mode=="missing"||mode=="wrong"||mode=="suffix"){const auto before=reads,alloc=allocationCount.load();{pub::MenuOwnerScope menu(g);live::MainScope scene(g);pub::OwnerScope iteration(g,1);live::IterationScope sceneIteration(g);iteration.start();sceneIteration.start();live::RoleScope role(g,live::Phase::Side);pub::ResolvedScope logo(g);live::ResolvedScope input(g);live::row(g,game_id_sub_482BD4);live::IndexedSideCallScope indexed(g,live::indexedSideCallee(g));live::UploadEndScope after(g);pub::UploadScope uploadScope(g);live::Intent i;live::begin(g,i);live::finish(i);live::cancel(i);}CHECK(reads==before&&allocationCount==alloc&&!pub::stateSlot()&&!live::slot());std::printf("PASS %s zero reads/allocations\n",mode.c_str());return 0;}
 auto fixture=std::unique_ptr<Fixture>(new Fixture{});auto trusted=std::make_shared<const bg::SourceCatalog>(fixture->catalog);CHECK(!nfs_background_scene_runtime::installCatalog({}));CHECK(nfs_background_scene_runtime::installCatalog(trusted));prepareMemory(*fixture);MainThreadHarness renderer;
 {pub::MenuOwnerScope menu(g);live::MainScope scene(g);CHECK(pub::owner()&&live::current(g));CHECK(!nfs_background_scene_runtime::installCatalog(trusted));
  {pub::OwnerScope warmup(g,1);live::IterationScope pending(g);warmup.start();pending.start();live::invalidateCache();pub::resetNativeCache();live::postGroup(g);g.ebp=off_4D3EC4;live::tailBegin(g);live::tailEnd(g);live::copyEnd(g);CHECK(!live::slot()->invalid&&!live::slot()->iteration);}
  basePass(g,*fixture);g.eax=off_4D3EC4;{pub::OwnerScope redraw(g,1);live::IterationScope sceneRedraw(g);redraw.start();sceneRedraw.start();sidePass(g,*fixture);wordPass(g,*fixture);CHECK(!live::slot()->invalid);originalGroupCopy(*fixture,g);upload(g);}
  auto owned=live::slot()->published;CHECK(owned&&owned->valid);const auto revision=owned->contentRevision;
  for(unsigned k=0;k<12;++k){g.eax=off_4D3EC4;pub::OwnerScope cached(g,1);live::IterationScope cachedScene(g);CHECK(live::slot()->published==owned&&owned->contentRevision==revision);present(g);}
  CHECK(gpuBegins==0&&gpuEnds==0);nfs::flush_renderer_commands();CHECK(gpuBegins==12&&gpuEnds==12);
  const auto before=reads,alloc=allocationCount.load();std::thread foreign([&]{Game other;other.eax=off_4D3EC4;pub::MenuOwnerScope m(other);live::MainScope s(other);CHECK(!pub::owner()&&!live::current(other)&&!pub::stateSlot()&&!live::slot());});foreign.join();CHECK(reads==before&&allocationCount>=alloc);
  live::invalidateCache();pub::resetNativeCache();CHECK(live::slot()->invalid&&!live::slot()->published&&!pub::store()->bound);CHECK(owned->valid&&owned->contentRevision==revision);
 }
 CHECK(!live::slot()->on&&!pub::store()->inMenu&&!leaseDepth&&allocationInLease==0);
 std::puts("PASS actual-reader synthetic base/side/word, original-record collector,300tile uploads,12cached ownedFIFO publications,retirement");return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}

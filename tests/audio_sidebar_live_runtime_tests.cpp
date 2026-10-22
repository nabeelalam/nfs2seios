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
static constexpr uint32_t off_4D2A20=0x404abd,dword_4DD770=0xc000,dword_4DD77C=0xc004,dword_4DD774=0xc008,dword_4DD714=0xc00c,dword_4D20A0=0xc010,byte_4DB25E=0xc014,dword_4DD6F4=0xc018,dword_4DD6F8=0xc01c,dword_4E54AC=0xc020,dword_4DD6EC=0xc024;
static constexpr uint32_t game_id_sub_4A5830=0xfade0010;

static constexpr uint32_t off_4D3348=0x4053e5,dword_4D243C=0xc028,byte_4D3ECC=0xc02c;
#include "hd/ContinuousLogoGameHooks.inc"
#include "hd/AudioSidebarCaptionGameHooks.inc"
#include "hd/AudioBackgroundProof.h"
namespace nfs_audio_live {
struct State{uint64_t epoch=1;uint32_t archive=0x123456;bool invalid=false;uintptr_t game=0,thread=0;};static State p;
inline State*current(Game&g){return p.game==reinterpret_cast<uintptr_t>(&g)&&p.thread==nfs_logo_publisher::threadToken()?&p:nullptr;}
inline bool span(uint32_t at,size_t n){return at&&n&&uint64_t(at)+n<=0x100000000ULL;}
inline bool archiveCurrent(State&s,bool){nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(dword_4D40B8)==s.archive;}
inline bool readBytes(uint32_t at,size_t n,std::vector<uint8_t>&v){try{if(!span(at,n))return false;v.resize(n);nfs::GuestCopyTransaction tx;std::memcpy(v.data(),nfs_host_address(at,n),n);return true;}catch(...){return false;}}
inline bool readTable(uint32_t at,std::array<uint32_t,9>&v){try{nfs::GuestCopyTransaction tx;for(unsigned k=0;k<9;++k)v[k]=tx.load<uint32_t>(at+4*k);return true;}catch(...){return false;}}
}
#define NFS_AUDIO_SIDEBAR_LIVE_TEST 1
#include "hd/AudioSidebarLiveGameHooks.inc"
namespace live=nfs_audio_sidebar_live;namespace txt=nfs_audio_sidebar_text;namespace pub=nfs_logo_publisher;
static void m32(uint32_t at,uint32_t v){put32(memory.data()+at,v);}static void m16(uint32_t at,uint16_t v){put16(memory.data()+at,v);}
static void select(const rec::Target&t){for(unsigned k=0;k<14;++k){m32(t.owner+4*k,t.w[k]);m32(dword_4DB240+4*k,t.w[k]);}for(unsigned k=0;k<4;++k)m32(t.w[8]+4*k,t.header[k]);for(unsigned k=0;k<t.w[1];++k)m32(t.w[12]+4*k,t.x[k]);for(unsigned k=0;k<480;++k)m32(t.w[11]+4*k,t.y[k]);m32(dword_4D231C,0x52000);m32(dword_4D2320,0x51000);m32(dword_4D4D24,0x41000);m32(dword_4DABE4,0);m32(dword_4D2314+2,0);}
static void prepare(Fixture&f,Game&g){std::fill(memory.begin(),memory.end(),0);select(f.scene.preSide.target);m32(dword_4D40B8,0x123456);m32(dword_4D243C,off_4D3348);m32(byte_4D3ECC,7u<<16);std::memcpy(memory.data()+0x500000,f.scene.draws[0].sourceBefore.data(),f.scene.draws[0].sourceBefore.size());std::memcpy(memory.data()+0x540000,f.scene.draws[1].sourceBefore.data(),f.scene.draws[1].sourceBefore.size());for(unsigned k=0;k<5;++k)std::memcpy(memory.data()+0x600000+0x10000*k,f.logoBytes[k].data(),f.logoBytes[k].size());memory[byte_4DB25C]=16;m32(dword_4DD770,0x980000);m32(dword_4DD77C,game_id_sub_4A5830);m32(dword_4DD774,0x990000);m32(dword_4E54AC,18);m32(dword_4DD714,0xffff);nfs_audio_live::p.game=reinterpret_cast<uintptr_t>(&g);nfs_audio_live::p.thread=pub::threadToken();nfs_audio_live::p.invalid=false;g.eax=off_4D3348;}
static void setTable(uint32_t at,const std::array<uint32_t,9>&v){for(unsigned k=0;k<9;++k)m32(at+4*k,v[k]);}
static void side(Game&g,Fixture&f){const auto&d=f.scene.draws[0];setTable(off_4DADB4,d.tableBefore);const auto&id=f.catalog.identities[0];for(unsigned k=0;k<256;++k)m16(word_56EF3C+2*k,opaque(bg::le32(d.sourceBefore.data()+id.paletteOffset+16+4*k)));g.eax=0x7804;g.edx=0x400;g.ebx=g.ecx=0;live::RoleScope side(g,live::Phase::side);g.eax=0x500000;{live::ResolvedScope resolved(g);g.eax=0x500010;g.edx=0x200010;g.ebx=bg::Side;m32(g.esp+0x38,0xf000221c);{live::IndexedSideCallScope callback(g,live::indexedSideCallee(g));for(unsigned k=0;k<bg::Side;++k)m16(0x200010+2*k,f.sideColor);}}}
static void word(Game&g,Fixture&f){const auto&d=f.scene.draws[1];setTable(off_4DAD64,d.tableBefore);g.eax=0x7810;g.edx=0x1cff;g.ebx=g.ecx=0;live::RoleScope word(g,live::Phase::word);g.eax=0x540000;{live::ResolvedScope resolved(g);setTable(off_4DAD64,d.tableAfter);m32(dword_5636E4,0x82000);for(unsigned k=0;k<256;++k)m32(0x82000+4*k,d.paletteBefore[k]);g.eax=0x540010;g.edx=0x200010;g.ebx=1;live::row(g,game_id_sub_482BD4);m16(g.edx,f.wordColor);}}
static void logos(Game&g,Fixture&f){live::preGroup(g);for(unsigned k=0;k<3;++k){const auto&d=f.group.draw[k];const bool full=k!=1;setTable(full?off_4DAD88:off_4DAD64,d.tableBefore);g.eax=0x600000+0x10000*d.role;g.edx=d.args[1];g.ebx=g.ecx=0;{live::ResolvedScope resolved(g);setTable(full?off_4DAD88:off_4DAD64,d.tableAfter);m32(full?dword_5636EC:dword_5636E4,0x82000);for(unsigned z=0;z<256;++z)m32(0x82000+4*z,d.paletteFirst[z]);g.eax+=16;g.edx=0x200010;g.ebx=1;live::row(g,full?game_id_sub_483288:game_id_sub_482BD4);if(full)m16(g.edx,uint16_t(d.paletteFirst[1]));}}live::postGroup(g);}
static void caption(Game&g,unsigned variant){live::TailScope cache(g);txt::TailScope tail(g);g.eax=0xa10000;std::memcpy(memory.data()+g.eax,"options",8);g.edx=122;g.ebx=364;{txt::StringScope text(g);if(variant!=3)live::bindCaption(g);{txt::GlyphScope glyph(g);const uint32_t at=0x200010+2*(369*343+123);{txt::WordCapture word(g,at,0xffff);m16(at,0xffff);}}m32(dword_4DD6EC,214);}if(variant==1)memory[0x200010+40]^=1;if(variant==2)m32(dword_4D40B8,0x654321);}
static nfs_audio_frame::BackgroundDraw sidebar(Game&g){nfs_audio_frame::BackgroundDraw d;d.complete=1;d.args={{0x200000,0x1000,0,0}};d.sidebarWord=0x10000;d.sidebarHandle=d.sidebarTableEntry=0x51000;d.sidebarBitmap=0x200000;CHECK(nfs_audio_live::readBytes(0x200000,nfs_audio_sidebar::Bytes,d.sourceBefore));d.sourceAfter=d.sourceBefore;return d;}
int main(){try{setenv("NFS_PRIVATE_BACKGROUND_SCENE_HD","1",1);auto f=std::unique_ptr<Fixture>(new Fixture);CHECK(nfs_background_scene_runtime::installCatalog(std::make_shared<const bg::SourceCatalog>(f->catalog)));nfs_soft::FontDescription font;for(auto&r:font.glyphs)for(auto&v:r){v.x=v.y=2;v.w=v.h=1;v.advance=65536;}std::vector<uint8_t>a(1024*512);a[2*1024+2]=255;CHECK(nfs_soft::installFontCatalog(nfs_soft::FontCatalog::create(font,a.data(),a.size(),17)));Game g;std::shared_ptr<const nfs_audio_sidebar::Packet>retained;
 for(unsigned variant=0;variant<4;++variant){prepare(*f,g);txt::OwnerScope textOwner(g);live::OwnerScope owner(g);auto*s=live::current(g);CHECK(s);{live::IterationScope lazy(g);lazy.start();side(g,*f);word(g,*f);logos(g,*f);caption(g,variant);}if(variant){CHECK(!s->published&&s->invalid);continue;}if(!s->published)std::fprintf(stderr,"failure=%u phase=%u text=%s\n",unsigned(s->failure),unsigned(s->phase),txt::failureName(txt::current(g)->failure));CHECK(s->published);CHECK(s->sideEvent<s->groupEvent&&s->groupEvent<s->captionEvent&&s->captionEvent<s->sealEvent);m32(dword_4D2314+2,0x10000);auto draw=sidebar(g);rec::Target observed{};auto p=live::acquireForSidebar(g,draw,&observed);CHECK(observed.ownerState[0]==0x10000&&observed.owner==0x51000);CHECK(p&&p->caption()->pixels()==343*480);retained=p;draw.complete=0;auto savedObserved=observed;CHECK(!live::acquireForSidebar(g,draw,&observed)&&rec::same(savedObserved,observed));draw.complete=1;draw.error=1;CHECK(!live::acquireForSidebar(g,draw));draw.error=0;draw.sidebarHandle^=4;CHECK(!live::acquireForSidebar(g,draw));draw.sidebarHandle^=4;draw.sourceBefore[20]^=1;CHECK(!live::acquireForSidebar(g,draw));draw.sourceBefore[20]^=1;auto epoch=nfs_audio_live::p.epoch;auto guestReads=reads;++nfs_audio_live::p.epoch;CHECK(!live::acquireForSidebar(g,draw)&&reads==guestReads);nfs_audio_live::p.epoch=epoch;auto count=reads;std::thread foreign([&]{Game other;other.eax=off_4D3348;live::OwnerScope denied(other);CHECK(!live::current(other));});foreign.join();CHECK(reads==count);auto original=p->binding();{live::IterationScope cached(g);}CHECK(s->published==p&&s->binding==original);m16(0x200010+200,0x1234);CHECK(!live::acquireForSidebar(g,draw));m16(0x200010+200,logo::le16(p->raw().data()+16+200));CHECK(live::acquireForSidebar(g,draw)==p);m32(0x84000+32,0x900000);m32(0x84000+48,p->target().w[12]);m32(p->target().w[12]-4,2);live::beforeDescriptorRetire(g,0x84000);CHECK(s->published==p);live::beforeDescriptorRetire(g,0x51000);CHECK(!s->published&&s->invalid);CHECK(p->raw()==retained->raw());auto oldCaptionLifetime=s->captionLifetime;select(f->scene.preSide.target);g.eax=off_4D3348;{live::IterationScope refresh(g);refresh.start();side(g,*f);word(g,*f);logos(g,*f);caption(g,0);}CHECK(s->published&&s->binding.cacheRevision>p->binding().cacheRevision&&s->binding.lifetime>p->binding().lifetime&&s->captionLifetime==oldCaptionLifetime);CHECK(s->binding.lifetime!=s->captionLifetime);}

 CHECK(retained&&retained->caption()->font());CHECK(!leaseDepth&&allocationInLease==0);std::puts("PASS Audio owned lazy343 cache producer: actual side/word/logo rows, options caption join, event ordering, selection flip persistence, exact raw acquisition, unknownwrite/archive/missingserial rejection, shared-LUT retirement, foreign no reads, no guest-lease allocations");return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}

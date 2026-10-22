// SPDX-License-Identifier: MIT
// Checked-memory original-operation simulation; scene owner is an explicit stub.
#define main existing_scene_runtime_fixture_main
#include "hd_scene_runtime_tests.cpp"
#undef main
namespace nfs_player_scene_live {
namespace bg=nfs_background_scene_cpu;
struct State{bg::Base base;bg::Plane actualSidebarPost;bg::Event sidebarValidationEvent{};std::shared_ptr<const rec::Copy>currentCopy;std::shared_ptr<const void>published;bg::Event currentCopyEvent{},copyEvent{};uint32_t form=0x4d3050,formGuest=0xe000,sourceOwner=0xe000,archive=0x123456;uintptr_t game=0,thread=0;uint64_t ownerEpoch=3,sequence=0,iteration=0,nextCopySerial=0,copySerial=0,groupSerial=17;bool on=true,invalid=false,baseVerified=true;bg::Event event(uint64_t i)noexcept{return {ownerEpoch,++sequence,i};}};
inline State*&slot()noexcept{static thread_local State*s=nullptr;return s;}static bool feature=true;
inline State*current(Game&g)noexcept{auto*s=slot();return feature&&s&&s->on&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==pub::threadToken()?s:nullptr;}
inline bool readPlane(bg::Plane&p)noexcept{uint32_t e=0;pub::plane(p,e);return !e&&p.valid;}
inline bool archiveCurrent(State&s)noexcept{try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(dword_4D40B8)==s.archive;}catch(...){return false;}}
inline uint64_t activeSidebarGroupSerial(State&s)noexcept{return s.groupSerial;}
}
#define NFS_PLAYER_COPY_RUNTIME_TEST 1
#include "hd/PlayerCopyGameHooks.inc"
#undef NFS_PLAYER_COPY_RUNTIME_TEST
namespace pc=nfs_player_copy_live;namespace pl=nfs_player_scene_live;
static void originalCopy(Game&g,Fixture&f,pl::State&o,pc::Site site,unsigned variant=0,unsigned rows=480){
 prepareMemory(f);std::memcpy(memory.data()+f.copy.args[0],f.copy.sourceBefore.data(),f.copy.span);select(f.copy.pre.target);std::memcpy(memory.data()+0x100010,f.copy.pre.pixels.data(),614400);o.invalid=false;o.baseVerified=true;o.archive=0x123456;o.formGuest=o.form==0x4d3050?0xe000:0xe100;o.sourceOwner=o.formGuest;o.base=f.base;CHECK(pl::readPlane(o.base.post));o.base.event=o.event(0);o.groupSerial=17;
 for(unsigned k=0;k<9;++k)m32(off_4DAD88+4*k,f.copy.tableBefore[k]);g.eax=f.copy.args[0];g.edx=0x2800;g.ebx=g.ecx=0;
 // Original lazy sidebar redraw toggles ambient state after the retained map.
 if(variant==19)m32(dword_4D2314+2,0x10000);if(variant==8)m32(dword_4D231C,0);if(variant==9)m32(0x51000+32,0x100000);
 {pc::Scope capture(g,site);if(variant==13)throw 13;if(variant==7){pc::Scope nested(g,site);}
  m32(dword_571264,0xf0001f44);m32(dword_571260,0xf0001844);m32(dword_4DD03C,0);
  for(unsigned y=0;y<rows;++y){const auto&r=f.copy.row[y];g.eax=r.row.args[0];g.edx=r.row.args[1];g.ebx=r.row.args[2];g.ecx=r.row.args[3];std::memcpy(memory.data()+g.eax,r.source.data(),r.bytes);if(variant==10&&y==0)memory[g.eax]^=1;if(variant==20&&y==0)m32(dword_4D2314+2,0x10000);if(variant==3)m32(dword_571264,0);const auto saved=g;pc::row(g,variant==4?0xf0001848:0xf0001844);CHECK(!std::memcmp(&saved,&g,sizeof g));for(unsigned x=0;x<343;++x)m16(g.edx+2*x,f.copy.post.pixels[y*640+x]);}
  for(unsigned k=0;k<9;++k)m32(off_4DAD88+4*k,f.copy.tableAfter[k]);if(variant==1)memory[f.copy.args[0]+16]^=1;if(variant==2)m32(0x33000,99);if(variant==5)m32(off_4DAD88,0);if(variant==6)m32(dword_4D40B8,0);if(variant==11)m16(0x100010+639*2,99);if(variant==12)o.base.event=o.event(0);if(variant==14)o.form^=1;if(variant==15)++o.formGuest;if(variant==16)++o.sourceOwner;if(variant==17){++o.archive;m32(dword_4D40B8,o.archive);}if(variant==18)++o.iteration;
 }
}
static logo::Failure replay(Fixture&f,pl::State&o){f.group.thread=pub::threadToken();return logo::verifyAndPrepare(f.group,*o.currentCopy,f.workspace.logo,f.output.logoScratch,f.catalog.logos);}
int main(){try{setenv("NFS_PRIVATE_BACKGROUND_SCENE_HD","1",1);auto f=std::unique_ptr<Fixture>(new Fixture);CHECK(f->verify()==bg::Failure::None);Game g;auto o=std::unique_ptr<pl::State>(new pl::State);pl::slot()=o.get();o->game=reinterpret_cast<uintptr_t>(&g);o->thread=pub::threadToken();
 std::shared_ptr<const rec::Copy>retained;
 for(unsigned player:{0x4d3050u,0x4d3100u})for(auto site:{pc::Site::Initial,pc::Site::Refresh,pc::Site::Lazy}){o->form=player;o->iteration=site==pc::Site::Lazy?9:0;originalCopy(g,*f,*o,site);CHECK(o->currentCopy&&o->currentCopy->complete&&o->currentCopy->rows==480&&o->currentCopy->linkedGroup==17);CHECK(o->currentCopyEvent.iteration==o->iteration);CHECK(o->copySerial==o->currentCopy->serial);CHECK(o->actualSidebarPost.valid&&o->actualSidebarPost.width==343&&o->sidebarValidationEvent.sequence<o->currentCopyEvent.sequence&&o->sidebarValidationEvent.iteration==o->currentCopyEvent.iteration);for(unsigned k=0;k<343*480;++k)CHECK(o->actualSidebarPost.pixels[k]==logo::le16(o->currentCopy->sourceBefore.data()+16+2*k));CHECK(replay(*f,*o)==logo::Failure::None);retained=o->currentCopy;}
 // Retained-map ambient changes are allowed; changes within the actual copy are rejected.
 for(unsigned player:{0x4d3050u,0x4d3100u}){o->form=player;o->iteration=9;originalCopy(g,*f,*o,pc::Site::Lazy,19);CHECK(o->currentCopy&&o->currentCopy->complete&&o->currentCopy->rows==480);CHECK(o->currentCopy->pre.target.ownerState[0]!=o->base.post.target.ownerState[0]);}
 for(unsigned v=1;v<=20;++v){if(v==13||v==19)continue;o->form=0x4d3050;o->iteration=0;originalCopy(g,*f,*o,pc::Site::Refresh,v);if(v==10||v==11){CHECK(o->currentCopy);CHECK(replay(*f,*o)!=logo::Failure::None);}else CHECK(!o->currentCopy);}
 o->form=0x4d3050;o->iteration=0;try{originalCopy(g,*f,*o,pc::Site::Refresh,13);CHECK(false);}catch(int value){CHECK(value==13&&!o->currentCopy&&pc::slot()->failure==pc::Failure::Unwind);}
 // Direct lexical sites must have no lazy iteration; lazy needs its actual one.
 o->iteration=4;originalCopy(g,*f,*o,pc::Site::Refresh);CHECK(!o->currentCopy);o->iteration=0;originalCopy(g,*f,*o,pc::Site::Lazy);CHECK(!o->currentCopy);
 // Row count is observed, not synthesized to 480; pure replay rejects omissions.
 originalCopy(g,*f,*o,pc::Site::Refresh,0,19);CHECK(o->currentCopy&&o->currentCopy->rows==19);CHECK(replay(*f,*o)!=logo::Failure::None);
 originalCopy(g,*f,*o,pc::Site::Refresh);const auto count=reads,alloc=allocationCount.load();pl::feature=false;g.eax=0x200000;g.edx=0x2800;g.ebx=g.ecx=0;{pc::Scope disabled(g,pc::Site::Initial);pc::row(g,0xf0001844);}pl::feature=true;g.edx=0;{pc::Scope args(g,pc::Site::Initial);}g.edx=0x2800;{pc::Scope badSite(g,static_cast<pc::Site>(99));}CHECK(reads==count&&allocationCount==alloc);
 std::thread foreign([&]{Game other;other.eax=0x200000;other.edx=0x2800;pc::Scope scope(other,pc::Site::Refresh);pc::row(other,0xf0001844);CHECK(!pc::slot());});foreign.join();CHECK(reads==count&&allocationInLease==0);CHECK(retained&&retained->complete&&retained->rows==480);pl::slot()=nullptr;std::puts("PASS Player copy checked scopes: both players/three sites, retained group/direct refresh, real rows, source/order/callback failures, pure payload/POST rejection, disabled/foreign zero reads, no lease allocations; owner stub only");return 0;}catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}

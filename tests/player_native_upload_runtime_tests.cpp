// SPDX-License-Identifier: MIT
// Actual upload observer with checked guest reads and independently sealed pure
// fixtures. Root source-join/current-owner callbacks are explicit test stubs.
#define main existing_scene_runtime_fixture_main
#include "hd_scene_runtime_tests.cpp"
#undef main
#include "PlayerSceneFixture.h"
#include "hd/PlayerNativeUploadWitness.h"
namespace nfs_player_scene_live {
struct State {
 std::shared_ptr<const nfs_player_scene_cpu::VerifiedSource>candidate;
 nfs_player_native::CurrentBinding independent{};
 uintptr_t game=0,thread=0;bool on=false,allowSource=true,allowCurrent=true;
 unsigned prepares=0,bindings=0;
};
inline State*&slot()noexcept{static thread_local State*s=nullptr;return s;}
inline State*current(Game&g)noexcept{auto*s=slot();return s&&s->on&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==pub::threadToken()?s:nullptr;}
}
#define NFS_PLAYER_NATIVE_RUNTIME_TEST 1
#include "hd/PlayerNativeUploadGameHooks.inc"
#undef NFS_PLAYER_NATIVE_RUNTIME_TEST
namespace pn=nfs_player_native_live;namespace nt=nfs_player_native;namespace owner_stub=nfs_player_scene_live;
namespace nfs_player_native_live {
std::shared_ptr<const Source>prepareSource(Game&g,uint64_t event)noexcept{auto*s=scene::current(g);if(!s||!s->allowSource||event!=pub::uploadEvents().load())return {};++s->prepares;return s->candidate;}
bool currentBinding(Game&g,const SourceBinding&,CurrentBinding&out)noexcept{
 out={};auto*s=scene::current(g);if(!s||!s->allowCurrent)return false;++s->bindings;
 // Independent mutable owner occurrence, not copied from the source argument.
 out=s->independent;try{nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=out.source.archive||tx.load<uint32_t>(out.fullTargetOwner+32)!=out.sourceBitmap)return false;}catch(...){return false;}return true;
}
}
static void installSource(Game&g,player_scene_fixture::Fixture&f,owner_stub::State&owner){
 f.binding.gameOwner=reinterpret_cast<uintptr_t>(&g);f.binding.thread=pub::threadToken();f.seal();CHECK(f.source);owner.candidate=f.source;owner.game=reinterpret_cast<uintptr_t>(&g);owner.thread=pub::threadToken();owner.on=owner.allowSource=owner.allowCurrent=true;owner.independent={};owner.independent.source=f.binding;owner.independent.sourceOwner=f.binding.sourceOwner;owner.independent.fullTargetOwner=f.actualUpload.target.owner;owner.independent.sourceBitmap=f.actualUpload.target.w[8];owner_stub::slot()=&owner;
 select(f.actualUpload.target);std::memcpy(memory.data()+f.actualUpload.target.w[8]+16,f.actualUpload.pixels.data(),614400);m32(dword_4D40B8,f.binding.archive);
 // Deliberately select the343 ambient display target; the real upload reads
 // the retained640 owner's bitmap, never global-current selected storage.
 select(f.scene.preSide.target);
}
static void originalTile(Game&g,const nt::Source&source,unsigned n,unsigned variant){
 const uint32_t descriptor=0x700000+64*n,payload=0xb00000;m32(dword_4EB74C+1200+4*n,descriptor);const uint32_t words[7]={3,3,3,11,0,0x1000000+2048*n,3};for(unsigned k=0;k<7;++k)m32(descriptor+4*k,words[k]);
 // Independent conversion of the captured original RGB565 to original1555:
 // discard G's least significant bit, pack5/5/5; black retains zero alpha.
 for(unsigned yy=0;yy<32;++yy)for(unsigned xx=0;xx<32;++xx){uint16_t p=source.original()[((n%15)*32+yy)*640+(n/15)*32+xx];unsigned r=p/2048,g6=(p/32)%64,b=p%32;uint16_t q=uint16_t(r*1024+(g6/2)*32+b);m16(payload+2*(yy*32+xx),q?uint16_t(q+0x8000):0);}
 m32(g.esp,descriptor);m32(g.esp+4,payload);m32(g.esp+8,0);
 if(n==1&&variant==1)m32(g.esp+8,0x1234);if(n==1&&variant==2)memory[payload+2047]^=1;if(n==1&&variant==5)m32(g.esp,descriptor+4);if(n==1&&variant==6)m32(dword_4EB74C+1200+4*n,descriptor+64);if(n==1&&variant==13)m32(g.esp+4,0xfffffff0);
 {pub::UploadTileScope existing(g);pn::UploadTileScope observed(g);
  // Simulate original49755C's descriptor install while retaining its payload.
  m32(descriptor+16,payload);if(n==1&&variant==3)memory[payload+2047]^=1;if(n==1&&variant==4)m32(descriptor+12,10);if(n==1&&variant==11)throw 7;
  if(n==1&&variant==14){pn::UploadTileScope nested(g);}
 }
}
static std::shared_ptr<const nt::Proof>record(Game&g,player_scene_fixture::Fixture&f,owner_stub::State&owner,unsigned variant=0){
 installSource(g,f,owner);g.esp=0x9000;g.eax=f.actualUpload.target.w[8];g.edx=1;const auto oldEvent=pub::uploadEvents().load();
 try{
  pub::UploadScope existing(g);const auto event=pub::uploadEvents().load();CHECK(event==oldEvent+1);
  pn::UploadScope observed(g);CHECK(observed.entered&&pn::slot()->active);CHECK(pub::uploadEvents().load()==event);
  for(unsigned n=0;n<(variant==7?299u:300u);++n){originalTile(g,*f.source,n,variant);if(n==1&&variant==9)++owner.independent.source.contentSerial;if(n==1&&variant==10)pub::overwriteEvent();if(n==1&&variant==15)pn::retire(f.actualUpload.target.owner);if(n==1&&variant==16){g.eax=f.actualUpload.target.w[8];g.edx=1;pub::UploadScope unrelated(g);pn::UploadScope nested(g);}}
  if(variant==8)m16(f.actualUpload.target.w[8]+16,0x2345);if(variant==12)throw 7;
 }catch(int){CHECK(variant==11||variant==12);}
 auto actual=std::unique_ptr<nt::Plane>(new nt::Plane);nt::CurrentBinding current{};return pn::proofForPresentation(g,current,*actual);
}
int main(){try{
 setenv("NFS_PRIVATE_BACKGROUND_SCENE_HD","1",1);auto mainFixture=std::unique_ptr<Fixture>(new Fixture);CHECK(nfs_background_scene_runtime::installCatalog(std::make_shared<const bg::SourceCatalog>(mainFixture->catalog)));
 Game g;auto owner=std::unique_ptr<owner_stub::State>(new owner_stub::State);owner_stub::slot()=owner.get();std::shared_ptr<const nt::Proof>escaped;nt::CurrentBinding escapedBinding{};
 for(unsigned player:{1u,2u})for(unsigned mode:{0u,1u,2u}){auto f=std::unique_ptr<player_scene_fixture::Fixture>(new player_scene_fixture::Fixture(player,mode));auto proof=record(g,*f,*owner);CHECK(proof);auto actual=std::unique_ptr<nt::Plane>(new nt::Plane);nt::CurrentBinding c{};CHECK(pn::proofForPresentation(g,c,*actual)==proof);CHECK(proof->source(c)==f->source&&proof->uploadEvent(c)==pub::uploadEvents().load());std::array<uint32_t,300>keys{};CHECK(proof->keys(c,keys));for(unsigned k=0;k<300;++k)CHECK(keys[k]==0x1000000+2048*k);auto prepared=std::unique_ptr<nfs_player_scene_cpu::Prepared>(new nfs_player_scene_cpu::Prepared);CHECK(proof->preparedForPresentation(c,*actual,*prepared)&&prepared->original==f->actualUpload.pixels);escaped=proof;escapedBinding=c;pn::reset();CHECK(!escaped->current(c));}
 auto f=std::unique_ptr<player_scene_fixture::Fixture>(new player_scene_fixture::Fixture(1,1));
 for(unsigned variant=1;variant<=16;++variant){CHECK(!record(g,*f,*owner,variant));CHECK(pn::slot()->failure!=pn::Failure::None);}
 escaped=record(g,*f,*owner);CHECK(escaped);escapedBinding=owner->independent;
 // An unrelated slot1 upload revokes escaped proof BEFORE Player site matching,
 // and the generic constructor is the sole event increment. No source reads.
 owner->on=false;g.edx=1;g.eax=0xabcdef;const auto before=reads,alloc=allocationCount.load();const auto event=pub::uploadEvents().load();{pub::UploadScope generic(g);pn::UploadScope unrelated(g);}CHECK(pub::uploadEvents().load()==event+1&&reads==before&&allocationCount==alloc&&!escaped->current(escapedBinding));owner->on=true;
 escaped=record(g,*f,*owner);CHECK(escaped);escapedBinding=owner->independent;g.edx=0;const auto event0=pub::uploadEvents().load();{pub::UploadScope generic(g);pn::UploadScope slot0(g);}CHECK(pub::uploadEvents().load()==event0&&escaped->current(escapedBinding));
 for(uint32_t retired:{f->binding.formGuest,f->actualUpload.target.owner,f->actualUpload.target.w[8],f->scene.preSide.target.owner,f->scene.preSide.target.w[8]}){escaped=record(g,*f,*owner);CHECK(escaped);escapedBinding=owner->independent;pn::retire(retired);CHECK(!escaped->current(escapedBinding));}
 // Declining independent CURRENT state revokes old proof tokens, even if the
 // CPU slot1 event has not changed. It cannot leave a stale replay authorized.
 escaped=record(g,*f,*owner);CHECK(escaped);escapedBinding=owner->independent;owner->allowCurrent=false;auto actual=std::unique_ptr<nt::Plane>(new nt::Plane);nt::CurrentBinding current{};CHECK(!pn::proofForPresentation(g,current,*actual)&&!escaped->current(escapedBinding));
 // Disabled/foreign observer paths do not read guest memory or allocate. The
 // real generic event increment is not duplicated by these optional scopes.
 owner->on=false;g.eax=f->actualUpload.target.w[8];g.edx=1;const auto disabledReads=reads,disabledAlloc=allocationCount.load();{pn::UploadScope off(g);pn::UploadTileScope tile(g);}CHECK(reads==disabledReads&&allocationCount==disabledAlloc);
 std::thread foreign([&]{Game other;other.eax=0x100000;other.edx=1;pn::UploadScope off(other);pn::UploadTileScope tile(other);CHECK(!pn::slot());});foreign.join();CHECK(reads==disabledReads&&allocationInLease==0&&!leaseDepth);pn::reset();owner_stub::slot()=nullptr;
 std::puts("PASS Player native upload runtime: actual checked-owner read+300 lexical descriptor/payload observations,both forms/all source modes,16 corruption/lifetime failures,global event single increment,unrelated overwrite,retire/reset/escaped proofs,foreign/disabled zero reads,no lease allocations; source/current callbacks explicitly stubbed,CPU-only");return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}

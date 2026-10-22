// SPDX-License-Identifier: MIT
// Actual Player queue and MainThreadBridge. Source/current/native callbacks are
// explicit stubs; every native proof is sealed by real300-tile Witness replay.
#define main existing_scene_runtime_fixture_main
#include "hd_scene_runtime_tests.cpp"
#undef main
#include "PlayerSceneFixture.h"
#include "hd/PlayerNativeUploadWitness.h"
namespace nfs_player_scene_live {
namespace bg=nfs_player_scene_cpu;
enum class CachedUploadSite{None,Initial,Refresh,Lazy};enum class Reason{Retire};
struct State{std::shared_ptr<const bg::Prepared>published;uintptr_t game=0,thread=0;uint64_t ownerEpoch=0,nativeUploadEvent=0;unsigned stage=0,cachedUploadDepth=1;CachedUploadSite cachedUploadSite=CachedUploadSite::Refresh;bool on=false,invalid=false;void fail(Reason,bool)noexcept{invalid=true;published.reset();}};
inline State*&slot()noexcept{static thread_local State*s=nullptr;return s;}
inline State*current(Game&g)noexcept{auto*s=slot();return s&&s->on&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==pub::threadToken()?s:nullptr;}
inline bool enabled()noexcept{return true;}
}
namespace nfs_player_source_live {
struct Cache{nfs_logo_capture::Plane upload;};inline Cache*&slot()noexcept{static thread_local Cache*c=nullptr;return c;}
inline void reset()noexcept{if(slot())slot()->upload.valid=0;}
inline bool relatedRetire(uint32_t at)noexcept{return at==0x100000||at==0x41000||at==0x51000;}
}
namespace nfs_player_native_live {
namespace nt=nfs_player_native;
struct NativeStub{nt::Witness witness;nt::CurrentBinding binding;std::shared_ptr<const nt::Source>source;};
inline NativeStub*&slot()noexcept{static thread_local NativeStub*s=nullptr;return s;}
inline void reset()noexcept{if(slot())slot()->witness.reset();}
inline void retire(uint32_t at)noexcept{if(at)reset();}
inline std::shared_ptr<const nt::Proof>proofForPresentation(Game&g,nt::CurrentBinding&out,nt::Plane&actual)noexcept{auto*s=slot();auto*c=nfs_player_source_live::slot();if(!nfs_player_scene_live::current(g)||!s||!c)return {};out=s->binding;actual=c->upload;return s->witness.proofForPresentation(out,actual);}
}
#define NFS_PLAYER_QUEUE_RUNTIME_TEST 1
#include "hd/PlayerSceneQueueAdapter.inc"
#undef NFS_PLAYER_QUEUE_RUNTIME_TEST
namespace pq=nfs_player_scene_live;namespace psource=nfs_player_source_live;namespace pnative=nfs_player_native_live;namespace nt=nfs_player_native;
struct Observation{unsigned kind;uint64_t serial;};static std::vector<Observation>trace;static uint64_t pending=0;static std::thread::id rendererThread;
static std::weak_ptr<const nfs_player_scene_cpu::Prepared>expectedPrepared;
extern "C" int nfs_private_player_scene_begin(const NfsPlayerSceneFrameView*v){CHECK(std::this_thread::get_id()==rendererThread&&!leaseDepth);auto owned=expectedPrepared.lock();CHECK(owned&&v&&v->scene.serial&&v->scene.original565==owned->original.data());CHECK(v->preview.pixels565==owned->overlay->pixels.data()&&v->caption.vertices==owned->caption->vertices().data());CHECK(v->scene.original565[234567]==owned->original[234567]);trace.push_back({1,v->scene.serial});pending=v->scene.serial;return 1;}
extern "C" int nfs_private_player_scene_finish(uint64_t serial){CHECK(std::this_thread::get_id()==rendererThread&&!leaseDepth);trace.push_back({3,serial});CHECK(pending==serial);pending=0;return 1;}
extern "C" int nfs_private_player_scene_cancel(uint64_t serial){CHECK(std::this_thread::get_id()==rendererThread&&!leaseDepth);trace.push_back({4,serial});if(pending==serial)pending=0;return 1;}
static nt::TileObservation originalTile(const nt::Source&s,unsigned k){nt::TileObservation t;t.descriptorGuest=t.expectedSlotDescriptor=0x700000+64*k;t.payloadGuest=0xb00000;t.before={{3,3,3,11,0,0x1000000+2048*k,3}};t.after=t.before;t.after[4]=t.payloadGuest;t.originalUploadReturned=true;for(unsigned y=0;y<32;++y)for(unsigned x=0;x<32;++x){uint16_t p=s.original()[((k%15)*32+y)*640+(k/15)*32+x];unsigned rgb=(p/2048)*1024+(((p/32)%64)/2)*32+p%32;uint16_t word=rgb?uint16_t(rgb+32768):0;t.pixels[2*(y*32+x)]=uint8_t(word);t.pixels[2*(y*32+x)+1]=uint8_t(word>>8);}return t;}
struct Context {
 Game game;std::unique_ptr<player_scene_fixture::Fixture>fixture;std::unique_ptr<pq::State>owner;std::unique_ptr<psource::Cache>cache;std::unique_ptr<pnative::NativeStub>native;
 explicit Context(unsigned player=1):fixture(new player_scene_fixture::Fixture(player,1)),owner(new pq::State),cache(new psource::Cache),native(new pnative::NativeStub){install();}
 void install(){pq::slot()=owner.get();psource::slot()=cache.get();pnative::slot()=native.get();owner->on=true;owner->invalid=false;owner->game=reinterpret_cast<uintptr_t>(&game);owner->thread=pub::threadToken();owner->ownerEpoch=1;owner->cachedUploadDepth=1;owner->cachedUploadSite=pq::CachedUploadSite::Refresh;fixture->binding.gameOwner=owner->game;fixture->binding.thread=owner->thread;fixture->seal();CHECK(fixture->source);native->source=fixture->source;auto&c=native->binding;c={};c.source=fixture->binding;c.sourceOwner=c.source.sourceOwner;c.fullTargetOwner=fixture->actualUpload.target.owner;c.sourceBitmap=fixture->actualUpload.target.w[8];cache->upload=fixture->actualUpload;const auto event=pub::overwriteEvent();owner->nativeUploadEvent=event;CHECK(native->witness.begin(native->source,c,cache->upload,event,pub::uploadEvents()));for(unsigned k=0;k<300;++k)CHECK(native->witness.tile(originalTile(*native->source,k),c));CHECK(native->witness.finish(cache->upload,true,c));game.eax=1;game.edx=game.ebx=0;game.ecx=32;game.esp=0x9000;m32(game.esp,UINT32_MAX);pq::activeRequest()=0;pq::accepted()={};pq::sealUpload(game);CHECK(owner->published&&pq::accepted().proof&&owner->stage==13);expectedPrepared=owner->published;}
 void forgetPrepared(){owner->published.reset();pq::accepted()={};}
 ~Context(){pq::accepted()={};pq::slot()=nullptr;psource::slot()=nullptr;pnative::slot()=nullptr;}
};
static void drain(){const auto before=reads;nfs::flush_renderer_commands();CHECK(reads==before&&!pending);}
static void nativeCommand(uint64_t serial){CHECK(nfs::enqueue_owned_renderer([serial](){CHECK(std::this_thread::get_id()==rendererThread);trace.push_back({2,serial});return int32_t(0);})==nfs::OwnedRendererAcceptance::Consumed);}
static void normalAndOwnership(){Context c;trace.clear();pq::Intent i;pq::begin(c.game,i);CHECK(i.beginConsumed&&i.serial&&pq::activeRequest()==i.serial);const auto serial=i.serial;nativeCommand(serial);pq::finish(i);CHECK(i.finishConsumed&&!pq::activeRequest()&&trace.empty());std::weak_ptr<const nfs_player_scene_cpu::Prepared>weak=i.owned;c.forgetPrepared();pq::cancel(i);CHECK(!weak.expired());drain();CHECK(weak.expired()&&trace.size()==3&&trace[0].kind==1&&trace[1].kind==2&&trace[2].kind==3);for(auto o:trace)CHECK(o.serial==serial);}
static void lateInvalidation(bool reset){Context c;trace.clear();pq::Intent i;pq::begin(c.game,i);const auto serial=i.serial;nativeCommand(serial);pq::finish(i);if(reset)pnative::reset();else pub::overwriteEvent();c.forgetPrepared();pq::cancel(i);drain();CHECK(trace.size()==2&&trace[0].kind==2&&trace[1].kind==4&&trace[1].serial==serial);}
static void afterBeginOverwrite(){Context c;trace.clear();pq::Intent i;pq::begin(c.game,i);const auto serial=i.serial;const auto before=reads;nfs::flush_renderer_commands();CHECK(reads==before&&pending==serial&&trace.size()==1);pub::overwriteEvent();pq::finish(i);pq::cancel(i);drain();CHECK(trace.size()==2&&trace[1].kind==4&&trace[1].serial==serial);}
static void guards(){Context c;for(unsigned k=0;k<5;++k){auto saved=c.game;if(k==0)c.game.eax=2;if(k==1)c.game.edx=1;if(k==2)c.game.ebx=1;if(k==3)c.game.ecx=31;if(k==4)m32(c.game.esp,0);pq::Intent i;pq::begin(c.game,i);CHECK(!i.beginConsumed&&!i.serial);c.game=saved;m32(c.game.esp,UINT32_MAX);}
 const auto accepted=pq::accepted();++c.owner->ownerEpoch;{pq::Intent i;pq::begin(c.game,i);CHECK(!i.serial);}--c.owner->ownerEpoch;pq::accepted().owner=nullptr;{pq::Intent i;pq::begin(c.game,i);CHECK(!i.serial);}pq::accepted()=accepted;
 c.native->binding.sourceOwner++;{pq::Intent i;pq::begin(c.game,i);CHECK(!i.serial);}c.install();auto actual=std::unique_ptr<nt::Plane>(new nt::Plane);nt::CurrentBinding binding{};auto old=pnative::proofForPresentation(c.game,binding,*actual);CHECK(old);c.native->witness.reset();CHECK(!old->current(binding));{pq::Intent i;pq::begin(c.game,i);CHECK(!i.serial);}
 c.install();trace.clear();pq::Intent first,second;pq::begin(c.game,first);CHECK(first.beginConsumed);pq::begin(c.game,second);CHECK(!second.serial);const auto serial=first.serial;pq::activeRequest()=serial+1;pq::finish(first);CHECK(!first.serial&&pq::activeRequest()==serial+1);pq::activeRequest()=0;drain();CHECK(trace.size()==2&&trace[0].kind==1&&trace[1].kind==4&&trace[1].serial==serial);
 c.install();c.owner->cachedUploadDepth=0;pq::sealUpload(c.game);CHECK(!c.owner->published&&!pq::accepted().proof);c.install();c.owner->cachedUploadSite=pq::CachedUploadSite::None;pq::sealUpload(c.game);CHECK(!c.owner->published&&!pq::accepted().proof);
}
static void scopeCancellation(){Context c;trace.clear();uint64_t serial=0;try{pq::PresentationScope scope(c.game);CHECK(scope.intent.beginConsumed);serial=scope.intent.serial;nativeCommand(serial);throw 7;}catch(int){}drain();CHECK(trace.size()==3&&trace[0].kind==1&&trace[1].kind==2&&trace[2].kind==4&&trace[2].serial==serial);
 c.install();trace.clear();nfs::WorkerControl stop;nfs::current_worker_control()=&stop;stop.stop=true;bool stopped=false;try{pq::PresentationScope before(c.game);}catch(const nfs::WorkerStopped&){stopped=true;}CHECK(stopped&&!pq::activeRequest());stop.stop=false;nfs::current_worker_control()=nullptr;drain();CHECK(trace.empty());
 c.install();trace.clear();nfs::current_worker_control()=&stop;stopped=false;try{pq::PresentationScope during(c.game);CHECK(during.intent.beginConsumed);serial=during.intent.serial;nativeCommand(serial);stop.stop=true;}catch(const nfs::WorkerStopped&){stopped=true;}stop.stop=false;nfs::current_worker_control()=nullptr;CHECK(stopped&&!pq::activeRequest());drain();CHECK(trace.size()==3&&trace[0].kind==1&&trace[1].kind==2&&trace[2].kind==4&&trace[2].serial==serial);
}
static void lifetimeAndRetire(){Context c;trace.clear();auto proof=pq::accepted().proof;auto binding=c.native->binding;CHECK(proof->current(binding));c.game.eax=0x3333;pq::retireSource(c.game);CHECK(proof->current(binding));c.game.eax=0x51000;pq::retireSource(c.game);CHECK(!proof->current(binding)&&c.owner->invalid&&!c.owner->published&&!pq::accepted().proof);c.install();proof=pq::accepted().proof;binding=c.native->binding;{pq::SourceLifetimeScope lifetime(c.game);CHECK(!proof->current(binding)&&!c.owner->published&&!pq::accepted().proof);}CHECK(!c.owner->nativeUploadEvent);}
int main(){try{
 MainThreadHarness harness;rendererThread=harness.owner_id();normalAndOwnership();lateInvalidation(false);lateInvalidation(true);afterBeginOverwrite();guards();scopeCancellation();lifetimeAndRetire();
 // A real renderer-owner caller receives fallback without queueing/executing
 // optional GPU work. This direct lexical entry may inspect its own stack;
 // deferred owner closures above never inspect borrowed guest memory.
 trace.clear();nfs::run_on_main([](void*){Context ownerContext(2);pq::Intent i;pq::begin(ownerContext.game,i);CHECK(!i.serial&&!i.beginConsumed&&!pq::activeRequest());pq::finish(i);pq::cancel(i);},nullptr);drain();CHECK(trace.empty());CHECK(allocationInLease==0&&!leaseDepth);
 std::puts("PASS Player queue runtime: genuine300tile sealed proof,real worker/owner FIFO,immutable lifetime,exact args/stack,current/accepted mismatch,nesting/wrongserial,overwrite/reset/retire,RAII stop+unwind cancellation,owner fallback; typed GPU/source stubs only,no GL claim");return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}

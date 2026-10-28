// PRIVATE dedicated LoadSave full-scene collector. Not registered or included
// by any generated unit yet. Optional flag is OFF by default. No GPU authority.
#pragma once
#include "FullSceneSource.h"
#include "hd/BackgroundSceneCatalog.h"
#ifndef NFS_LS_SCENE_CONTRACT
#include "hd/ContinuousLogoGameHooks.inc"
#endif
#include "ProfileIdentity.h"
#include "RetirePolicy.h"
#include <atomic>
#include <cerrno>
#include <exception>
#include <cstdlib>
namespace nfs_ls_scene_live {
namespace sl=nfs_small_logo_live;
using namespace nfs_ls_scene;
namespace read {
inline bool span(uint32_t p,size_t n)noexcept{return p&&n&&uint64_t(p)+n<=UINT64_C(0x100000000);}
inline bool readBytes(uint32_t p,size_t n,std::vector<uint8_t>&v)noexcept{if(!span(p,n)||n>614416)return false;try{v.resize(n);nfs::GuestCopyTransaction tx;std::memcpy(v.data(),nfs_host_address(p,n),n);return true;}catch(...){return false;}}
inline bool readTable(uint32_t p,std::array<uint32_t,9>&v)noexcept{try{nfs::GuestCopyTransaction tx;for(unsigned k=0;k<9;++k)v[k]=tx.load<uint32_t>(p+4*k);return true;}catch(...){return false;}}
}
enum class Phase {idle,side,word,logos,tail,complete};
enum class Failure {none,owner,nested,order,read,source,target,row,palette,caption,packet,retired,overflow,unwind};
struct State {
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:1
unsigned initStage=0,initIndex=UINT32_MAX,initRole=UINT32_MAX;
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:20
unsigned initWordMask=0,initLogoMask=0;
// NFS_LS_INIT_DIAGNOSTIC_END:20
std::array<uint32_t,4>initHeader{};
// NFS_LS_INIT_DIAGNOSTIC_END:1

 uintptr_t game=0,thread=0;unsigned depth=0,permitDepth=0,invocations=0;uint64_t epoch=0,revision=0,sequence=0;unsigned traceLines=0;
 bool ready=false,invalid=false,iteration=false,resolved=false,rowActive=false,groupDone=false,capturedCopy=false;unsigned copyDepth=0;int pending=-1;Phase phase=Phase::idle;Failure failure=Failure::none;
 Owner owner;Slots slotsBefore,slotsAfter;std::array<uint32_t,4>copyArgs{};Directory directory;
 struct {uint32_t form=Form,sourceOwner=0,bitmap=0;}binding;
 std::shared_ptr<const bg::SourceCatalog>catalog;std::shared_ptr<const Trusted>trusted;
 bg::Scene scene;nfs_logo_capture::Group group;Target target;
 Caption caption;CaptionProfile captionProfile;std::unique_ptr<nfs_soft::FramePacketBuilder>captionBuilder;
 std::vector<uint8_t>rawBefore,rawAfter;std::array<uint64_t,5>events{};Collector collector;
 std::shared_ptr<const Cache>published;std::shared_ptr<const OwnedScene>issuedScene;
 bool retiring=false,refOpen=false;unsigned released=0;uint32_t refInput=0,refBefore=0;nfs_ls_retire::Plan retirePlan;Slots retiredSlots;Target retiredFull;
 bool frameOpen=false,frameSide=false,frameResolved=false,fullRowOpen=false;uint32_t mapName=0;unsigned mapRole=2;Target fullTarget;FullDraw resetDraw,copyDraw;std::vector<uint16_t>fullPre,postReset,clean;
 uint32_t fontAddress=0,glyphAddress=0;bool measured=false;uint32_t measuredText=0;size_t writes=0;bool stringOpen=false,glyphOpen=false;size_t glyphIndex=0;
 nfs_loadsave_source::GlyphUse glyph;std::vector<uint16_t>captionPlane;
 void fail(Failure why)noexcept{int saved=errno;if(!invalid&&why!=Failure::owner&&traceLines<24){const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(p&&!std::strcmp(p,"1")){++traceLines;
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:19

// NFS_LS_INIT_DIAGNOSTIC_BEGIN:22
std::fprintf(stderr,"LOADSAVE INIT roles words=%x logos=%x\n",initWordMask,initLogoMask);
// NFS_LS_INIT_DIAGNOSTIC_END:22
std::fprintf(stderr,"LOADSAVE INIT refused stage=%u index=%u role=%u magic=%08x size=%u count=%u\n",initStage,initIndex,initRole,initHeader[0],initHeader[1],initHeader[2]);
// NFS_LS_INIT_DIAGNOSTIC_END:19
/* NFS_ASSEMBLY_DIAG_BEGIN:210 */const auto&assembly=nfs_ls_scene::assemblyDiagnostic();std::fprintf(stderr,"LOADSAVE ASSEMBLY refused checkpoint=%u count=%u code0=%u code1=%u code2=%u code3=%u index=%u\n",assembly.checkpoint,assembly.count,assembly.codes[0],assembly.codes[1],assembly.codes[2],assembly.codes[3],assembly.index);
/* NFS_ASSEMBLY_DIAG_END:210 */std::fprintf(stderr,"LOADSAVE SCENE collector-refused epoch=%llu revision=%llu phase=%u reason=%u slot=%u\n",(unsigned long long)epoch,(unsigned long long)revision,unsigned(phase),unsigned(why),slotsAfter.selected);}}errno=saved;invalid=true;ready=false;failure=why;published.reset();issuedScene.reset();collector.cancel();if(captionBuilder)captionBuilder->invalidate();}
 bool event(uint64_t&v)noexcept{if(sequence==UINT64_MAX){fail(Failure::overflow);return false;}v=++sequence;return true;}
 bg::Draw*draw()noexcept{return pending>=0&&size_t(pending)<scene.draws.size()?&scene.draws[size_t(pending)]:nullptr;}
};
inline uintptr_t token()noexcept{static thread_local unsigned char t;return reinterpret_cast<uintptr_t>(&t);}
inline std::atomic<uintptr_t>&ownerThread(){static std::atomic<uintptr_t>v{0};return v;}
inline std::atomic<uint64_t>&epochs(){static std::atomic<uint64_t>v{0};return v;}
inline std::shared_ptr<State>&storage(){static thread_local std::shared_ptr<State>v;return v;}
inline State*lexical(Game&g)noexcept{if(ownerThread().load(std::memory_order_acquire)!=token())return nullptr;auto&s=storage();return s&&s->depth==1&&s->game==reinterpret_cast<uintptr_t>(&g)?s.get():nullptr;}
#ifdef NFS_LS_SCENE_CONTRACT
inline bool&failStateAllocation(){static thread_local bool v=false;return v;}
#endif
inline std::shared_ptr<State>newState(){
#ifdef NFS_LS_SCENE_CONTRACT
 if(failStateAllocation())throw std::bad_alloc();
#endif
 return std::make_shared<State>();
}
inline State*current(Game&g)noexcept{auto*s=lexical(g);return s&&s->ready?s:nullptr;}
inline bool enabled()noexcept{const char*p=std::getenv("NFS_PRIVATE_LOADSAVE_FULLSCENE");return p&&!std::strcmp(p,"1");}
struct Preserve {Game&g;int saved=errno;std::array<unsigned char,sizeof(CPU)>cpu{};explicit Preserve(Game&v)noexcept:g(v){std::memcpy(cpu.data(),static_cast<CPU*>(&g),sizeof(CPU));}~Preserve()noexcept{if(std::memcmp(cpu.data(),static_cast<CPU*>(&g),sizeof(CPU))){std::memcpy(static_cast<CPU*>(&g),cpu.data(),sizeof(CPU));if(auto*s=lexical(g))s->fail(Failure::read);}errno=saved;}};
template<class F>inline void safe(Game&g,F f)noexcept{Preserve p(g);try{f();}catch(...){if(auto*s=lexical(g))s->fail(Failure::read);}}
inline void trace(State&s,const char*kind,uint64_t request=0,unsigned phase=0)noexcept{int saved=errno;const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(p&&!std::strcmp(p,"1")&&s.traceLines<24){++s.traceLines;std::fprintf(stderr,"LOADSAVE SCENE %s epoch=%llu revision=%llu slot=%u request=%llu phase=%u captionCommands=%zu captionStores=%zu\n",kind,(unsigned long long)s.epoch,(unsigned long long)s.revision,s.slotsAfter.selected,(unsigned long long)request,phase,s.caption.packet?s.caption.packet->commands():0,s.caption.packet?s.caption.packet->writes():0);}errno=saved;}
inline Directory directory(){
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:2
auto diag=storage();if(diag&&diag->depth==1)diag->initStage=401;
// NFS_LS_INIT_DIAGNOSTIC_END:2
Directory d;{nfs::GuestCopyTransaction tx;d.address=tx.load<uint32_t>(dword_4D40B8);
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:3
if(diag&&diag->depth==1)diag->initStage=402;
// NFS_LS_INIT_DIAGNOSTIC_END:3
if(!read::span(d.address,16))throw 1;for(unsigned k=0;k<4;++k)d.header[k]=tx.load<uint32_t>(d.address+4*k);}
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:4
if(diag&&diag->depth==1){diag->initStage=403;diag->initHeader=d.header;}
// NFS_LS_INIT_DIAGNOSTIC_END:4
if(d.header[2]>4096||!read::readBytes(d.address+16,size_t(d.header[2])*8,d.bytes)||!d.valid())throw 1;
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:5
if(diag&&diag->depth==1)diag->initStage=404;
// NFS_LS_INIT_DIAGNOSTIC_END:5
return d;}
inline bool bound(State&s,Game&g)noexcept{if(current(g)!=&s||s.invalid)return false;try{return s.directory==directory();}catch(...){return false;}}
inline bool targetNow(Target&t,uint32_t at=uint32_t(guest_address(dword_4DB240)))noexcept{try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t,at);return bg::fullClip(t,343);}catch(...){return false;}}
inline bool readPlane(Plane&p)noexcept{p.valid=0;try{Target t,after;nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t);if(!bg::fullClip(t,343))return false;std::memcpy(p.pixels.data(),nfs_host_address(t.w[8]+16,Side*2),Side*2);nfs_logo_publisher::target(tx,after);if(!nfs_logo_capture::same(t,after))return false;p.target=t;p.valid=1;p.width=343;p.height=480;return true;}catch(...){return false;}}
inline bool table(std::array<uint32_t,9>&v,Phase phase,bool full=false)noexcept{return read::readTable(uint32_t(guest_address(phase==Phase::side?off_4DADB4:full?off_4DAD88:off_4DAD64)),v);}
inline bool bank(std::array<uint32_t,256>&v,uint32_t at,bool narrow)noexcept{if(!read::span(at,narrow?512:1024))return false;try{nfs::GuestCopyTransaction tx;for(unsigned k=0;k<256;++k)v[k]=narrow?tx.load<uint16_t>(at+2*k):tx.load<uint32_t>(at+4*k);return true;}catch(...){return false;}}
inline Slots slots(State&s){Slots v;{nfs::GuestCopyTransaction tx;int selected=int32_t(tx.load<uint32_t>(dword_4D2314+2))>>16;if(selected<0||selected>1)throw 1;v.selected=unsigned(selected);for(unsigned k=0;k<2;++k){uint32_t at=tx.load<uint32_t>(uint32_t(guest_address(dword_4D231C))+4*k);nfs_logo_publisher::target(tx,v.target[k],at);
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:14
s.initStage=71;s.initIndex=k;
// NFS_LS_INIT_DIAGNOSTIC_END:14
v.lifetime[k]=s.epoch*2+k;}}
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:15
s.initStage=72;
// NFS_LS_INIT_DIAGNOSTIC_END:15
if(!v.valid())throw 1;return v;}
inline void authenticate(State&s){
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:6
s.initStage=30;
// NFS_LS_INIT_DIAGNOSTIC_END:6

 s.directory=directory();
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:7
s.initStage=31;
// NFS_LS_INIT_DIAGNOSTIC_END:7
s.owner.archive=s.directory.address;s.catalog=nfs_background_scene_runtime::acquireCatalog();
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:8
s.initStage=32;
// NFS_LS_INIT_DIAGNOSTIC_END:8
if(!s.catalog)throw 1;
 std::array<uint32_t,13>names{};std::array<uint32_t,5>logoNames{};
 for(unsigned k=0;k<s.directory.header[2];++k){
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:9
s.initStage=40;s.initIndex=k;s.initRole=UINT32_MAX;
// NFS_LS_INIT_DIAGNOSTIC_END:9
const auto*p=s.directory.bytes.data()+8*k;uint32_t name=bg::le32(p),offset=bg::le32(p+4);std::vector<uint8_t>head;if(!read::readBytes(s.directory.address+offset,16,head))throw 1;
  for(unsigned role=0;role<13;++role){const auto&id=s.catalog->identities[role];
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:10
s.initStage=41;s.initRole=role;
// NFS_LS_INIT_DIAGNOSTIC_END:10
if(std::memcmp(head.data(),id.header.data(),16))continue;std::vector<uint8_t>bytes;if(!s.directory.resolves(name,s.directory.address+offset,id.span)||!read::readBytes(s.directory.address+offset,id.span,bytes))continue;if(!logo::hashMatches(bytes.data(),id.span,id.sha))continue;if(names[role])throw 1;names[role]=name;}
  for(unsigned role=0;role<5;++role){const auto&id=s.catalog->logos.identities[role];
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:11
s.initStage=42;s.initRole=role;
// NFS_LS_INIT_DIAGNOSTIC_END:11
bool match=true;for(unsigned j=0;j<4;++j)match&=bg::le32(head.data()+j*4)==id.header[j];if(!match)continue;std::vector<uint8_t>bytes;if(!s.directory.resolves(name,s.directory.address+offset,id.span)||!read::readBytes(s.directory.address+offset,id.span,bytes))continue;if(!logo::hashMatches(bytes.data(),id.span,id.sha))continue;if(logoNames[role])throw 1;logoNames[role]=name;}
 }
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:21
s.initWordMask=s.initLogoMask=0;for(unsigned k=0;k<13;++k)if(names[k])s.initWordMask|=1u<<k;for(unsigned k=0;k<5;++k)if(logoNames[k])s.initLogoMask|=1u<<k;
// NFS_LS_INIT_DIAGNOSTIC_END:21
s.trusted=Trusted::create(*s.catalog,names,logoNames);
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:12
s.initStage=50;
// NFS_LS_INIT_DIAGNOSTIC_END:12
if(!s.trusted||!(directory()==s.directory))throw 1;
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:13
s.initStage=51;
// NFS_LS_INIT_DIAGNOSTIC_END:13

}
struct OwnerScope {Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bool entered=false,claimed=false;int unwind=0;
 explicit OwnerScope(Game&v)noexcept:g(v){if(!enabled()||uint32_t(g.eax)!=uint32_t(guest_address(off_4D39A8)))return;safe(g,[&]{uintptr_t expected=0;if(!ownerThread().compare_exchange_strong(expected,token())&&expected!=token())return;claimed=expected==0;if(storage()&&storage()->depth){s=storage().get();pin=storage();++s->depth;entered=true;s->fail(Failure::nested);return;}storage()=newState();s=storage().get();pin=storage();s->depth=1;s->game=reinterpret_cast<uintptr_t>(&g);s->thread=token();entered=true;unwind=nfs_logo_publisher::exceptions();auto value=epochs().load();if(value>=UINT64_MAX/2-1)throw 1;s->epoch=value+1;epochs().store(s->epoch);{nfs::GuestCopyTransaction tx;if(tx.load<uint16_t>(uint32_t(guest_address(off_4D39A8))+8)!=9)throw 1;
// NFS_LS_INIT_DIAGNOSTIC_BEGIN:16
s->initStage=2;
// NFS_LS_INIT_DIAGNOSTIC_END:16
}s->owner={Form,Handler,9,0,s->game,s->thread,s->epoch};s->ready=false;});if(claimed&&!entered){auto expected=token();ownerThread().compare_exchange_strong(expected,0,std::memory_order_release,std::memory_order_relaxed);}}
 ~OwnerScope(){if(!s||!pin||storage().get()!=s)return;if(!entered||!s)return;safe(g,[&]{if(nfs_logo_publisher::exceptions()>unwind)s->fail(Failure::unwind);if(s->depth&&!--s->depth){s->fail(Failure::owner);s->game=0;auto expected=s->thread;ownerThread().compare_exchange_strong(expected,0,std::memory_order_release,std::memory_order_relaxed);}});}
};
// ONLY after the actual40938C->410E80 normal return: the original call may
// free/reload4D40B8. Its preceding archive is not this source generation.
inline void afterArchiveReady(Game&g)noexcept{
 auto*s=lexical(g);if(!s||s->invalid)return;safe(g,[&]{
  s->ready=false;
  if(uint32_t(g.ebp)!=uint32_t(guest_address(off_4D39A8))||s->permitDepth||s->invocations||s->iteration||s->frameOpen||s->retiring||s->refOpen)throw 1;
  s->ready=false;s->collector.cancel();s->published.reset();s->issuedScene.reset();s->captionBuilder.reset();s->caption={};s->captionProfile={};s->fontAddress=s->glyphAddress=0;s->revision=s->sequence=0;s->phase=Phase::idle;s->events={};s->stringOpen=s->glyphOpen=s->frameOpen=s->frameResolved=s->resolved=s->groupDone=false;s->pending=-1;s->fullPre.clear();s->postReset.clear();s->clean.clear();s->rawBefore.clear();s->rawAfter.clear();s->scene.draws.clear();s->scene.complete=s->scene.error=0;
  auto value=epochs().load();if(value>=UINT64_MAX/2-1)throw 1;s->epoch=value+1;epochs().store(s->epoch);s->owner={Form,Handler,9,0,s->game,s->thread,s->epoch};
  authenticate(*s);s->slotsBefore=slots(*s);if(!s->collector.enter(s->owner))throw 1;s->ready=true;
 });
}
// Only inserted lexically around the40938C->40BBE0 call. It is NOT a generic
// caller boolean and cannot be obtained at any other original callsite.
struct CachePermit {Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bool entered=false;explicit CachePermit(Game&v)noexcept:g(v){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid)return;safe(g,[&]{if(s->permitDepth||s->invocations||uint32_t(g.eax)!=uint32_t(guest_address(off_4D39A8)))throw 1;++s->permitDepth;entered=true;});}~CachePermit(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!entered)return;safe(g,[&]{if(s->permitDepth)--s->permitDepth;else s->fail(Failure::order);});}};
struct IterationScope {Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bool entered=false,started=false;int unwind=0;explicit IterationScope(Game&v)noexcept:g(v){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid||s->permitDepth!=1)return;safe(g,[&]{if(s->invocations++||uint32_t(g.eax)!=uint32_t(guest_address(off_4D39A8)))throw 1;entered=true;unwind=nfs_logo_publisher::exceptions();});}
 void start()noexcept{if(!entered||started||!pin||storage().get()!=s||s!=current(g)||s->epoch!=sourceEpoch)return;safe(g,[&]{started=true;if(s->iteration||!bound(*s,g)||s->revision==UINT64_MAX)throw 1;s->published.reset();s->issuedScene.reset();s->scene.complete=s->scene.error=0;s->scene.draws.clear();s->scene.preSide.valid=s->scene.postSide.valid=s->scene.postWords.valid=0;s->group.serial=s->group.thread=0;s->group.kind=s->group.complete=s->group.error=s->group.stage=s->group.draws=0;s->group.pre.valid=s->group.post.valid=0;s->caption={};s->events={};s->phase=Phase::idle;s->pending=-1;s->resolved=s->groupDone=s->stringOpen=s->glyphOpen=false;s->iteration=true;s->slotsBefore=slots(*s);++s->revision;s->scene.form=Form;});}
 ~IterationScope(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!entered)return;safe(g,[&]{if(nfs_logo_publisher::exceptions()>unwind)s->fail(Failure::unwind);if(started&&!s->invalid){s->slotsAfter=slots(*s);if(s->phase!=Phase::complete||!s->slotsBefore.sameCatalog(s->slotsAfter)||s->slotsAfter.selected!=1-s->slotsBefore.selected)throw 1;CacheInput i{s->owner,s->slotsBefore,s->slotsAfter,s->directory,directory(),s->revision,s->events,CacheSite,CacheStart,s->scene,s->group,s->caption,s->rawBefore,s->rawAfter};if(!s->collector.publish(i,s->trusted,s->captionProfile))throw 1;s->published=s->collector.current(s->owner,s->slotsAfter,s->directory,s->revision,s->rawAfter.data(),s->rawAfter.size());if(!s->published)throw 1;trace(*s,"cache-accepted");}s->iteration=false;if(s->invocations)--s->invocations;});}
};
struct RoleScope {Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;Phase phase;bool active=false;int unwind=0;RoleScope(Game&v,Phase p)noexcept:g(v),phase(p){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid||!s->iteration)return;safe(g,[&]{if(s->phase!=Phase::idle||s->resolved||s->groupDone||(p==Phase::side&&!s->scene.draws.empty())||(p==Phase::word&&s->scene.draws.empty()))throw 1;s->phase=p;active=true;unwind=nfs_logo_publisher::exceptions();if(p==Phase::side){if(!s->event(s->events[0])||!readPlane(s->scene.preSide))throw 1;s->target=s->scene.preSide.target;s->binding.sourceOwner=s->target.owner;s->binding.bitmap=s->target.w[8];if(!retained(s->target,s->slotsBefore.target[1-s->slotsBefore.selected]))throw 1;}});}
 ~RoleScope(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!active)return;safe(g,[&]{if(nfs_logo_publisher::exceptions()>unwind||s->resolved)throw 1;if(phase==Phase::side&&(!readPlane(s->scene.postSide)||!retained(s->target,s->scene.postSide.target)))throw 1;s->phase=Phase::idle;});}
};
inline void preGroup(Game&g)noexcept{auto*s=current(g);if(!s||s->invalid||!s->iteration)return;safe(g,[&]{if(s->phase!=Phase::idle||s->scene.draws.empty()||s->groupDone||!readPlane(s->group.pre)||!retained(s->target,s->group.pre.target)||!s->event(s->events[1]))throw 1;s->scene.postWords=s->group.pre;s->group.serial=s->events[1];s->group.thread=s->thread;s->group.kind=1;s->group.ownerArg=Form;s->group.stage=1;s->phase=Phase::logos;});}
inline void postGroup(Game&g)noexcept{auto*s=current(g);if(!s||s->invalid||!s->iteration)return;safe(g,[&]{if(s->phase!=Phase::logos||s->resolved||s->group.draws!=3||!readPlane(s->group.post)||!retained(s->target,s->group.post.target))throw 1;s->group.complete=1;s->group.stage=2;s->groupDone=true;s->phase=Phase::idle;});}
struct ResolvedInner {Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bg::Draw*d=nullptr;nfs_logo_capture::Draw*l=nullptr;Phase phase=Phase::idle;uint32_t source=0;bool active=false;int unwind=0;explicit ResolvedInner(Game&game)noexcept:g(game){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||!s->iteration||s->invalid||!(s->phase==Phase::side||s->phase==Phase::word||s->phase==Phase::logos))return;if(s->resolved||!bound(*s,g)){s->fail(Failure::nested);return;}s->resolved=true;active=true;phase=s->phase;source=uint32_t(g.eax);unwind=nfs_logo_publisher::exceptions();try{const std::array<uint32_t,4>args{{source,uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};Target before;if(!targetNow(before)||!nfs_logo_capture::same(before,s->target))throw 1;std::array<uint8_t,16>head{};{nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(source,16);std::memcpy(head.data(),nfs_host_address(source,16),16);}
 if(phase==Phase::logos){if(s->group.draws>=3)throw 1;l=&s->group.draw[s->group.draws++];l->role=UINT32_MAX;l->error=l->complete=l->rows=0;l->paletteValid=0;l->paletteFirst.fill(0);l->paletteLast.fill(0);l->args=args;l->resolves=1;l->before=before;for(unsigned k=0;k<5;++k){const auto&id=s->catalog->logos.identities[k];bool same=true;for(unsigned j=0;j<4;++j)if(sl::le32(head.data()+4*j)!=id.header[j])same=false;if(!same)continue;std::vector<uint8_t>copy;if(!read::readBytes(source,id.span,copy)||!sl::hashMatches(copy.data(),id.span,id.sha))continue;l->role=k;l->span=id.span;l->paletteCount=id.paletteCount;l->actualMaxIndex=id.maxIndex;l->sourceHeader=id.header;l->shaBefore=id.sha;std::copy(copy.begin(),copy.end(),l->sourceBefore.begin());break;}if(l->role>=5||!table(l->tableBefore,phase,(args[1]&~511u)==0x2800))throw 1;}
 else{if(s->scene.draws.size()>=5||(phase==Phase::side&&!s->scene.draws.empty())||(phase==Phase::word&&s->scene.draws.empty()))throw 1;s->scene.draws.emplace_back();d=&s->scene.draws.back();s->pending=int(s->scene.draws.size()-1);d->rows.reserve(480);d->args=args;d->before=before;if(!table(d->tableBefore,phase))throw 1;for(unsigned k=phase==Phase::side?0:1;k<(phase==Phase::side?1:13);++k){const auto&id=s->catalog->identities[k];if(head==id.header&&read::readBytes(source,id.span,d->sourceBefore)&&sl::hashMatches(d->sourceBefore.data(),id.span,id.sha)){d->role=k;break;}}if(d->role>12)throw 1;}
 }catch(...){s->fail(Failure::source);}}
 ~ResolvedInner(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!active)return;if(!s->invalid){try{if(nfs_logo_publisher::exceptions()>unwind||!bound(*s,g)||s->rowActive)throw 1;Target after;if(!targetNow(after)||!nfs_logo_capture::same(after,s->target))throw 1;if(l){l->after=after;if(!table(l->tableAfter,phase,(l->args[1]&~511u)==0x2800))throw 1;std::vector<uint8_t>copy;if(!read::readBytes(source,l->span,copy)||!sl::hashMatches(copy.data(),l->span,l->shaBefore))throw 1;std::copy(copy.begin(),copy.end(),l->sourceAfter.begin());l->shaAfter=l->shaBefore;l->complete=l->rows&&l->paletteValid?1:0;if(!l->complete)throw 1;}else if(d){d->after=after;if(!table(d->tableAfter,phase)||!read::readBytes(source,d->sourceBefore.size(),d->sourceAfter)||d->sourceBefore!=d->sourceAfter||d->rows.empty())throw 1;if(d->paletteValid){if(!bank(d->paletteAfter,d->paletteGuest,phase==Phase::side&&d->callback==0xf000221c)||d->paletteAfter!=d->paletteBefore)throw 1;}d->complete=1;}}
 catch(...){s->fail(Failure::source);}}s->resolved=false;s->pending=-1;s->capturedCopy=false;}};
inline void rowInner(Game&g,uint32_t callback,uint32_t unit=1)noexcept{auto*s=current(g);if(!s||!s->iteration||s->invalid||!s->resolved)return;const std::array<uint32_t,4>a{{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};if(s->copyDepth&&callback==uint32_t(game_id_sub_4A14D4))return;s->capturedCopy=false;try{Target now;if(!targetNow(now)||!nfs_logo_capture::same(now,s->target))throw 1;
 if(s->phase==Phase::logos){if(!s->group.draws||s->group.draws>3)throw 1;auto&d=s->group.draw[s->group.draws-1];const bool full=(d.args[1]&~511u)==0x2800;const uint32_t expected=full?uint32_t(game_id_sub_483288):uint32_t(game_id_sub_482BD4);if(callback!=expected||unit!=1||!a[2]||a[2]>343||d.rows>=480||d.role>=5||uint64_t(a[0])<uint64_t(d.args[0])+16||uint64_t(a[0])+a[2]>uint64_t(d.args[0])+16+uint64_t(d.sourceHeader[1]&65535)*(d.sourceHeader[1]>>16))throw 1;d.row[d.rows++]={callback,a};nfs::GuestCopyTransaction tx;uint32_t global=tx.load<uint32_t>(full?dword_4DADA0:dword_4DAD7C),format=tx.load<uint32_t>(full?dword_4DADA8:dword_4DAD84),pal=tx.load<uint32_t>(full?dword_5636EC:dword_5636E4);if(global!=callback||format!=0x7b||!read::span(pal,1024))throw 1;if(d.rows==1){d.callback=callback;d.paletteGuest=pal;d.callbackGlobal=global;d.callbackFormat=format;}else if(d.callback!=callback||d.paletteGuest!=pal||d.callbackGlobal!=global||d.callbackFormat!=format)throw 1;for(unsigned k=0;k<d.paletteCount;++k){uint32_t v=tx.load<uint32_t>(pal+4*k);if(d.rows==1)d.paletteFirst[k]=v;else if(d.paletteFirst[k]!=v)throw 1;d.paletteLast[k]=v;}if(d.paletteCount<256&&tx.load<uint32_t>(pal+255*4))throw 1;d.paletteValid=1;return;}
 auto*d=s->draw();if(!d)return;const uint64_t first=uint64_t(s->target.w[8])+16,bytes=unit==2?a[2]:uint64_t(a[2])*2;if(a[1]<first||uint64_t(a[1])+bytes>first+Side*2)return;const bool indexed=s->phase==Phase::side&&callback==0xf000221c,word=s->phase==Phase::word&&callback==uint32_t(game_id_sub_482BD4),copied=s->phase==Phase::side&&(callback==uint32_t(game_id_sub_483438)||callback==uint32_t(game_id_sub_4A14D4));if((!indexed&&!word&&!copied)||!a[2]||d->rows.size()>=480||(unit!=1&&unit!=2)||a[2]>329280)throw 1;const size_t n=indexed||word?a[2]:(unit==2?a[2]:size_t(a[2])*2);if(n>329280||!read::span(a[0],n))throw 1;d->rows.emplace_back();auto&r=d->rows.back();r.callback=callback;r.unit=unit;r.args=a;r.payload.resize(n);uint32_t pal=0,global=0,format=0;{nfs::GuestCopyTransaction tx;r.converter=tx.load<uint32_t>(dword_571264);nfs_logo_publisher::checked(a[0],n);std::memcpy(r.payload.data(),nfs_host_address(a[0],n),n);if(indexed){pal=uint32_t(guest_address(word_56EF3C));global=r.converter;format=tx.load<uint8_t>(byte_4DB25C);}else if(word){pal=tx.load<uint32_t>(dword_5636E4);global=tx.load<uint32_t>(dword_4DAD7C);format=tx.load<uint32_t>(dword_4DAD84);}}
 if(indexed||word){if(!pal)throw 1;if(d->rows.size()==1){d->callback=callback;d->callbackGlobal=global;d->format=format;d->paletteGuest=pal;if(!bank(d->paletteBefore,pal,indexed))throw 1;}else if(d->callback!=callback||d->callbackGlobal!=global||d->format!=format||d->paletteGuest!=pal)throw 1;if(!bank(d->paletteAfter,pal,indexed)||d->paletteBefore!=d->paletteAfter)throw 1;d->paletteValid=1;}if(callback==uint32_t(game_id_sub_483438)){s->capturedCopy=true;s->copyArgs=a;}
 }catch(...){s->fail(Failure::row);}}
struct CopyWrapperScope{std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bool active=false;explicit CopyWrapperScope(Game&g)noexcept{s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid||!s->capturedCopy)return;const std::array<uint32_t,4>a{{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};if(a!=s->copyArgs)return;s->capturedCopy=false;++s->copyDepth;active=true;}~CopyWrapperScope(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(active&&s->copyDepth)--s->copyDepth;}};
inline uint32_t indexedSideCalleeInner(Game&g)noexcept{auto*s=current(g);if(!s||s->invalid||!s->resolved||s->phase!=Phase::side)return 0;try{if(uint32_t(g.esp)>UINT32_MAX-0x3c)throw 1;nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(uint32_t(g.esp)+0x38);}catch(...){s->fail(Failure::read);return 0;}}
struct IndexedSideInner{std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;int unwind=0;IndexedSideInner(Game&g,uint32_t callee)noexcept{if(callee!=0xf000221c)return;s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid||!s->resolved||s->phase!=Phase::side){s=nullptr;return;}unwind=nfs_logo_publisher::exceptions();rowInner(g,callee);}~IndexedSideInner(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(!s||s->invalid)return;auto*d=s->draw();if(nfs_logo_publisher::exceptions()>unwind||!d||!bank(d->paletteAfter,d->paletteGuest,true)||d->paletteAfter!=d->paletteBefore)s->fail(Failure::palette);}};

inline uint32_t indexedSideCallee(Game&g)noexcept{auto*s=current(g);if(!s||s->invalid||!s->iteration||s->phase!=Phase::side)return 0;uint32_t out=0;safe(g,[&]{out=indexedSideCalleeInner(g);});return out;}
struct IndexedSideCallScope {Game&g;std::unique_ptr<IndexedSideInner>inner;IndexedSideCallScope(Game&v,uint32_t callee)noexcept:g(v){auto*s=current(g);if(callee!=0xf000221c||!s||s->invalid||!s->iteration)return;safe(g,[&]{inner.reset(new IndexedSideInner(g,callee));});}~IndexedSideCallScope(){if(!inner)return;safe(g,[&]{inner.reset();});}};
struct CacheResolved {Game&g;std::unique_ptr<ResolvedInner>inner;explicit CacheResolved(Game&v)noexcept:g(v){auto*s=current(g);if(!s||s->invalid||!s->iteration)return;safe(g,[&]{inner.reset(new ResolvedInner(g));});}~CacheResolved(){if(!inner)return;safe(g,[&]{inner.reset();});}};
inline void cacheRow(Game&g,uint32_t callback,uint32_t unit=1)noexcept{auto*s=current(g);if(!s||s->invalid||!s->iteration||!s->resolved)return;safe(g,[&]{rowInner(g,callback,unit);});}

inline nfs_soft::SurfaceIdentity surface(const Target&t,uint64_t life){nfs_soft::SurfaceIdentity v;v.lifetime=life;std::copy(t.w.begin(),t.w.end(),v.geometry.begin());auto&w=v.geometry;w[14]=t.x[0];w[15]=t.x[1];w[16]=t.x[342];w[17]=t.y[0];w[18]=t.y[1];w[19]=t.y[479];if(!v.valid())throw 1;return v;}
inline void originalFont(State&s,uint32_t address,uint32_t glyphTable,uint32_t count,uint32_t stride){
 auto&v=s.captionProfile.source;std::vector<uint8_t>font,records;
 if(count!=224||stride!=68||!read::readBytes(address,16+68*368,font)||!read::readBytes(glyphTable,224*11,records)||!nfs_ls_live::exactHash(font.data(),font.size(),nfs_ls_live::FontSHA)||!nfs_ls_live::exactHash(records.data(),records.size(),nfs_ls_live::GlyphSHA))throw 1;
 if(s.fontAddress&&(s.fontAddress!=address||s.glyphAddress!=glyphTable||v.fontBytes!=font||v.glyphRecords!=records))throw 1;
 s.fontAddress=address;s.glyphAddress=glyphTable;v.fontBytes=std::move(font);v.glyphRecords=std::move(records);v.atlasWidth=136;v.atlasHeight=368;v.atlasStride=68;v.identity=UINT64_C(0x4c5343415054494f);
 for(unsigned code=32;code<128;++code){if(code!=32&&!std::strchr("options",int(code)))continue;bool seen=false;for(unsigned k=0;k<count;++k){const auto*r=v.glyphRecords.data()+11*k;if((uint32_t(r[0])|uint32_t(r[1])<<8)!=code)continue;if(seen)throw 1;seen=true;auto&g=v.glyphs[code-32];g.code=uint16_t(code);g.width=r[2];g.height=r[3];g.sourceX=uint32_t(r[4])|uint32_t(r[5])<<8;g.sourceY=uint32_t(r[6])|uint32_t(r[7])<<8;g.advance=int(int8_t(r[8]));g.bearingX=int(int8_t(r[9]));g.bearingY=int(int8_t(r[10]));g.recordOffset=11*k;}if(!seen)throw 1;}
 if(!nfs_texture_sha256(v.fontBytes.data(),v.fontBytes.size(),s.captionProfile.fontSha.data()))throw 1;
}
struct FontState {Target target;uint32_t font=0,palette=0,callback=0,table=0,count=0,stride=0;std::array<uint32_t,16>colors{};};
inline FontState fontState(State&s){FontState v;uint32_t color=0;{
 nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,v.target);if(!retained(s.target,v.target))throw 1;v.font=tx.load<uint32_t>(dword_4DD770);v.palette=tx.load<uint32_t>(dword_4DD774);v.callback=tx.load<uint32_t>(dword_4DD77C);v.table=tx.load<uint32_t>(dword_4DD76C);v.count=tx.load<uint32_t>(dword_4DD75C);v.stride=tx.load<uint32_t>(dword_4DD760);color=tx.load<uint32_t>(dword_4DD714);
 if(v.callback!=uint32_t(game_id_sub_4A5830)||tx.load<uint32_t>(dword_4D20A0)||tx.load<uint8_t>(byte_4DB25C)!=16||tx.load<uint8_t>(byte_4DB25E)||tx.load<int32_t>(dword_4DD6F4)||tx.load<int32_t>(dword_4DD6F8)||tx.load<int32_t>(dword_4E54AC)!=18||tx.load<uint32_t>(dword_4DD790)||color!=65535||!read::span(v.palette,64))throw 1;
 for(unsigned k=0;k<16;++k)v.colors[k]=tx.load<uint32_t>(v.palette+4*k);
 } // End real nonrecursive transaction BEFORE originalFont/readBytes helpers.
 originalFont(s,v.font,v.table,v.count,v.stride);
 // The already captured selected foreground palette is exactly white. This
 // independently checks source bytes, not a caller-selected semantic role.
 if(!nfs_ls_live::knownPalette(3,v.colors.data(),64))throw 1;
 auto&cp=s.captionProfile;cp.glyphCallback=v.callback;cp.source.paletteObserved[1]=true;cp.source.palettes[1]=v.colors;if(!cp.source.valid())throw 1;return v;
}
inline bool sameFont(const FontState&a,const FontState&b)noexcept{return retained(a.target,b.target)&&a.font==b.font&&a.palette==b.palette&&a.callback==b.callback&&a.table==b.table&&a.count==b.count&&a.stride==b.stride&&a.colors==b.colors;}
struct CaptionTail {
 Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bool entered=false;int unwind=0;
 explicit CaptionTail(Game&v)noexcept:g(v){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid||!s->iteration)return;safe(g,[&]{if(s->phase!=Phase::idle||!s->groupDone||s->resolved||uint32_t(g.ebp)!=uint32_t(guest_address(off_4D39A8)))throw 1;s->phase=Phase::tail;entered=true;unwind=nfs_logo_publisher::exceptions();s->caption={};auto&c=s->caption;c.site=CaptionSite;c.form=c.incomingEbp=Form;c.archive=s->owner.archive;c.serial=s->revision;c.lifetime=s->slotsBefore.lifetime[1-s->slotsBefore.selected];s->measured=false;s->measuredText=0;s->glyphIndex=s->writes=0;s->stringOpen=s->glyphOpen=false;
 {nfs::GuestCopyTransaction tx;int selector=int32_t(tx.load<uint32_t>(byte_4D3ECC))>>16;if(selector!=6)throw 1;c.selector=unsigned(selector);c.entry=tx.load<uint32_t>(uint32_t(guest_address(off_4D3ED0))+4*c.selector);if(!read::span(c.entry,4))throw 1;c.entryWord=tx.load<uint32_t>(c.entry);uint32_t table=tx.load<uint32_t>(dword_4D4AE0),resource=c.entryWord>>16;if(!read::span(table,12)||uint64_t(table)+12*uint64_t(resource)+12>UINT64_C(0x100000000))throw 1;uint32_t at=table+12*resource,offset=tx.load<uint32_t>(at+8);if(uint64_t(table)+offset+8>UINT64_C(0x100000000))throw 1;c.textGuest=table+offset;for(unsigned k=0;k<8;++k)if(tx.load<uint8_t>(c.textGuest+k)!=uint8_t("options"[k]))throw 1;}
 auto font=nfs_soft::acquireFontCatalog();if(!font)throw 1;s->captionProfile.source.displayFont=font;s->captionBuilder.reset(new nfs_soft::FramePacketBuilder(font));if(!s->captionBuilder->begin(surface(s->target,c.lifetime),c.serial,s->group.post.pixels.data(),Side)||!s->event(s->events[2]))throw 1;
 });}
 ~CaptionTail(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!entered)return;safe(g,[&]{if(s->invalid)return;if(nfs_logo_publisher::exceptions()>unwind||s->stringOpen||s->glyphOpen||s->glyphIndex!=7||!s->captionBuilder||!bound(*s,g))throw 1;Plane after;if(!readPlane(after)||!retained(s->target,after.target))throw 1;s->caption.packet=s->captionBuilder->finish(surface(s->target,s->caption.lifetime),after.pixels.data(),Side);if(!s->caption.packet||!read::readBytes(s->target.w[8],RawSide,s->rawBefore)||!read::readBytes(s->target.w[8],RawSide,s->rawAfter)||s->rawBefore!=s->rawAfter||!s->event(s->events[3]))throw 1;s->scene.complete=1;s->phase=Phase::complete;if(!s->event(s->events[4]))throw 1;});}
};
// Only the original405580 right-alignment measurement return calls this.
inline void measured(Game&g,uint32_t text,int width,unsigned alignment)noexcept{auto*s=current(g);if(!s||s->invalid||s->phase!=Phase::tail)return;safe(g,[&]{if(s->measured||s->stringOpen||s->glyphOpen||alignment!=2||text!=s->caption.textGuest||width!=48||int32_t(g.esi)!=168)throw 1;s->measured=true;s->measuredText=text;s->caption.measured=width;s->caption.anchor=168;});}
struct CaptionString {
 Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bool entered=false;FontState before;nfs_soft::TextCommand command{};int unwind=0;
 explicit CaptionString(Game&v)noexcept:g(v){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid||s->phase!=Phase::tail)return;safe(g,[&]{if(!s->measured||s->measuredText!=uint32_t(g.eax)||s->caption.textGuest!=uint32_t(g.eax)||s->stringOpen||s->glyphOpen||s->glyphIndex||g.edx!=120||g.ebx!=364)throw 1;before=fontState(*s);auto&c=s->caption;c.font=before.font;c.palette=before.palette;c.callback=before.callback;c.format=16;c.originalFontSha=s->captionProfile.fontSha;c.paletteWords=before.colors;command.length=7;std::memcpy(command.text.data(),"options",7);command.role=1;command.alignment=2;command.sourceSize=18;command.x=120;command.y=364;command.advance=48;command.color=65535;command.clip={0,0,343,480};if(!s->captionBuilder->beginString(command))throw 1;s->stringOpen=true;entered=true;unwind=nfs_logo_publisher::exceptions();});}
 ~CaptionString(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!entered)return;safe(g,[&]{if(s->invalid)return;if(nfs_logo_publisher::exceptions()>unwind||s->glyphOpen||s->glyphIndex!=7||!sameFont(before,fontState(*s)))throw 1;{nfs::GuestCopyTransaction tx;if(tx.load<int32_t>(dword_4DD6EC)!=168)throw 1;}if(!s->captionBuilder->endString(command))throw 1;s->stringOpen=false;});}
};
struct CaptionGlyph {
 Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bool entered=false;int unwind=0;
 explicit CaptionGlyph(Game&v)noexcept:g(v){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid||s->phase!=Phase::tail||!s->stringOpen)return;safe(g,[&]{if(s->glyphOpen||s->glyphIndex>=7||uint32_t(g.ebx)!=s->fontAddress)throw 1;s->glyph={};auto&u=s->glyph;u.code=uint8_t("options"[s->glyphIndex]);u.textIndex=s->glyphIndex;u.x=int32_t(g.eax);u.y=int32_t(g.edx);u.sourceX=uint32_t(g.ecx);{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);if(!read::span(sp,20))throw 1;u.sourceY=tx.load<uint32_t>(sp+4);u.width=tx.load<uint32_t>(sp+8);u.height=tx.load<uint32_t>(sp+12);u.stride=tx.load<uint32_t>(sp+16);}s->glyphOpen=true;entered=true;unwind=nfs_logo_publisher::exceptions();});}
 ~CaptionGlyph(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!entered)return;safe(g,[&]{if(s->invalid)return;if(nfs_logo_publisher::exceptions()>unwind||!s->glyphOpen)throw 1;s->caption.glyphs.push_back(std::move(s->glyph));++s->glyphIndex;s->glyphOpen=false;});}
};
struct CaptionWord {
 Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bool entered=false;uint32_t address=0;nfs_loadsave_source::Store store;
 CaptionWord(Game&v,uint32_t at,uint16_t computed,unsigned path)noexcept:g(v){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid||s->phase!=Phase::tail||!s->glyphOpen)return;safe(g,[&]{if(path>1||s->writes>=262144)throw 1;uint64_t begin=uint64_t(s->target.w[8])+16;if(at<begin||uint64_t(at)+2>begin+Side*2||(at-begin)%2)throw 1;uint32_t source=uint32_t(g.edi);if(source<s->fontAddress||uint64_t(source)>=uint64_t(s->fontAddress)+s->captionProfile.source.fontBytes.size()||uint32_t(g.ebp)!=s->caption.palette)throw 1;address=at;store.index=uint32_t((at-begin)/2);store.computed=computed;store.sourceOffset=source-s->fontAddress;store.sourceParity=uint8_t(path);{nfs::GuestCopyTransaction tx;store.before=tx.load<uint16_t>(at);store.sourceByte=tx.load<uint8_t>(source);unsigned nib=path?store.sourceByte&15:store.sourceByte>>4;store.paletteValue=tx.load<uint32_t>(s->caption.palette+4*nib);}entered=true;});}
 ~CaptionWord(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!entered)return;safe(g,[&]{if(s->invalid)return;{nfs::GuestCopyTransaction tx;store.after=tx.load<uint16_t>(address);}if(store.after!=store.computed||!s->captionBuilder->word(store.index,store.before,store.computed,store.after))throw 1;s->glyph.stores.push_back(store);++s->writes;});}
};

inline Target fullTarget(){Target t;nfs::GuestCopyTransaction tx;const uint32_t at=tx.load<uint32_t>(dword_4D4D24);nfs_logo_publisher::target(tx,t,at);if(!bg::fullClip(t,640))throw 1;return t;}
inline std::vector<uint16_t>fullPlane(const Target&t){std::vector<uint8_t>b;if(!read::readBytes(t.w[8]+16,bg::Full*2,b))throw 1;std::vector<uint16_t>v(bg::Full);std::memcpy(v.data(),b.data(),b.size());return v;}
inline nfs_ls_retire::Resources resources(State&s){nfs_ls_retire::Resources r;auto side=slots(s);auto full=fullTarget();r.owners={{full.owner,side.target[0].owner,side.target[1].owner}};r.bitmaps={{full.w[8],side.target[0].w[8],side.target[1].w[8]}};r.luts={{full.w[11],full.w[12],side.target[0].w[11],side.target[0].w[12],side.target[1].w[11],side.target[1].w[12]}};r.font=s.fontAddress;if(!r.font){nfs::GuestCopyTransaction tx;r.font=tx.load<uint32_t>(dword_4DD770);}return r;}
inline bool protectedAddress(const nfs_ls_retire::Resources&r,uint32_t at)noexcept{if(at==r.font)return true;for(auto x:r.owners)if(x==at)return true;for(auto x:r.bitmaps)if(x==at)return true;for(auto x:r.luts)if(x==at)return true;return false;}
struct RetireScope {
 Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bool active=false;int unwind=0;
 RetireScope(Game&v,uint32_t site)noexcept:g(v){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid)return;safe(g,[&]{if(s->retiring||s->refOpen)throw 1;nfs_ls_retire::Input input;input.site=site;input.handle=uint32_t(g.eax);input.newPitch=uint32_t(g.edx);if(!read::span(input.handle,56))throw 1;{nfs::GuestCopyTransaction tx;for(unsigned k=0;k<14;++k)input.descriptor[k]=tx.load<uint32_t>(input.handle+4*k);for(unsigned k=0;k<2;++k){auto ptr=input.descriptor[11+k];if(ptr<4)throw 1;input.refs[k]=tx.load<uint32_t>(ptr-4);}}
 auto r=resources(*s);bool related=protectedAddress(r,input.handle)||protectedAddress(r,input.descriptor[8]);for(auto ptr:r.luts)for(unsigned k=11;k<13;++k)related|=ptr&&ptr==input.descriptor[k];if(!related)return;
 s->retirePlan=nfs_ls_retire::plan(input,r);if(!s->retirePlan.valid)throw 1;s->retiredSlots=slots(*s);s->retiredFull=fullTarget();s->released=0;s->retiring=true;active=true;unwind=nfs_logo_publisher::exceptions();});}
 ~RetireScope(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!active)return;safe(g,[&]{if(s->invalid)return;if(nfs_logo_publisher::exceptions()>unwind||s->refOpen||s->released!=s->retirePlan.count||!s->retiredSlots.sameCatalog(slots(*s))||!retained(s->retiredFull,fullTarget()))throw 1;s->retiring=false;});}
};
struct RefReleaseScope {
 Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bool active=false;uint32_t ptr=0,before=0;
 explicit RefReleaseScope(Game&v)noexcept:g(v){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid)return;safe(g,[&]{ptr=uint32_t(g.eax);auto r=resources(*s);if(!s->retiring){if(protectedAddress(r,ptr))throw 1;return;}if(s->refOpen||s->released>=s->retirePlan.count||ptr<4)throw 1;{nfs::GuestCopyTransaction tx;before=tx.load<uint32_t>(ptr-4);}if(ptr!=s->retirePlan.ordered[s->released]||before!=s->retirePlan.before[s->released])throw 1;s->refOpen=true;active=true;});}
 ~RefReleaseScope(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!active)return;safe(g,[&]{if(s->invalid)return;uint32_t after;{nfs::GuestCopyTransaction tx;after=tx.load<uint32_t>(ptr-4);}if(!nfs_ls_retire::released(s->retirePlan,s->released,ptr,before,after))throw 1;++s->released;s->refOpen=false;});}
};
// Exact40938C reset/copy source calls only. The named reset and selected
// sidebar must be proven before any resulting source scene becomes available.
struct FrameScope {
 Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;bool entered=false,side=false;
 FrameScope(Game&v,bool sidebar)noexcept:g(v),side(sidebar){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid||s->iteration)return;safe(g,[&]{if(/* NFS_ASSEMBLY_DIAG_BEGIN:107 */nfs_ls_scene::assemblyGate(3001,/* NFS_ASSEMBLY_DIAG_END:107 */s->frameOpen/* NFS_ASSEMBLY_DIAG_BEGIN:108 */)/* NFS_ASSEMBLY_DIAG_END:108 */||/* NFS_ASSEMBLY_DIAG_BEGIN:109 */nfs_ls_scene::assemblyGate(3002,/* NFS_ASSEMBLY_DIAG_END:109 */s->retiring/* NFS_ASSEMBLY_DIAG_BEGIN:110 */)/* NFS_ASSEMBLY_DIAG_END:110 */||/* NFS_ASSEMBLY_DIAG_BEGIN:111 */nfs_ls_scene::assemblyGate(3003,/* NFS_ASSEMBLY_DIAG_END:111 */s->refOpen/* NFS_ASSEMBLY_DIAG_BEGIN:112 */)/* NFS_ASSEMBLY_DIAG_END:112 */||/* NFS_ASSEMBLY_DIAG_BEGIN:113 */nfs_ls_scene::assemblyGate(3004,/* NFS_ASSEMBLY_DIAG_END:113 */!bound(*s,g)/* NFS_ASSEMBLY_DIAG_BEGIN:114 */)/* NFS_ASSEMBLY_DIAG_END:114 */)throw 1;/* NFS_ASSEMBLY_DIAG_BEGIN:203 */nfs_ls_scene::assemblyReset(side?31000u:30000u);/* NFS_ASSEMBLY_DIAG_END:203 */s->frameOpen=true;s->frameSide=side;s->frameResolved=false;entered=true;
 if(!side){s->issuedScene.reset();s->resetDraw=FullDraw{};s->copyDraw=FullDraw{};/* NFS_ASSEMBLY_DIAG_BEGIN:204 */nfs_ls_scene::assemblyCheckpoint(30100);/* NFS_ASSEMBLY_DIAG_END:204 */s->fullTarget=fullTarget();s->fullPre=fullPlane(s->fullTarget);s->mapRole=2;uint32_t word=0,nameAt=0;{nfs::GuestCopyTransaction tx;nameAt=tx.load<uint32_t>(uint32_t(guest_address(off_4D39A8))+4);if(/* NFS_ASSEMBLY_DIAG_BEGIN:115 */nfs_ls_scene::assemblyGate(3005,/* NFS_ASSEMBLY_DIAG_END:115 */uint32_t(g.eax)!=nameAt/* NFS_ASSEMBLY_DIAG_BEGIN:116 */)/* NFS_ASSEMBLY_DIAG_END:116 */||/* NFS_ASSEMBLY_DIAG_BEGIN:117 */nfs_ls_scene::assemblyGate(3006,/* NFS_ASSEMBLY_DIAG_END:117 */g.edx!=0x400/* NFS_ASSEMBLY_DIAG_BEGIN:118 */)/* NFS_ASSEMBLY_DIAG_END:118 */||/* NFS_ASSEMBLY_DIAG_BEGIN:119 */nfs_ls_scene::assemblyGate(3007,/* NFS_ASSEMBLY_DIAG_END:119 */g.ebx/* NFS_ASSEMBLY_DIAG_BEGIN:120 */)/* NFS_ASSEMBLY_DIAG_END:120 */||/* NFS_ASSEMBLY_DIAG_BEGIN:121 */nfs_ls_scene::assemblyGate(3008,/* NFS_ASSEMBLY_DIAG_END:121 */g.ecx/* NFS_ASSEMBLY_DIAG_BEGIN:122 */)/* NFS_ASSEMBLY_DIAG_END:122 */||/* NFS_ASSEMBLY_DIAG_BEGIN:123 */nfs_ls_scene::assemblyGate(3009,/* NFS_ASSEMBLY_DIAG_END:123 */!read::span(nameAt,4)/* NFS_ASSEMBLY_DIAG_BEGIN:124 */)/* NFS_ASSEMBLY_DIAG_END:124 */)throw 1;word=tx.load<uint32_t>(nameAt);}s->mapName=word;
 }else{if(/* NFS_ASSEMBLY_DIAG_BEGIN:125 */nfs_ls_scene::assemblyGate(3010,/* NFS_ASSEMBLY_DIAG_END:125 */s->postReset.size()!=bg::Full/* NFS_ASSEMBLY_DIAG_BEGIN:126 */)/* NFS_ASSEMBLY_DIAG_END:126 */||/* NFS_ASSEMBLY_DIAG_BEGIN:127 */nfs_ls_scene::assemblyGate(3011,/* NFS_ASSEMBLY_DIAG_END:127 */!retained(s->fullTarget,fullTarget())/* NFS_ASSEMBLY_DIAG_BEGIN:128 */)/* NFS_ASSEMBLY_DIAG_END:128 */)throw 1;/* NFS_ASSEMBLY_DIAG_BEGIN:205 */nfs_ls_scene::assemblyCheckpoint(30200);/* NFS_ASSEMBLY_DIAG_END:205 */auto now=slots(*s);if(/* NFS_ASSEMBLY_DIAG_BEGIN:129 */nfs_ls_scene::assemblyGate(3012,/* NFS_ASSEMBLY_DIAG_END:129 */!s->slotsBefore.sameCatalog(now)/* NFS_ASSEMBLY_DIAG_BEGIN:130 */)/* NFS_ASSEMBLY_DIAG_END:130 */)throw 1;if(!s->collector.has(now.selected)){s->published.reset();s->issuedScene.reset();s->frameOpen=false;entered=false;return;}std::vector<uint8_t>raw;if(/* NFS_ASSEMBLY_DIAG_BEGIN:131 */nfs_ls_scene::assemblyGate(3013,/* NFS_ASSEMBLY_DIAG_END:131 */!read::readBytes(uint32_t(g.eax),RawSide,raw)/* NFS_ASSEMBLY_DIAG_BEGIN:132 */)/* NFS_ASSEMBLY_DIAG_END:132 */)throw 1;/* NFS_ASSEMBLY_DIAG_BEGIN:206 */nfs_ls_scene::assemblyCheckpoint(30300);/* NFS_ASSEMBLY_DIAG_END:206 */s->published=s->collector.current(s->owner,now,s->directory,s->revision,raw.data(),raw.size());if(/* NFS_ASSEMBLY_DIAG_BEGIN:133 */nfs_ls_scene::assemblyGate(3014,/* NFS_ASSEMBLY_DIAG_END:133 */!s->published/* NFS_ASSEMBLY_DIAG_BEGIN:134 */)/* NFS_ASSEMBLY_DIAG_END:134 */||/* NFS_ASSEMBLY_DIAG_BEGIN:135 */nfs_ls_scene::assemblyGate(3015,/* NFS_ASSEMBLY_DIAG_END:135 */uint32_t(g.eax)!=now.target[now.selected].w[8]/* NFS_ASSEMBLY_DIAG_BEGIN:136 */)/* NFS_ASSEMBLY_DIAG_END:136 */||/* NFS_ASSEMBLY_DIAG_BEGIN:137 */nfs_ls_scene::assemblyGate(3016,/* NFS_ASSEMBLY_DIAG_END:137 */g.edx!=0x1000/* NFS_ASSEMBLY_DIAG_BEGIN:138 */)/* NFS_ASSEMBLY_DIAG_END:138 */||/* NFS_ASSEMBLY_DIAG_BEGIN:139 */nfs_ls_scene::assemblyGate(3017,/* NFS_ASSEMBLY_DIAG_END:139 */g.ebx/* NFS_ASSEMBLY_DIAG_BEGIN:140 */)/* NFS_ASSEMBLY_DIAG_END:140 */||/* NFS_ASSEMBLY_DIAG_BEGIN:141 */nfs_ls_scene::assemblyGate(3018,/* NFS_ASSEMBLY_DIAG_END:141 */g.ecx/* NFS_ASSEMBLY_DIAG_BEGIN:142 */)/* NFS_ASSEMBLY_DIAG_END:142 */)throw 1;s->slotsAfter=now;}
 });}
 ~FrameScope(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!entered)return;safe(g,[&]{if(s->invalid)return;if(/* NFS_ASSEMBLY_DIAG_BEGIN:143 */nfs_ls_scene::assemblyGate(3019,/* NFS_ASSEMBLY_DIAG_END:143 */s->frameResolved/* NFS_ASSEMBLY_DIAG_BEGIN:144 */)/* NFS_ASSEMBLY_DIAG_END:144 */||/* NFS_ASSEMBLY_DIAG_BEGIN:145 */nfs_ls_scene::assemblyGate(3020,/* NFS_ASSEMBLY_DIAG_END:145 */s->fullRowOpen/* NFS_ASSEMBLY_DIAG_BEGIN:146 */)/* NFS_ASSEMBLY_DIAG_END:146 */||/* NFS_ASSEMBLY_DIAG_BEGIN:147 */nfs_ls_scene::assemblyGate(3021,/* NFS_ASSEMBLY_DIAG_END:147 */!retained(s->fullTarget,fullTarget())/* NFS_ASSEMBLY_DIAG_BEGIN:148 */)/* NFS_ASSEMBLY_DIAG_END:148 */||/* NFS_ASSEMBLY_DIAG_BEGIN:149 */nfs_ls_scene::assemblyGate(3022,/* NFS_ASSEMBLY_DIAG_END:149 */!bound(*s,g)/* NFS_ASSEMBLY_DIAG_BEGIN:150 */)/* NFS_ASSEMBLY_DIAG_END:150 */)throw 1;/* NFS_ASSEMBLY_DIAG_BEGIN:207 */nfs_ls_scene::assemblyCheckpoint(30400);/* NFS_ASSEMBLY_DIAG_END:207 */auto pixels=fullPlane(s->fullTarget);if(side){s->clean=std::move(pixels);SceneInput i{s->owner,s->slotsAfter,s->directory,s->fullTarget,s->revision,s->revision,s->mapName,s->mapRole,s->resetDraw,s->copyDraw,s->fullPre,s->postReset,s->clean};/* NFS_ASSEMBLY_DIAG_BEGIN:208 */nfs_ls_scene::assemblyCheckpoint(30500);/* NFS_ASSEMBLY_DIAG_END:208 */s->issuedScene=OwnedScene::create(i,s->published,s->trusted);if(/* NFS_ASSEMBLY_DIAG_BEGIN:151 */nfs_ls_scene::assemblyGate(3023,/* NFS_ASSEMBLY_DIAG_END:151 */!s->issuedScene/* NFS_ASSEMBLY_DIAG_BEGIN:152 */)/* NFS_ASSEMBLY_DIAG_END:152 */)throw 1;trace(*s,"scene-sealed");}else{s->postReset=std::move(pixels);}s->frameOpen=false;});}
};
inline uint32_t fullCopyTableAddress(bool side)noexcept{return side?uint32_t(guest_address(off_4DB54C)):uint32_t(guest_address(off_4DADB4));}
struct FullResolved {
 Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;FullDraw*d=nullptr;bool entered=false;
 explicit FullResolved(Game&v)noexcept:g(v){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid||!s->frameOpen)return;safe(g,[&]{if(/* NFS_ASSEMBLY_DIAG_BEGIN:153 */nfs_ls_scene::assemblyGate(4001,/* NFS_ASSEMBLY_DIAG_END:153 */s->frameResolved/* NFS_ASSEMBLY_DIAG_BEGIN:154 */)/* NFS_ASSEMBLY_DIAG_END:154 */)throw 1;s->frameResolved=true;entered=true;d=s->frameSide?&s->copyDraw:&s->resetDraw;/* NFS_ASSEMBLY_DIAG_BEGIN:209 */nfs_ls_scene::assemblyCheckpoint(40100);/* NFS_ASSEMBLY_DIAG_END:209 */d->args={{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,d->before);if(/* NFS_ASSEMBLY_DIAG_BEGIN:155 */nfs_ls_scene::assemblyGate(4002,/* NFS_ASSEMBLY_DIAG_END:155 */!retained(d->before,s->fullTarget)/* NFS_ASSEMBLY_DIAG_BEGIN:156 */)/* NFS_ASSEMBLY_DIAG_END:156 */)throw 1;});if(!entered||!s||s->invalid)return;safe(g,[&]{if(/* NFS_ASSEMBLY_DIAG_BEGIN:157 */nfs_ls_scene::assemblyGate(4003,/* NFS_ASSEMBLY_DIAG_END:157 */!read::readBytes(d->args[0],s->frameSide?RawSide:614416,d->sourceBefore)/* NFS_ASSEMBLY_DIAG_BEGIN:158 */)/* NFS_ASSEMBLY_DIAG_END:158 */||/* NFS_ASSEMBLY_DIAG_BEGIN:159 */nfs_ls_scene::assemblyGate(4004,/* NFS_ASSEMBLY_DIAG_END:159 */!read::readTable(fullCopyTableAddress(s->frameSide),d->tableBefore)/* NFS_ASSEMBLY_DIAG_BEGIN:160 */)/* NFS_ASSEMBLY_DIAG_END:160 */)throw 1;if(!s->frameSide){if(/* NFS_ASSEMBLY_DIAG_BEGIN:161 */nfs_ls_scene::assemblyGate(4005,/* NFS_ASSEMBLY_DIAG_END:161 */!s->directory.resolves(s->mapName,d->args[0],614416)/* NFS_ASSEMBLY_DIAG_BEGIN:162 */)/* NFS_ASSEMBLY_DIAG_END:162 */)throw 1;for(unsigned k=0;k<2;++k)if(logo::hashMatches(d->sourceBefore.data(),uint32_t(d->sourceBefore.size()),s->catalog->mapSha[k])){if(/* NFS_ASSEMBLY_DIAG_BEGIN:163 */nfs_ls_scene::assemblyGate(4006,/* NFS_ASSEMBLY_DIAG_END:163 */s->mapRole<2/* NFS_ASSEMBLY_DIAG_BEGIN:164 */)/* NFS_ASSEMBLY_DIAG_END:164 */)throw 1;s->mapRole=k;}if(/* NFS_ASSEMBLY_DIAG_BEGIN:165 */nfs_ls_scene::assemblyGate(4007,/* NFS_ASSEMBLY_DIAG_END:165 */s->mapRole>1/* NFS_ASSEMBLY_DIAG_BEGIN:166 */)/* NFS_ASSEMBLY_DIAG_END:166 */)throw 1;}});}
 ~FullResolved(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!entered)return;safe(g,[&]{if(s->invalid)return;{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,d->after);}if(/* NFS_ASSEMBLY_DIAG_BEGIN:167 */nfs_ls_scene::assemblyGate(4008,/* NFS_ASSEMBLY_DIAG_END:167 */!retained(d->after,s->fullTarget)/* NFS_ASSEMBLY_DIAG_BEGIN:168 */)/* NFS_ASSEMBLY_DIAG_END:168 */||/* NFS_ASSEMBLY_DIAG_BEGIN:169 */nfs_ls_scene::assemblyGate(4009,/* NFS_ASSEMBLY_DIAG_END:169 */!read::readBytes(d->args[0],d->sourceBefore.size(),d->sourceAfter)/* NFS_ASSEMBLY_DIAG_BEGIN:170 */)/* NFS_ASSEMBLY_DIAG_END:170 */||/* NFS_ASSEMBLY_DIAG_BEGIN:171 */nfs_ls_scene::assemblyGate(4010,/* NFS_ASSEMBLY_DIAG_END:171 */d->sourceAfter!=d->sourceBefore/* NFS_ASSEMBLY_DIAG_BEGIN:172 */)/* NFS_ASSEMBLY_DIAG_END:172 */||/* NFS_ASSEMBLY_DIAG_BEGIN:173 */nfs_ls_scene::assemblyGate(4011,/* NFS_ASSEMBLY_DIAG_END:173 */!read::readTable(fullCopyTableAddress(s->frameSide),d->tableAfter)/* NFS_ASSEMBLY_DIAG_BEGIN:174 */)/* NFS_ASSEMBLY_DIAG_END:174 */||/* NFS_ASSEMBLY_DIAG_BEGIN:175 */nfs_ls_scene::assemblyGate(4012,/* NFS_ASSEMBLY_DIAG_END:175 */d->tableBefore!=d->tableAfter/* NFS_ASSEMBLY_DIAG_BEGIN:176 */)/* NFS_ASSEMBLY_DIAG_END:176 */||/* NFS_ASSEMBLY_DIAG_BEGIN:177 */nfs_ls_scene::assemblyGate(4013,/* NFS_ASSEMBLY_DIAG_END:177 */s->fullRowOpen/* NFS_ASSEMBLY_DIAG_BEGIN:178 */)/* NFS_ASSEMBLY_DIAG_END:178 */)throw 1;s->frameResolved=false;});}
};
// Paired around the real raw-copy callback. CopyRow.before/after is the actual
// destination memory, never a computed prediction substituted as observation.
struct FullRow {
 Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;CopyRow*row=nullptr;bool entered=false;
 FullRow(Game&v,uint32_t callback)noexcept:g(v){s=current(g);if(s){pin=storage();sourceEpoch=s->epoch;}if(!s||s->invalid||!s->frameOpen||!s->frameResolved)return;safe(g,[&]{auto&d=s->frameSide?s->copyDraw:s->resetDraw;unsigned width=s->frameSide?343:640;uint32_t n=uint32_t(g.ebx);const uint32_t expected=s->frameSide?0xf00019cc:0xf0001878;if(/* NFS_ASSEMBLY_DIAG_BEGIN:179 */nfs_ls_scene::assemblyGate(5001,/* NFS_ASSEMBLY_DIAG_END:179 */callback!=expected/* NFS_ASSEMBLY_DIAG_BEGIN:180 */)/* NFS_ASSEMBLY_DIAG_END:180 */||/* NFS_ASSEMBLY_DIAG_BEGIN:181 */nfs_ls_scene::assemblyGate(5002,/* NFS_ASSEMBLY_DIAG_END:181 */s->fullRowOpen/* NFS_ASSEMBLY_DIAG_BEGIN:182 */)/* NFS_ASSEMBLY_DIAG_END:182 */||/* NFS_ASSEMBLY_DIAG_BEGIN:183 */nfs_ls_scene::assemblyGate(5003,/* NFS_ASSEMBLY_DIAG_END:183 */!n/* NFS_ASSEMBLY_DIAG_BEGIN:184 */)/* NFS_ASSEMBLY_DIAG_END:184 */||/* NFS_ASSEMBLY_DIAG_BEGIN:185 */nfs_ls_scene::assemblyGate(5004,/* NFS_ASSEMBLY_DIAG_END:185 */n>bg::Full/* NFS_ASSEMBLY_DIAG_BEGIN:186 */)/* NFS_ASSEMBLY_DIAG_END:186 */||/* NFS_ASSEMBLY_DIAG_BEGIN:187 */nfs_ls_scene::assemblyGate(5005,/* NFS_ASSEMBLY_DIAG_END:187 */d.rows.size()>=480/* NFS_ASSEMBLY_DIAG_BEGIN:188 */)/* NFS_ASSEMBLY_DIAG_END:188 */||/* NFS_ASSEMBLY_DIAG_BEGIN:189 */nfs_ls_scene::assemblyGate(5006,/* NFS_ASSEMBLY_DIAG_END:189 */!read::span(uint32_t(g.eax),size_t(n)*2)/* NFS_ASSEMBLY_DIAG_BEGIN:190 */)/* NFS_ASSEMBLY_DIAG_END:190 */||/* NFS_ASSEMBLY_DIAG_BEGIN:191 */nfs_ls_scene::assemblyGate(5007,/* NFS_ASSEMBLY_DIAG_END:191 */!read::span(uint32_t(g.edx),size_t(n)*2)/* NFS_ASSEMBLY_DIAG_BEGIN:192 */)/* NFS_ASSEMBLY_DIAG_END:192 */)throw 1;const uint64_t at=uint32_t(g.edx),first=uint64_t(s->fullTarget.w[8])+16;if(/* NFS_ASSEMBLY_DIAG_BEGIN:193 */nfs_ls_scene::assemblyGate(5008,/* NFS_ASSEMBLY_DIAG_END:193 */at<first/* NFS_ASSEMBLY_DIAG_BEGIN:194 */)/* NFS_ASSEMBLY_DIAG_END:194 */||/* NFS_ASSEMBLY_DIAG_BEGIN:195 */nfs_ls_scene::assemblyGate(5009,/* NFS_ASSEMBLY_DIAG_END:195 */at+uint64_t(n)*2>first+bg::Full*2/* NFS_ASSEMBLY_DIAG_BEGIN:196 */)/* NFS_ASSEMBLY_DIAG_END:196 */||/* NFS_ASSEMBLY_DIAG_BEGIN:197 */nfs_ls_scene::assemblyGate(5010,/* NFS_ASSEMBLY_DIAG_END:197 */(at-first)%2/* NFS_ASSEMBLY_DIAG_BEGIN:198 */)/* NFS_ASSEMBLY_DIAG_END:198 */)throw 1;if(/* NFS_ASSEMBLY_DIAG_BEGIN:199 */nfs_ls_scene::assemblyGate(5011,/* NFS_ASSEMBLY_DIAG_END:199 */s->frameSide&&n!=width/* NFS_ASSEMBLY_DIAG_BEGIN:200 */)/* NFS_ASSEMBLY_DIAG_END:200 */)throw 1;d.rows.emplace_back();row=&d.rows.back();row->callback=callback;row->args={{uint32_t(g.eax),uint32_t(g.edx),n}};if(/* NFS_ASSEMBLY_DIAG_BEGIN:201 */nfs_ls_scene::assemblyGate(5012,/* NFS_ASSEMBLY_DIAG_END:201 */!read::readBytes(row->args[0],size_t(n)*2,row->source)/* NFS_ASSEMBLY_DIAG_BEGIN:202 */)/* NFS_ASSEMBLY_DIAG_END:202 */)throw 1;row->before.resize(n);row->after.resize(n);{nfs::GuestCopyTransaction tx;std::memcpy(row->before.data(),nfs_host_address(row->args[1],size_t(n)*2),size_t(n)*2);}s->fullRowOpen=true;entered=true;});}
 ~FullRow(){if(!s||s->epoch!=sourceEpoch)return;if(!s||!pin||storage().get()!=s)return;if(s!=current(g))return;if(!entered)return;safe(g,[&]{if(s->invalid)return;{nfs::GuestCopyTransaction tx;std::memcpy(row->after.data(),nfs_host_address(row->args[1],row->after.size()*2),row->after.size()*2);}s->fullRowOpen=false;});}
};
struct ResolvedScope {Game&g;std::unique_ptr<CacheResolved>cache;std::unique_ptr<FullResolved>full;explicit ResolvedScope(Game&v)noexcept:g(v){auto*s=current(g);if(!s||s->invalid)return;safe(g,[&]{if(s->frameOpen)full.reset(new FullResolved(g));else if(s->iteration)cache.reset(new CacheResolved(g));});}~ResolvedScope(){if(!cache&&!full)return;safe(g,[&]{full.reset();cache.reset();});}};
inline void row(Game&g,uint32_t callback,uint32_t unit=1)noexcept{cacheRow(g,callback,unit);}
// A genuine original child OwnedFrame must be acquired/reacquired by the
// unchanged issuer outside this adapter. Matching it is a necessary additional
// join, not a substitute for that issuer's exact presentation permit.
inline std::shared_ptr<const OwnedScene>currentScene(Game&g,const std::shared_ptr<const nfs_ls_live::OwnedFrame>&child)noexcept{
 auto*s=current(g);if(!s||s->invalid||!child||!s->issuedScene)return {};std::shared_ptr<const OwnedScene>out;safe(g,[&]{if(s->iteration||s->frameOpen||s->retiring||s->refOpen||!bound(*s,g)||!retained(s->fullTarget,fullTarget())||!s->slotsAfter.sameCatalog(slots(*s))||slots(*s).selected!=s->slotsAfter.selected||child->packet()->clean()!=s->issuedScene->clean())throw 1;const auto&o=child->evidence().exit.owner;if(!nfs_ls_live::LiveIssuer::currentFrameForScene(g,child)||o.game!=s->game||o.runtimeForm!=uint32_t(guest_address(off_4D39A8))||o.archive!=s->owner.archive)throw 1;out=s->issuedScene;});return out;
}
struct SelectionScope {Game&g;std::shared_ptr<State>pin;State*s=nullptr;uint64_t sourceEpoch=0;Slots before;Target full;bool entered=false;explicit SelectionScope(Game&v)noexcept:g(v){s=current(g);if(!s||s->invalid)return;pin=storage();sourceEpoch=s->epoch;safe(g,[&]{before=slots(*s);full=fullTarget();entered=true;});}~SelectionScope(){if(!s||s->epoch!=sourceEpoch)return;if(!entered||!pin||storage().get()!=s||s!=current(g))return;safe(g,[&]{if(!before.sameCatalog(slots(*s))||!retained(full,fullTarget()))throw 1;});}};
inline bool observeScene(Game&g,const std::shared_ptr<const nfs_ls_live::OwnedFrame>&child,uint64_t request,unsigned phase)noexcept{auto*s=current(g);if(!s||s->invalid||!request||!child)return false;auto scene=currentScene(g,child);if(!scene)return false;safe(g,[&]{trace(*s,"typed-child-accepted",request,phase);});return true;}
// Unregistered until exact-source insertion/reversal + production-lock/ABI,
// lifecycle, fallback and source/packet contracts pass root review. No GL call.
} // namespace nfs_ls_scene_live

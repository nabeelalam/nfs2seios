// SPDX-License-Identifier: MIT
// Actual source join over sealed Player fixtures and checked guest memory.
// Lexical owner/copy metadata and preview selection reads are explicit stubs;
// no stub supplies a VerifiedSource or bypasses either pure verifier.
#define main existing_scene_runtime_fixture_main
#include "hd_scene_runtime_tests.cpp"
#undef main
#include "PlayerSceneFixture.h"
#include "hd/PlayerInitialSidebarProof.h"
static constexpr uint32_t byte_512DBF=0xe200,byte_512DDA=0xe201,byte_512DC1=0xe220,byte_512DDC=0xe240,dword_4D62C8=0xe400;
namespace nfs_player_scene_live {
namespace bg=nfs_player_scene_cpu;using Plane=nfs_logo_capture::Plane;
struct SidebarCapture{bg::Scene scene;rec::Group group;rec::Copy originalCopy;std::shared_ptr<const nfs_soft::FramePacket>caption;std::shared_ptr<const bg::SourceCatalog>catalogue;uint64_t captionSerial=0,captionLifetime=0,groupSerial=0,copySerial=0;bg::Event groupEvent{},tailEvent{},copyEvent{};uint32_t archive=0,form=0;std::shared_ptr<const nfs_compact_caption::Proof>compactCaption;uintptr_t captionGame=0;uint32_t captionFormGuest=0,captionArchive=0;};
enum class CachedUploadSite:uint32_t{None,Initial,Refresh,Lazy};
struct State{
 std::shared_ptr<const nfs_player_initial_sidebar::Proof>pendingInitialSidebar;
 bg::Base base;bg::Workspace workspace;std::shared_ptr<const bg::SourceCatalog>catalogue;std::shared_ptr<const SidebarCapture>retainedSidebar;std::shared_ptr<const rec::Copy>currentCopy;std::shared_ptr<const nfs_player_preview::Packet>preview;std::shared_ptr<const nfs_player_swatch::Packet>swatch;nfs_player_swatch::Authority swatchAuthority;
 nfs_player_preview::Selection previewSelection{};Plane actualSidebarPost{},actualPreviewPost{},swatchCopyPost{},actualSwatchPost{};
 bg::Event currentCopyEvent{},copyEvent{},sidebarValidationEvent{},previewEvent{},previewBaseEvent{},swatchCopyEvent{},swatchEvent{},uploadEvent{},retainedSidebarValidationEvent{};
 uint32_t form=0,formGuest=0,sourceOwner=0,archive=0;uintptr_t game=0,thread=0;uint64_t ownerEpoch=1,sequence=100,copySerial=0,previewSerial=7,previewLifetime=1,acceptedSwatchCopySerial=0,nextContent=0,nativeUploadEvent=0;unsigned cachedUploadDepth=1;CachedUploadSite cachedUploadSite=CachedUploadSite::None;bool on=true,invalid=false,baseVerified=true,swatchCurrent=false,retainedSidebarVerified=false;
 bg::Event event(uint64_t iteration)noexcept{return {ownerEpoch,++sequence,iteration};}
};
static bool feature=true;inline State*&slot()noexcept{static thread_local State*s=nullptr;return s;}
inline State*current(Game&g)noexcept{auto*s=slot();return feature&&s&&s->on&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==pub::threadToken()?s:nullptr;}
inline uint32_t relocatedForm(uint32_t form)noexcept{return form==0x4d3050?0x1000100:form==0x4d3100?0x1000200:0;}
inline bool archiveCurrent(State&s)noexcept{try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(dword_4D40B8)==s.archive;}catch(...){return false;}}
// Ordinary source cases carry no initial proof and cannot manufacture one.
inline bool initialSidebarCurrent(State&)noexcept{return false;}
inline const SidebarCapture*retainedSidebarCapture(const State&s)noexcept{return s.retainedSidebar&&s.retainedSidebar->form==s.form&&s.retainedSidebar->archive==s.archive?s.retainedSidebar.get():nullptr;}
}
namespace nfs_player_preview_live {static bool selectionOkay=true;inline bool selection(nfs_player_scene_live::State&s,nfs_player_preview::Selection&out)noexcept{out=s.previewSelection;if(!selectionOkay)++out.selectedCar;return selectionOkay;}}
namespace nfs_player_copy_live {enum class Site:uint32_t{Initial,Refresh,Lazy};struct State{std::shared_ptr<const nfs_player_initial_sidebar::Proof>initialSidebar;bool invalid=false;unsigned depth=0;nfs_player_scene_live::State*owner=nullptr;uintptr_t game=0,thread=0;uint64_t epoch=0,serial=0,iteration=0;uint32_t form=0,formGuest=0,sourceOwner=0,archive=0;nfs_player_scene_cpu::Event baseEvent{};Site site=Site::Initial;};inline State*&slot()noexcept{static thread_local State*s=nullptr;return s;}}
#define NFS_PLAYER_SOURCE_RUNTIME_TEST 1
#include "hd/PlayerSourceGameHooks.inc"
#undef NFS_PLAYER_SOURCE_RUNTIME_TEST
namespace source=nfs_player_source_live;namespace sl=nfs_player_scene_live;namespace cp=nfs_player_copy_live;namespace ps=nfs_player_scene_cpu;namespace pn=nfs_player_native;
static void load(Game&g,player_scene_fixture::Fixture&f,sl::State&l,cp::State&c){
 source::reset();prepareMemory(f);l.pendingInitialSidebar.reset();l.invalid=false;l.on=l.baseVerified=true;l.form=f.binding.form;l.formGuest=l.sourceOwner=sl::relocatedForm(l.form);l.archive=f.binding.archive;l.ownerEpoch=1;l.game=reinterpret_cast<uintptr_t>(&g);l.thread=pub::threadToken();l.sequence=100;l.nextContent=l.nativeUploadEvent=0;l.base=f.base;l.catalogue=std::make_shared<const ps::SourceCatalog>(f.catalog);
 auto raw=std::make_shared<sl::SidebarCapture>();raw->scene=f.scene;raw->group=f.group;raw->originalCopy=f.copy;raw->caption=f.caption;raw->catalogue=l.catalogue;raw->captionSerial=5;raw->captionLifetime=7;raw->copySerial=19;raw->groupSerial=f.group.serial;raw->groupEvent={1,21,1};raw->tailEvent={1,22,1};raw->archive=l.archive;raw->form=l.form;l.retainedSidebar=raw;
 auto input=f.input();l.currentCopy=std::make_shared<const rec::Copy>(f.copy);l.copySerial=f.copy.serial;l.currentCopyEvent=l.copyEvent=input.copyEvent;l.actualSidebarPost=f.scene.tailPost;l.sidebarValidationEvent=input.sidebarValidationEvent;l.preview=f.preview;l.previewSelection=f.preview->selection();l.previewSerial=input.acceptedPreviewSerial;l.previewLifetime=input.previewLifetime;l.previewEvent=input.previewEvent;l.previewBaseEvent=input.previewBaseEvent;l.actualPreviewPost=*input.actualPreviewPost;l.swatch=f.swatch;l.swatchCurrent=bool(f.swatch);l.acceptedSwatchCopySerial=input.acceptedSwatchCopySerial;
 if(f.swatch){l.swatchAuthority=input.swatchAuthority;l.swatchCopyEvent=input.swatchCopyEvent;l.swatchEvent=input.swatchEvent;l.swatchCopyPost=*input.swatchCopyPost;l.actualSwatchPost=*input.actualSwatchPost;}
 l.cachedUploadDepth=1;l.cachedUploadSite=f.mode==0?sl::CachedUploadSite::Initial:f.mode==1?sl::CachedUploadSite::Refresh:sl::CachedUploadSite::Lazy;c={};c.owner=&l;c.game=l.game;c.thread=l.thread;c.epoch=1;c.serial=l.copySerial;c.iteration=l.currentCopyEvent.iteration;c.form=l.form;c.formGuest=l.formGuest;c.sourceOwner=l.sourceOwner;c.archive=l.archive;c.baseEvent=l.base.event;c.site=f.mode==0?cp::Site::Initial:f.mode==1?cp::Site::Refresh:cp::Site::Lazy;
 m32(dword_4D40B8,l.archive);select(f.actualUpload.target);std::memcpy(memory.data()+f.actualUpload.target.w[8]+16,f.actualUpload.pixels.data(),ps::Full*2);
 // Original4824BC selects display storage before cached upload. The join must
 // read the anchored cached descriptor, never this differing current selection.
 auto display=f.actualUpload.target;display.owner=0x61000;display.w[8]=0x600000;display.ownerWords=display.w;select(display);CHECK(bg::le32(memory.data()+dword_4DB240+32)!=f.actualUpload.target.w[8]);
 if(f.swatch){const auto&b=f.swatch->binding();memory[l.form==ps::Player1Form?byte_512DBF:byte_512DDA]=uint8_t(b.car);memory[(l.form==ps::Player1Form?byte_512DC1:byte_512DDC)+b.car]=uint8_t(b.color);for(unsigned k=0;k<4;++k)m32(dword_4D62C8+16*b.color+4*k,b.table[k]);}
 pub::uploadEvents().store(77);g.eax=f.actualUpload.target.w[8];g.edx=1;g.ebx=g.ecx=0;
}
int main(){try{auto l=std::unique_ptr<sl::State>(new sl::State);cp::State copy;sl::slot()=l.get();cp::slot()=&copy;Game g;std::shared_ptr<const ps::VerifiedSource>retained;
 for(unsigned player:{1u,2u})for(unsigned mode:{0u,1u,2u}){auto f=std::unique_ptr<player_scene_fixture::Fixture>(new player_scene_fixture::Fixture(player,mode));CHECK(f->source);load(g,*f,*l,copy);auto sealed=source::prepareSource(g,77);CHECK(sealed&&sealed->form()==f->binding.form&&sealed->binding().kind==f->binding.kind);CHECK(sealed->target().owner==f->actualUpload.target.owner&&sealed->target().w[8]==g.eax);CHECK(std::equal(sealed->original().begin(),sealed->original().end(),f->actualUpload.pixels.begin()));pn::CurrentBinding now;CHECK(source::currentBinding(g,sealed->binding(),now));CHECK(now.sourceOwner==l->formGuest&&now.fullTargetOwner==0x41000&&now.sourceBitmap==g.eax);retained=sealed;
  // Candidate occurrence fields cannot supply a missing current identity.
  auto occurrence=sealed->binding();++occurrence.archive;CHECK(!source::currentBinding(g,occurrence,now));CHECK(!now.source.gameOwner);
 }
 auto f=std::unique_ptr<player_scene_fixture::Fixture>(new player_scene_fixture::Fixture(1,1));
 for(unsigned v=0;v<29;++v){load(g,*f,*l,copy);
  if(v==0)++l->form;if(v==1)++l->formGuest;if(v==2)++l->sourceOwner;if(v==3){++l->archive;m32(dword_4D40B8,l->archive);}if(v==4)++l->ownerEpoch;if(v==5)++copy.serial;if(v==6)copy.site=cp::Site::Initial;if(v==7)l->cachedUploadSite=sl::CachedUploadSite::Initial;if(v==8)++copy.iteration;if(v==9)++copy.baseEvent.sequence;if(v==10)pub::uploadEvents().store(78);if(v==11)++l->copySerial;if(v==12)++l->copyEvent.sequence;if(v==13)++l->previewSelection.selectedCar;if(v==14)nfs_player_preview_live::selectionOkay=false;
  if(v==15)l->actualSidebarPost.pixels[13]^=1;if(v==16){auto cat=std::make_shared<ps::SourceCatalog>(*l->catalogue);cat->mapSha[0][0]^=1;l->catalogue=cat;}if(v==17)m16(f->actualUpload.target.w[8]+16+639*2,99);if(v==18)m32(dword_4D4D24,0x61000);if(v==19)m32(0x41000+32,0x600000);if(v==20)++l->sidebarValidationEvent.sequence;if(v==21)l->cachedUploadDepth=0;if(v==22)copy.depth=1;if(v==23){l->currentCopyEvent.sequence=l->copyEvent.sequence=51;}if(v==24)l->sidebarValidationEvent.sequence=50;if(v==25)l->actualPreviewPost.pixels[0]^=1;if(v==26)++l->previewSerial;if(v==27)m32(dword_4D40B8,0x99999);if(v==28){auto raw=std::make_shared<sl::SidebarCapture>(*l->retainedSidebar);++raw->captionSerial;l->retainedSidebar=raw;}
  auto sealed=source::prepareSource(g,77);CHECK(!sealed);nfs_player_preview_live::selectionOkay=true;
 }
 // Null initial authority cannot replace a missing ordinary raw sidebar,
 // independently of the copied lexical kind (including Initial).
 for(unsigned mode:{0u,1u,2u}){auto absent=std::unique_ptr<player_scene_fixture::Fixture>(new player_scene_fixture::Fixture(1,mode));load(g,*absent,*l,copy);l->retainedSidebar.reset();CHECK(!l->pendingInitialSidebar&&!copy.initialSidebar&&!sl::initialSidebarCurrent(*l));CHECK(!source::prepareSource(g,77));}
 // A cached valid sidebar proof cannot authorize a different source catalog.
 load(g,*f,*l,copy);CHECK(source::prepareSource(g,77));auto other=std::make_shared<ps::SourceCatalog>(*l->catalogue);other->identities[1].sha[0]^=1;l->catalogue=other;CHECK(!source::prepareSource(g,77));
 // Current callbacks rebuild state; forged occurrence fields cannot hide a
 // same-owner source change after a previously successful preparation.
 for(unsigned v=0;v<9;++v){load(g,*f,*l,copy);auto sealed=source::prepareSource(g,77);CHECK(sealed);auto old=sealed->binding();if(v==0)++l->formGuest;if(v==1)++l->sourceOwner;if(v==2)++l->archive;if(v==3)++l->nativeUploadEvent;if(v==4)++l->nextContent;if(v==5)++copy.serial;if(v==6)m32(0x41000+32,0x600000);if(v==7)pub::uploadEvents().store(78);if(v==8){auto cat=std::make_shared<ps::SourceCatalog>(*l->catalogue);cat->identities[1].sha[0]^=1;l->catalogue=cat;}pn::CurrentBinding now;CHECK(!source::currentBinding(g,old,now)&&!now.source.gameOwner);}
 load(g,*f,*l,copy);auto before=reads,alloc=allocationCount.load();sl::feature=false;CHECK(!source::prepareSource(g,77));pn::CurrentBinding none;CHECK(!source::currentBinding(g,{},none));sl::feature=true;CHECK(reads==before&&allocationCount==alloc);
 std::thread foreign([&]{Game other;CHECK(!source::prepareSource(other,77));pn::CurrentBinding out;CHECK(!source::currentBinding(other,{},out));CHECK(!source::slot());});foreign.join();CHECK(reads==before&&allocationInLease==0&&!leaseDepth);CHECK(retained&&retained->original().size()==ps::Full);sl::slot()=nullptr;cp::slot()=nullptr;std::puts("PASS Player source join: sealed both-player Initial/Refresh/Lazy records, actual cached owner/default selection separation, independent current callbacks, source/event/catalog/current-plane rejections, disabled/foreign inert/no lease allocations; lexical owner/copy metadata and selection reader are explicit stubs");return 0;
 }catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}

// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerScenePrepare.h"
#include "PlayerPreviewPacket.h"
#include "Player1CaptionGameHooks.inc"
#include "Player2CaptionGameHooks.inc"
#include "PlayerSwatchPacket.h"
#include "ContinuousNativeUploadHooks.inc"
#include <memory>
#include <atomic>

namespace nfs_player_scene_live {
inline uint32_t canonicalForm(uint32_t guest)noexcept{if(guest==uint32_t(guest_address(off_4D3050)))return 0x4d3050;if(guest==uint32_t(guest_address(off_4D3100)))return 0x4d3100;return 0;}
inline uint32_t relocatedForm(uint32_t canonical)noexcept{return canonical==0x4d3050?uint32_t(guest_address(off_4D3050)):canonical==0x4d3100?uint32_t(guest_address(off_4D3100)):0;}
}
namespace nfs_logo_publisher {
struct PlayerMenuOwnerScope {
 Collector*s=nullptr;uintptr_t game=0;bool active=false,claimed=false;int unwind=0;
 explicit PlayerMenuOwnerScope(Game&g)noexcept {if(!enabled()||!nfs_player_scene_live::canonicalForm(uint32_t(g.eax)))return;uintptr_t expected=0,token=threadToken();if(!publisherThread().compare_exchange_strong(expected,token)&&expected!=token)return;claimed=expected==0;auto trusted=nfs_background_scene_runtime::acquireCatalog();if(!trusted){if(claimed)publisherThread().store(0);return;}auto*&slot=stateSlot();if(!slot)slot=new(std::nothrow)Collector{};s=slot;if(!s){if(claimed)publisherThread().store(0);return;}s->catalog=&trusted->logos;game=reinterpret_cast<uintptr_t>(&g);active=s->beginMenu(game,threadToken());unwind=exceptions();}
 ~PlayerMenuOwnerScope(){if(active)s->endMenu(game,exceptions()>unwind);if(claimed)publisherThread().store(0);}
};
}
namespace nfs_player_scene_live {
namespace bg=nfs_player_scene_cpu;
using nfs_logo_capture::Target;using nfs_logo_capture::Plane;
enum class Phase:uint32_t{None,Base,Side,Word};
enum class BaseArgument:uint32_t{Descriptor,Blk4Direct};
enum class Reason:uint32_t{None,Owner,Nested,Order,Read,Source,Target,Row,Palette,Unwind,Overflow,Missing,Verify,Upload,Retire,Allocation};
inline bool enabled()noexcept{static const bool yes=[](){const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_HD");return p&&p[0]=='1'&&!p[1];}();return yes&&nfs_logo_publisher::enabled();}
// Retained original draw evidence; this is NOT automatically verified authority.
// A direct cached refresh reuses343 and must revalidate its actual current owner,
// full LUT/header/bytes at the new copy instead of inventing new draw events.
struct SidebarCapture{bg::Scene scene;nfs_logo_capture::Group group;nfs_logo_capture::Copy originalCopy;std::shared_ptr<const nfs_soft::FramePacket>caption;std::shared_ptr<const nfs_compact_caption::Proof>compactCaption;uintptr_t captionGame=0;uint32_t captionFormGuest=0;std::shared_ptr<const bg::SourceCatalog>catalogue;uint64_t captionSerial=0,captionLifetime=0,groupSerial=0,copySerial=0;bg::Event groupEvent{},tailEvent{},copyEvent{};uint32_t archive=0,form=0;};
enum class CachedUploadSite:uint32_t{None=0,Initial=0x40c3cc,Refresh=0x40c5c6,Lazy=0x40bbe0};
struct State{
 uint32_t form=0,formGuest=0,sourceOwner=0;
 nfs_player_initial_sidebar::Store initialSidebarStore;std::shared_ptr<const nfs_player_initial_sidebar::Proof>pendingInitialSidebar;
 std::shared_ptr<const SidebarCapture>retainedSidebar;bool retainedSidebarVerified=false;bg::Event retainedSidebarValidationEvent{};
 CachedUploadSite cachedUploadSite=CachedUploadSite::None;unsigned cachedUploadDepth=0;
 std::shared_ptr<const nfs_logo_capture::Copy>currentCopy;bg::Event currentCopyEvent{};uint64_t nextCopySerial=0;
 Plane actualSidebarPost{},actualPreviewPost{},swatchCopyPost{},actualSwatchPost{};bg::Event sidebarValidationEvent{};bg::Event swatchCopyEvent{};uint64_t acceptedSwatchCopySerial=0;
 bg::Base base;bg::Scene scene;bg::Workspace workspace;
 std::shared_ptr<const bg::Prepared>published;
 std::shared_ptr<const nfs_soft::FramePacket>caption;std::shared_ptr<const nfs_compact_caption::Proof>compactCaption;uint64_t captionSerial=0,captionLifetime=0;
 std::shared_ptr<const nfs_player_preview::Packet> preview;
 nfs_player_preview::Selection previewSelection{};
 std::shared_ptr<const nfs_player_swatch::Packet>swatch;nfs_player_swatch::Authority swatchAuthority{};bg::Event swatchEvent{};uint64_t nextSwatchSerial=0;bool swatchCurrent=false;
 uint64_t nextPreviewSerial=0,previewSerial=0,previewLifetime=0;
 bg::Event previewEvent{},previewBaseEvent{};
 uintptr_t game=0,thread=0;uint64_t nextOwner=0,ownerEpoch=0,sequence=0,iteration=0,nextIteration=0,nextContent=0;
 uint64_t groupSerial=0,copySerial=0,nativeUploadEvent=0;
 bg::Event groupEvent{},tailEvent{},copyEvent{},uploadEvent{};
 uint32_t mainDepth=0,invocationDepth=0,archive=0,baseName=0,baseNameWord=0,baseSource=0;
 Target baseTarget{};std::array<uint32_t,9>baseTable{};uint32_t baseRows=0,baseCount=0;bool baseRowsValid=true,baseVerified=false,on=false,invalid=false,baseResolved=false,resolved=false;
 uint32_t stage=0,detail=0;
 std::shared_ptr<const bg::SourceCatalog> catalogue;
 Phase phase=Phase::None;int32_t pending=-1;unsigned copyWrapperDepth=0;bool captured483438=false;
 std::array<uint32_t,4>capturedArgs{};Reason reason=Reason::None;
 bg::Draw*draw()noexcept{return pending>=0&&unsigned(pending)<scene.draws.size()?&scene.draws[unsigned(pending)]:nullptr;}
 void clearSwatch()noexcept{swatch.reset();swatchAuthority={};swatchEvent={};swatchCurrent=false;swatchCopyEvent={};acceptedSwatchCopySerial=0;swatchCopyPost.valid=actualSwatchPost.valid=0;}
 void clearInitial()noexcept{initialSidebarStore.clear();pendingInitialSidebar.reset();}
 void clearPreview()noexcept{currentCopy.reset();currentCopyEvent={};actualSidebarPost.valid=0;sidebarValidationEvent={};clearSwatch();actualPreviewPost.valid=0;previewSelection={};preview.reset();previewSerial=previewLifetime=0;previewEvent=previewBaseEvent={};}
 void fail(Reason why,bool baseToo=false)noexcept{clearInitial();reason=why;invalid=true;retainedSidebarVerified=false;retainedSidebarValidationEvent={};currentCopy.reset();currentCopyEvent={};published.reset();clearPreview();if(baseToo)baseVerified=false;}
 bg::Event event(uint64_t redraw)noexcept{if(sequence==UINT64_MAX){fail(Reason::Overflow,true);return {};}return {ownerEpoch,++sequence,redraw};}
 // Starting an original lazy iteration is not a pixel-write observation:
 // its timer/animation branch may return before any sidebar/copy/upload.
 // Preserve the separately verified cached source until actual painting starts.
 void discardCachedSource()noexcept{published.reset();currentCopy.reset();currentCopyEvent={};actualSidebarPost.valid=0;sidebarValidationEvent={};copySerial=nativeUploadEvent=0;copyEvent=uploadEvent={};}
 void freshScene(bool preserveCached=false)noexcept{if(!preserveCached)discardCachedSource();caption.reset();compactCaption.reset();captionSerial=captionLifetime=0;scene.draws.clear();scene.form=form;scene.complete=scene.error=scene.tailKind=scene.tailPredicate=scene.tailSymbol=0;scene.preSide.valid=scene.postSide.valid=scene.postWords.valid=scene.tailPre.valid=scene.tailPost.valid=0;pending=-1;phase=Phase::None;resolved=false;groupSerial=0;groupEvent=tailEvent={};captured483438=false;copyWrapperDepth=0;}
};
inline std::atomic<uintptr_t>&ownerThread()noexcept{static std::atomic<uintptr_t>t{0};return t;}
inline State*&slot()noexcept{struct Storage{State*value=nullptr;~Storage(){delete value;}};static thread_local Storage storage;return storage.value;}
inline State*current(Game&g)noexcept{if(!enabled())return nullptr;auto*s=slot();return s&&s->on&&s->mainDepth==1&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==nfs_logo_publisher::threadToken()?s:nullptr;}
inline bool unarmed(State*s)noexcept{return !s->iteration;}
inline bool readPlane(Plane&p)noexcept{uint32_t error=0;p.valid=0;nfs_logo_publisher::plane(p,error);return !error&&p.valid;}
inline bool readBytes(uint32_t at,size_t n,std::vector<uint8_t>&out)noexcept{try{out.resize(n);{nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(at,n);std::memcpy(out.data(),nfs_host_address(at,n),n);}return true;}catch(...){return false;}}
inline bool readTable(std::array<uint32_t,9>&out,Phase phase)noexcept{try{nfs::GuestCopyTransaction tx;uint32_t at=uint32_t(guest_address(phase==Phase::Word?off_4DAD64:off_4DADB4));for(unsigned k=0;k<9;++k)out[k]=tx.load<uint32_t>(at+4*k);return true;}catch(...){return false;}}
struct PlayerScope{State*s=nullptr;bool entered=false;int unwind=0;explicit PlayerScope(Game&g)noexcept{if(!enabled()||!nfs_player_scene_live::canonicalForm(uint32_t(g.eax)))return;auto trusted=nfs_background_scene_runtime::acquireCatalog();if(!trusted)return;uintptr_t expected=0,t=nfs_logo_publisher::threadToken();if(!ownerThread().compare_exchange_strong(expected,t)&&expected!=t)return;auto*&p=slot();if(!p)p=new(std::nothrow)State;if(!p){ownerThread().store(0);return;}s=p;s->catalogue=std::move(trusted);entered=true;unwind=nfs_logo_publisher::exceptions();if(s->mainDepth++){s->fail(Reason::Nested,true);return;}if(s->nextOwner==UINT64_MAX){s->fail(Reason::Overflow,true);return;}s->clearInitial();s->retainedSidebar.reset();s->retainedSidebarVerified=false;s->retainedSidebarValidationEvent={};s->cachedUploadSite=CachedUploadSite::None;s->cachedUploadDepth=0;s->form=canonicalForm(uint32_t(g.eax));s->formGuest=relocatedForm(s->form);s->sourceOwner=s->formGuest;s->clearPreview();s->detail=0;s->ownerEpoch=++s->nextOwner;s->sequence=s->iteration=s->nextIteration=0;s->on=true;s->invalid=false;s->baseVerified=false;s->game=reinterpret_cast<uintptr_t>(&g);s->thread=nfs_logo_publisher::threadToken();s->freshScene();}~PlayerScope(){if(!entered)return;if(!s->mainDepth){s->fail(Reason::Order,true);return;}if(--s->mainDepth)return;s->stage=18;if(nfs_logo_publisher::exceptions()>unwind)s->fail(Reason::Unwind,true);else{s->reason=Reason::Owner;s->invalid=true;s->published.reset();s->clearPreview();s->baseVerified=false;}s->retainedSidebar.reset();s->retainedSidebarVerified=false;s->retainedSidebarValidationEvent={};s->cachedUploadDepth=0;s->cachedUploadSite=CachedUploadSite::None;s->clearInitial();s->on=false;s->game=0;s->invocationDepth=0;ownerThread().store(0);}};
struct IterationScope{State*s=nullptr;bool entered=false,started=false;uint32_t form=0;int unwind=0;explicit IterationScope(Game&g)noexcept{s=current(g);if(!s)return;entered=true;unwind=nfs_logo_publisher::exceptions();form=uint32_t(g.eax);if(s->invocationDepth++)s->fail(Reason::Nested);}void start()noexcept{if(!entered||started)return;started=true;if(!s->baseVerified){s->published.reset();s->iteration=0;return;}if(s->invocationDepth!=1||form!=s->formGuest||s->nextIteration==UINT64_MAX){s->fail(Reason::Owner);return;}s->freshScene(true);s->invalid=false;s->reason=Reason::None;s->iteration=++s->nextIteration;s->scene.event=s->event(s->iteration);}~IterationScope(){if(!entered)return;if(nfs_logo_publisher::exceptions()>unwind)s->fail(Reason::Unwind);if(s->invocationDepth)--s->invocationDepth;}};
inline bool captionBusy(Game&g,const State&s)noexcept{if(s.form==0x4d3050){auto*c=nfs_player1_caption::current(g);return c&&(c->recording||c->tailDepth||c->string||c->glyph);}if(s.form==0x4d3100){auto*c=nfs_player2_caption::current(g);return c&&(c->recording||c->tailDepth||c->string||c->glyph);}return true;}
inline bool captureCaption(Game&g,State&s)noexcept{
 std::shared_ptr<const nfs_soft::FramePacket>packet;std::shared_ptr<const nfs_compact_caption::Proof>proof;uint64_t serial=0,lifetime=0;
 if(s.form==0x4d3050){auto*c=nfs_player1_caption::current(g);packet=nfs_player1_caption::lastPacket(g);if(!c||!packet)return false;serial=c->serial;lifetime=c->lifetime;proof=nfs_player1_caption::acquireCurrentProof(g);}
 else if(s.form==0x4d3100){auto*c=nfs_player2_caption::current(g);packet=nfs_player2_caption::lastPacket(g);if(!c||!packet)return false;serial=c->serial;lifetime=c->lifetime;proof=nfs_player2_caption::acquireCurrentProof(g);}else return false;
 if(proof){const auto&e=proof->evidence();if(!proof->current()||proof->origin()||proof->packet()!=packet||e.route!=nfs_compact_caption::Route::OrdinaryEbp||e.policy!=(s.form==0x4d3050?nfs_compact_caption::Policy::Player1:nfs_compact_caption::Policy::Player2)||e.game!=s.game||e.formGuest!=s.formGuest||e.incomingEbp!=s.formGuest||e.serial!=serial||e.lifetime!=lifetime)return false;}
 s.captionSerial=serial;s.captionLifetime=lifetime;s.caption=std::move(packet);s.compactCaption=std::move(proof);return true;
}
inline bool initialSidebarCurrent(State&s)noexcept{
 const auto&p=s.pendingInitialSidebar;if(!p||!p->current()||!s.on||s.invalid||!s.catalogue||!p->catalogMatches(*s.catalogue))return false;const auto&b=p->binding();if(b.form!=s.form||b.formGuest!=s.formGuest||s.sourceOwner!=s.formGuest||b.game!=s.game||b.thread!=s.thread||b.archive!=s.archive||b.epoch!=s.ownerEpoch||b.captionEvent.ownerEpoch!=s.ownerEpoch||b.captionEvent.iteration||!b.captionEvent.sequence)return false;
 auto&g=*reinterpret_cast<Game*>(s.game);auto caption=s.form==0x4d3050?nfs_player1_caption::lastProof(g):nfs_player2_caption::lastProof(g);return caption==p->captionProof()&&caption&&caption->current();
}
// Runs only after both exact initial caller scopes have closed, before the
// original selected343 is left. No replacement draw or guest write occurs.
inline void completeInitialCaption(Game&g)noexcept{
 auto*s=current(g);if(!s||s->invalid||uint32_t(g.esi)!=s->formGuest)return;s->clearInitial();auto caption=s->form==0x4d3050?nfs_player1_caption::acquireCurrentProof(g):nfs_player2_caption::acquireCurrentProof(g);if(!caption||caption->evidence().route!=nfs_compact_caption::Route::InitialMain||!caption->origin())return;
 uint32_t archive=0;try{nfs::GuestCopyTransaction tx;archive=tx.load<uint32_t>(dword_4D40B8);}catch(...){return;}if(!archive||archive!=caption->origin()->archive())return;
 if(!readPlane(s->actualSidebarPost)||!bg::plane(s->actualSidebarPost,343))return;const auto event=s->event(0);if(!event.sequence||s->invalid)return;s->archive=archive;nfs_player_initial_sidebar::Binding b;b.form=s->form;b.formGuest=s->formGuest;b.archive=archive;b.game=s->game;b.thread=s->thread;b.epoch=s->ownerEpoch;b.captionEvent=event;
 s->pendingInitialSidebar=s->initialSidebarStore.accept(caption->origin(),caption,b,s->actualSidebarPost,s->workspace,s->catalogue);
}
struct RoleScope{State*s=nullptr;Phase phase=Phase::None;bool active=false;int unwind=0;RoleScope(Game&g,Phase requested,BaseArgument baseArgument=BaseArgument::Descriptor)noexcept{s=current(g);if(!s||(s->invalid&&requested!=Phase::Base)){return;}if(s->phase!=Phase::None||s->resolved){s->fail(Reason::Nested);return;}if(requested!=Phase::Base&&(!s->iteration||s->invocationDepth!=1))return;phase=requested;s->phase=requested;active=true;unwind=nfs_logo_publisher::exceptions();if(phase==Phase::Base){if(captionBusy(g,*s)){s->fail(Reason::Nested,true);return;}s->clearPreview();s->freshScene();s->phase=Phase::Base;s->iteration=0;s->baseVerified=false;s->invalid=false;s->reason=Reason::None;s->detail=0;s->published.reset();s->baseRows=s->baseCount=0;s->baseRowsValid=true;try{nfs::GuestCopyTransaction tx;s->archive=tx.load<uint32_t>(dword_4D40B8);s->retainedSidebarValidationEvent={};s->baseName=tx.load<uint32_t>(s->formGuest+4);s->baseNameWord=tx.load<uint32_t>(s->baseName);const bool nameMatches=baseArgument==BaseArgument::Descriptor?uint32_t(g.eax)==s->baseName:
 uint32_t(g.eax)==uint32_t(guest_address(aBlk4))&&s->baseNameWord==879455330u&&tx.load<uint32_t>(uint32_t(guest_address(aBlk4)))==s->baseNameWord;
 if(!s->archive||!nameMatches||uint32_t(g.edx)!=0x400||g.ebx||g.ecx)throw 1;}catch(...){s->fail(Reason::Source,true);}if(s->pendingInitialSidebar&&!initialSidebarCurrent(*s))s->clearInitial();if(s->retainedSidebar&&(s->retainedSidebar->archive!=s->archive||s->retainedSidebar->form!=s->form)){s->retainedSidebar.reset();s->retainedSidebarVerified=false;}}else if(phase==Phase::Side){s->discardCachedSource();s->scene.event=s->event(s->iteration);if(!readPlane(s->scene.preSide)||!bg::plane(s->scene.preSide,343))s->fail(Reason::Target);}}
 ~RoleScope(){if(!active)return;if(nfs_logo_publisher::exceptions()>unwind)s->fail(Reason::Unwind,phase==Phase::Base);if(phase==Phase::Side&&!s->invalid){if(!readPlane(s->scene.postSide)||!bg::plane(s->scene.postSide,343))s->fail(Reason::Target);}s->phase=Phase::None;s->pending=-1;}};
inline void beginGroup(Game&g)noexcept{auto*s=current(g);auto*c=nfs_logo_publisher::store();if(!s||s->invalid||!s->iteration)return;if(!c||!c->groupActive||c->gameOwner!=s->game||s->phase!=Phase::None){s->fail(Reason::Order);return;}s->scene.postWords=c->group.pre;s->groupSerial=c->group.serial;s->groupEvent=s->event(s->iteration);}
inline void postGroup(Game&g)noexcept{auto*s=current(g);auto*c=nfs_logo_publisher::store();if(!s||s->invalid)return;s->stage=4;if(unarmed(s))return;if(!c||!c->group.complete||c->group.serial!=s->groupSerial)s->fail(Reason::Order);}
struct TailScope {Game&g;State*s=nullptr;bool entered=false;int unwind=0;
 explicit TailScope(Game&game)noexcept:g(game){s=current(g);if(!s||s->invalid||unarmed(s))return;entered=true;unwind=nfs_logo_publisher::exceptions();if(s->invocationDepth!=1||!readPlane(s->scene.tailPre)||!bg::plane(s->scene.tailPre,343)){s->fail(Reason::Target);return;}s->scene.tailKind=s->form==0x4d3050?NFS_BACKGROUND_TAIL_PLAYER1_CAPTION:NFS_BACKGROUND_TAIL_PLAYER2_CAPTION;s->scene.tailPredicate=s->scene.tailSymbol=s->form;s->tailEvent=s->event(s->iteration);}
 ~TailScope(){if(!entered||s->invalid)return;if(nfs_logo_publisher::exceptions()>unwind){s->fail(Reason::Unwind);return;}if(!captureCaption(g,*s)||!readPlane(s->scene.tailPost)||!bg::plane(s->scene.tailPost,343)||!nfs_logo_capture::same(s->scene.tailPre.target,s->scene.tailPost.target)){s->fail(Reason::Missing);return;}}
};
inline void copyEnd(Game&g)noexcept{auto*s=current(g);auto*c=nfs_logo_publisher::store();if(!s||s->invalid)return;if(unarmed(s))return;if(!c||!c->copy.complete||c->copy.linkedGroup!=s->groupSerial||!s->tailEvent.sequence){s->fail(Reason::Order);return;}const auto captureEvent=s->event(s->iteration);s->scene.complete=1;
 try{std::shared_ptr<SidebarCapture>r(new SidebarCapture);r->scene=s->scene;r->group=c->group;r->originalCopy=c->copy;r->caption=s->caption;r->compactCaption=s->compactCaption;r->captionGame=s->game;r->captionFormGuest=s->formGuest;r->catalogue=s->catalogue;r->captionSerial=s->captionSerial;r->captionLifetime=s->captionLifetime;r->groupSerial=s->groupSerial;r->copySerial=s->copySerial;r->groupEvent=s->groupEvent;r->tailEvent=s->tailEvent;r->copyEvent=captureEvent;r->archive=s->archive;r->form=s->form;s->clearInitial();s->retainedSidebar=std::move(r);s->retainedSidebarVerified=false;s->retainedSidebarValidationEvent={};}catch(...){s->fail(Reason::Allocation);}}

// Correlation only: the pure side proof must still vet every retained record.
inline const SidebarCapture*retainedSidebarCapture(const State&s)noexcept{const auto&r=s.retainedSidebar;return r&&r->form==s.form&&r->archive==s.archive&&r->groupEvent.ownerEpoch==s.ownerEpoch?r.get():nullptr;}
inline uint64_t activeSidebarGroupSerial(State&s)noexcept{if(s.invalid)return 0;auto*c=nfs_logo_publisher::store();if(s.groupSerial&&s.groupEvent.ownerEpoch==s.ownerEpoch&&s.scene.form==s.form&&s.scene.tailPost.valid&&s.caption&&c&&c->group.complete&&c->group.serial==s.groupSerial)return s.groupSerial;auto*r=retainedSidebarCapture(s);return r?r->groupSerial:0;}
inline void invalidateCache()noexcept{if(!enabled())return;if(auto*s=slot()){s->stage=16;if(s->on&&s->mainDepth==1&&s->thread==nfs_logo_publisher::threadToken()&&!s->invalid&&!s->iteration&&!s->baseVerified&&!s->published&&!s->groupSerial&&!s->copySerial&&!s->nativeUploadEvent){if(unarmed(s))return;}s->fail(Reason::Retire);}}
inline void retire(Game&g)noexcept{auto*s=current(g);if(!s)return;uint32_t owner=uint32_t(g.eax);if(owner&&((s->baseVerified&&owner==s->base.post.target.owner)||owner==s->scene.preSide.target.owner))s->fail(Reason::Retire,true);if(s->retainedSidebar&&(owner==s->retainedSidebar->scene.tailPost.target.owner||owner==s->retainedSidebar->scene.preSide.target.owner)){s->retainedSidebar.reset();s->retainedSidebarVerified=false;s->retainedSidebarValidationEvent={};}}
// Resolved source/callback and upload sealing are defined below; no diagnostic Store is consulted.
static constexpr uint32_t mapNames[2]={879455330,913009762};
inline bool targetNow(Target&t)noexcept{try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t);return true;}catch(...){return false;}}
inline bool archiveCurrent(State&s)noexcept{try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(dword_4D40B8)==s.archive;}catch(...){return false;}}
inline bool sameBytesHash(const std::vector<uint8_t>&a,const std::array<uint8_t,32>&sha)noexcept{return nfs_small_logo_live::hashMatches(a.data(),uint32_t(a.size()),sha);}
inline bool bank(std::array<uint32_t,256>&out,uint32_t at,bool words)noexcept{try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(at,words?512:1024);for(unsigned k=0;k<256;++k)out[k]=words?tx.load<uint16_t>(at+2*k):tx.load<uint32_t>(at+4*k);return true;}catch(...){return false;}}
struct ResolvedScope{
 State*s=nullptr;bg::Draw*d=nullptr;Phase phase=Phase::None;uint32_t source=0;bool active=false;int unwind=0;
 explicit ResolvedScope(Game&g)noexcept{s=current(g);if(s&&s->phase!=Phase::None){}if(!s||s->invalid||s->phase==Phase::None)return;if(s->resolved){s->fail(Reason::Nested);return;}phase=s->phase;source=uint32_t(g.eax);unwind=nfs_logo_publisher::exceptions();s->resolved=true;active=true;if(!archiveCurrent(*s)){s->fail(Reason::Source,true);return;}
  if(phase==Phase::Base){s->baseResolved=true;s->base.event=s->event(0);s->base.form=s->form;s->base.mapRole=UINT32_MAX;s->baseSource=source;if(uint32_t(g.edx)!=0x400||g.ebx||g.ecx||!readBytes(source,614416,s->base.sourceBefore)||!targetNow(s->baseTarget)||!bg::fullClip(s->baseTarget,640)||!readTable(s->baseTable,phase)){s->fail(Reason::Source,true);return;}if(s->baseTarget.ownerState[3]!=s->baseTarget.owner||s->baseTable[3]!=uint32_t(game_id_sub_483438)||s->baseTable[6]||s->baseTable[7]!=2){s->fail(Reason::Target,true);return;}for(unsigned k=0;k<2;++k)if(s->baseNameWord==mapNames[k]&&sameBytesHash(s->base.sourceBefore,s->catalogue->mapSha[k]))s->base.mapRole=k;if(s->base.mapRole>1)s->fail(Reason::Source,true);return;}
  if(!s->iteration||s->scene.draws.size()>=5||(phase==Phase::Side&&!s->scene.draws.empty())||(phase==Phase::Word&&s->scene.draws.empty())){s->fail(Reason::Order);return;}
  try{s->scene.draws.emplace_back();d=&s->scene.draws.back();s->pending=int32_t(s->scene.draws.size()-1);d->rows.reserve(480);d->args={{source,uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};if(!targetNow(d->before)||!bg::fullClip(d->before,343)||!readTable(d->tableBefore,phase))throw 1;std::array<uint8_t,16>head{};{nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(source,16);std::memcpy(head.data(),nfs_host_address(source,16),16);}for(unsigned k=phase==Phase::Side?0:1;k<(phase==Phase::Side?1:13);++k){const auto&id=s->catalogue->identities[k];if(head!=id.header)continue;if(!readBytes(source,id.span,d->sourceBefore))throw 1;if(sameBytesHash(d->sourceBefore,id.sha)){d->role=k;break;}}if(d->role>12)throw 1;}catch(...){s->fail(Reason::Source);}
 }
 ~ResolvedScope(){if(!active)return;if(nfs_logo_publisher::exceptions()>unwind)s->fail(Reason::Unwind,phase==Phase::Base);
  if(!s->invalid){if(phase==Phase::Base){std::array<uint32_t,9>after{};if(!readBytes(source,614416,s->base.sourceAfter)||s->base.sourceBefore!=s->base.sourceAfter||!readPlane(s->base.post)||!nfs_logo_capture::same(s->baseTarget,s->base.post.target)||!readTable(after,phase)||after!=s->baseTable||!s->baseRowsValid||s->baseCount!=307200||(s->baseRows!=1&&s->baseRows!=480))s->fail(Reason::Source,true);else{for(unsigned k=0;k<307200;++k)if(s->base.post.pixels[k]!=bg::le16(s->base.sourceBefore.data()+16+2*k)){s->fail(Reason::Source,true);break;}if(!s->invalid)s->baseVerified=true;}}
   else if(d){if(!readBytes(source,uint32_t(d->sourceBefore.size()),d->sourceAfter)||!targetNow(d->after)||!nfs_logo_capture::same(d->before,d->after)||!readTable(d->tableAfter,phase)||d->sourceBefore!=d->sourceAfter||d->rows.empty())s->fail(Reason::Source);if(!s->invalid&&d->paletteValid){const bool indexed=phase==Phase::Side&&d->callback==0xf000221c;if(!bank(d->paletteAfter,d->paletteGuest,indexed)||d->paletteAfter!=d->paletteBefore)s->fail(Reason::Palette);}d->complete=!s->invalid;d->error=s->invalid?1:0;}}
  s->stage=8;s->resolved=false;s->baseResolved=false;s->pending=-1;s->captured483438=false;
 }
};
inline void row(Game&g,uint32_t callback,uint32_t unit=1)noexcept{
 auto*s=current(g);if(!s||s->invalid||!s->resolved)return;const std::array<uint32_t,4>a{{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};s->captured483438=false;
 if(s->copyWrapperDepth&&callback==uint32_t(game_id_sub_4A14D4))return;
 if(s->phase==Phase::Base&&s->baseResolved){uint32_t converter=1;try{nfs::GuestCopyTransaction tx;converter=tx.load<uint32_t>(dword_571264);}catch(...){s->fail(Reason::Read,true);return;}bool okay=callback==uint32_t(game_id_sub_483438)&&unit==1&&!converter&&(a[2]==640||a[2]==307200)&&s->baseRows<480&&uint64_t(s->baseCount)+a[2]<=307200&&a[0]==s->baseSource+16+s->baseCount*2&&a[1]==s->baseTarget.w[8]+16+s->baseCount*2;if(!okay){s->fail(Reason::Row,true);return;}++s->baseRows;s->baseCount+=a[2];s->captured483438=true;s->capturedArgs=a;return;}
 auto*d=s->draw();if(!d)return;const uint64_t first=uint64_t(d->before.w[8])+16,last=first+164640*2;uint64_t bytes=unit==2?a[2]:uint64_t(a[2])*2;
 // Ignore unrelated conversion scratch copies before touching guest memory.
 if(a[1]<first||uint64_t(a[1])+bytes>last)return;
 const bool indexed=s->phase==Phase::Side&&callback==0xf000221c;const bool word=s->phase==Phase::Word&&callback==uint32_t(game_id_sub_482BD4);const bool copied=s->phase==Phase::Side&&(callback==uint32_t(game_id_sub_483438)||callback==uint32_t(game_id_sub_4A14D4));
 if((!indexed&&!word&&!copied)||!a[2]||d->rows.size()>=480||(unit!=1&&unit!=2)||a[2]>329280){s->fail(Reason::Row);return;}
 try{const size_t n=indexed||word?a[2]:(unit==2?a[2]:size_t(a[2])*2);if(n>329280)throw 1;d->rows.emplace_back();auto&r=d->rows.back();r.callback=callback;r.unit=unit;r.args=a;r.payload.resize(n);uint32_t converter=0,pal=0,global=0,format=0;{nfs::GuestCopyTransaction tx;Target now;nfs_logo_publisher::target(tx,now);if(!nfs_logo_capture::same(now,d->before))throw 1;converter=tx.load<uint32_t>(dword_571264);r.converter=converter;nfs_logo_publisher::checked(a[0],n);std::memcpy(r.payload.data(),nfs_host_address(a[0],n),n);if(indexed){pal=uint32_t(guest_address(word_56EF3C));global=converter;format=tx.load<uint8_t>(byte_4DB25C);}else if(word){pal=tx.load<uint32_t>(dword_5636E4);global=tx.load<uint32_t>(dword_4DAD7C);format=tx.load<uint32_t>(dword_4DAD84);}}
  if(indexed||word){if(!pal)throw 1;if(d->rows.size()==1){d->callback=callback;d->callbackGlobal=global;d->format=format;d->paletteGuest=pal;if(!bank(d->paletteBefore,pal,indexed))throw 1;}else if(d->callback!=callback||d->callbackGlobal!=global||d->format!=format||d->paletteGuest!=pal)throw 1;if(!bank(d->paletteAfter,pal,indexed)||d->paletteAfter!=d->paletteBefore)throw 1;d->paletteValid=1;}
  if(callback==uint32_t(game_id_sub_483438)){s->captured483438=true;s->capturedArgs=a;}
 }catch(...){s->fail(Reason::Row);}
}
inline void byteCopyRow(Game&g)noexcept{row(g,uint32_t(game_id_sub_4A14D4),2);}
struct CopyWrapperScope{State*s=nullptr;bool active=false;explicit CopyWrapperScope(Game&g)noexcept{s=current(g);if(!s||s->invalid||!s->captured483438)return;const std::array<uint32_t,4>a{{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};if(a!=s->capturedArgs)return;s->captured483438=false;++s->copyWrapperDepth;active=true;}~CopyWrapperScope(){if(active&&s->copyWrapperDepth)--s->copyWrapperDepth;}};
inline uint32_t indexedSideCallee(Game&g)noexcept{auto*s=current(g);if(!s||s->invalid||!s->resolved||s->phase!=Phase::Side||!s->draw())return 0;try{nfs::GuestCopyTransaction tx;if(uint32_t(g.esp)>UINT32_MAX-0x3c)throw 1;return tx.load<uint32_t>(uint32_t(g.esp)+0x38);}catch(...){s->fail(Reason::Read);return 0;}}
// Only actual call-bound4A6AA0, never a guessed dispatcher/table slot.
struct IndexedSideCallScope{State*s=nullptr;bg::Draw*d=nullptr;int unwind=0;IndexedSideCallScope(Game&g,uint32_t callee)noexcept{if(callee!=0xf000221c)return;s=current(g);if(!s||s->invalid||s->phase!=Phase::Side||!s->resolved)return;unwind=nfs_logo_publisher::exceptions();row(g,callee,1);d=s->draw();}~IndexedSideCallScope(){if(!d||s->invalid)return;if(nfs_logo_publisher::exceptions()>unwind){s->fail(Reason::Unwind);return;}if(!bank(d->paletteAfter,d->paletteGuest,true)||d->paletteAfter!=d->paletteBefore)s->fail(Reason::Palette);}};
inline bool captionCurrent(Game&g,State&s)noexcept{if(s.form==0x4d3050){auto*c=nfs_player1_caption::current(g);return c&&s.caption&&c->packet==s.caption&&!c->recording&&!c->tailDepth&&c->serial==s.captionSerial&&c->lifetime==s.captionLifetime;}if(s.form==0x4d3100){auto*c=nfs_player2_caption::current(g);return c&&s.caption&&c->packet==s.caption&&!c->recording&&!c->tailDepth&&c->serial==s.captionSerial&&c->lifetime==s.captionLifetime;}return false;}
inline bool previewCurrent(const State&s)noexcept{const auto&p=s.preview;if(s.invalid||!s.baseVerified||!p||!s.previewSerial||!s.previewLifetime||s.previewLifetime!=s.ownerEpoch)return false;const auto&b=s.base.event;const auto&a=s.previewBaseEvent;const auto&e=s.previewEvent;return p->form()==s.form&&p->selection()==s.previewSelection&&p->serial()==s.previewSerial&&p->lifetime()==s.previewLifetime&&p->archive()==s.archive&&a.ownerEpoch==b.ownerEpoch&&a.sequence==b.sequence&&a.iteration==b.iteration&&e.ownerEpoch==s.ownerEpoch&&!e.iteration&&b.sequence<e.sequence&&e.sequence<s.scene.event.sequence;}
// Only exact original cached-upload call sites may claim this lexical scope.
// The unrelated slot1 upload near menu exit must never construct it.
struct CachedUploadSiteScope{State*s=nullptr;bool entered=false;explicit CachedUploadSiteScope(Game&g,CachedUploadSite site)noexcept{s=current(g);if(!s||s->invalid||!s->baseVerified||(site!=CachedUploadSite::Initial&&site!=CachedUploadSite::Refresh&&site!=CachedUploadSite::Lazy)||uint32_t(g.edx)!=1||uint32_t(g.eax)!=s->base.post.target.w[8]){s=nullptr;return;}if(s->cachedUploadDepth){s->fail(Reason::Nested);s=nullptr;return;}s->cachedUploadDepth=1;s->cachedUploadSite=site;entered=true;}~CachedUploadSiteScope(){if(entered){s->cachedUploadDepth=0;s->cachedUploadSite=CachedUploadSite::None;}}};
// Deliberately no publication until root wires actual direct343copy capture,
// post-copy Refresh swatch and separate original native upload witness.
inline void sealUpload(Game&g)noexcept;
struct UploadEndScope{Game*g=nullptr;State*s=nullptr;bool active=false;int unwind=0;explicit UploadEndScope(Game&game)noexcept{if(uint32_t(game.edx)!=1)return;s=current(game);if(!s||s->cachedUploadDepth!=1||s->cachedUploadSite==CachedUploadSite::None)return;g=&game;active=true;unwind=nfs_logo_publisher::exceptions();s->published.reset();}~UploadEndScope(){if(!active)return;if(nfs_logo_publisher::exceptions()>unwind)s->fail(Reason::Unwind);else sealUpload(*g);}};
} // namespace nfs_player_scene_live

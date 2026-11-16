// SPDX-License-Identifier: MIT
#pragma once
#include "ChildReturnPreludeAbi.h"
#include "ChildExitPreludeQueueAdapter.inc"
namespace nfs_child_return_live {
namespace old=nfs_child_exit_live;namespace bg=nfs_background_scene_cpu;namespace mt=nfs_menu_transition_live;
using Errno=old::Errno;using Life=old::Life;
struct Owner {
 std::shared_ptr<old::Owner> semantic;std::shared_ptr<Life> life;std::shared_ptr<const mt::Packet> bare,initial,cachedSide;
 std::shared_ptr<const nfs_setup_scene_cpu::Prepared>setupFrame;bool setupSidebarFromCaption=false;
 std::shared_ptr<const nfs_options_scene_cpu::Prepared>optionsFrame;bool optionsSidebarFromCaption=false;
 std::vector<uint8_t> sideRaw;std::array<uint16_t,bg::Full> side{};std::array<uint32_t,300>sideKeys{};
 NfsChildReturnPreludeView view{};std::atomic<unsigned>refs{1};bool sideSealed=false,sideAuthenticated=false,failed=false,sideUploading=false;unsigned sideTiles=0;
 static void retain(void*p){if(p)static_cast<Owner*>(p)->refs.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p){if(p&&static_cast<Owner*>(p)->refs.fetch_sub(1,std::memory_order_acq_rel)==1)delete static_cast<Owner*>(p);}
 static bool setupFrameCurrent(const Owner&o)noexcept{
  const auto&p=o.setupFrame;const auto&s=o.semantic;
  if(!p||!p->valid||!s||!s->full||!s->full->valid||!p->contentRevision||
     static_cast<const bg::Prepared*>(p.get())!=s->full.get()||p.owner_before(s->full)||s->full.owner_before(p)||
     o.view.route!=NFS_CHILD_RETURN_FADE||o.view.semantic.family!=NFS_CHILD_EXIT_SETUP||o.view.semantic.form!=0x4d2790u||
     !p->caption||!p->captionWord||!p->captionWord->current()||p->captionWord->packet()!=p->caption)return false;
  const auto&packet=*p->caption;const auto&surface=packet.surface();
  return surface.valid()&&surface.geometry[1]==343&&surface.geometry[2]==480&&
   packet.pixels()==bg::Side&&packet.original().size()==bg::Side&&p->captionLifetime&&
   surface.lifetime==p->captionLifetime&&p->captionSerial&&packet.serial()==p->captionSerial;
 }
 static bool optionsFrameCurrent(const Owner&o)noexcept{
  const auto&p=o.optionsFrame;const auto&s=o.semantic;
  if(!p||!p->valid||!s||!s->full||!s->full->valid||!p->contentRevision||
     static_cast<const bg::Prepared*>(p.get())!=s->full.get()||p.owner_before(s->full)||s->full.owner_before(p)||
     o.view.route!=NFS_CHILD_RETURN_FADE||o.view.semantic.family!=NFS_CHILD_EXIT_OPTIONS||o.view.semantic.form!=0x4d3a88u||
     !p->caption||!p->captionWord||!p->captionWord->current()||p->captionWord->packet()!=p->caption)return false;
  const auto&packet=*p->caption;const auto&surface=packet.surface();
  return surface.valid()&&surface.geometry[1]==343&&surface.geometry[2]==480&&
   packet.pixels()==bg::Side&&packet.original().size()==bg::Side&&p->captionLifetime&&
   surface.lifetime==p->captionLifetime&&p->captionSerial&&packet.serial()==p->captionSerial;
 }
 static unsigned invalidReason(void*p){const auto*o=static_cast<const Owner*>(p);
  if(!o)return 1;if(o->failed)return 2;if(!o->life||!o->life->alive.load(std::memory_order_acquire))return 3;if(!o->semantic)return 4;
  if(!o->initial&&(!o->semantic->full||!o->semantic->full->valid))return 5;
  if(!o->semantic->gameLife||!o->semantic->gameLife->current())return 6;
  if(!o->semantic->catalog||nfs_background_scene_runtime::acquireCatalog()!=o->semantic->catalog)return 7;
  if(!o->view.semantic.uploadEvent)return 8;if(nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)!=o->view.semantic.uploadEvent)return 9;
  if(o->cachedSide&&!mt::unchanged(*o->cachedSide))return 10;
  if(o->setupSidebarFromCaption&&!setupFrameCurrent(*o))return 24;
  if(o->optionsSidebarFromCaption&&!optionsFrameCurrent(*o))return 25;
  if(o->initial){const auto&p=*o->initial;
   if(p.kind==8){if(!p.setupInitialFull||!p.setupInitialFull->child)return 11;if(!p.setupInitialFull->child->current())return 12;}
   else if(p.kind==9){if(!p.locationInitialFull||!p.locationInitialFull->child)return 13;if(!p.locationInitialFull->child->current())return 14;if(!p.locationInitialFull->previewView().schema)return 15;}
   else if(p.kind==10){if(!p.opponentInitialFull||!p.opponentInitialFull->child)return 16;if(!p.opponentInitialFull->child->current())return 17;if(!p.opponentInitialFull->preview||(p.opponentInitialFull->art&&!p.opponentInitialFull->art->current()))return 18;}
   else return 19;
  }
  return o->view.route==NFS_CHILD_RETURN_FADE||(o->view.route==NFS_CHILD_RETURN_REVERSE&&o->sideSealed&&o->bare&&o->bare->life&&o->bare->life->current())?0:20;
 }
 static int current(void*p){return invalidReason(p)==0;}
};
inline bool framePhase(unsigned route,unsigned index,int32_t phase)noexcept{return route==1?index<12&&phase==140+10*int32_t(index):route==2&&index<19&&phase==-15*int32_t(index);}
inline std::shared_ptr<Owner>adopt(Owner*p){return std::shared_ptr<Owner>(p,[](Owner*q){Owner::release(q);});}
// Optional MT upload history is not required to identify actual slot0 textures.
// This Setup-fade-only route binds checked descriptors to independently
// authenticated sidebar bytes. GPU cached_tiles and the full300/600 native
// sidebar pass remain mandatory before any owned image can be committed.
inline bool bindMissingSetupFadeSidebar(Game&g,Owner&o)noexcept {
 try{
  const auto&v=o.view.semantic;auto sem=o.semantic;auto*source=nfs_setup_scene_live::current(g);
  const auto current=[&](){return source&&source==nfs_setup_scene_live::current(g)&&!source->invalid&&source->ownerEpoch==v.ownerEpoch&&source->game==v.sourceGame&&source->catalogue==sem->catalog&&source->archive==v.sourceArchive&&sem->gameLife&&sem->gameLife->current()&&v.sourceLife==sem->gameLife->serial()&&sem->catalog&&v.sourceCatalog==reinterpret_cast<uintptr_t>(sem->catalog.get())&&nfs_background_scene_runtime::acquireCatalog()==sem->catalog&&o.life&&o.life->alive.load(std::memory_order_acquire)&&v.uploadEvent&&nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)==v.uploadEvent;};
  if(!sem||o.view.route!=NFS_CHILD_RETURN_FADE||v.family!=NFS_CHILD_EXIT_SETUP||v.form!=0x4d2790u||sem->formGuest!=uint32_t(guest_address(off_4D2790))||uint32_t(g.esi)!=sem->formGuest||v.sourceGame!=reinterpret_cast<uintptr_t>(&g)||!sem->full||!sem->full->valid||!o.sideAuthenticated||o.sideRaw.size()!=16+bg::Side*2||!current())return false;
  // SAME typed full source owns the independently verified raw343 caption
  // packet. Use its original pixels BEFORE map/key compositing, not full.POST.
  if(!Owner::setupFrameCurrent(o))return false;
  const auto full=sem->full;const auto frame=o.setupFrame;const auto packet=frame->caption;
  std::fill(o.side.begin(),o.side.end(),uint16_t(0));
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x)
   o.side[y*640+x]=packet->original()[y*343+x];
  std::array<uint32_t,300>keys{};
  {nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=v.sourceArchive)return false;
   for(unsigned i=0;i<300;++i){const uint32_t d=tx.load<uint32_t>(dword_4EB74C+i*4);uint32_t w[7];
    if(!old::descriptor(tx,d,w)||w[0]!=3||w[1]!=3||w[2]!=3||w[3]!=11)return false;keys[i]=w[5];}
  }
  if(!current()||sem->full!=full||o.setupFrame!=frame||frame->caption!=packet||!Owner::setupFrameCurrent(o))return false;const uint64_t content=nfs_background_scene_live::nextRequest();if(!content)return false;
  o.setupSidebarFromCaption=true;o.sideAuthenticated=true;
  o.sideKeys=keys;o.sideSealed=true;o.view.semantic.sidebar565=o.side.data();o.view.semantic.sidebarWords=bg::Full;
  std::copy(keys.begin(),keys.end(),o.view.semantic.sidebarKeys);o.view.semantic.sidebarContent=content;return true;
 }catch(...){return false;}
}
// Options has its own typed full/caption owner; no cross-form fallback.
inline bool bindMissingOptionsFadeSidebar(Game&g,Owner&o)noexcept {
 try{
  const auto&v=o.view.semantic;auto sem=o.semantic;auto*source=nfs_options_scene_live::current(g);
  const auto current=[&](){return source&&source==nfs_options_scene_live::current(g)&&!source->invalid&&source->ownerEpoch==v.ownerEpoch&&source->game==v.sourceGame&&source->catalogue==sem->catalog&&source->archive==v.sourceArchive&&sem->gameLife&&sem->gameLife->current()&&v.sourceLife==sem->gameLife->serial()&&sem->catalog&&v.sourceCatalog==reinterpret_cast<uintptr_t>(sem->catalog.get())&&nfs_background_scene_runtime::acquireCatalog()==sem->catalog&&o.life&&o.life->alive.load(std::memory_order_acquire)&&v.uploadEvent&&nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)==v.uploadEvent;};
  if(!sem||o.view.route!=NFS_CHILD_RETURN_FADE||v.family!=NFS_CHILD_EXIT_OPTIONS||v.form!=0x4d3a88u||sem->formGuest!=uint32_t(guest_address(off_4D3A88))||uint32_t(g.esi)!=sem->formGuest||v.sourceGame!=reinterpret_cast<uintptr_t>(&g)||!sem->full||!sem->full->valid||!o.sideAuthenticated||o.sideRaw.size()!=16+bg::Side*2||!current())return false;
  // SAME typed full source owns the independently verified raw343 caption
  // packet. Use its original pixels BEFORE map/key compositing, not full.POST.
  if(!Owner::optionsFrameCurrent(o))return false;
  const auto full=sem->full;const auto frame=o.optionsFrame;const auto packet=frame->caption;
  std::fill(o.side.begin(),o.side.end(),uint16_t(0));
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x)
   o.side[y*640+x]=packet->original()[y*343+x];
  std::array<uint32_t,300>keys{};
  {nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=v.sourceArchive)return false;
   for(unsigned i=0;i<300;++i){const uint32_t d=tx.load<uint32_t>(dword_4EB74C+i*4);uint32_t w[7];
    if(!old::descriptor(tx,d,w)||w[0]!=3||w[1]!=3||w[2]!=3||w[3]!=11)return false;keys[i]=w[5];}
  }
  if(!current()||sem->full!=full||o.optionsFrame!=frame||frame->caption!=packet||!Owner::optionsFrameCurrent(o))return false;const uint64_t content=nfs_background_scene_live::nextRequest();if(!content)return false;
  o.optionsSidebarFromCaption=true;o.sideAuthenticated=true;
  o.sideKeys=keys;o.sideSealed=true;o.view.semantic.sidebar565=o.side.data();o.view.semantic.sidebarWords=bg::Full;
  std::copy(keys.begin(),keys.end(),o.view.semantic.sidebarKeys);o.view.semantic.sidebarContent=content;return true;
 }catch(...){return false;}
}

inline void note(unsigned r,const Owner*o=nullptr)noexcept{Errno e;static thread_local unsigned n;const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(p&&p[0]=='1'&&!p[1]&&n++<64)std::fprintf(stderr,"CHILD RETURN source reason=%u route=%u family=%u side=%u\n",r,o?o->view.route:0,o?o->view.semantic.family:0,o?o->sideTiles:0);}
/* Borrow only an independently current photo identity. Native100 receipts
 * remain mandatory; missing photo artwork never grants entry permission. */
inline void attachPhoto(Game&g,old::Owner&o)noexcept{
 if(o.view.family==NFS_CHILD_EXIT_LOCATION||o.view.family==NFS_CHILD_EXIT_OPPONENT){auto q=nfs_child_photo_live::published();if(q&&q->game==o.view.sourceGame&&q->archive==o.view.sourceArchive&&nfs_child_photo_live::current(g,*q)){o.childPhoto=q;o.view.photoFamily=q->family;o.view.photoSerial=q->serial;o.view.photoEpoch=q->epoch;}}
 else if(o.view.family==NFS_CHILD_EXIT_PLAYER1||o.view.family==NFS_CHILD_EXIT_PLAYER2){auto q=nfs_player_photo_live::published();if(q&&nfs_player_photo_live::current(g,*q)){o.playerPhoto=q;o.view.photoFamily=1;o.view.photoSerial=q->serial;o.view.photoEpoch=q->epoch;}}
}
// Default-off exact Options fullSource refusal diagnostics. No source/lease authority.
inline void optionsFullSourceRefusal(Game&g,nfs_options_scene_live::State*s)noexcept {
 Errno preserve;const char*t=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");static thread_local unsigned lines[28]{};
 if(!t||t[0]!='1'||t[1])return;auto*c=nfs_logo_publisher::store();unsigned reason=0;
 if(!s)reason=1;
 else if(s->invalid)reason=2;
 else if(!s->on)reason=3;
 else if(s->mainDepth!=1)reason=4;
 else if(!s->published)reason=5;
 else if(!s->published->valid)reason=6;
 else if(s->game!=reinterpret_cast<uintptr_t>(&g))reason=7;
 else if(s->thread!=nfs_logo_publisher::threadToken())reason=8;
 else if(!s->catalogue)reason=9;
 else if(!s->ownerEpoch)reason=10;
 else if(!s->archive)reason=11;
 else if(!s->nativeUploadEvent)reason=12;
 else if(s->nativeUploadEvent!=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire))reason=13;
 else if(!c)reason=14;
 else if(c->gameOwner!=s->game)reason=15;
 else if(c->thread!=s->thread)reason=16;
 else if(!c->copySourceVerified)reason=17;
 else if(!c->prepared)reason=18;
 else if(!c->bound)reason=19;
 else if(c->uploaded!=300)reason=20;
 else if(!c->copy.complete)reason=21;
 else if(!c->group.complete)reason=22;
 else if(c->copy.serial!=s->copySerial)reason=23;
 else if(c->group.serial!=s->groupSerial)reason=24;
 else if(c->nativeUploadEvent!=s->nativeUploadEvent)reason=25;
 else if(s->published->original!=c->packet.originalCopyPost565)reason=26;
 else if(s->published->keys!=c->rendererAddresses)reason=27;
 if(reason>=28||lines[reason]>=8)return;++lines[reason];std::fprintf(stderr,"OPTIONS RETURN FULLSOURCE refusal=%u epoch=%llu iteration=%llu published=%u invalid=%u stage=%u stateReason=%u stateUpload=%llu liveUpload=%llu copy=%llu group=%llu collectorCopy=%llu collectorGroup=%llu collectorRevision=%llu collectorDepth=%u invocationDepth=%u\n",reason,(unsigned long long)(s?s->ownerEpoch:0),(unsigned long long)(s?s->iteration:0),unsigned(s&&s->published),unsigned(s&&s->invalid),s?s->stage:0,s?unsigned(s->reason):0,(unsigned long long)(s?s->nativeUploadEvent:0),(unsigned long long)nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire),(unsigned long long)(s?s->copySerial:0),(unsigned long long)(s?s->groupSerial:0),(unsigned long long)(c?c->copy.serial:0),(unsigned long long)(c?c->group.serial:0),(unsigned long long)(c?c->iterationRevision:0),c?c->depth:0,s?s->invocationDepth:0);
}

inline std::shared_ptr<old::Owner>captureSemantic(Game&g,uint64_t session,const std::shared_ptr<Life>&life,std::shared_ptr<const nfs_setup_scene_cpu::Prepared>&setupFrameOut,std::shared_ptr<const nfs_options_scene_cpu::Prepared>&optionsFrameOut)noexcept{Errno e;try{
 std::shared_ptr<const nfs_setup_scene_cpu::Prepared>capturedSetup;
 std::shared_ptr<const nfs_options_scene_cpu::Prepared>capturedOptions;
 auto o=old::adopt(new old::Owner);struct Trace{old::Owner*o;bool ok=false;~Trace(){if(!ok)old::note(4,o);}}trace{o.get()};o->captureStage=1;o->life=life;o->view.schema=1;o->view.session=session;const uint32_t f=uint32_t(g.esi);o->formGuest=f;
 if(f==uint32_t(guest_address(off_4D2790))){
  auto*s=nfs_setup_scene_live::current(g);if(!s)return {};
  const auto published=s->published;
  if(old::fullSource(s,published,g,*o,NFS_CHILD_EXIT_SETUP,0x4d2790)){capturedSetup=published;o->view.setup=published->view();}
  else {
   auto uploaded=nfs_setup_uploaded_source::pendingForReturn(g,*s);if(!uploaded)return {};
   // Transfer immutable completed-upload content to this exact return session.
   // The return renderer still proves the actual full300/600 native pass; no
   // partial current collector or older cached widget packet is adopted.
   capturedSetup=uploaded->frame;
   o->typed=uploaded;o->full=uploaded->frame;o->catalog=uploaded->catalog;
   o->authenticatedSidebar=uploaded->sidebarHistory;
   o->view.family=NFS_CHILD_EXIT_SETUP;o->view.form=0x4d2790;
   o->view.ownerEpoch=uploaded->domain.ownerEpoch;o->view.sourceGame=uploaded->domain.game;
   o->view.sourceCatalog=uploaded->domain.catalog;o->view.sourceArchive=uploaded->domain.archive;
   o->view.uploadEvent=uploaded->domain.uploadEvent;o->view.setup=uploaded->frame->view();
  }
  if(!o->view.setup.schema)return {};
 }
 else if(f==uint32_t(guest_address(off_4D3A88))){auto*s=nfs_options_scene_live::current(g);if(!s||!old::fullSource(s,s->published,g,*o,NFS_CHILD_EXIT_OPTIONS,0x4d3a88)){optionsFullSourceRefusal(g,s);return {};}capturedOptions=s->published;o->view.options=capturedOptions->view();if(!o->view.options.schema)return {};}
 else if(f==uint32_t(guest_address(off_4D2A20))){auto*s=nfs_location_scene_live::current(g);if(!s||!old::fullSource(s,s->published,g,*o,NFS_CHILD_EXIT_LOCATION,0x4d2a20))return {};o->view.location=s->published->view();if(!o->view.location.schema)return {};}
 else if(f==uint32_t(guest_address(off_4D3204))){auto*s=nfs_opponent_scene_live::current(g);if(!s||!old::fullSource(s,s->published,g,*o,NFS_CHILD_EXIT_OPPONENT,0x4d3204))return {};o->view.opponent=s->published->view();if(!o->view.opponent.schema)return {};}
 else if(nfs_player_scene_live::canonicalForm(f)){auto*s=nfs_player_scene_live::current(g);const uint32_t c=nfs_player_scene_live::canonicalForm(f);const auto a=nfs_player_scene_live::accepted();auto*source=nfs_player_source_live::slot();if(!s||s->invalid||s->form!=c||s->formGuest!=f||!s->published||a.owner!=s||a.epoch!=s->ownerEpoch||!a.proof||a.prepared!=s->published||!source)return {};nfs_player_native::CurrentBinding binding{};auto proof=nfs_player_native_live::proofForPresentation(g,binding,source->upload);if(!proof||proof!=a.proof||!proof->current(binding)||!proof->source(binding))return {};o->typed=a.prepared;o->full=a.prepared;o->catalog=s->catalogue;o->playerNative=proof;o->playerBinding=binding;o->view.family=c==0x4d3050?NFS_CHILD_EXIT_PLAYER1:NFS_CHILD_EXIT_PLAYER2;o->view.form=c;o->view.ownerEpoch=s->ownerEpoch;o->view.sourceGame=s->game;o->view.sourceCatalog=reinterpret_cast<uintptr_t>(s->catalogue.get());o->view.sourceArchive=binding.source.archive;o->view.uploadEvent=proof->uploadEvent(binding);o->view.player=a.prepared->view();if(!o->view.player.schema)return {};}
 else return {};
 o->captureStage=2;o->gameLife=nfs_main_sidebar_origin::acquireGameLife(o->view.sourceGame);if(!o->gameLife)return {};o->view.sourceLife=o->gameLife->serial(); o->captureStage=4; // Actualphoto100 is captured only for moving exit frames, never required by first2 full-only clears.
 attachPhoto(g,*o);
 o->captureStage=5;o->view.owner=o.get();o->view.retain=old::Owner::retain;o->view.release=old::Owner::release;o->view.current=old::Owner::current;setupFrameOut=capturedSetup;optionsFrameOut=capturedOptions;trace.ok=true;return o;
 }catch(...){return {};}}

inline bool initialSemantic(Game&g,Owner&o,uint64_t serial)noexcept{try{
 auto p=mt::state().slots[1];if(!p||p->slot!=1||p->kind<8||p->kind>10||p->game!=reinterpret_cast<uintptr_t>(&g)||p->thread!=nfs_logo_publisher::threadToken()||!p->catalog||!p->life||!p->life->current()||!mt::unchanged(*p))return false;
 auto sem=old::adopt(new old::Owner);auto&v=sem->view;v.schema=1;v.session=serial;v.sourceGame=p->game;v.sourceCatalog=reinterpret_cast<uintptr_t>(p->catalog.get());v.sourceArchive=p->archive;v.sourceLife=p->life->serial();v.uploadEvent=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire);sem->catalog=p->catalog;sem->gameLife=p->life;sem->life=o.life;sem->formGuest=uint32_t(g.esi);
 NfsBackgroundSceneFrameView b{};b.schema=1;b.width=640;b.height=480;b.mapRole=p->mapRole;b.wordCount=p->layers.wordCount;b.serial=serial;b.contentRevision=p->content;b.planeWords=bg::Full;b.original565=p->original.data();std::copy(p->addresses.begin(),p->addresses.end(),b.rendererAddress);std::copy(p->layers.panelClip,p->layers.panelClip+4,b.panelClip);std::copy(p->layers.words,p->layers.words+4,b.words);std::copy(p->layers.logos,p->layers.logos+3,b.logos);
 const bg::Plane*side=nullptr;
 if(p->kind==8){auto q=p->setupInitialFull;if(!q||!q->child||!q->child->current()||!nfs_setup_initial_full_live::owned(g,*q)||!nfs_setup_initial_full_live::rawMatches(*q,p->raw))return false;v.form=q->form;v.ownerEpoch=q->epoch;side=&q->child->post();if(q->form==0x4d2790){v.family=NFS_CHILD_EXIT_SETUP;v.setup.schema=NFS_SETUP_SCENE_RUNTIME_SCHEMA;b.tailKind=4;v.setup.scene=b;v.setup.captionWord=q->child->wordView();}else if(q->form==0x4d3a88){v.family=NFS_CHILD_EXIT_OPTIONS;v.options.schema=NFS_OPTIONS_SCENE_RUNTIME_SCHEMA;b.tailKind=3;v.options.scene=b;v.options.captionWord=q->child->wordView();}else return false;}
 else if(p->kind==9){auto q=p->locationInitialFull;if(!q||!q->child||!q->child->current()||!nfs_location_initial_full_live::owned(g,*q))return false;v.form=0x4d2a20;v.family=NFS_CHILD_EXIT_LOCATION;v.ownerEpoch=q->epoch;side=&q->child->post();v.location.schema=NFS_LOCATION_SCENE_RUNTIME_SCHEMA;b.tailKind=2;v.location.scene=b;v.location.captionWord=q->child->view().caption;v.location.preview=q->previewView();if(!v.location.preview.schema)return false;}
 else {auto q=p->opponentInitialFull;if(!q||!q->child||!q->child->current()||!nfs_opponent_initial_full_live::owned(g,*q))return false;v.form=0x4d3204;v.family=NFS_CHILD_EXIT_OPPONENT;v.ownerEpoch=q->epoch;side=&q->child->post();v.opponent.schema=NFS_OPPONENT_SCENE_RUNTIME_SCHEMA;b.tailKind=6;v.opponent.scene=b;v.opponent.caption=q->child->view().caption;v.opponent.preview=q->previewView();if(!v.opponent.preview.schema)return false;}
 const uint32_t expected=v.form==0x4d2790?uint32_t(guest_address(off_4D2790)):v.form==0x4d3a88?uint32_t(guest_address(off_4D3A88)):v.form==0x4d2a20?uint32_t(guest_address(off_4D2A20)):v.form==0x4d3204?uint32_t(guest_address(off_4D3204)):0;if(!expected||uint32_t(g.esi)!=expected||!side||!bg::plane(*side,343)||!v.uploadEvent||p->raw.size()!=16+bg::Full*2)return false;for(unsigned k=0;k<bg::Full;++k)if(bg::le16(p->raw.data()+16+2*k)!=p->original[k])return false;
 o.sideRaw.resize(16+bg::Side*2);for(unsigned k=0;k<4;++k)for(unsigned byte=0;byte<4;++byte)o.sideRaw[k*4+byte]=uint8_t(side->target.header[k]>>(8*byte));for(unsigned k=0;k<bg::Side;++k){o.sideRaw[16+2*k]=uint8_t(side->pixels[k]);o.sideRaw[17+2*k]=uint8_t(side->pixels[k]>>8);}for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x)o.side[y*640+x]=side->pixels[y*343+x];
 attachPhoto(g,*sem);o.semantic=sem;o.initial=p;o.sideAuthenticated=true;return true;
 }catch(...){return false;}}
inline void offerTrackInfoSidebar(const std::shared_ptr<Owner>& owner) {
 if(!owner||!owner->sideSealed||!owner->sideAuthenticated||
    owner->view.route!=NFS_CHILD_RETURN_REVERSE||!owner->semantic||
    owner->view.semantic.family!=NFS_CHILD_EXIT_LOCATION||
    owner->view.semantic.form!=0x4d2a20u||!owner->view.semantic.sidebarContent||
    !owner->semantic->gameLife||!owner->semantic->gameLife->current())return;
 const auto& location=owner->view.semantic.location;
 if(location.schema!=NFS_LOCATION_SCENE_RUNTIME_SCHEMA||
    !location.captionWord.schema||location.scene.wordCount>4)return;

 auto proof=std::make_shared<mt::OutgoingLocationSidebar>();
 proof->owner=owner;
 proof->catalog=owner->semantic->catalog;
 proof->life=owner->semantic->gameLife;
 proof->game=owner->view.semantic.sourceGame;
 proof->thread=nfs_logo_publisher::threadToken();
 proof->archive=owner->view.semantic.sourceArchive;
 proof->content=owner->view.semantic.sidebarContent;
 proof->original=owner->side;
 proof->keys=owner->sideKeys;
 proof->caption=location.captionWord;
 proof->layers.mapRole=location.scene.mapRole;
 proof->layers.wordCount=location.scene.wordCount;
 proof->layers.panelClip[2]=343;
 proof->layers.panelClip[3]=480;
 std::copy_n(location.scene.words,4,proof->layers.words);
 std::copy_n(location.scene.logos,3,proof->layers.logos);
 mt::outgoingLocationSidebar()=std::move(proof);
 mt::fullTrace("track-info-outgoing-sidebar","sealed",13,0);
}

class Session;inline Session*&session(){static thread_local Session*p=nullptr;return p;}
class Frame;inline Frame*&active(){static thread_local Frame*p=nullptr;return p;}
class Session {
 Game&g_;std::shared_ptr<Owner>owner_;unsigned frames_=0;
public:
 explicit Session(Game&g):g_(g){}~Session(){Errno e;if(session()==this)session()=nullptr;if(owner_){auto life=owner_->life;try{nfs::ScopedWorkerStopDeferral d;(void)nfs::enqueue_owned_renderer([life]{life->alive.store(false,std::memory_order_release);return int32_t(1);});}catch(...){life->alive.store(false,std::memory_order_release);}}}
 void arm(unsigned route)noexcept{Errno e;note(30);if(owner_||session())return;
  mt::outgoingLocationSidebar().reset();
  try{
  auto o=adopt(new Owner);o->life=std::make_shared<Life>();const auto id=nfs_background_scene_live::nextRequest();o->semantic=captureSemantic(g_,id,o->life,o->setupFrame,o->optionsFrame);
  // Selecting an information screen or leaving Opponents can interrupt
  // a fresh CPU redraw. Require the current scoped accepted-upload
  // record, including the actual slot descriptors and upload event.
  const uint32_t form=uint32_t(g_.esi);
  if(!o->semantic&&(form==uint32_t(guest_address(off_4D3204))||
                    form==uint32_t(guest_address(off_4D2A20)))){
   o->semantic=old::captureHistory(g_,form,id,o->life);
   if(o->semantic){
    // History authenticates the completed sidebar and full scene. The
    // independently current moving photo still needs its own receipt.
    attachPhoto(g_,*o->semantic);
    note(32);
   }
  }
  if(!o->semantic&&!initialSemantic(g_,*o,id)){note(1);return;}
  auto&v=o->view;v.schema=1;v.route=route;v.semantic=o->semantic->view;v.semantic.owner=o.get();v.semantic.retain=Owner::retain;v.semantic.release=Owner::release;v.semantic.current=Owner::current;note(31,o.get());
  v.minimum=route==1?140:0;v.maximum=route==1?230:255;v.direction=0;v.originX=v.originY=0;v.stepX=32;v.rgba=route==NFS_CHILD_RETURN_REVERSE?0xff8c8c8c:0;
  v.baseWords=bg::Full;if(o->initial){v.baseOriginal565=o->initial->original.data();std::copy(o->initial->addresses.begin(),o->initial->addresses.end(),v.baseKeys);v.baseMapRole=o->initial->mapRole;v.baseContent=o->initial->content;}else{v.baseOriginal565=o->semantic->full->original.data();std::copy(o->semantic->full->keys.begin(),o->semantic->full->keys.end(),v.baseKeys);v.baseMapRole=o->semantic->full->mapRole;v.baseContent=o->semantic->full->contentRevision;}
  if(!o->initial){
   const nfs_logo_capture::Copy*copy=nullptr;if(v.semantic.family>=NFS_CHILD_EXIT_PLAYER1){auto*s=nfs_player_scene_live::current(g_);if(s&&s->currentCopy)copy=s->currentCopy.get();}else{auto*c=nfs_logo_publisher::store();if(c&&c->copySourceVerified&&c->copy.complete)copy=&c->copy;}
   if(o->semantic->history&&
      (v.semantic.family==NFS_CHILD_EXIT_OPPONENT||v.semantic.family==NFS_CHILD_EXIT_LOCATION)&&
      o->semantic->authenticatedSidebar&&o->semantic->authenticatedSidebar->size()==16+bg::Side*2){
    // Same accepted-upload history as semantic.full, never a newer unuploaded CPUcopy.
    o->sideRaw=*o->semantic->authenticatedSidebar;note(29,o.get());
   }else if(copy&&!copy->error&&copy->complete&&copy->span==16+bg::Side*2&&copy->span<=copy->sourceBefore.size()&&copy->sourceBefore==copy->sourceAfter)
    o->sideRaw.assign(copy->sourceBefore.begin(),copy->sourceBefore.begin()+copy->span);
   else if(!copy&&o->semantic->authenticatedSidebar&&o->semantic->authenticatedSidebar->size()==16+bg::Side*2){
    // Historical bytes came from the SAME fully authenticated completed upload.
    // Fade still requires an exact current slot0 match; reverse still verifies
    // the actual subsequent source upload byte-for-byte and all native tiles.
    o->sideRaw=*o->semantic->authenticatedSidebar;note(26,o.get());
   }else{note(2,o.get());return;}
   for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x)o->side[y*640+x]=bg::le16(o->sideRaw.data()+16+2*(y*343+x));
   o->sideAuthenticated=true;
  }
  if(route==NFS_CHILD_RETURN_FADE){auto side=mt::state().slots[0];
   if(!side){if(bindMissingSetupFadeSidebar(g_,*o))note(27,o.get());else if(bindMissingOptionsFadeSidebar(g_,*o))note(28,o.get());else{note(21,o.get());return;}}
   else {
    if(side->slot){note(22,o.get());return;}
    if(side->raw!=o->sideRaw){note(23,o.get());return;}
    if(side->game!=v.semantic.sourceGame||side->thread!=nfs_logo_publisher::threadToken()||side->archive!=v.semantic.sourceArchive||side->catalog!=o->semantic->catalog){note(24,o.get());return;}
    if(!mt::unchanged(*side)){note(25,o.get());return;}
    o->sideKeys=side->addresses;o->sideSealed=true;v.semantic.sidebar565=o->side.data();v.semantic.sidebarWords=bg::Full;std::copy(o->sideKeys.begin(),o->sideKeys.end(),v.semantic.sidebarKeys);v.semantic.sidebarContent=side->content;
   }
  }else if(route!=NFS_CHILD_RETURN_REVERSE)return;
  owner_=o;session()=this;note(0,o.get());
 }catch(...){note(3);}}
 std::shared_ptr<Owner>owner(){return owner_;}
 void uploaded()noexcept{Errno e;if(!owner_||owner_->view.route!=NFS_CHILD_RETURN_REVERSE)return;auto&o=*owner_;if(o.bare)return;auto p=mt::state().slots[1];const auto event=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire);
  if(!o.sideSealed||!p||p->kind!=1||p->slot!=1||p->catalog!=o.semantic->catalog||p->game!=o.view.semantic.sourceGame||p->thread!=nfs_logo_publisher::threadToken()||p->archive!=o.view.semantic.sourceArchive||!event||event==o.view.semantic.uploadEvent||!mt::unchanged(*p)){o.failed=true;note(4,&o);return;}
  o.bare=p;o.view.semantic.uploadEvent=event;o.view.baseOriginal565=p->original.data();o.view.baseWords=bg::Full;std::copy(p->addresses.begin(),p->addresses.end(),o.view.baseKeys);o.view.baseMapRole=p->mapRole;o.view.baseContent=p->content;o.view.semantic.sidebar565=o.side.data();o.view.semantic.sidebarWords=bg::Full;std::copy(o.sideKeys.begin(),o.sideKeys.end(),o.view.semantic.sidebarKeys);o.view.semantic.sidebarContent=nfs_background_scene_live::nextRequest();note(5,&o);
  try { offerTrackInfoSidebar(owner_); } catch (...) { mt::outgoingLocationSidebar().reset(); }
 }
 std::shared_ptr<Owner>frame(unsigned route,int32_t phase){Errno e;if(!owner_||!Owner::current(owner_.get())||owner_->view.route!=route||!framePhase(route,frames_,phase))return {};owner_->view.frameIndex=frames_++;return owner_;}
};
struct SidebarUploadScope {std::shared_ptr<Owner>owner;std::shared_ptr<const mt::Packet>previous;uint32_t source=0;bool active=false;int unwind=0;explicit SidebarUploadScope(Game&g)noexcept{Errno e;auto*s=session();if(!s)return;owner=s->owner();if(!owner||owner->view.route!=2||uint32_t(g.edx)!=0)return;try{nfs::GuestCopyTransaction tx;const uint32_t at=uint32_t(g.eax);if(!at){note(6,owner.get());throw 1;}if(owner->sideRaw.empty()){note(7,owner.get());throw 1;}if(std::memcmp(nfs_host_address(at,owner->sideRaw.size()),owner->sideRaw.data(),owner->sideRaw.size())){
 note(8,owner.get());const char*trace=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");static thread_local unsigned mismatchNotes;
 if(trace&&trace[0]=='1'&&!trace[1]&&mismatchNotes++<4){const auto*actual=static_cast<const uint8_t*>(nfs_host_address(at,owner->sideRaw.size()));const uint32_t wh=tx.load<uint32_t>(at+4);size_t first=0;while(first<owner->sideRaw.size()&&actual[first]==owner->sideRaw[first])++first;std::fprintf(stderr,"CHILD RETURN sidebar-mismatch initial=%u family=%u actualWidth=%u actualHeight=%u expectedBytes=%zu headerEqual=%u pixelsEqual=%u firstOffset=%zu\n",unsigned(bool(owner->initial)),owner->view.semantic.family,wh&65535,wh>>16,owner->sideRaw.size(),unsigned(!std::memcmp(actual,owner->sideRaw.data(),16)),unsigned(!std::memcmp(actual+16,owner->sideRaw.data()+16,owner->sideRaw.size()-16)),first);}
 throw 1;}source=at;previous=mt::state().slots[0];owner->sideTiles=0;unwind=nfs_logo_publisher::exceptions();owner->sideUploading=true;active=true;}catch(...){owner->failed=true;}}~SidebarUploadScope(){Errno e;if(active){owner->sideUploading=false;
 /* The original upload may early-return on a cached source. MT restores the
  * exact prior Packet only after verifying that same no-tile path; its scope
  * finishes before this outer scope. GPU still verifies all native tiles. */
 if(!owner->failed&&!owner->sideTiles&&nfs_logo_publisher::exceptions()==unwind&&previous&&mt::state().slots[0]==previous&&previous->slot==0&&previous->source==source&&previous->game==owner->view.semantic.sourceGame&&previous->thread==nfs_logo_publisher::threadToken()&&previous->catalog==owner->semantic->catalog&&previous->archive==owner->view.semantic.sourceArchive&&previous->raw==owner->sideRaw&&previous->original==owner->side&&mt::unchanged(*previous)){owner->cachedSide=previous;owner->sideKeys=previous->addresses;owner->sideSealed=true;note(11,owner.get());}
 else owner->sideSealed=!owner->failed&&owner->sideTiles==300&&nfs_logo_publisher::exceptions()==unwind;if(!owner->sideSealed){note(owner->sideTiles?10:9,owner.get());owner->failed=true;}}}};
inline bool tileMatches(const std::array<uint16_t,bg::Full>&original,const std::array<uint32_t,300>&keys,unsigned index,const nfs_logo_publisher::TileObservation&t)noexcept{if(index>=300)return false;bool ok=true;ok=ok&&t.descriptorGuest&&t.descriptorGuest==t.expectedSlotDescriptor&&!t.tableGuest&&t.before[0]==3&&t.before[1]==3&&t.before[2]==3&&t.before[3]==11&&t.before[6]==3&&t.after[4]==t.payloadGuest&&(t.after[5]&3)==0&&t.after[5]<=0x200000u-2048u;for(unsigned k=0;k<7;++k)if(k!=4&&t.before[k]!=t.after[k])ok=false;for(unsigned k=0;k<index;++k)if(uint64_t(keys[k])<uint64_t(t.after[5])+2048&&uint64_t(t.after[5])<uint64_t(keys[k])+2048)ok=false;unsigned x=(index/15)*32,y=(index%15)*32;for(unsigned yy=0;yy<32;++yy)for(unsigned xx=0;xx<32;++xx)if(bg::le16(t.pixels.data()+2*(yy*32+xx))!=nfs_logo_publisher::to1555(original[(y+yy)*640+x+xx]))ok=false;return ok;}
struct SidebarTileScope{std::shared_ptr<Owner>s;nfs_logo_publisher::TileObservation t{};unsigned index=0;int unwind=0;bool active=false;explicit SidebarTileScope(Game&g)noexcept{Errno preserve;if(!session())return;s=session()->owner();if(!s||s->view.route!=2||!s->sideUploading||s->failed||s->sideSealed||s->sideTiles>=300)return;index=s->sideTiles;unwind=nfs_logo_publisher::exceptions();try{nfs::GuestCopyTransaction tx;uint32_t sp=uint32_t(g.esp);t.descriptorGuest=tx.load<uint32_t>(sp);t.payloadGuest=tx.load<uint32_t>(sp+4);t.tableGuest=tx.load<uint32_t>(sp+8);t.expectedSlotDescriptor=tx.load<uint32_t>(dword_4EB74C+0*1200+index*4);nfs_logo_publisher::descriptor(tx,t.descriptorGuest,t.before);nfs_logo_publisher::checked(t.payloadGuest,2048);std::memcpy(t.pixels.data(),nfs_host_address(t.payloadGuest,2048),2048);active=true;}catch(...){s->failed=true;}}
 ~SidebarTileScope(){Errno preserve;if(!active)return;bool ok=nfs_logo_publisher::exceptions()==unwind;try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::descriptor(tx,t.descriptorGuest,t.after);if(std::memcmp(t.pixels.data(),nfs_host_address(t.payloadGuest,2048),2048))ok=false;}catch(...){ok=false;}auto p=s;if(!p){s->failed=true;return;}ok=ok&&tileMatches(p->side,p->sideKeys,index,t);if(!ok){s->failed=true;return;}p->sideKeys[index]=t.after[5];++s->sideTiles;}};
class Frame final{
 Game&g_;std::shared_ptr<Owner>owner_;uint64_t serial_=0;unsigned stage_=0,section_=0,ordinal_=0;bool accepted_=false;uint32_t selectedDescriptor_=0,selected_[7]{};NfsChildReturnQuad texture_{};uint32_t resourceId_=0,resourceCategory_=0;int32_t resourceAnchor_=0,resourceY_=0;
 void cancel()noexcept{Errno e;if(!accepted_)return;accepted_=false;try{auto keep=owner_;const auto serial=serial_;nfs::ScopedWorkerStopDeferral d;(void)nfs::enqueue_owned_renderer([keep,serial]{return int32_t(nfs_private_child_return_cancel(serial));});}catch(...){}}
 bool send(unsigned stage,int32_t x=0,int32_t y=0){Errno e;if(!accepted_||!owner_||!Owner::current(owner_.get()))return false;auto keep=owner_;const auto serial=serial_;const auto r=nfs::enqueue_owned_renderer([keep,serial,stage,x,y]{if(!Owner::current(keep.get()))return int32_t(nfs_private_child_return_cancel(serial));return int32_t(nfs_private_child_return_stage(serial,stage,x,y));});if(r!=nfs::OwnedRendererAcceptance::Consumed)return false;stage_=stage;return true;}
public:Frame(Game&g,Session&s,unsigned route):g_(g){Errno e;if(active())return;const int32_t motion=route==1?int32_t(g.esi):int32_t(int16_t(g.esi));owner_=s.frame(route,motion);if(!owner_){note(12,s.owner().get());return;}serial_=nfs_background_scene_live::nextRequest();if(!serial_)return;auto keep=owner_;auto v=owner_->view;v.semantic.serial=serial_;v.phase=route==1?motion:-1;v.semantic.motion=motion;v.semantic.clearedFrame=1;if(route==2){if(!old::photoDescriptorsView(v.semantic,true)){note(13,owner_.get());return;}v.semantic.photoDescriptorCount=100;}switch(v.semantic.family){case NFS_CHILD_EXIT_SETUP:v.semantic.setup.scene.serial=serial_;break;case NFS_CHILD_EXIT_OPTIONS:v.semantic.options.scene.serial=serial_;break;case NFS_CHILD_EXIT_LOCATION:v.semantic.location.scene.serial=serial_;break;case NFS_CHILD_EXIT_OPPONENT:v.semantic.opponent.scene.serial=serial_;break;default:v.semantic.player.scene.serial=serial_;break;}const auto r=nfs::enqueue_owned_renderer([keep,v]{const unsigned reason=Owner::invalidReason(keep.get());if(reason){note(32+reason,keep.get());return int32_t(0);}return int32_t(nfs_private_child_return_begin(&v));});accepted_=r==nfs::OwnedRendererAcceptance::Consumed;note(accepted_?15:14,owner_.get());if(accepted_)active()=this;}
 void stage(unsigned stage,int32_t x=0,int32_t y=0){if(accepted_)(void)send(stage,x,y);}void section(unsigned start){if(!accepted_||section_)return;section_=start;ordinal_=0;selectedDescriptor_=0;(void)send(start,start==NFS_CHILD_RETURN_PHOTO_BEGIN?int32_t(g_.eax):0,start==NFS_CHILD_RETURN_PHOTO_BEGIN?int32_t(g_.edx):0);}void sectionEnd(unsigned end){if(!accepted_||!section_)return;(void)send(end);section_=0;selectedDescriptor_=0;}void ready(){stage(NFS_CHILD_RETURN_READY);}void end(){stage(NFS_CHILD_RETURN_END);}
 void resourceBegin(Game&g)noexcept{Errno e;if(!accepted_)return;if(resourceId_||uint32_t(g.eax)<0x58||uint32_t(g.eax)>0x5b){cancel();return;}resourceId_=uint32_t(g.eax);resourceCategory_=uint32_t(g.ecx);resourceAnchor_=int32_t(g.edx);resourceY_=int32_t(g.ebx);}
 void resourceEnd()noexcept{resourceId_=resourceCategory_=0;resourceAnchor_=resourceY_=0;}
 void selected(Game&g)noexcept{Errno e;if(!accepted_||!section_)return;try{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);NfsChildReturnQuad q{};q.tmu=tx.load<uint32_t>(sp);q.address=tx.load<uint32_t>(sp+4);q.mask=tx.load<uint32_t>(sp+8);q.descriptor=tx.load<uint32_t>(sp+12);if(q.tmu||q.mask!=3||!old::descriptor(tx,q.descriptor,q.descriptorWords)||q.address!=q.descriptorWords[5])throw 1;q.descriptorWords[4]=0;texture_=q;selectedDescriptor_=q.descriptor;}catch(...){cancel();}}
 void quad(Game&g)noexcept{Errno e;if(!accepted_||!section_)return;if(!selectedDescriptor_||ordinal_>=2048){cancel();return;}try{NfsChildReturnQuad q=texture_;q.serial=serial_;q.section=section_;q.ordinal=ordinal_++;q.resourceId=resourceId_;q.resourceCategory=resourceCategory_;q.resourceAnchor=resourceAnchor_;q.resourceY=resourceY_;{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp),a=tx.load<uint32_t>(sp),b=tx.load<uint32_t>(sp+4),c=tx.load<uint32_t>(sp+8);if(uint64_t(sp)+12!=a||uint64_t(a)+60!=b||uint64_t(a)+120!=c||uint64_t(a)+240>UINT64_C(0x100000000))throw 1;std::memcpy(q.nativeVertices,nfs_host_address(a,240),240);for(unsigned k=0;k<4;++k){const uint32_t index=tx.load<uint32_t>(uint32_t(g.esi)+4*k);if(index>4095)throw 1;q.indices[k]=index;const uint64_t at=uint64_t(uint32_t(g.edi))+32*uint64_t(index);if(at+32>UINT64_C(0x100000000))throw 1;std::memcpy(q.sourceVertices[k],nfs_host_address(uint32_t(at),32),32);}}auto keep=owner_;(void)nfs::enqueue_owned_renderer([keep,q]{return int32_t(Owner::current(keep.get())?nfs_private_child_return_quad(&q):nfs_private_child_return_cancel(q.serial));});}catch(...){cancel();}}
 ~Frame(){Errno e;if(active()==this)active()=nullptr;if(accepted_&&stage_!=NFS_CHILD_RETURN_END)try{const auto serial=serial_;auto keep=owner_;nfs::ScopedWorkerStopDeferral d;(void)nfs::enqueue_owned_renderer([keep,serial]{return int32_t(nfs_private_child_return_cancel(serial));});}catch(...){}}
};

inline void selected(Game&g)noexcept{if(auto*p=active())p->selected(g);}
inline void nativeQuad(Game&g)noexcept{if(auto*p=active())p->quad(g);}
} // namespace nfs_child_return_live

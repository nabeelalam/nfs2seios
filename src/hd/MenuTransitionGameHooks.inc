// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundSceneQueueAdapter.inc"
#include "MenuTransitionAbi.h"
#include "MenuInitialSidebarGameHooks.inc"
#include "ChildInitialSidebarGameHooks.inc"
#include "SetupInitialFullGameHooks.inc"
#include "LocationInitialFullGameHooks.inc"
#include "ChildStaticInitialFullHistory.inc"
#include "OpponentInitialFullGameHooks.inc"
#include "OpponentInitialSidebarGameHooks.inc"
#include "LocationInitialSidebarGameHooks.inc"
#include "PlayerSceneQueueAdapter.inc"
#include "PlayerInitialNativeUploadGameHooks.inc"
#include "PlayerIntroPrepare.h"
#include "ClassicScreens/TrackInfoGameHooks.inc"
namespace nfs_menu_transition_live {
namespace bg=nfs_background_scene_cpu;
struct WorkerErrno{int saved=errno;~WorkerErrno()noexcept{errno=saved;}};
inline void fullTrace(const char*stage,const char*reason,unsigned kind,unsigned slot,unsigned a=0,unsigned b=0,unsigned c=0)noexcept{WorkerErrno preserve;const char*on=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(!on||on[0]!='1'||on[1])return;struct Key{const char*stage;const char*reason;unsigned kind,slot,count;};static thread_local Key keys[64];static thread_local unsigned used=0,total=0;if(total>=128)return;unsigned k=0;for(;k<used;++k)if(!std::strcmp(keys[k].stage,stage)&&!std::strcmp(keys[k].reason,reason)&&keys[k].kind==kind&&keys[k].slot==slot)break;if(k==used){if(used==64)return;keys[used++]={stage,reason,kind,slot,0};}if(keys[k].count++>=2)return;++total;std::fprintf(stderr,"INITIAL FULL MT stage=%s reason=%s kind=%u slot=%u a=%u b=%u c=%u\n",stage,reason,kind,slot,a,b,c);}
struct Packet{nfs_track_info_ui_live::EntryArtSource trackInfoArt{};std::shared_ptr<const bg::SourceCatalog>catalog;uint64_t content=0;uintptr_t game=0,thread=0;uint32_t archive=0,source=0,slot=0,kind=0,mapRole=0;std::vector<uint8_t>raw;std::array<uint16_t,307200>original{};std::array<uint32_t,300>addresses{};NfsBackgroundSceneLayers layers{};bg::Target sourceTarget{};bool targetBound=false;std::shared_ptr<const nfs_player_initial_sidebar::Proof>playerInitial;std::shared_ptr<const nfs_player_intro_cpu::Prepared>playerPrepared;std::shared_ptr<const nfs_initial_sidebar_transition::Proof>initial;std::shared_ptr<const nfs_child_initial_sidebar::Proof>childInitial;std::shared_ptr<const nfs_setup_initial_full_live::Proof>setupInitialFull;std::shared_ptr<const nfs_location_initial_full_live::Proof>locationInitialFull;std::shared_ptr<const nfs_opponent_initial_full_live::Proof>opponentInitialFull;std::shared_ptr<const nfs_opponent_initial_sidebar::Proof>opponentInitial;std::shared_ptr<const nfs_location_initial_sidebar::Proof>locationInitial;std::shared_ptr<const nfs_main_sidebar_origin::Proof>origin;std::shared_ptr<const nfs_main_sidebar_origin::GameLife>life;};
struct State{std::array<std::shared_ptr<const Packet>,2>slots{};std::shared_ptr<Packet>pending;std::shared_ptr<const Packet>lastDraw;uint64_t lastDrawSerial=0,nextContent=0;unsigned tiles=0;bool failed=false;};
inline State&state(){static thread_local State s;return s;}
struct OutgoingLocationSidebar {
 std::shared_ptr<const void> owner;
 std::shared_ptr<const bg::SourceCatalog> catalog;
 std::shared_ptr<const nfs_main_sidebar_origin::GameLife> life;
 uintptr_t game=0,thread=0;
 uint64_t content=0;
 uint32_t archive=0;
 std::array<uint16_t,bg::Full> original{};
 std::array<uint32_t,300> keys{};
 NfsBackgroundSceneLayers layers{};
 NfsCompactCaptionWordView caption{};
};
inline std::shared_ptr<const OutgoingLocationSidebar>& outgoingLocationSidebar() {
 static thread_local std::shared_ptr<const OutgoingLocationSidebar> current;
 return current;
}

inline void rememberStaticFull(Game&g,const Packet&p)noexcept{
 if(p.slot!=1)return;
 bool remembered=false;
 if(p.kind==9){
  remembered=nfs_child_static_scene::rememberInitialFull(g,p.locationInitialFull,p.catalog,p.life,p.content,p.source,p.raw,p.original,p.addresses);
 }else if(p.kind==10&&p.opponentInitialFull&&p.opponentInitialFull->refresh){
  remembered=nfs_child_static_scene::rememberInitialFull(g,p.opponentInitialFull,p.catalog,p.life,p.content,p.source,p.raw,p.original,p.addresses);
 }else return;
 fullTrace("static-history",remembered?"sealed":"refused",p.kind,p.slot);
}
// Slot number is an original428250 argument, not a source-kind enum. Main's
// exit uploads its343 sidebar toslot0; its gradient still reads whole Main slot1.
inline bool unchanged(const Packet&p)noexcept{
 if(!p.catalog||nfs_background_scene_runtime::acquireCatalog()!=p.catalog)return false;
 if((p.playerInitial&&!p.playerInitial->current())||(p.playerPrepared&&!p.playerPrepared->current())||(p.locationInitialFull&&!nfs_location_initial_full_live::owned(*reinterpret_cast<Game*>(p.game),*p.locationInitialFull))||(p.opponentInitialFull&&!nfs_opponent_initial_full_live::owned(*reinterpret_cast<Game*>(p.game),*p.opponentInitialFull))||(p.setupInitialFull&&!nfs_setup_initial_full_live::owned(*reinterpret_cast<Game*>(p.game),*p.setupInitialFull))||(p.locationInitial&&!p.locationInitial->current())||(p.opponentInitial&&!p.opponentInitial->current())||(p.childInitial&&!p.childInitial->current())||!p.life||!p.life->current()||(p.origin&&!p.origin->current())||(p.initial&&!nfs_initial_sidebar_transition::current(*p.initial)))return false;
 try{nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=p.archive)return false;
  nfs_logo_publisher::checked(p.source,p.raw.size());
  if(std::memcmp(nfs_host_address(p.source,p.raw.size()),p.raw.data(),p.raw.size()))return false;
  if(p.targetBound){bg::Target actual{};if(p.initial)nfs_initial_sidebar_transition::readTarget(tx,actual,p.sourceTarget.owner);else nfs_logo_publisher::target(tx,actual,p.sourceTarget.owner);
   if(!nfs_main_sidebar_origin::sameRetainedTarget(actual,p.sourceTarget))return false;}
  return true;
 }catch(...){return false;}
}
inline bool bindOrigin(Packet&p,Game&g,bool full)noexcept{try{
 auto*history=nfs_main_sidebar_origin_live::current(g);auto catalog=nfs_background_scene_runtime::acquireCatalog();
 if(!history||!catalog)return false;
 for(auto proof:history->origins.entries()){
  if(!proof||!proof->current()||proof->game()!=p.game||proof->publisherThread()!=p.thread||proof->archive()!=p.archive||!nfs_main_sidebar_origin::sameCatalog(*proof->catalog(),*catalog))continue;
  const auto&t=full?proof->copy().post.target:proof->post().target;
  if(t.w[8]!=p.source||p.raw.size()<16||std::memcmp(p.raw.data(),t.header.data(),16))continue;
  auto frame=proof->frame();if(!frame||!frame->valid)continue;
  bool exact=true;
  if(full){if(frame->original!=p.original||frame->keys!=p.addresses)exact=false;}
  else for(unsigned k=0;k<bg::Side;++k)if(bg::le16(p.raw.data()+16+2*k)!=proof->post().pixels[k]){exact=false;break;}
  if(!exact)continue;
  bg::Target actual{};{nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,actual,t.owner);}
  if(!nfs_main_sidebar_origin::sameRetainedTarget(actual,t))continue;
  p.origin=proof;p.sourceTarget=t;p.targetBound=true;p.kind=full?3:2;p.mapRole=frame->mapRole;
  p.layers.mapRole=frame->mapRole;p.layers.wordCount=frame->wordCount;p.layers.panelClip[2]=343;p.layers.panelClip[3]=480;
  std::copy(frame->words.begin(),frame->words.end(),p.layers.words);std::copy(frame->logos.begin(),frame->logos.end(),p.layers.logos);
  return unchanged(p);
 }
 return false;
 }catch(...){return false;}
}
// Candidate admission observes the real343 source; native authorization is
// deliberately deferred until the independent inner300-tile witness returns.
inline bool bindPlayerSidebar(Packet&p,Game&g)noexcept{try{
 if(p.slot||p.raw.size()!=16+bg::Side*2)return false;
 nfs_player_initial_native::Binding binding{};if(!nfs_player_initial_native_live::currentBinding(g,binding))return false;
 auto*owner=nfs_player_scene_live::current(g);auto source=owner?owner->pendingInitialSidebar:nullptr;
 if(!source||!source->current()||source->binding().game!=p.game||source->binding().thread!=p.thread||source->binding().archive!=p.archive||!source->catalogMatches(p.catalog)||binding.sourceBitmap!=p.source)return false;
 const auto frame=source->origin()->frame();if(!frame||!frame->valid||std::memcmp(p.raw.data(),source->post().target.header.data(),16))return false;
 for(unsigned k=0;k<bg::Side;++k)if(bg::le16(p.raw.data()+16+2*k)!=source->post().pixels[k])return false;
 p.playerInitial=source;p.sourceTarget=source->post().target;p.targetBound=true;p.kind=11;p.mapRole=frame->mapRole;p.layers.mapRole=frame->mapRole;p.layers.wordCount=frame->wordCount;p.layers.panelClip[2]=343;p.layers.panelClip[3]=480;std::copy(frame->words.begin(),frame->words.end(),p.layers.words);std::copy(frame->logos.begin(),frame->logos.end(),p.layers.logos);return unchanged(p);
 }catch(...){return false;}}
inline bool sealPlayerSidebar(Packet&p,Game&g)noexcept{try{
 if(p.kind!=11||!p.playerInitial||!p.content)return false;
 std::unique_ptr<nfs_player_initial_native::Plane>actual(new nfs_player_initial_native::Plane);
 nfs_player_initial_native::Binding binding{};auto proof=nfs_player_initial_native_live::proofForPresentation(g,binding,*actual);if(!proof||binding.sourceBitmap!=p.source||binding.sidebar.form!=p.playerInitial->binding().form||binding.sidebar.epoch!=p.playerInitial->binding().epoch)return false;
 auto prepared=nfs_player_intro_cpu::Prepared::create(proof,binding,*actual,p.catalog,p.content);if(!prepared||prepared->source()!=p.playerInitial||!prepared->current())return false;const auto v=prepared->view();if(!v.schema||!v.captionWord.schema||v.wordsCount!=bg::Full||!v.paddedOriginal565||std::memcmp(v.paddedOriginal565,p.original.data(),bg::Full*2)||std::memcmp(v.keys,p.addresses.data(),300*4))return false;
 p.playerPrepared=std::move(prepared);return unchanged(p);
 }catch(...){return false;}}
// Only real428250 source+300 real49755C uploads can create a packet.
struct UploadScope{State*s=nullptr;Game*g=nullptr;std::shared_ptr<const Packet>previous;bool active=false;int unwind=0;explicit UploadScope(Game&game)noexcept{WorkerErrno preserve;if(!nfs_background_scene_live::enabled()||uint32_t(game.edx)>1)return;g=&game;s=&state();
 nfs_track_info_ui_live::EntryArtSource trackInfo{};const bool trackInfoEntry=nfs_track_info_ui_live::identifyEntryArt(game,trackInfo);try{nfs::GuestCopyTransaction tx;const uint32_t wh=tx.load<uint32_t>(uint32_t(game.eax)+4);if((wh>>16)!=480||((wh&65535)!=640&&(wh&65535)!=343&&!trackInfoEntry)){s->slots[uint32_t(game.edx)].reset();return;}}catch(...){s->slots[uint32_t(game.edx)].reset();return;}previous=s->slots[uint32_t(game.edx)];s->slots[uint32_t(game.edx)].reset();if(s->pending){s->failed=true;return;}try{auto p=std::make_shared<Packet>();p->game=reinterpret_cast<uintptr_t>(&game);p->thread=nfs_logo_publisher::threadToken();p->life=nfs_main_sidebar_origin::acquireGameLife(p->game);if(!p->life)return;p->source=uint32_t(game.eax);p->slot=uint32_t(game.edx);auto catalog=nfs_background_scene_runtime::acquireCatalog();if(!catalog)return;p->catalog=catalog;uint32_t width=0,height=0;{nfs::GuestCopyTransaction tx;p->archive=tx.load<uint32_t>(dword_4D40B8);const uint32_t wh=tx.load<uint32_t>(p->source+4);width=wh&65535;height=wh>>16;if((width!=640&&width!=343&&!trackInfoEntry)||height!=480)return;const size_t span=16+size_t(width)*height*2;p->raw.resize(span);nfs_logo_publisher::checked(p->source,span);std::memcpy(p->raw.data(),nfs_host_address(p->source,span),span);}
 if(trackInfoEntry){
  p->kind=12;
  p->trackInfoArt=trackInfo;
 }else if(width==640){auto firstFull=nfs_setup_initial_full_live::acquire(game,p->source,p->raw);
 if(firstFull&&p->slot==1){const auto frame=firstFull->child->origin()->frame();if(!frame||!frame->valid||!firstFull->child->wordView().schema)return;p->setupInitialFull=firstFull;p->childInitial=firstFull->child;p->sourceTarget=firstFull->full.target;p->targetBound=true;p->kind=8;p->mapRole=firstFull->mapRole;p->layers.mapRole=p->mapRole;p->layers.wordCount=frame->wordCount;p->layers.panelClip[2]=343;p->layers.panelClip[3]=480;std::copy(frame->words.begin(),frame->words.end(),p->layers.words);std::copy(frame->logos.begin(),frame->logos.end(),p->layers.logos);}
 if(!p->kind&&p->slot==1){auto first=nfs_location_initial_full_live::acquire(game,p->source,p->raw);if(first){if(!first->child->view().schema||!first->child->layers(p->layers))return;p->locationInitialFull=first;p->locationInitial=first->child;p->sourceTarget=first->full.target;p->targetBound=true;p->kind=9;p->mapRole=first->mapRole;p->layers.mapRole=p->mapRole;}}
 if(!p->kind&&p->slot==1){auto first=nfs_opponent_initial_full_live::acquire(game,p->source,p->raw);if(first){const auto frame=first->child->origin()->frame();if(!frame||!frame->valid||!first->child->view().schema)return;p->opponentInitialFull=first;p->opponentInitial=first->child;p->sourceTarget=first->full.target;p->targetBound=true;p->kind=10;p->mapRole=first->mapRole;p->layers.mapRole=p->mapRole;p->layers.wordCount=frame->wordCount;p->layers.panelClip[2]=343;p->layers.panelClip[3]=480;std::copy(frame->words.begin(),frame->words.end(),p->layers.words);std::copy(frame->logos.begin(),frame->logos.end(),p->layers.logos);}}
 if(p->slot==1)fullTrace("upload-source",p->kind>=8?"typed-full":"no-typed-full",p->kind,p->slot,p->source,unsigned(p->raw.size()));
 if(!p->kind)for(unsigned role=0;role<2;++role)if(nfs_small_logo_live::hashMatches(p->raw.data(),uint32_t(p->raw.size()),catalog->mapSha[role])){p->kind=1;p->mapRole=role;break;}
  // The whole-Main join is minted by the existing upload-seal destructor,
  // after all actual tiles have been observed. It is never adopted at entry.
  if(!p->kind)p->kind=3;
 }else if(!bindOrigin(*p,game,false)&&!bindPlayerSidebar(*p,game)){
  auto child=nfs_child_initial_sidebar_live::acquire(game,p->source,p->raw);
  if(child){const auto&frame=child->origin()->frame();if(!frame||!frame->valid||!child->wordView().schema)return;p->childInitial=child;p->sourceTarget=child->post().target;p->targetBound=true;p->kind=5;p->mapRole=frame->mapRole;
   p->layers.mapRole=frame->mapRole;p->layers.wordCount=frame->wordCount;p->layers.panelClip[2]=343;p->layers.panelClip[3]=480;std::copy(frame->words.begin(),frame->words.end(),p->layers.words);std::copy(frame->logos.begin(),frame->logos.end(),p->layers.logos);
  }else{
   auto opponent=nfs_opponent_initial_sidebar_live::acquire(game,p->source,p->raw);
   if(opponent){const auto&frame=opponent->origin()->frame();if(!frame||!frame->valid||!opponent->view().schema)return;p->opponentInitial=opponent;p->sourceTarget=opponent->post().target;p->targetBound=true;p->kind=6;p->mapRole=frame->mapRole;
    p->layers.mapRole=frame->mapRole;p->layers.wordCount=frame->wordCount;p->layers.panelClip[2]=343;p->layers.panelClip[3]=480;std::copy(frame->words.begin(),frame->words.end(),p->layers.words);std::copy(frame->logos.begin(),frame->logos.end(),p->layers.logos);
   }else{
    auto location=nfs_location_initial_sidebar_live::acquire(game,p->source,p->raw);
    if(location){if(!location->view().schema||!location->layers(p->layers))return;p->locationInitial=location;p->sourceTarget=location->post().target;p->targetBound=true;p->kind=7;p->mapRole=p->layers.mapRole;
    }else{auto first=nfs_initial_sidebar_transition::acquire(game,p->source,p->raw);if(!first)return;
     p->initial=first;p->sourceTarget=first->target;p->targetBound=true;p->layers=first->layers;p->kind=4;p->mapRole=0;
    }
   }
  }
 }
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<width;++x)p->original[y*640+x]=bg::le16(p->raw.data()+16+2*(y*width+x));if(!unchanged(*p)||s->nextContent==UINT64_MAX)return;p->content=++s->nextContent;s->pending=std::move(p);s->tiles=0;s->failed=false;active=true;unwind=nfs_logo_publisher::exceptions();}catch(...){s->pending.reset();}}
 ~UploadScope(){WorkerErrno preserve;if(!active||!s||!g||!s->pending)return;auto p=std::move(s->pending);if(s->failed||nfs_logo_publisher::exceptions()>unwind||!unchanged(*p))return;if(!s->tiles&&previous&&previous->game==p->game&&previous->thread==p->thread&&previous->source==p->source&&previous->kind==p->kind&&previous->raw==p->raw&&unchanged(*previous)){rememberStaticFull(*g,*previous);s->slots[p->slot]=std::move(previous);return;}if(s->tiles!=300)return;if(p->kind==3&&!bindOrigin(*p,*g,true))return;if(p->kind==11&&!sealPlayerSidebar(*p,*g))return;if(!unchanged(*p)){if(p->kind>=8)fullTrace("upload-finish","changed",p->kind,p->slot);return;}rememberStaticFull(*g,*p);if(p->kind>=8)fullTrace("upload-finish","sealed",p->kind,p->slot,s->tiles);
 // Keep the outgoing proof through redundant upload calls. Retire it
 // when the incoming Track Info sidebar has replaced all original tiles.
 if(p->kind==12&&p->slot==0)outgoingLocationSidebar().reset();
 s->slots[p->slot]=std::move(p);}};
struct TileScope{State*s=nullptr;nfs_logo_publisher::TileObservation t{};unsigned index=0;int unwind=0;bool active=false;explicit TileScope(Game&g)noexcept{WorkerErrno preserve;s=&state();if(!s->pending||s->failed||s->tiles>=300)return;index=s->tiles;unwind=nfs_logo_publisher::exceptions();try{nfs::GuestCopyTransaction tx;uint32_t sp=uint32_t(g.esp);t.descriptorGuest=tx.load<uint32_t>(sp);t.payloadGuest=tx.load<uint32_t>(sp+4);t.tableGuest=tx.load<uint32_t>(sp+8);t.expectedSlotDescriptor=tx.load<uint32_t>(dword_4EB74C+s->pending->slot*1200+index*4);nfs_logo_publisher::descriptor(tx,t.descriptorGuest,t.before);nfs_logo_publisher::checked(t.payloadGuest,2048);std::memcpy(t.pixels.data(),nfs_host_address(t.payloadGuest,2048),2048);active=true;}catch(...){s->failed=true;}}
 ~TileScope(){WorkerErrno preserve;if(!active)return;bool ok=nfs_logo_publisher::exceptions()==unwind;try{nfs::GuestCopyTransaction tx;nfs_logo_publisher::descriptor(tx,t.descriptorGuest,t.after);if(std::memcmp(t.pixels.data(),nfs_host_address(t.payloadGuest,2048),2048))ok=false;}catch(...){ok=false;}auto p=s->pending;if(!p){s->failed=true;return;}ok=ok&&t.descriptorGuest&&t.descriptorGuest==t.expectedSlotDescriptor&&!t.tableGuest&&t.before[0]==3&&t.before[1]==3&&t.before[2]==3&&t.before[3]==11&&t.before[6]==3&&t.after[4]==t.payloadGuest&&(t.after[5]&3)==0&&t.after[5]<=0x200000u-2048u;for(unsigned k=0;k<7;++k)if(k!=4&&t.before[k]!=t.after[k])ok=false;for(unsigned k=0;k<index;++k)if(uint64_t(p->addresses[k])<uint64_t(t.after[5])+2048&&uint64_t(t.after[5])<uint64_t(p->addresses[k])+2048)ok=false;unsigned x=(index/15)*32,y=(index%15)*32;for(unsigned yy=0;yy<32;++yy)for(unsigned xx=0;xx<32;++xx)if(bg::le16(t.pixels.data()+2*(yy*32+xx))!=nfs_logo_publisher::to1555(p->original[(y+yy)*640+x+xx]))ok=false;if(!ok){s->failed=true;return;}p->addresses[index]=t.after[5];++s->tiles;}};
// Bind only the actual lexical cached-widget owner. Its epoch is distinct
// from the initial-caption proof. The independent initial-full proof remains
// the background authority even while ordinary scene verification is pending.
inline void responsiveOwner(Game&g,NfsMenuTransitionView&v)noexcept{
 WorkerErrno preserve;if(v.kind<8||v.kind>10)return;
 auto bind=[&](auto*owner,uint32_t form,uint32_t descriptor){
  if(!owner||!owner->on||owner->mainDepth!=1||owner->game!=v.sourceGame||!owner->ownerEpoch||
     !owner->catalogue||reinterpret_cast<uintptr_t>(owner->catalogue.get())!=v.sourceCatalog)return;
  v.responsiveForm=form;v.responsiveDescriptor=descriptor;v.responsiveProducerEpoch=owner->ownerEpoch;
  // Empty before the first measured widget observation is allowed. Tail
  // admission must acquire and prove a current snapshot before presentation.
  nfs_menu_cached_snapshot(form,descriptor,owner->ownerEpoch,v.sourceGame,v.serial,v.content,&v.responsive);
 };
 try{
  if(v.kind==8){if(v.childInitial.form==0x4d2790u)bind(nfs_setup_scene_live::current(g),NFS_MENU_CACHED_SETUP,uint32_t(guest_address(off_4D2790)));
   else if(v.childInitial.form==0x4d3a88u)bind(nfs_options_scene_live::current(g),NFS_MENU_CACHED_OPTIONS,uint32_t(guest_address(off_4D3A88)));}
  else if(v.kind==9)bind(nfs_location_scene_live::current(g),NFS_MENU_CACHED_LOCATION,uint32_t(guest_address(off_4D2A20)));
  else bind(nfs_opponent_scene_live::current(g),NFS_MENU_CACHED_OPPONENT,uint32_t(guest_address(off_4D3204)));
 }catch(...){v.responsiveForm=v.responsiveDescriptor=0;v.responsiveProducerEpoch=0;v.responsive={};}
}
struct DrawScope{
 std::shared_ptr<const Packet>packet;
 std::shared_ptr<const OutgoingLocationSidebar> outgoing;
 uint64_t serial=0;bool accepted=false,tracePreviewCommit=false;
 explicit DrawScope(Game&g,bool plain=false)noexcept{WorkerErrno preserve;state().lastDraw.reset();state().lastDrawSerial=0;
  if(!nfs_background_scene_live::enabled()||nfs_background_scene_live::activeRequest()||uint32_t(g.eax)>1)return;
  auto p=state().slots[uint32_t(g.eax)];
  const auto* trackInfoOwner=nfs_track_info_ui_live::ownerCurrent(g);
  if(trackInfoOwner&&trackInfoOwner->entryPhase){
   const unsigned phase=trackInfoOwner->entryPhase;
   const char* phaseName=phase==1?"track-info-outgoing-fade":phase==2?"track-info-outgoing-slide":phase==3?"track-info-incoming-slide":"track-info-incoming-fade";
   fullTrace(phaseName,"draw-slot",p?p->kind:0,uint32_t(g.eax),unsigned(plain));
  }
  if(p&&p->kind==12){
   const auto* owner=trackInfoOwner;
   if(plain!=(p->slot==0)||!owner||owner->entryPhase<3||owner->epoch!=p->trackInfoArt.ownerEpoch||p->archive!=p->trackInfoArt.archive)return;
  }
  if(!p&&plain&&uint32_t(g.eax)==0&&trackInfoOwner&&
     trackInfoOwner->entryPhase>=1&&trackInfoOwner->entryPhase<=2){
   const auto& side=outgoingLocationSidebar();
   if(side&&side->game==reinterpret_cast<uintptr_t>(&g)&&
      side->thread==nfs_logo_publisher::threadToken()&&side->life&&
      side->life->current()&&side->catalog==nfs_background_scene_runtime::acquireCatalog())
    outgoing=side;
   if(!outgoing)
    fullTrace("track-info-outgoing-sidebar",side?"stale-at-draw":"missing-at-draw",13,0,
              trackInfoOwner->entryPhase,side&&side->life&&side->life->current(),
              side&&side->game==reinterpret_cast<uintptr_t>(&g));
  }
  // Ordinary whole-Main427B30 has its maintained full-scene owner; don't nest.
  if(!outgoing&&(!p||(plain&&p->kind==3)||p->game!=reinterpret_cast<uintptr_t>(&g)||p->thread!=nfs_logo_publisher::threadToken()||!unchanged(*p))){if(p&&p->kind>=8)fullTrace("draw-source","source-current",p->kind,p->slot,unsigned(plain));return;}if(p&&p->kind>=8)fullTrace("draw-source","accepted",p->kind,p->slot,unsigned(plain));
  NfsMenuTransitionView v{};
  if(outgoing||(p&&p->kind==12))v.trackInfoPhase=trackInfoOwner->entryPhase;
  try{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);v.originX=int32_t(g.edx);
   if(plain){if(tx.load<uint32_t>(sp+4)!=UINT32_MAX)return;v.drawKind=2;v.originY=int32_t(g.ebx);v.stepX=int32_t(g.ecx);v.minimum=0;v.maximum=255;v.phase=-1;}
   else{v.drawKind=1;v.originY=0;v.stepX=int32_t(g.ebx);v.minimum=int32_t(g.ecx);v.maximum=tx.load<int32_t>(sp+4);v.direction=tx.load<int32_t>(sp+8);v.phase=tx.load<int32_t>(sp+12);}
   if(v.minimum<0||v.maximum>255||v.minimum>v.maximum||v.direction<0||v.direction>1||v.phase< -1||v.phase>255||v.originX< -32768||v.originX>32767||v.originY< -32768||v.originY>32767||v.stepX<0||v.stepX>32)return;
  }catch(...){return;}
  serial=nfs_background_scene_live::nextRequest();if(!serial)return;
  if(outgoing){
   const auto side=outgoing;
   v.sourceLife=side->life->serial();
   v.sourceGame=side->game;
   v.sourceCatalog=reinterpret_cast<uintptr_t>(side->catalog.get());
   v.sourceArchive=side->archive;
   v.serial=serial;
   v.content=side->content;
   v.kind=13;
   v.mapRole=side->layers.mapRole;
   v.original565=side->original.data();
   std::copy(side->keys.begin(),side->keys.end(),v.addresses);
   v.layers=side->layers;
   v.outgoingLocationCaption=side->caption;
   try {
    const auto receipt=nfs::enqueue_owned_renderer([side,v](){
     return side->life->current()?int32_t(nfs_private_menu_transition_begin(&v)):int32_t(0);
    });
    accepted=receipt==nfs::OwnedRendererAcceptance::Consumed;
    fullTrace("track-info-outgoing-sidebar",accepted?"consumed":"refused",13,0,v.trackInfoPhase,unsigned(v.originX),unsigned(v.stepX));
   } catch (...) {}
   return;
  }
  v.sourceLife=p->life->serial();v.sourceGame=p->game;v.sourceCatalog=reinterpret_cast<uintptr_t>(p->catalog.get());v.sourceArchive=p->archive;v.serial=serial;v.content=p->content;v.slot=p->slot;v.kind=p->kind;v.mapRole=p->mapRole;v.original565=p->original.data();std::copy(p->addresses.begin(),p->addresses.end(),v.addresses);v.layers=p->layers;if(p->kind==11){if(!p->playerPrepared||!p->playerPrepared->current()||!p->playerInitial)return;const auto pv=p->playerPrepared->view();v.playerInitial.schema=1;v.playerInitial.form=p->playerInitial->binding().form;v.playerInitial.ownerEpoch=p->playerInitial->binding().epoch;v.playerInitial.captionSerial=p->playerInitial->captionSerial();v.playerInitial.captionLifetime=p->playerInitial->captionLifetime();v.playerInitial.caption=pv.captionWord;if(!v.playerInitial.caption.schema)return;}if(p->childInitial){v.childInitial.schema=1;v.childInitial.form=p->childInitial->binding().form;v.childInitial.ownerEpoch=p->childInitial->binding().epoch;v.childInitial.captionSerial=p->childInitial->captionSerial();v.childInitial.captionLifetime=p->childInitial->captionLifetime();v.childInitial.caption=p->childInitial->wordView();if(!v.childInitial.caption.schema)return;}if(p->opponentInitial){v.opponentInitial=p->opponentInitial->view();if(!v.opponentInitial.schema)return;}if(p->locationInitial){v.locationInfoReturn=p->kind==7&&p->locationInitial->locationInfoReturn();v.locationInitial=p->locationInitial->view();if(!v.locationInitial.schema)return;}if(p->locationInitialFull)v.initialFullPreview=p->locationInitialFull->previewView();if(p->opponentInitialFull)v.initialFullPreview=p->opponentInitialFull->previewView();responsiveOwner(g,v);packet=p;
  auto* opponent = p->kind == 10 ? nfs_opponent_scene_live::current(g) : nullptr;
  const bool firstPreview = opponent && !opponent->loggedPreviewRenderer;
  try{const auto receipt=nfs::enqueue_owned_renderer([p,v,firstPreview](){if(p->playerPrepared&&!p->playerPrepared->current())return int32_t(0);if(p->locationInitial&&!p->locationInitial->current())return int32_t(0);if(p->opponentInitial&&!p->opponentInitial->current())return int32_t(0);if(p->childInitial&&!p->childInitial->current())return int32_t(0);if(p->kind!=12){
     if(firstPreview)nfs::log_opponents_visit(nfs::OpponentsVisit::PreviewBegin);
     const auto result=int32_t(nfs_private_menu_transition_begin(&v));
     if(firstPreview)nfs::log_opponents_visit(result?nfs::OpponentsVisit::PreviewReady:nfs::OpponentsVisit::PreviewDeclined);
     return result;
    }
    auto view=v;
    if(!nfs_track_info_art_acquire_entry(p->trackInfoArt.index,&view.trackInfoArt))return int32_t(0);
    const int result=nfs_private_menu_transition_begin(&view);
    view.trackInfoArt.release(view.trackInfoArt.owner);
    return int32_t(result);});
   if(firstPreview&&receipt!=nfs::OwnedRendererAcceptance::UseExistingOwnerPath)opponent->loggedPreviewRenderer=true;
   accepted=receipt==nfs::OwnedRendererAcceptance::Consumed;tracePreviewCommit=accepted&&firstPreview;if(p->kind>=8)fullTrace("draw-renderer",accepted?"consumed":"refused",p->kind,p->slot,unsigned(v.originX),unsigned(v.stepX),v.drawKind);}
  catch(...){packet.reset();serial=0;}
 }
 ~DrawScope(){
  WorkerErrno preserve;
  if(!accepted||(!packet&&!outgoing)||!serial)return;
  auto p=packet;
  auto side=outgoing;
  const uint64_t id=serial;
  const bool trace=tracePreviewCommit;
  try{
   const auto receipt=nfs::enqueue_owned_renderer([p,side,id,trace](){
    if(trace)nfs::log_opponents_visit(nfs::OpponentsVisit::PreviewCommitBegin);
    const auto result=int32_t(nfs_private_menu_transition_finish(id));
    if(trace)nfs::log_opponents_visit(result?nfs::OpponentsVisit::PreviewCommitted:nfs::OpponentsVisit::PreviewCommitDeclined);
    return result;
   });
   if(p&&receipt==nfs::OwnedRendererAcceptance::Consumed&&((p->kind>=5&&p->kind<=7)||p->kind==11)&&unchanged(*p)){
    state().lastDraw=p;
    state().lastDrawSerial=id;
   }
  }catch(...){}
 }
};
struct PlainDrawScope:DrawScope{explicit PlainDrawScope(Game&g)noexcept:DrawScope(g,true){}};
}

#include "MenuTransitionBrandingGameHooks.inc"

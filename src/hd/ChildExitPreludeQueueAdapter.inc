// SPDX-License-Identifier: MIT
#pragma once
#include "ChildExitPreludeAbi.h"
#include "SetupSceneQueueAdapter.inc"
#include "OptionsSceneQueueAdapter.inc"
#include "LocationSceneQueueAdapter.inc"
#include "OpponentSceneQueueAdapter.inc"
#include "PlayerSceneQueueAdapter.inc"
#include "MenuTransitionGameHooks.inc"
#include "ChildPhotoGameHooks.inc"
#include "PlayerPhotoGameHooks.inc"
#include "MainThreadBridge.h"
#include "WorkerControl.h"
#include <atomic>
#include <memory>
#include <algorithm>
namespace nfs_child_exit_live {
namespace bg=nfs_background_scene_cpu;namespace mt=nfs_menu_transition_live;
struct Errno{int value=errno;~Errno(){errno=value;}};
struct Life{std::atomic<bool>alive{true};};
struct Owner{
 std::shared_ptr<const void>typed;std::shared_ptr<const bg::Prepared>full;
 std::shared_ptr<const std::vector<uint8_t>>authenticatedSidebar;
 std::shared_ptr<const nfs_child_static_scene::Record>history;std::shared_ptr<const nfs_child_static_scene::SidebarRecord>historySidebarSource;bool historicalNativeRevalidation=false;std::array<uint16_t,bg::Full>historySidebar{};
 std::shared_ptr<const mt::Packet>sidebar;std::shared_ptr<const bg::SourceCatalog>catalog;
 std::shared_ptr<const nfs_main_sidebar_origin::GameLife>gameLife;std::shared_ptr<Life>life;
 std::shared_ptr<const nfs_player_native::Proof>playerNative;nfs_player_native::CurrentBinding playerBinding{};
 std::shared_ptr<const nfs_child_photo_live::Proof>childPhoto;std::shared_ptr<const nfs_player_photo_live::Proof>playerPhoto;
 bool playerPhotoReceipt=false;uint32_t photoStack=0,photoOldCar=0,photoNewCar=0;
 unsigned captureStage=0;uint32_t formGuest=0;NfsChildExitPreludeView view{};std::atomic<unsigned>refs{1};
 static void rejectLease(void*p){auto*o=static_cast<Owner*>(p);if(o&&o->historicalNativeRevalidation&&o->life)o->life->alive.store(false,std::memory_order_release);}
 static void retain(void*p){if(p)static_cast<Owner*>(p)->refs.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p){if(p&&static_cast<Owner*>(p)->refs.fetch_sub(1,std::memory_order_acq_rel)==1)delete static_cast<Owner*>(p);}
 static int current(void*p){const auto*o=static_cast<const Owner*>(p);return o&&o->life&&o->life->alive.load(std::memory_order_acquire)&&o->full&&o->full->valid&&((o->history&&o->authenticatedSidebar&&o->history->gameLife==o->gameLife&&o->history->token&&o->history->token->alive.load(std::memory_order_acquire)&&(!o->historySidebarSource||(o->historySidebarSource->alive.load(std::memory_order_acquire)&&o->historySidebarSource->token==o->history->token)))||(!o->history&&o->sidebar&&o->sidebar->life&&o->sidebar->life->current()))&&o->gameLife&&o->gameLife->current()&&o->catalog&&nfs_background_scene_runtime::acquireCatalog()==o->catalog&&o->view.uploadEvent&&nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)==o->view.uploadEvent&&(!o->playerNative||o->playerNative->current(o->playerBinding))&&(!o->childPhoto||nfs_child_photo_live::ownedCurrent(*o->childPhoto))&&(!o->playerPhoto||(o->playerPhoto->art&&o->playerPhoto->art->current()));}
};
inline std::shared_ptr<Owner>adopt(Owner*p){return std::shared_ptr<Owner>(p,[](Owner*q){Owner::release(q);});}
inline void note(unsigned reason,const Owner*o=nullptr)noexcept{Errno e;static thread_local unsigned n;const char*p=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(p&&p[0]=='1'&&!p[1]&&n++<64)std::fprintf(stderr,"CHILD EXIT source reason=%u family=%u session=%llu upload=%llu stage=%u\n",reason,o?o->view.family:0,(unsigned long long)(o?o->view.session:0),(unsigned long long)(o?o->view.uploadEvent:0),o?o->captureStage:0);}
inline bool descriptor(nfs::GuestCopyTransaction&tx,uint32_t at,uint32_t*out){if(!at||(at&3)||at>UINT32_MAX-27)return false;for(unsigned k=0;k<7;++k)out[k]=tx.load<uint32_t>(at+4*k);return out[6]==3&&(out[3]==11||out[3]==12)&&!(out[5]&3)&&out[5]<=0x200000u-2048u;}
inline bool photoDescriptorsView(NfsChildExitPreludeView&view,bool mint)noexcept{try{nfs::GuestCopyTransaction tx;for(unsigned i=0;i<100;++i){const uint32_t table=uint32_t(guest_address(i<50?dword_4EB5BC:dword_4EB684));const uint32_t d=tx.load<uint32_t>(table+4*(i%50));uint32_t w[7];if(!descriptor(tx,d,w)||w[0]!=3||w[1]!=3||w[2]!=3)return false;auto&r=view.photo[i];if(mint){r.descriptor=d;std::copy(w,w+7,r.words);r.words[4]=0;}else{if(r.descriptor!=d)return false;for(unsigned k=0;k<7;++k)if(k!=4&&r.words[k]!=w[k])return false;}}return true;}catch(...){return false;}}
inline bool photoDescriptors(Owner&o,bool mint)noexcept{return photoDescriptorsView(o.view,mint);}
template<class State,class Prepared>inline bool fullSource(State*s,const std::shared_ptr<const Prepared>&p,Game&g,Owner&o,unsigned family,uint32_t form){
 o.view.family=family;o.view.form=form;auto*c=nfs_logo_publisher::store();if(!s||s->invalid||!s->on||s->mainDepth!=1||!p||!p->valid||s->published!=p||s->game!=reinterpret_cast<uintptr_t>(&g)||s->thread!=nfs_logo_publisher::threadToken()||!s->catalogue||!s->ownerEpoch||!s->archive||!s->nativeUploadEvent||s->nativeUploadEvent!=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)||!c||c->gameOwner!=s->game||c->thread!=s->thread||!c->copySourceVerified||!c->prepared||!c->bound||c->uploaded!=300||!c->copy.complete||!c->group.complete||c->copy.serial!=s->copySerial||c->group.serial!=s->groupSerial||c->nativeUploadEvent!=s->nativeUploadEvent||p->original!=c->packet.originalCopyPost565||p->keys!=c->rendererAddresses)return false;
 o.typed=p;o.full=p;o.catalog=s->catalogue;o.view.family=family;o.view.form=form;o.view.ownerEpoch=s->ownerEpoch;o.view.sourceGame=s->game;o.view.sourceCatalog=reinterpret_cast<uintptr_t>(s->catalogue.get());o.view.sourceArchive=s->archive;o.view.uploadEvent=s->nativeUploadEvent;return true;
}
inline std::shared_ptr<Owner>capture(Game&g,uint64_t session,const std::shared_ptr<Life>&life,uint32_t f)noexcept{Errno e;try{
 auto o=adopt(new Owner);struct Trace{Owner*o;bool ok=false;~Trace(){if(!ok)note(4,o);}}trace{o.get()};o->captureStage=1;o->life=life;o->view.schema=1;o->view.session=session;o->formGuest=f;
 if(f==uint32_t(guest_address(off_4D2790))){auto*s=nfs_setup_scene_live::current(g);if(!s||!fullSource(s,s->published,g,*o,NFS_CHILD_EXIT_SETUP,0x4d2790))return {};o->view.setup=s->published->view();if(!o->view.setup.schema)return {};}
 else if(f==uint32_t(guest_address(off_4D3A88))){auto*s=nfs_options_scene_live::current(g);if(!s||!fullSource(s,s->published,g,*o,NFS_CHILD_EXIT_OPTIONS,0x4d3a88))return {};o->view.options=s->published->view();if(!o->view.options.schema)return {};}
 else if(f==uint32_t(guest_address(off_4D2A20))){auto*s=nfs_location_scene_live::current(g);if(!s||!fullSource(s,s->published,g,*o,NFS_CHILD_EXIT_LOCATION,0x4d2a20))return {};o->view.location=s->published->view();if(!o->view.location.schema)return {};}
 else if(f==uint32_t(guest_address(off_4D3204))){auto*s=nfs_opponent_scene_live::current(g);if(!s||!fullSource(s,s->published,g,*o,NFS_CHILD_EXIT_OPPONENT,0x4d3204))return {};o->view.opponent=s->published->view();if(!o->view.opponent.schema)return {};}
 else if(nfs_player_scene_live::canonicalForm(f)){auto*s=nfs_player_scene_live::current(g);const uint32_t c=nfs_player_scene_live::canonicalForm(f);const auto a=nfs_player_scene_live::accepted();auto*source=nfs_player_source_live::slot();if(!s||s->invalid||s->form!=c||s->formGuest!=f||!s->published||a.owner!=s||a.epoch!=s->ownerEpoch||!a.proof||a.prepared!=s->published||!source)return {};nfs_player_native::CurrentBinding binding{};auto proof=nfs_player_native_live::proofForPresentation(g,binding,source->upload);if(!proof||proof!=a.proof||!proof->current(binding)||!proof->source(binding))return {};o->typed=a.prepared;o->full=a.prepared;o->catalog=s->catalogue;o->playerNative=proof;o->playerBinding=binding;o->view.family=c==0x4d3050?NFS_CHILD_EXIT_PLAYER1:NFS_CHILD_EXIT_PLAYER2;o->view.form=c;o->view.ownerEpoch=s->ownerEpoch;o->view.sourceGame=s->game;o->view.sourceCatalog=reinterpret_cast<uintptr_t>(s->catalogue.get());o->view.sourceArchive=binding.source.archive;o->view.uploadEvent=proof->uploadEvent(binding);o->view.player=a.prepared->view();if(!o->view.player.schema)return {};}
 else return {};
 o->captureStage=2;o->gameLife=nfs_main_sidebar_origin::acquireGameLife(o->view.sourceGame);if(!o->gameLife)return {};o->view.sourceLife=o->gameLife->serial();auto p=mt::state().slots[0];if(!p||p->slot||!p->content||!p->targetBound||p->game!=o->view.sourceGame||p->thread!=nfs_logo_publisher::threadToken()||p->archive!=o->view.sourceArchive||p->catalog!=o->catalog||p->raw.size()!=16+bg::Side*2||!mt::unchanged(*p))return {};
 // Revalidate this actual uploaded343 resource independently of any entrance stage.
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)if(p->original[y*640+x]!=(x<343?bg::le16(p->raw.data()+16+2*(y*343+x)):0))return {};
 o->captureStage=3;bool ownSidebar=false;
 if(o->view.family==NFS_CHILD_EXIT_SETUP||o->view.family==NFS_CHILD_EXIT_OPTIONS)ownSidebar=p->kind==5&&p->childInitial&&p->childInitial->current()&&p->childInitial->binding().form==o->view.form&&p->childInitial->binding().formGuest==f;
 else if(o->view.family==NFS_CHILD_EXIT_LOCATION)ownSidebar=p->kind==7&&p->locationInitial&&p->locationInitial->current()&&p->locationInitial->binding().form==o->view.form&&p->locationInitial->binding().formGuest==f;
 else if(o->view.family==NFS_CHILD_EXIT_OPPONENT)ownSidebar=p->kind==6&&p->opponentInitial&&p->opponentInitial->current()&&p->opponentInitial->binding().form==o->view.form&&p->opponentInitial->binding().formGuest==f;
 else ownSidebar=p->kind==11&&p->playerInitial&&p->playerInitial->current()&&p->playerInitial->binding().form==o->view.form&&p->playerInitial->binding().formGuest==f;
 if(!ownSidebar)return {};
 o->sidebar=p;o->view.sidebarContent=p->content;o->view.sidebar565=p->original.data();o->view.sidebarWords=bg::Full;std::copy(p->addresses.begin(),p->addresses.end(),o->view.sidebarKeys);
 o->captureStage=4; // Actualphoto100 is captured only for moving preview-change frames, never required by first2 full-only clears.
 if(o->view.family==NFS_CHILD_EXIT_LOCATION||o->view.family==NFS_CHILD_EXIT_OPPONENT){auto q=nfs_child_photo_live::published();if(q&&q->game==o->view.sourceGame&&q->archive==o->view.sourceArchive&&nfs_child_photo_live::current(g,*q)){o->childPhoto=q;o->view.photoFamily=q->family;o->view.photoSerial=q->serial;o->view.photoEpoch=q->epoch;}}
 else if(o->view.family==NFS_CHILD_EXIT_PLAYER1||o->view.family==NFS_CHILD_EXIT_PLAYER2){auto q=nfs_player_photo_live::published();if(q&&nfs_player_photo_live::current(g,*q)){o->playerPhoto=q;o->view.photoFamily=1;o->view.photoSerial=q->serial;o->view.photoEpoch=q->epoch;}}
 o->captureStage=5;o->view.owner=o.get();o->view.retain=Owner::retain;o->view.release=Owner::release;o->view.current=Owner::current;if(!Owner::current(o.get()))return {};note(0,o.get());trace.ok=true;return o;
 }catch(...){return {};}}
// Actual slot0 metadata is only an index for the renderer's mandatory native
// pixel/300-select/600-triangle proofs; it never authenticates source bytes.
inline bool actualSidebarKeys(Owner&o,bool mint)noexcept{try{nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=o.view.sourceArchive)return false;
 for(unsigned i=0;i<300;++i){const uint32_t d=tx.load<uint32_t>(dword_4EB74C+4*i);uint32_t w[7];if(!descriptor(tx,d,w)||w[0]!=3||w[1]!=3||w[2]!=3||w[3]!=11)return false;
  for(unsigned j=0;j<i;++j)if(uint64_t(o.view.sidebarKeys[j])<uint64_t(w[5])+2048&&uint64_t(w[5])<uint64_t(o.view.sidebarKeys[j])+2048)return false;
  if(mint)o.view.sidebarKeys[i]=w[5];else if(o.view.sidebarKeys[i]!=w[5])return false;
 }return true;}catch(...){return false;}}
inline std::shared_ptr<Owner>captureHistory(Game&g,uint32_t form,uint64_t session,const std::shared_ptr<Life>&life)noexcept{Errno e;try{
 const uint64_t event=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire);
 auto r=nfs_child_static_scene::acquire(reinterpret_cast<uintptr_t>(&g),nfs_logo_publisher::threadToken(),form,event);std::shared_ptr<const nfs_child_static_scene::SidebarRecord>side=r?r->sidebarOrigin:nullptr;bool revalidate=false;
 if(!r){auto*l=nfs_player_scene_live::current(g);const auto canonical=nfs_player_scene_live::canonicalForm(form);if(l&&!l->invalid&&l->on&&l->mainDepth==1&&canonical&&l->form==canonical&&l->formGuest==form&&nfs_player_scene_live::archiveCurrent(*l)){
  const nfs_child_static_scene::Domain live{l->game,l->thread,l->formGuest,l->ownerEpoch,l->archive,event,0,0,l->catalogue};r=nfs_child_static_scene::acquirePlayerHistoryForNativeRevalidation(live,canonical,side);revalidate=bool(r);
 }}if(!r){note(6);return {};}
 const auto sidebarRaw=side?side->raw:r->sidebarRaw;
 if(!sidebarRaw||sidebarRaw->size()!=16+bg::Side*2||!r->full||!r->full->valid||nfs_background_scene_runtime::acquireCatalog()!=r->domain.catalog){note(7);return {};}
 auto o=adopt(new Owner);o->history=r;o->historySidebarSource=side;o->historicalNativeRevalidation=revalidate;o->typed=r->typed;o->full=r->full;o->catalog=r->domain.catalog;o->gameLife=r->gameLife;o->life=life;o->formGuest=form;o->view=r->view;o->view.session=session;o->view.uploadEvent=event;o->authenticatedSidebar=sidebarRaw;
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x)o->historySidebar[y*640+x]=bg::le16(sidebarRaw->data()+16+2*(y*343+x));
 o->view.sidebar565=o->historySidebar.data();o->view.sidebarWords=bg::Full;o->view.sidebarContent=side?side->content:r->domain.copySerial;
 if(!actualSidebarKeys(*o,true)){note(8,o.get());return {};}if(side&&!std::equal(side->keys.begin(),side->keys.end(),o->view.sidebarKeys)){note(8,o.get());return {};}if(revalidate)nfs_child_static_scene::diagnostic(14,r->domain.uploadEvent,event);
 o->view.owner=o.get();o->view.retain=Owner::retain;o->view.release=Owner::release;o->view.current=Owner::current;o->view.rejectLease=revalidate?Owner::rejectLease:nullptr;o->captureStage=6;
 if(!Owner::current(o.get())){note(9,o.get());return {};}note(0,o.get());return o;
 }catch(...){note(10);return {};}}
inline bool historyLeaseCurrent(Game&g,const Owner&o)noexcept{
 if(!o.history)return false;
 if(!o.historicalNativeRevalidation)return nfs_child_static_scene::current(*o.history,reinterpret_cast<uintptr_t>(&g),nfs_logo_publisher::threadToken(),o.formGuest,o.view.uploadEvent);
 auto*l=nfs_player_scene_live::current(g);if(!l||l->invalid||!l->on||l->mainDepth!=1||l->formGuest!=o.formGuest||!nfs_player_scene_live::archiveCurrent(*l))return false;
 const nfs_child_static_scene::Domain live{l->game,l->thread,l->formGuest,l->ownerEpoch,l->archive,o.view.uploadEvent,0,0,l->catalogue};std::shared_ptr<const nfs_child_static_scene::SidebarRecord>side;
 return nfs_child_static_scene::acquirePlayerHistoryForNativeRevalidation(live,l->form,side)==o.history&&side==o.historySidebarSource&&Owner::current(const_cast<Owner*>(&o));
}
// Original40D1B0 temporarily restores the outgoing Player selection while
// animating incoming photo tiles. This receipt is minted ONLY immediately after
// its427ED0 return, when the ordinary current-photo proof still matches new car.
inline bool playerPreviewPhotoCurrent(Game&g,const Owner&o,bool restored)noexcept{try{
 const auto q=o.playerPhoto;auto*l=nfs_player_scene_live::current(g);
 if(!o.playerPhotoReceipt||!q||!l||l->invalid||q!=nfs_player_photo_live::published()||q->game!=o.view.sourceGame||q->game!=reinterpret_cast<uintptr_t>(&g)||q->thread!=nfs_logo_publisher::threadToken()||q->epoch!=o.view.ownerEpoch||q->epoch!=l->ownerEpoch||q->form!=o.view.form||q->form!=l->form||l->formGuest!=o.formGuest||l->catalogue!=o.catalog||q->archive!=o.view.sourceArchive||q->car!=o.photoNewCar||q->car>=NFS_PLAYER_PHOTO_SOURCE_COUNT||!q->art||!q->art->current()||uint32_t(g.esi)!=o.formGuest||uint32_t(g.esp)!=o.photoStack||uint64_t(o.photoStack)+8>UINT64_C(0x100000000)){note(22,&o);return false;}
 nfs::GuestCopyTransaction tx;const uint32_t selected=q->form==nfs_player_preview::Player1Form?uint32_t(guest_address(byte_512DBF)):uint32_t(guest_address(byte_512DDA));
 if(tx.load<uint32_t>(dword_4D40B8)!=q->archive||(!q->graph&&q->car<NFS_PLAYER_PHOTO_STANDARD_COUNT&&tx.load<uint8_t>(byte_512DD1))||tx.load<uint8_t>(o.photoStack)!=o.photoOldCar||tx.load<uint32_t>(o.photoStack+4)!=o.photoNewCar||tx.load<uint8_t>(selected)!=(restored?o.photoOldCar:o.photoNewCar)){note(23,&o);return false;}
 if(q->graph){const bool sourceOkay=nfs_player_photo_live::graphSourceCurrent(tx,*q);if(!sourceOkay)note(24,&o);return sourceOkay;}
 const auto&source=nfs_player_photo_sources[q->car];if(uint64_t(q->archive)+source.offset+source.span>UINT64_C(0x100000000)){note(24,&o);return false;}
 const bool sourceOkay=q->source.size()==source.span&&!std::memcmp(q->source.data(),nfs_host_address(q->archive+source.offset,source.span),source.span);if(!sourceOkay)note(24,&o);return sourceOkay;
 }catch(...){return false;}}
inline bool capturePlayerPreviewPhoto(Game&g,Owner&o)noexcept{try{
 if(o.view.family!=NFS_CHILD_EXIT_PLAYER1&&o.view.family!=NFS_CHILD_EXIT_PLAYER2)return true;
 auto q=nfs_player_photo_live::published();if(!q||!nfs_player_photo_live::current(g,*q)){note(16,&o);return false;}
 uint32_t oldCar=0,newCar=0;{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);if(uint64_t(sp)+8>UINT64_C(0x100000000))return false;oldCar=tx.load<uint8_t>(sp);newCar=tx.load<uint32_t>(sp+4);}
 if(oldCar>=NFS_PLAYER_PHOTO_SOURCE_COUNT||newCar>=NFS_PLAYER_PHOTO_SOURCE_COUNT||q->car!=newCar||q->form!=o.view.form||q->epoch!=o.view.ownerEpoch||q->game!=o.view.sourceGame||q->archive!=o.view.sourceArchive){note(17,&o);return false;}
 o.playerPhoto=q;o.view.photoFamily=1;o.view.photoSerial=q->serial;o.view.photoEpoch=q->epoch;o.photoStack=uint32_t(g.esp);o.photoOldCar=oldCar;o.photoNewCar=newCar;o.playerPhotoReceipt=true;
 if(!playerPreviewPhotoCurrent(g,o,false)||!Owner::current(&o)){note(18,&o);o.playerPhotoReceipt=false;return false;}note(19,&o);return true;
 }catch(...){note(20,&o);return false;}}
inline bool refreshPhoto(Game&g,Owner&o)noexcept{try{
 if(o.view.family==NFS_CHILD_EXIT_PLAYER1||o.view.family==NFS_CHILD_EXIT_PLAYER2){if(!playerPreviewPhotoCurrent(g,o,true)){note(21,&o);return false;}return Owner::current(&o);}
 o.childPhoto.reset();o.playerPhoto.reset();o.view.photoFamily=0;o.view.photoSerial=o.view.photoEpoch=0;
 if(o.view.family==NFS_CHILD_EXIT_LOCATION||o.view.family==NFS_CHILD_EXIT_OPPONENT){auto q=nfs_child_photo_live::published();if(!q||q->game!=o.view.sourceGame||q->archive!=o.view.sourceArchive||!nfs_child_photo_live::current(g,*q))return false;o.childPhoto=q;o.view.photoFamily=q->family;o.view.photoSerial=q->serial;o.view.photoEpoch=q->epoch;}
 return Owner::current(&o);
 }catch(...){return false;}}
class FrameScope;inline FrameScope*&active(){static thread_local FrameScope*p=nullptr;return p;}
class SessionScope final{
 Game&g_;std::shared_ptr<Life>life_;std::shared_ptr<Owner>owner_;uint64_t serial_=0;uint32_t entryForm_=0;unsigned frames_=0;bool armed_=false;
public:explicit SessionScope(Game&g):g_(g){Errno e;try{life_=std::make_shared<Life>();serial_=nfs_background_scene_live::nextRequest();const uint32_t f=uint32_t(g.eax);entryForm_=f;if(serial_){owner_=captureHistory(g,f,serial_,life_);if(!owner_)owner_=capture(g,serial_,life_,f);}if(!owner_)note(11);}catch(...){}}
 void primePlayerPhoto()noexcept{Errno e;if(owner_&&!capturePlayerPreviewPhoto(g_,*owner_))owner_.reset();}
 void arm()noexcept{Errno e;if(armed_)return;armed_=true;
  // The scoped historical handoff is only Location/P1/P2. Keep the prior late
  // source-factory opportunity for the other independently validated families.
  if(!owner_&&life_&&serial_&&(entryForm_==uint32_t(guest_address(off_4D2790))||entryForm_==uint32_t(guest_address(off_4D3A88))||entryForm_==uint32_t(guest_address(off_4D3204)))&&uint32_t(g_.esi)==entryForm_)owner_=capture(g_,serial_,life_,entryForm_);
  if(!owner_)return;
  if(nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)!=owner_->view.uploadEvent){note(12,owner_.get());owner_.reset();return;}
  if(owner_->history&&!historyLeaseCurrent(g_,*owner_)){note(13,owner_.get());owner_.reset();return;}
  if(owner_->history&&!actualSidebarKeys(*owner_,!owner_->historySidebarSource)){note(15,owner_.get());owner_.reset();return;}
  if(!refreshPhoto(g_,*owner_)){note(14,owner_.get());owner_.reset();}
 }
 std::shared_ptr<Owner>frame(int32_t motion,bool cleared){Errno e;if(!armed_||!owner_||!Owner::current(owner_.get())||((owner_->view.family==NFS_CHILD_EXIT_PLAYER1||owner_->view.family==NFS_CHILD_EXIT_PLAYER2)&&!playerPreviewPhotoCurrent(g_,*owner_,true))||motion!=-29+int32_t(frames_)*12||frames_>=21||cleared!=(frames_<2)||(owner_->history?(!historyLeaseCurrent(g_,*owner_)||!actualSidebarKeys(*owner_,false)):(mt::state().slots[0]!=owner_->sidebar||!mt::unchanged(*owner_->sidebar)))||uint32_t(g_.esi)!=owner_->formGuest){Owner::rejectLease(owner_.get());note(2,owner_.get());return {};}++frames_;return owner_;}
 ~SessionScope(){Errno e;if(!life_)return;auto life=life_;try{nfs::ScopedWorkerStopDeferral d;(void)nfs::enqueue_owned_renderer([life]{life->alive.store(false,std::memory_order_release);return int32_t(1);});}catch(...){life->alive.store(false,std::memory_order_release);}}
};
class FrameScope final{
 Game&g_;std::shared_ptr<Owner>owner_;uint64_t serial_=0;unsigned stage_=0,section_=0,ordinal_=0;bool accepted_=false,playerPhotoBorrow_=false;uint32_t selectedDescriptor_=0,selected_[7]{};NfsChildExitQuad texture_{};uint32_t resourceId_=0,resourceCategory_=0;int32_t resourceAnchor_=0,resourceY_=0;
 void cancel()noexcept{Errno e;if(!accepted_)return;accepted_=false;const bool photo=playerPhotoBorrow_;playerPhotoBorrow_=false;try{auto keep=owner_;const auto serial=serial_;nfs::ScopedWorkerStopDeferral d;(void)nfs::enqueue_owned_renderer([keep,serial,photo]{if(photo)(void)nfs_private_player_photo_end(0);return int32_t(nfs_private_child_exit_cancel(serial));});}catch(...){}}
 bool send(unsigned stage,int32_t x=0,int32_t y=0){Errno e;if(!accepted_||!owner_||!Owner::current(owner_.get()))return false;auto keep=owner_;const auto serial=serial_;const auto r=nfs::enqueue_owned_renderer([keep,serial,stage,x,y]{if(!Owner::current(keep.get()))return int32_t(nfs_private_child_exit_cancel(serial));return int32_t(nfs_private_child_exit_stage(serial,stage,x,y));});if(r!=nfs::OwnedRendererAcceptance::Consumed)return false;stage_=stage;return true;}
public:FrameScope(Game&g,SessionScope&s):g_(g){Errno e;if(active())return;const int32_t motion=int32_t(g.edi);const bool clear=uint32_t(g.ebp)<2;owner_=s.frame(motion,clear);if(!owner_)return;serial_=nfs_background_scene_live::nextRequest();if(!serial_)return;auto keep=owner_;const auto serial=serial_;auto v=owner_->view;v.serial=serial;v.motion=motion;v.clearedFrame=clear;if(!clear){if(!photoDescriptorsView(v,true)){note(5,owner_.get());return;}v.photoDescriptorCount=100;}switch(v.family){case NFS_CHILD_EXIT_SETUP:v.setup.scene.serial=serial;break;case NFS_CHILD_EXIT_OPTIONS:v.options.scene.serial=serial;break;case NFS_CHILD_EXIT_LOCATION:v.location.scene.serial=serial;break;case NFS_CHILD_EXIT_OPPONENT:v.opponent.scene.serial=serial;break;case NFS_CHILD_EXIT_PLAYER1:case NFS_CHILD_EXIT_PLAYER2:v.player.scene.serial=serial;break;default:return;}if(!nfs_menu_cached_snapshot(v.form,owner_->formGuest,v.ownerEpoch,v.sourceGame,serial,owner_->full->contentRevision,&v.widgets)){note(3,owner_.get());return;}const auto r=nfs::enqueue_owned_renderer([keep,v]{return int32_t(Owner::current(keep.get())?nfs_private_child_exit_begin(&v):0);});accepted_=r==nfs::OwnedRendererAcceptance::Consumed;if(accepted_)active()=this;}
 void cleared(){if(accepted_)(void)send(NFS_CHILD_EXIT_CLEAR);}void full(){if(accepted_)(void)send(NFS_CHILD_EXIT_FULL);}void photoBegin(){if(accepted_&&!section_){section_=NFS_CHILD_EXIT_PHOTO_BEGIN;ordinal_=0;selectedDescriptor_=0;if(!send(NFS_CHILD_EXIT_PHOTO_BEGIN,int32_t(g_.eax),int32_t(g_.edx))){cancel();return;}
  // Ordinary DrawScope intentionally rejects incoming art while the original
  // restores old Player selection. Only this exact preview receipt may bridge it.
  if(owner_->playerPhotoReceipt&&owner_->photoOldCar!=owner_->photoNewCar){
   if(!playerPreviewPhotoCurrent(g_,*owner_,true)||nfs_player_photo_live::current(g_,*owner_->playerPhoto)){note(27,owner_.get());cancel();return;}
   auto keep=owner_;const auto serial=serial_;const int32_t x=int32_t(g_.eax),y=int32_t(g_.edx);
   const auto r=nfs::enqueue_owned_renderer([keep,serial,x,y]{const int okay=Owner::current(keep.get())&&keep->playerPhoto&&nfs_private_player_photo_begin(keep->playerPhoto->serial,x,y);if(!okay)(void)nfs_private_child_exit_cancel(serial);return int32_t(okay);});
   if(r!=nfs::OwnedRendererAcceptance::Consumed){note(27,owner_.get());cancel();return;}playerPhotoBorrow_=true;note(25,owner_.get());
  }
 }}void photoEnd(){if(accepted_&&section_==NFS_CHILD_EXIT_PHOTO_BEGIN){
  if(playerPhotoBorrow_){auto keep=owner_;const auto serial=serial_;playerPhotoBorrow_=false;
   const auto r=nfs::enqueue_owned_renderer([keep,serial]{const int okay=nfs_private_player_photo_end(Owner::current(keep.get())&&keep->playerPhoto?keep->playerPhoto->serial:0);if(!okay)(void)nfs_private_child_exit_cancel(serial);return int32_t(okay);});
   if(r!=nfs::OwnedRendererAcceptance::Consumed){note(28,owner_.get());cancel();return;}note(26,owner_.get());
  }
  (void)send(NFS_CHILD_EXIT_PHOTO_END);section_=0;selectedDescriptor_=0;
 }}void sidebar(){if(accepted_)(void)send(NFS_CHILD_EXIT_SIDEBAR);}
 void section(unsigned start){if(!accepted_||section_)return;section_=start;ordinal_=0;selectedDescriptor_=0;(void)send(start);}void sectionEnd(unsigned end){if(!accepted_||!section_)return;section_=0;selectedDescriptor_=0;(void)send(end);}void ready(){if(accepted_)(void)send(NFS_CHILD_EXIT_READY);}void end(){if(accepted_)(void)send(NFS_CHILD_EXIT_END);}
 void resourceBegin(Game&g)noexcept{Errno e;if(!accepted_||section_!=NFS_CHILD_EXIT_WORDS_BEGIN)return;if(resourceId_||uint32_t(g.eax)<0x58||uint32_t(g.eax)>0x5b){cancel();return;}resourceId_=uint32_t(g.eax);resourceCategory_=uint32_t(g.ecx);resourceAnchor_=int32_t(g.edx);resourceY_=int32_t(g.ebx);}
 void resourceEnd()noexcept{resourceId_=resourceCategory_=0;resourceAnchor_=resourceY_=0;}
 void selected(Game&g)noexcept{Errno e;if(!accepted_||!section_)return;try{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);NfsChildExitQuad q{};q.tmu=tx.load<uint32_t>(sp);q.address=tx.load<uint32_t>(sp+4);q.mask=tx.load<uint32_t>(sp+8);q.descriptor=tx.load<uint32_t>(sp+12);if(q.tmu||q.mask!=3||!descriptor(tx,q.descriptor,q.descriptorWords)||q.address!=q.descriptorWords[5])throw 1;q.descriptorWords[4]=0;texture_=q;selectedDescriptor_=q.descriptor;}catch(...){cancel();}}
 void quad(Game&g)noexcept{Errno e;if(!accepted_||!section_)return;if(!selectedDescriptor_||ordinal_>=2048){cancel();return;}try{NfsChildExitQuad q=texture_;q.serial=serial_;q.section=section_;q.ordinal=ordinal_++;q.resourceId=resourceId_;q.resourceCategory=resourceCategory_;q.resourceAnchor=resourceAnchor_;q.resourceY=resourceY_;{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp),a=tx.load<uint32_t>(sp),b=tx.load<uint32_t>(sp+4),c=tx.load<uint32_t>(sp+8);if(uint64_t(sp)+12!=a||uint64_t(a)+60!=b||uint64_t(a)+120!=c||uint64_t(a)+240>UINT64_C(0x100000000))throw 1;std::memcpy(q.nativeVertices,nfs_host_address(a,240),240);for(unsigned k=0;k<4;++k){const uint32_t index=tx.load<uint32_t>(uint32_t(g.esi)+4*k);if(index>4095)throw 1;q.indices[k]=index;const uint64_t at=uint64_t(uint32_t(g.edi))+32*uint64_t(index);if(at+32>UINT64_C(0x100000000))throw 1;std::memcpy(q.sourceVertices[k],nfs_host_address(uint32_t(at),32),32);}}auto keep=owner_;(void)nfs::enqueue_owned_renderer([keep,q]{return int32_t(Owner::current(keep.get())?nfs_private_child_exit_quad(&q):nfs_private_child_exit_cancel(q.serial));});}catch(...){cancel();}}
 ~FrameScope(){Errno e;if(active()==this)active()=nullptr;if(accepted_&&stage_!=NFS_CHILD_EXIT_END)cancel();}
};
struct ResourceScope{FrameScope*p=nullptr;explicit ResourceScope(Game&g)noexcept{p=active();if(p)p->resourceBegin(g);}~ResourceScope(){if(p&&active()==p)p->resourceEnd();}};
inline void selected(Game&g)noexcept{if(auto*p=active())p->selected(g);}inline void nativeQuad(Game&g)noexcept{if(auto*p=active())p->quad(g);}
} // namespace

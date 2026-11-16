// SPDX-License-Identifier: MIT
#pragma once
#include "ChildExitPreludeAbi.h"
#include "BackgroundScenePrepare.h"
#include "LocationScenePrepare.h"
#include "OpponentScenePrepare.h"
#include "PlayerScenePrepare.h"
#include "Main343OriginLifetime.h"
#include "OriginalUiRecords.h"
#include <atomic>
#include <memory>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
namespace nfs_child_static_scene {
namespace bg=nfs_background_scene_cpu;
struct Domain {
 uintptr_t game=0,thread=0;uint32_t formGuest=0;uint64_t ownerEpoch=0;uint32_t archive=0;
 uint64_t uploadEvent=0,groupSerial=0,copySerial=0;
 std::shared_ptr<const bg::SourceCatalog>catalog;
};
struct Token {uintptr_t game=0,thread=0;uint32_t formGuest=0;uint64_t epoch=0;std::atomic<bool>alive{true};};
struct Record {
 Domain domain;std::shared_ptr<const void>typed;std::shared_ptr<const bg::Prepared>full;
 NfsChildExitPreludeView view{};std::shared_ptr<const std::vector<uint8_t>>sidebarRaw;
 nfs_logo_capture::Target sidebarTarget{};std::array<uint8_t,32>sidebarSha{};
 std::shared_ptr<const nfs_main_sidebar_origin::GameLife>gameLife;std::shared_ptr<const Token>token;
};
class MenuScope;
inline MenuScope*&active()noexcept{static thread_local MenuScope*p=nullptr;return p;}
class MenuScope final {
 MenuScope*previous_=nullptr;std::shared_ptr<Token>token_;std::shared_ptr<const Record>record_;
public:
 MenuScope(uintptr_t game,uintptr_t thread,uint32_t formGuest)noexcept:previous_(active()){
  try{static std::atomic<uint64_t>next{0};uint64_t n=next.load();do{if(n==UINT64_MAX){active()=this;return;}}while(!next.compare_exchange_weak(n,n+1));
   token_=std::make_shared<Token>();token_->game=game;token_->thread=thread;token_->formGuest=formGuest;token_->epoch=n+1;
  }catch(...){}active()=this;
 }
 ~MenuScope(){if(token_)token_->alive.store(false,std::memory_order_release);record_.reset();if(active()==this)active()=previous_;}
 MenuScope(const MenuScope&)=delete;MenuScope&operator=(const MenuScope&)=delete;
 const std::shared_ptr<Token>&token()const noexcept{return token_;}
 const std::shared_ptr<const Record>&record()const noexcept{return record_;}
 void store(std::shared_ptr<const Record>r)noexcept{record_=std::move(r);}
};
inline bool matching(const Token&t,uintptr_t game,uintptr_t thread,uint32_t form)noexcept{return t.alive.load(std::memory_order_acquire)&&t.epoch&&t.game==game&&t.thread==thread&&t.formGuest==form;}
inline bool current(const Record&r,uintptr_t game,uintptr_t thread,uint32_t form,uint64_t event)noexcept{
 auto*s=active();return s&&r.token&&s->token()==r.token&&matching(*r.token,game,thread,form)&&r.gameLife&&r.gameLife->current()&&r.domain.uploadEvent==event&&event&&r.domain.game==game&&r.domain.thread==thread&&r.domain.formGuest==form;
}
inline void diagnostic(unsigned reason,uint64_t a=0,uint64_t b=0)noexcept{const int saved=errno;const char*on=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(on&&on[0]=='1'&&!on[1]){static thread_local unsigned count[32]{};if(reason<32&&count[reason]++<3)std::fprintf(stderr,"CHILD STATIC source reason=%u a=%llu b=%llu\n",reason,(unsigned long long)a,(unsigned long long)b);}errno=saved;}
inline std::shared_ptr<const Record>acquire(uintptr_t game,uintptr_t thread,uint32_t form,uint64_t event)noexcept{
 auto*s=active();if(!s||!s->token()){diagnostic(1);return {};}if(!s->record()){diagnostic(2);return {};}const auto&r=*s->record();
 if(s->token()->formGuest!=form||r.domain.formGuest!=form){diagnostic(3,s->token()->formGuest,form);return {};}
 if(r.domain.uploadEvent!=event){diagnostic(4,r.domain.uploadEvent,event);return {};}
 if(!current(r,game,thread,form,event)){diagnostic(5);return {};}diagnostic(6,r.domain.ownerEpoch,event);return s->record();}
template<class T>inline bool validFamily(const T&,const NfsChildExitPreludeView&)noexcept{return false;}
// Opponent is enrolled only by its exact accepted full-source factory below.
// Immutable original343 bytes are evidence; transition GPU checks remain live.
inline bool validFamily(const nfs_opponent_scene_cpu::Prepared&p,const NfsChildExitPreludeView&v)noexcept{return v.family==NFS_CHILD_EXIT_OPPONENT&&v.form==0x4d3204u&&v.opponent.schema&&p.captionProof&&p.captionProof->current()&&p.preview&&p.preview->sourceAuthenticated();}
inline bool validFamily(const nfs_location_scene_cpu::Prepared&,const NfsChildExitPreludeView&v)noexcept{return v.family==NFS_CHILD_EXIT_LOCATION&&v.form==0x4d2a20&&v.location.schema;}
inline bool validFamily(const nfs_player_scene_cpu::Prepared&p,const NfsChildExitPreludeView&v)noexcept{return ((v.family==NFS_CHILD_EXIT_PLAYER1&&v.form==0x4d3050)||(v.family==NFS_CHILD_EXIT_PLAYER2&&v.form==0x4d3100))&&v.player.schema&&p.form==v.form;}
// Called only after the family-specific accepted-source proof, never from a
// partial collector. Immutable bytes preserve historical evidence, not live GPU
// authority: each transition still proves actual descriptors/pixels and passes.
template<class Prepared>inline bool remember(const Domain&d,const std::shared_ptr<const Prepared>&p,NfsChildExitPreludeView v,const nfs_logo_capture::Copy&c)noexcept{
 try{auto*s=active();if(!s||!s->token()||!matching(*s->token(),d.game,d.thread,d.formGuest)||!p||!p->valid||!d.catalog||!d.ownerEpoch||!d.archive||!d.uploadEvent||!d.copySerial||!d.groupSerial)return false;
 if(!validFamily(*p,v))return false;
 if(!c.complete||c.error||!c.rows||c.span!=nfs_logo_capture::MaxCopySource||c.serial!=d.copySerial||c.linkedGroup!=d.groupSerial||c.thread!=d.thread||!c.args[0]||c.sourceOwner.w[8]!=c.args[0]||c.sourceBefore!=c.sourceAfter||c.shaBefore!=c.shaAfter||c.post.pixels!=p->original)return false;
 for(unsigned k=0;k<4;++k)if(bg::le32(c.sourceAfter.data()+4*k)!=c.sourceOwner.header[k])return false;
 auto life=nfs_main_sidebar_origin::acquireGameLife(d.game);if(!life||!life->current())return false;
 auto q=std::make_shared<Record>();q->domain=d;q->typed=p;q->full=p;q->token=s->token();q->gameLife=life;
 q->sidebarTarget=c.sourceOwner;q->sidebarSha=c.shaAfter;q->sidebarRaw=std::make_shared<const std::vector<uint8_t>>(c.sourceAfter.begin(),c.sourceAfter.begin()+c.span);
 v.schema=1;v.ownerEpoch=d.ownerEpoch;v.uploadEvent=d.uploadEvent;v.sourceGame=d.game;v.sourceCatalog=reinterpret_cast<uintptr_t>(d.catalog.get());v.sourceArchive=d.archive;v.sourceLife=life->serial();
 v.owner=nullptr;v.retain=nullptr;v.release=nullptr;v.current=nullptr;q->view=v;s->store(q);return true;
 }catch(...){return false;}
}
} // namespace nfs_child_static_scene

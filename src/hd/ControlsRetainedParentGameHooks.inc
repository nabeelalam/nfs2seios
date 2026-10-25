// SPDX-License-Identifier: MIT
#pragma once
// Retain one genuinely prepared Controls scene. Historical pixels do not grant
// current scene/native authority: Feedback entry independently reacquires the
// explicit Main bitmap, original owners, authority and retained sidebar again.
#include "ControlsScenePrepare.h"
#include "ControlsLiveGameHooks.inc"
#include "ControlsSidebarLiveGameHooks.inc"
#include "FeedbackModalInvocationGameHooks.inc"
namespace nfs_controls_lazy_parent_live {struct CopyScope;}
namespace nfs_controls_retained_parent {
namespace live=nfs_controls_live;
namespace cache=nfs_controls_sidebar_live;
namespace modal=nfs_feedback_modal_live;
using Prepared=nfs_controls_scene_cpu::Prepared;
using DialogProof=nfs_controls_dialog_word::Proof;
using Invocation=modal::Invocation;
using Target=nfs_logo_capture::Target;

namespace detail {
struct Retained {
 std::shared_ptr<const Prepared>prepared;
 std::shared_ptr<const DialogProof>dialog;
 nfs_audio_frame::Binding binding{};
 nfs_audio_sidebar::Binding cacheBinding{};
 nfs_graphics_frame::SidebarBinding sidebar{};
 Target main{},cacheTarget{};
 uintptr_t game=0,thread=0;
};
inline std::shared_ptr<const Retained>&slot()noexcept{
 static thread_local std::shared_ptr<const Retained>p;
 return p;
}
// Source-sealed historical update: only the actual original-copy scope may
// construct it, after current source ownership, exact replay and recipe checks.
class LazyUpdate final {
 std::shared_ptr<const Retained>retained_;
 std::shared_ptr<const nfs_audio_sidebar::Packet>source_;
 std::vector<uint16_t>after_;Target main_{};uint32_t key_=0;
 LazyUpdate(std::shared_ptr<const Retained>r,std::shared_ptr<const nfs_audio_sidebar::Packet>s,std::vector<uint16_t>after,const Target&m,uint32_t key)
  :retained_(std::move(r)),source_(std::move(s)),after_(std::move(after)),main_(m),key_(key){}
 friend struct ::nfs_controls_lazy_parent_live::CopyScope;
public:
 const std::shared_ptr<const Retained>&retained()const noexcept{return retained_;}
 const std::shared_ptr<const nfs_audio_sidebar::Packet>&source()const noexcept{return source_;}
 const std::vector<uint16_t>&after()const noexcept{return after_;}
 const Target&main()const noexcept{return main_;}
 uint32_t key()const noexcept{return key_;}
};
inline std::shared_ptr<const LazyUpdate>&updateSlot()noexcept{static thread_local std::shared_ptr<const LazyUpdate>p;return p;}
// The ambient retained-sidebar selector is not a Main bitmap generation. This
// is the sole ignored field; every Main descriptor/LUT/header and all four
// original owner handles still match exactly. Original collectors separately
// authenticate the current selected sidebar table entry and raw source below.
inline bool sameMain(const Target&a,const Target&b)noexcept{
 Target selected=b;selected.ownerState[0]=a.ownerState[0];
 return nfs_logo_capture::same(a,selected);
}
inline bool owners(Game&g,const Retained&r,
                   nfs_controls_live::State*&s,nfs_controls_sidebar_live::State*&c)noexcept{
 s=live::current(g);c=cache::current(g);
 if(!r.prepared||!r.dialog||!s||!c||r.game!=reinterpret_cast<uintptr_t>(&g)||
    r.thread!=nfs_logo_publisher::threadToken()||!live::bound(*s,g,true)||!cache::bound(*c,g)||
    !(s->binding==r.binding)||!(c->binding==r.cacheBinding)||s->epoch!=c->epoch||
    s->epoch!=r.binding.ownerEpoch||live::lastPacket(g)!=r.prepared->frame()||
    r.prepared->frame()->text()!=r.dialog->packet()||!r.dialog->current())return false;
 auto*t=nfs_controls_text::current(g);
 if(!t||t->ownerEpoch!=r.binding.ownerEpoch||t->lifetime!=r.binding.lifetime||
    t->packet!=r.dialog->packet()||t->dialogProof!=r.dialog||t->serial!=s->acceptedTextSerial)return false;
 nfs_controls_dialog_word::Authority authority;
 if(!nfs_controls_text::readDialogAuthority(authority)||authority.selectedIndex!=2||
    !(authority==r.dialog->evidence().authority)||r.dialog->evidence().game!=r.game||
    r.dialog->evidence().thread!=r.thread||r.dialog->evidence().serial!=t->serial||
    r.dialog->evidence().lifetime!=t->lifetime)return false;
 return true;
}
inline bool sidebar(Game&g,const Retained&r,nfs_controls_live::State&s)noexcept{
 nfs_graphics_frame::SidebarBinding current;
 if(!live::acquireCurrentSidebarBinding(g,current)||!(current==r.sidebar))return false;
 Target target;
 auto p=cache::acquireForSidebar(g,s.background.sidebar,current,&target);
 return p&&p==r.prepared->sidebar()&&nfs_logo_capture::same(target,r.cacheTarget)&&
        p->binding()==r.cacheBinding;
}
// Original unavailable-Feedback entry starts a partial child-sidebar draw.
// That invalidates the live343 packet while leaving the complete Main bitmap
// unchanged. The underlay is the owned prior scene, never the invalid new cache.
inline bool partialFeedbackCache(const nfs_controls_sidebar_live::State&c)noexcept{
 // The original child can stop before words, during logos, or at another
 // incomplete stage. It never supplies an accepted replacement cache here.
 return c.invalid&&c.failure==cache::Failure::order&&
    c.phase!=cache::Phase::complete&&!c.published;
}
inline bool feedbackOwners(Game&g,const Retained&r,live::State*&s,cache::State*&c)noexcept{
 s=live::current(g);c=cache::current(g);
 if(!r.prepared||!r.dialog||!s||!c||r.game!=reinterpret_cast<uintptr_t>(&g)||
    r.thread!=nfs_logo_publisher::threadToken()||!live::bound(*s,g,true)||
    !cache::ownerCurrent(*c,g)||s->epoch!=c->epoch||c->thread!=r.thread||
    c->archive!=r.cacheBinding.archive||c->binding.archive!=r.cacheBinding.archive||
    c->binding.form!=r.binding.form||c->binding.ownerEpoch!=r.binding.ownerEpoch||
    !(s->binding==r.binding)||s->epoch!=r.binding.ownerEpoch||
    live::lastPacket(g)!=r.prepared->frame()||
    r.prepared->frame()->text()!=r.dialog->packet()||!r.dialog->current())return false;
 // A newer valid sidebar revision does not replace the already displayed
 // Main underlay. Unknown invalid producers still reject this finite route.
 if(c->invalid&&!partialFeedbackCache(*c))return false;
 auto*t=nfs_controls_text::current(g);
 if(!t||t->ownerEpoch!=r.binding.ownerEpoch||t->lifetime!=r.binding.lifetime||
    t->packet!=r.dialog->packet()||t->dialogProof!=r.dialog||t->serial!=s->acceptedTextSerial)return false;
 nfs_controls_dialog_word::Authority authority;
 return nfs_controls_text::readDialogAuthority(authority)&&authority.selectedIndex==2&&
    authority==r.dialog->evidence().authority&&r.dialog->evidence().game==r.game&&
    r.dialog->evidence().thread==r.thread&&r.dialog->evidence().serial==t->serial&&
    r.dialog->evidence().lifetime==t->lifetime;
}
inline bool feedbackSidebar(Game&g,const Retained&r,live::State&s,cache::State&c)noexcept{
 (void)g;(void)s;
 if(c.invalid&&!partialFeedbackCache(c))return false;
 // These are immutable copies retained at the fully verified ordinary queue
 // boundary, regardless of newer cache revisions. Fresh Main equality below
 // proves this recipe is still the shown underlay; no new cache pixels are used.
 const auto&frozen=r.prepared->sidebar();
 return frozen&&frozen->binding()==r.cacheBinding&&frozen->raw()==r.sidebar.source&&
    r.prepared->frame()->background()&&
    r.prepared->frame()->background()->sidebar()==r.sidebar;
}
} // namespace detail

inline void forget()noexcept{detail::updateSlot().reset();detail::slot().reset();}

// Call only after the existing ordinary Controls queue's repeat acquisition
// and native EAX/Main-owner gates. Recheck them here; no caller-supplied numeric
// evidence can substitute for the independently current collector packets.
inline void remember(Game&g,const std::shared_ptr<const Prepared>&prepared,
                     const std::shared_ptr<const DialogProof>&dialog)noexcept{
 forget();
 try{
  if(!prepared||!dialog||!prepared->frame()||!prepared->sidebar()||
     dialog->evidence().authority.selectedIndex!=2||!dialog->current())return;
  auto*s=live::current(g);auto*c=cache::current(g);
  if(!s||!c||!live::bound(*s,g,true)||!cache::bound(*c,g)||s->epoch!=c->epoch||
     live::acquireCurrentPacket(g)!=prepared->frame()||
     nfs_controls_text::acquireCurrentPacket(g)!=prepared->frame()->text()||
     nfs_controls_text::acquireCurrentDialogProof(g)!=dialog||
     dialog->packet()!=prepared->frame()->text()||
     uint32_t(g.eax)!=s->binding.surface.geometry[8])return;
  auto r=std::make_shared<detail::Retained>();r->prepared=prepared;r->dialog=dialog;
  r->game=reinterpret_cast<uintptr_t>(&g);r->thread=nfs_logo_publisher::threadToken();
  r->binding=s->binding;r->cacheBinding=c->binding;
  if(!live::acquireCurrentSidebarBinding(g,r->sidebar)||
     cache::acquireForSidebar(g,s->background.sidebar,r->sidebar,&r->cacheTarget)!=prepared->sidebar())return;
  std::vector<uint16_t>pixels;
  if(!live::plane(pixels,r->main)||!nfs_logo_capture::same(r->main,s->target)||
     !prepared->frame()->matches(r->binding,s->acceptedTextSerial,pixels.data(),pixels.size()))return;
  {nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D4D24)!=r->binding.owner)return;}
  live::State*repeat=nullptr;cache::State*repeatCache=nullptr;
  if(!detail::owners(g,*r,repeat,repeatCache)||repeat!=s||repeatCache!=c||
     !detail::sidebar(g,*r,*s)||!live::plane(pixels,r->main)||
     !nfs_logo_capture::same(r->main,s->target)||
     !prepared->frame()->matches(r->binding,s->acceptedTextSerial,pixels.data(),pixels.size()))return;
  detail::slot()=std::move(r);
 }catch(...){forget();}
}

// This receipt records only independently checked pre-popup parent retention.
// Its private construction does not admit a final modal/native scene. A later
// join must still prove backdrop.before == this exact parent original plane,
// backdrop.pretext == modal text clean, and the final source/native pass.
class Parent final {
 std::shared_ptr<const detail::Retained>retained_;
 std::shared_ptr<const Invocation>invocation_;
 std::shared_ptr<const detail::LazyUpdate>update_;
 Parent(std::shared_ptr<const detail::Retained>r,std::shared_ptr<const Invocation>i,std::shared_ptr<const detail::LazyUpdate>u)
  :retained_(std::move(r)),invocation_(std::move(i)),update_(std::move(u)){}
 friend std::shared_ptr<const Parent>acquireForFeedback(Game&)noexcept;
public:
 NfsControlsSceneFrameView view(uint64_t serial)const noexcept{
  auto v=retained_->prepared->view(serial);if(!v.schema||!update_)return v;
  v=retained_->prepared->viewWithSidebar(serial,update_->source(),v.layers.background.mapRole);
  if(v.schema){v.original565=update_->after().data();v.planeWords=update_->after().size();}return v;
 }
 const std::shared_ptr<const Prepared>&prepared()const noexcept{return retained_->prepared;}
 const std::shared_ptr<const Invocation>&invocation()const noexcept{return invocation_;}
 const std::vector<uint16_t>&original()const noexcept{return update_?update_->after():retained_->prepared->frame()->text()->original();}
 // Historical join identity only; this is intentionally not named "current".
 bool sameInvocation(const std::shared_ptr<const Invocation>&i)const noexcept{
  return i&&i==invocation_&&original()==i->before();
 }
};

// Must be called at genuine Feedback entry, before original popup painting.
// The ambient selected window may be unselected; ONLY the explicit Main owner
// is read here. Existing Controls full-plane acquisition rules are unchanged.
inline std::shared_ptr<const Parent>acquireForFeedback(Game&g)noexcept{
 try{
  const auto retained=detail::slot();auto invocation=modal::activeInvocation(g);
  if(!retained||!invocation||uint32_t(g.eax)!=2||uint32_t(g.edx)!=invocation->binding().owner)return {};
  const auto update=detail::updateSlot();
  if(update&&(update->retained()!=retained||!update->source()||!detail::sameMain(update->main(),retained->main)))return {};
  const auto&expected=update?update->after():retained->prepared->frame()->text()->original();
  live::State*s=nullptr;cache::State*c=nullptr;
  if(!detail::feedbackOwners(g,*retained,s,c)||
     invocation->parentEpoch()!=retained->binding.ownerEpoch||
     invocation->lifetime()!=retained->binding.lifetime||
     invocation->game()!=retained->game||invocation->thread()!=retained->thread||
     invocation->binding().form!=retained->binding.form||
     invocation->binding().targetOwner!=retained->binding.owner||
     invocation->authority().archive!=retained->binding.archive||
     !(invocation->authority()==retained->dialog->evidence().authority)||
     !(invocation->binding().surface==retained->binding.surface)||
     invocation->before()!=expected||
     !detail::sameMain(invocation->initialTarget(),retained->main)||
     !detail::feedbackSidebar(g,*retained,*s,*c))return {};
  Target now;modal::bd::Plane pixels;
  if(!modal::explicitMain(now,pixels)||!detail::sameMain(now,retained->main)||
     !nfs_logo_capture::same(now,invocation->initialTarget())||
     pixels!=invocation->before()||
     (!update&&!retained->prepared->frame()->matches(retained->binding,s->acceptedTextSerial,pixels.data(),pixels.size())))return {};
  live::State*repeat=nullptr;cache::State*repeatCache=nullptr;
  if(detail::slot()!=retained||detail::updateSlot()!=update||modal::activeInvocation(g)!=invocation||
     !detail::feedbackOwners(g,*retained,repeat,repeatCache)||repeat!=s||repeatCache!=c||
     !detail::feedbackSidebar(g,*retained,*s,*c)||!modal::explicitMain(now,pixels)||
     !nfs_logo_capture::same(now,invocation->initialTarget())||pixels!=invocation->before())return {};
  return std::shared_ptr<const Parent>(new Parent(retained,invocation,update));
 }catch(...){return {};}
}
} // namespace nfs_controls_retained_parent

// SPDX-License-Identifier: MIT
#pragma once
// CPU-owned complete popup join only. Native upload, renderer ownership,
// context/fade, staging and final publication require separate admission.
#include "ControlsRetainedParentGameHooks.inc"
#include "FeedbackModalBackdropGameHooks.inc"
#include "FeedbackModalTextGameHooks.inc"
namespace nfs_feedback_modal_scene_live {
namespace modal=nfs_feedback_modal_live;
namespace bd=nfs_feedback_modal_backdrop_live;
namespace txt=nfs_feedback_modal_text_live;
namespace proof=nfs_feedback_modal_text;
using Parent=nfs_controls_retained_parent::Parent;
using Invocation=modal::Invocation;
namespace detail {
inline proof::Evidence evidence(const txt::Pending&t)noexcept{
 proof::Evidence e;const auto&i=t.invocation();if(!i||!t.packet()||!t.packet()->font())return e;
 e.source=t.source();e.binding=i->binding();e.game=t.game();e.thread=t.thread();
 e.serial=t.packet()->serial();e.fontIdentity=t.packet()->font()->identity();
 e.caller=e.source.caller;e.modal=e.source.modal;e.count=2;e.feedbackAvailable=0;
 e.controlsArchive=t.authority().archive;e.authority=t.authority();e.resources=t.resources();e.post=t.post();return e;
}
inline bool chain(const std::shared_ptr<const Parent>&parent,
                  const std::shared_ptr<const bd::Pending>&backdrop,
                  const std::shared_ptr<const txt::Pending>&text,
                  const std::shared_ptr<const Invocation>&invocation)noexcept{
 if(!parent||!backdrop||!text||!invocation||!parent->prepared()||!backdrop->verified()||!text->packet()||
    parent->invocation()!=invocation||backdrop->invocation()!=invocation||text->invocation()!=invocation||
    !parent->sameInvocation(invocation)||!(backdrop->verified()->binding()==invocation->binding())||
    text->epoch()!=invocation->modalEpoch()||text->array()!=invocation->binding().owner||
    text->game()!=invocation->game()||text->thread()!=invocation->thread()||
    !(text->authority()==invocation->authority()))return false;
 const auto&f=parent->prepared()->frame();if(!f||!f->text()||
    f->binding().ownerEpoch!=invocation->parentEpoch()||f->binding().lifetime!=invocation->lifetime()||
    f->binding().form!=invocation->binding().form||f->binding().owner!=invocation->binding().targetOwner||
    !(f->binding().surface==invocation->binding().surface))return false;
 // Both complete transitions are mandatory. Matching owners or command counts
 // alone cannot hide unobserved writes between the backdrop and first string.
 return parent->original()==backdrop->verified()->before()&&
    text->beforeMain()==parent->original()&&
    backdrop->verified()->pretext()==text->packet()->clean();
}
inline bool finalPlane(Game&g,const std::shared_ptr<const Invocation>&invocation,
                       const txt::Pending&t,modal::bd::Plane&pixels)noexcept{
 modal::bd::Target target;
 return modal::activeInvocation(g)==invocation&&modal::explicitMain(target,pixels)&&
    nfs_logo_capture::same(target,t.post().target)&&
    t.packet()->matches(invocation->binding().surface,t.epoch(),pixels.data(),pixels.size());
}
} // namespace detail
class Joined;
inline std::shared_ptr<const Joined>acquire(Game&,const std::shared_ptr<const Parent>&)noexcept;
class Joined final {
 std::shared_ptr<const Parent>parent_;std::shared_ptr<const bd::Pending>backdrop_;
 std::shared_ptr<const txt::Pending>text_;std::shared_ptr<const proof::CheckedText>checked_;
 Joined(std::shared_ptr<const Parent>p,std::shared_ptr<const bd::Pending>b,
        std::shared_ptr<const txt::Pending>t,std::shared_ptr<const proof::CheckedText>c)
  :parent_(std::move(p)),backdrop_(std::move(b)),text_(std::move(t)),checked_(std::move(c)){}
 friend std::shared_ptr<const Joined>acquire(Game&,const std::shared_ptr<const Parent>&)noexcept;
public:
 const auto&parent()const noexcept{return parent_;}const auto&backdrop()const noexcept{return backdrop_;}
 const auto&text()const noexcept{return text_;}const auto&checkedText()const noexcept{return checked_;}
 const auto&invocation()const noexcept{return parent_->invocation();}
 const std::vector<uint16_t>&original()const noexcept{return text_->packet()->original();}
 bool current(Game&g)const noexcept{
  try{const auto i=modal::activeInvocation(g);if(!checked_||!detail::chain(parent_,backdrop_,text_,i)||
       bd::acquireCurrentPending(g)!=backdrop_||txt::acquireCurrentPending(g)!=text_)return false;
   modal::bd::Plane pixels;if(!detail::finalPlane(g,i,*text_,pixels))return false;
   proof::Current now;now.observed=detail::evidence(*text_);now.packet=text_->packet();
   now.original565=pixels.data();now.planeWords=pixels.size();
   return checked_->matches(now)&&modal::activeInvocation(g)==i&&
       bd::acquireCurrentPending(g)==backdrop_&&txt::acquireCurrentPending(g)==text_;
  }catch(...){return false;}
 }
};
inline std::shared_ptr<const Joined>acquire(Game&g,const std::shared_ptr<const Parent>&parent)noexcept{
 try{auto i=modal::activeInvocation(g);auto b=bd::acquireCurrentPending(g);auto t=txt::acquireCurrentPending(g);
  if(!detail::chain(parent,b,t,i))return {};
  auto checked=proof::CheckedText::create(t->packet(),detail::evidence(*t),b->verified());if(!checked)return {};
  // Construction remains private to the actual source-bound acquisition path.
  auto joined=std::shared_ptr<const Joined>(new Joined(parent,b,t,checked));
  return joined->current(g)?joined:std::shared_ptr<const Joined>{};
 }catch(...){return {};}
}
} // namespace nfs_feedback_modal_scene_live

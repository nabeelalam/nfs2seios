// SPDX-License-Identifier: MIT
#pragma once
#include "FeedbackModalSceneGameHooks.inc"
#include "FeedbackModalWordArtwork.h"
#include "FeedbackModalWordRuntimeAbi.h"
namespace nfs_feedback_modal_word_display {
namespace scene=nfs_feedback_modal_scene_live;
namespace text=nfs_feedback_modal_text;
namespace art=nfs_feedback_modal_word;
class Prepared final {
 std::shared_ptr<const scene::Joined>scene_;
 std::array<std::shared_ptr<const art::Artwork>,2>art_{};
 Prepared()=default;
 static bool consistent(const std::shared_ptr<const scene::Joined>&s)noexcept{
  if(!s||!s->checkedText()||!s->text()||!s->backdrop()||!s->invocation()||
     !scene::detail::chain(s->parent(),s->backdrop(),s->text(),s->invocation()))return false;
  const auto&p=s->text()->packet();const auto&checked=s->checkedText();
  if(!p||checked->packet()!=p||checked->backdrop()!=s->backdrop()->verified()||
     !text::consistent(p,checked->evidence(),s->backdrop()->verified())||
     !(checked->evidence().binding==s->invocation()->binding())||
     !(checked->evidence().authority==s->invocation()->authority()))return false;
  size_t end=0;for(unsigned i=0;i<2;++i){const auto&r=p->records()[i];
   if(!text::command(r.command,i)||r.vertexFirst!=end||!r.vertexCount||r.vertexCount%6||
      end>p->vertices().size()||r.vertexCount>p->vertices().size()-end)return false;end+=r.vertexCount;
  }
  return end==p->vertices().size();
 }
public:
 // A Joined object can only come from actual source-bound scene acquisition.
 // This adapter retains historical immutable display data only. The queue must
 // independently call scene()->current(Game&) before original native admission.
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const scene::Joined>s)noexcept{
  if(!consistent(s))return {};
  try{auto out=std::shared_ptr<Prepared>(new Prepared);
   for(unsigned i=0;i<2;++i){auto a=art::artwork(i+1);const unsigned width=(i?108u:96u)*4;
    if(!a||a->word()!=i+1||a->width()!=width||a->height()!=128||a->rgba().size()!=size_t(width)*128*4||
       std::memcmp(a->sha().data(),nfs_feedback_modal_word_sha256[i],32))return {};out->art_[i]=std::move(a);
   }out->scene_=std::move(s);return out;
  }catch(...){return {};}
 }
 const std::shared_ptr<const scene::Joined>&scene()const noexcept{return scene_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return scene_->text()->packet();}
 bool historicalConsistent()const noexcept{return consistent(scene_);}
 NfsFeedbackModalWordsView view()const noexcept{
  NfsFeedbackModalWordsView view{};if(!historicalConsistent())return view;
  view.schema=NFS_FEEDBACK_MODAL_WORD_SCHEMA;view.count=NFS_FEEDBACK_MODAL_WORD_PLACEMENTS;
  for(unsigned i=0;i<2;++i){if(!art_[i])return {};const auto&a=*art_[i];auto&image=view.images[i];
   image.word=i+1;image.width=a.width();image.height=a.height();image.rgba=a.rgba().data();image.bytes=a.rgba().size();std::copy(a.sha().begin(),a.sha().end(),image.sha256);
   const auto&r=packet()->records()[i];auto&p=view.placements[i];p.word=i+1;p.color565=r.command.color;
   p.vertexFirst=r.vertexFirst;p.vertexCount=r.vertexCount;p.left=float(r.command.x);p.top=float(r.command.y);
   p.right=p.left+text::width(i);p.bottom=p.top+32;
  }return view;
 }
};
} // namespace nfs_feedback_modal_word_display

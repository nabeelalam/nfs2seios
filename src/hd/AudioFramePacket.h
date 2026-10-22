// SPDX-License-Identifier: MIT
#pragma once
#include "AudioBackgroundReplay.h"
#include "AudioBarReplay.h"
namespace nfs_audio_frame {
class AudioFramePacket final {
 Binding binding_;uint64_t publication_=0;std::shared_ptr<const VerifiedPreBars>background_;std::shared_ptr<const VerifiedBars>bars_;std::shared_ptr<const nfs_soft::FramePacket>text_;AudioFramePacket()=default;
public:
 // current and acceptedTextSerial come from the source-bound live owner, never
 // from candidate getters. All original calls/stores must still execute.
 static std::shared_ptr<const AudioFramePacket>create(const std::shared_ptr<const VerifiedPreBars>&background,const std::shared_ptr<const VerifiedBars>&bars,const std::shared_ptr<const nfs_soft::FramePacket>&text,const Binding&current,uint64_t acceptedTextSerial,const uint16_t*currentPost,size_t count)noexcept{
  if(!current.valid()||!background||!bars||!text||!acceptedTextSerial||!currentPost||count!=Pixels||!(background->binding()==current)||!(bars->binding()==current)||background->pixels()!=bars->pre()||bars->post()!=text->clean()||!text->font()||!text->commands()||!text->writes()||text->vertices().empty()||text->pixels()!=Pixels||!text->matches(current.surface,acceptedTextSerial,currentPost,count))return {};
  try{auto p=std::shared_ptr<AudioFramePacket>(new AudioFramePacket);p->binding_=current;p->publication_=acceptedTextSerial;p->background_=background;p->bars_=bars;p->text_=text;return p;}catch(...){return {};}
 }
 const std::shared_ptr<const VerifiedPreBars>&background()const noexcept{return background_;}
 const Binding&binding()const noexcept{return binding_;}uint64_t publication()const noexcept{return publication_;}const std::shared_ptr<const nfs_soft::FramePacket>&text()const noexcept{return text_;}const std::vector<uint16_t>&preBars()const noexcept{return background_->pixels();}const OwnedBarLayout&layout()const noexcept{return bars_->layout();}const std::shared_ptr<const ArtBinding>&art()const noexcept{return bars_->art();}
 bool matches(const Binding&current,uint64_t acceptedTextSerial,const uint16_t*currentPost,size_t count)const noexcept{return current==binding_&&acceptedTextSerial==publication_&&text_->matches(current.surface,acceptedTextSerial,currentPost,count);}
};
}

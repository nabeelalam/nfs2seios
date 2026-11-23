// SPDX-License-Identifier: MIT
#pragma once
#include "CreditsWholeRuntimeAbi.h"
#include "CreditsFullFrame.h"
#include "CreditsFont.h"
#include "ClassicTextDisplay.h"
namespace nfs_credits_ui {
class PreparedWhole final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const WholeFrame>source_;
 std::shared_ptr<const nfs_classic_ui::PreparedText>text_;
 std::vector<NfsSoftwareTextVertex>vertices_;std::vector<NfsCreditsWholeEvent>events_;
 NfsCreditsArtView art_{};NfsCreditsWholeView view_{};
 PreparedWhole()=default;
 ~PreparedWhole(){if(art_.owner&&art_.release)art_.release(art_.owner);}
 static void retain(void*p)noexcept{static_cast<PreparedWhole*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p)noexcept{auto*s=static_cast<PreparedWhole*>(p);if(s->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
 static int current(const void*p)noexcept{const auto*s=static_cast<const PreparedWhole*>(p);return s&&s->source_&&s->source_->current()&&s->art_.generation&&s->art_.generation==nfs_credits_art_generation();}
public:
 static std::shared_ptr<const PreparedWhole>create(std::shared_ptr<const WholeFrame>source)noexcept{
  if(!source||!source->current()||!source->serial()||!source->scene()||source->scene()->locale()>=6||source->commands().size()!=source->colors().size())return {};
  try{auto p=std::shared_ptr<PreparedWhole>(new PreparedWhole,[](const PreparedWhole*x){release(const_cast<PreparedWhole*>(x));});p->source_=std::move(source);
   if(!nfs_credits_art_acquire(&p->art_)||p->art_.schema!=1||!p->art_.owner||!p->art_.release||!p->art_.retain||!current(p.get()))return {};
   auto&out=p->view_;out.schema=1;out.content=p->source_->serial();out.sourceDomain=p->source_->scene()->domain();out.locale=p->source_->scene()->locale();out.art=p->art_;out.text.logicalWidth=640;out.text.logicalHeight=480;
   const auto&original=p->source_->commands();std::vector<std::pair<size_t,size_t>>ranges;
   if(!original.empty()){
    auto display=original;std::vector<nfs_classic_ui::TextBox>boxes;
    for(auto&c:display){for(unsigned k=0;k<c.length;++k){auto&ch=c.text[k];switch(ch){case 0x99:ch=96;break;case 0xe0:ch=123;break;case 0xa9:ch=124;break;case 96:case 123:case 124:return {};default:if(ch<32||ch>126)return {};}}nfs_classic_ui::TextBox b;b.role=c.role;boxes.push_back(b);}
    auto font=nfs_credits_display::creditsFont();if(!font)return {};
    p->text_=nfs_classic_ui::PreparedText::createCommands(display,std::move(font),p->source_,boxes,true);if(!p->text_)return {};
    const auto layer=p->text_->layer();if(layer.vertexCount)out.text=layer;if(out.text.vertexCount)p->vertices_.assign(out.text.vertices,out.text.vertices+out.text.vertexCount);ranges=p->text_->ranges();if(ranges.size()!=original.size())return {};
    for(size_t i=0;i<ranges.size();++i){const auto range=ranges[i];if(range.first>p->vertices_.size()||range.second>p->vertices_.size()-range.first)return {};const auto color=p->source_->colors()[i];for(size_t k=range.first;k<range.first+range.second;++k){auto&v=p->vertices_[k];v.r=float((color>>16)&255)/255;v.g=float((color>>8)&255)/255;v.b=float(color&255)/255;v.a=1;}}
   }
   out.text.vertices=p->vertices_.empty()?nullptr:p->vertices_.data();out.text.vertexCount=p->vertices_.size();
   for(const auto&e:p->source_->events()){
    if(e.kind==FrameEvent::Text){if(e.index>=ranges.size())return {};p->events_.push_back({1,ranges[e.index].first,ranges[e.index].second});}
    else if(e.kind==FrameEvent::Partners){if(e.index||!p->source_->partner())return {};p->events_.push_back({2,0,0});}
    else return {};
   }
   if(const auto*partner=p->source_->partner()){
    out.hasPartner=1;out.partnerClip={partner->clip[0],partner->clip[1],partner->clip[2],partner->clip[3]};
    for(unsigned i=0;i<2;++i){const auto&d=p->source_->scene()->draws()[i];const auto&m=nfs_credits_sources[d.sourceIndex];const float x=float(d.x)+partner->x,y=float(d.y)+partner->y;out.partner[i]={x,y,x+m.width,y+m.height};}
   }
   const auto&heading=p->source_->scene()->draws()[3];const auto&m=nfs_credits_sources[heading.sourceIndex];out.heading={float(heading.x),float(heading.y),float(heading.x)+m.width,float(heading.y)+m.height};
   out.events=p->events_.empty()?nullptr:p->events_.data();out.eventCount=p->events_.size();out.owner=p.get();out.retain=retain;out.release=release;out.current=current;
   return current(p.get())?p:std::shared_ptr<const PreparedWhole>{};
  }catch(...){return {};}
 }
 NfsCreditsWholeView view(uint64_t request)const noexcept{if(!request||!current(this))return {};auto v=view_;v.request=request;return v;}
};
}

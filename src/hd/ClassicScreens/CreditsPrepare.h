// PRIVATE immutable Credits source strings, mapped only for complete HD glyph coverage.
#pragma once
#include "CreditsRuntimeAbi.h"
#include "CreditsFont.h"
#include "ClassicTextDisplay.h"
#include <atomic>
namespace nfs_credits_ui {
struct Payload {uintptr_t game=0,thread=0;uint32_t form=0,archive=0;uint64_t serial=0;std::vector<nfs_soft::TextCommand>commands;std::vector<uint32_t>argb;};
class Prepared final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const Payload>source_;std::shared_ptr<const nfs_classic_ui::PreparedText>text_;std::vector<NfsSoftwareTextVertex>vertices_;
 Prepared()=default;static void retain(void*p)noexcept{static_cast<Prepared*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}static void release(void*p)noexcept{auto*s=static_cast<Prepared*>(p);if(s->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
public:
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const Payload>source)noexcept{
  if(!source||!source->game||!source->thread||!source->form||!source->archive||!source->serial||source->commands.empty()||source->commands.size()>128||source->argb.size()!=source->commands.size())return {};
  auto font=nfs_credits_display::creditsFont();if(!font)return {};
  try{auto p=std::shared_ptr<Prepared>(new Prepared,[](const Prepared*p){release(const_cast<Prepared*>(p));});auto display=source->commands;std::vector<nfs_classic_ui::TextBox>boxes;
   for(auto&c:display){for(unsigned i=0;i<c.length;++i){auto&ch=c.text[i];switch(ch){case 0x99:ch=96;break;case 0xe0:ch=123;break;case 0xa9:ch=124;break;case 96:case 123:case 124:return {};default:if(ch<32||ch>126)return {};}}
    nfs_classic_ui::TextBox b;b.role=c.role;boxes.push_back(b);
   }
   p->text_=nfs_classic_ui::PreparedText::createCommands(display,std::move(font),source,boxes);if(!p->text_)return {};auto view=p->text_->layer();p->vertices_.assign(view.vertices,view.vertices+view.vertexCount);const auto&ranges=p->text_->ranges();if(ranges.size()!=source->commands.size())return {};
   for(size_t n=0;n<ranges.size();++n){uint32_t color=source->argb[n]|0xff000000u;for(size_t k=0;k<ranges[n].second;++k){auto&v=p->vertices_[ranges[n].first+k];v.r=float((color>>16)&255)/255;v.g=float((color>>8)&255)/255;v.b=float(color&255)/255;v.a=1;}}
   p->source_=std::move(source);return p;
  }catch(...){return {};}
 }
 NfsCreditsUiView view(uint64_t request)const noexcept{NfsCreditsUiView v{};if(!request||!source_||!text_||vertices_.empty())return v;v.schema=1;v.request=request;v.content=source_->serial;v.text=text_->layer();v.text.vertices=vertices_.data();v.text.vertexCount=vertices_.size();v.owner=const_cast<Prepared*>(this);v.retain=retain;v.release=release;return v;}
};
}

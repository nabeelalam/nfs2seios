#pragma once
#include "ChildDialogRuntimeAbi.h"
#include "ClassicTextDisplay.h"
#include "RecordsFont.h"
#include <atomic>
namespace nfs_ls_child {
constexpr size_t Pixels=640*480;
struct Payload {uintptr_t game=0;uint64_t serial=0;uint32_t handler=0;std::vector<uint16_t>parent,clean;std::vector<nfs_soft::TextCommand>commands;std::vector<unsigned char>selected;std::vector<NfsLoadSaveChildShape>shapes;};
class Prepared final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const Payload>source_;std::shared_ptr<const nfs_classic_ui::PreparedText>text_;std::vector<unsigned char>rgba_;std::vector<NfsLoadSaveChildShape>shapes_;
 static void retain(void*p)noexcept{static_cast<Prepared*>(p)->refs_.fetch_add(1);}static void release(void*p)noexcept{auto*s=static_cast<Prepared*>(p);if(s->refs_.fetch_sub(1)==1)delete s;}
public:
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const Payload>s)noexcept{try{if(!s||!s->game||!s->serial||s->parent.size()!=Pixels||s->clean.size()!=Pixels)return {};auto p=std::shared_ptr<Prepared>(new Prepared,[](const Prepared*q){release(const_cast<Prepared*>(q));});if(s->selected.size()!=s->commands.size())return {};if(!s->commands.empty()){auto font=nfs_classic_ui::recordsFont();if(!font)return {};p->text_=nfs_classic_ui::PreparedText::createCommands(s->commands,std::move(font),s,{},true);if(!p->text_)return {};}p->shapes_=s->shapes;
// Selected row follows the source style, and the actual shaped ink bounds.
// No guessed row index, hover state or user action is introduced.
if(p->text_){const auto layer=p->text_->layer();const auto&ranges=p->text_->ranges();for(size_t n=0;n<ranges.size();++n)if(s->selected[n]&&ranges[n].second){NfsLoadSaveChildShape q{};q.role=8;q.bounds[0]=640;q.bounds[1]=480;for(size_t k=0;k<ranges[n].second;++k){const auto&v=layer.vertices[ranges[n].first+k];q.bounds[0]=std::min(q.bounds[0],v.x);q.bounds[1]=std::min(q.bounds[1],v.y);q.bounds[2]=std::max(q.bounds[2],v.x);q.bounds[3]=std::max(q.bounds[3],v.y);}q.bounds[0]-=4;q.bounds[1]-=2;q.bounds[2]+=4;q.bounds[3]+=2;const auto&c=s->commands[n];q.clip[0]=c.clip.left;q.clip[1]=c.clip.top;q.clip[2]=c.clip.right;q.clip[3]=c.clip.bottom;p->shapes_.push_back(q);}}
// Parent palette/redraw differences are not child artwork. Only admitted
// authored/material geometry and actual labels belong over the retained parent.
// Original filename caret is a_ through4054E0/405CF0, retaining its real blink.
p->rgba_.assign(Pixels*4,0);p->source_=std::move(s);return p;}catch(...){return {};}}
 NfsLoadSaveChildView view(uint64_t request)const noexcept{NfsLoadSaveChildView v{};v.schema=1;v.request=request;v.content=source_->serial;v.rgba=rgba_.data();v.bytes=rgba_.size();if(text_)v.text=text_->layer();v.shapes=shapes_.data();v.shapeCount=shapes_.size();v.owner=const_cast<Prepared*>(this);v.retain=retain;v.release=release;return v;}
};
}

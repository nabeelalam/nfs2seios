#pragma once
#include "ChildDialogRuntimeAbi.h"
#include "ClassicTextDisplay.h"
#include "RecordsFont.h"
#include <atomic>
namespace nfs_ls_child {
constexpr size_t Pixels=640*480;
struct ChildLife {
 std::atomic<unsigned>refs{1};std::atomic<bool>alive{true};std::atomic<uint64_t>content{0};
 uintptr_t game=0;uint64_t request=0,epoch=0,parentContent=0;
 std::shared_ptr<const void>parentKeep;void*parentOwner=nullptr;int(*parentCurrent)(void*)=nullptr;
 static void retain(void*p){static_cast<ChildLife*>(p)->refs.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p){auto*s=static_cast<ChildLife*>(p);if(s->refs.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
 static int current(void*p){auto*s=static_cast<ChildLife*>(p);return s&&s->alive.load(std::memory_order_acquire)&&s->game&&s->epoch&&s->parentContent&&s->request&&s->parentKeep&&s->parentCurrent&&s->parentCurrent(s->parentOwner);}
 static std::shared_ptr<ChildLife>create(){return std::shared_ptr<ChildLife>(new ChildLife,[](ChildLife*p){release(p);});}
 NfsResultsNameSource view()const{NfsResultsNameSource v{};v.schema=1;v.request=request;v.game=game;v.epoch=epoch;v.parentContent=parentContent;v.owner=const_cast<ChildLife*>(this);v.retain=retain;v.release=release;v.current=current;return v;}
};
struct Payload {std::shared_ptr<ChildLife>life;uintptr_t game=0;uint64_t serial=0;uint32_t handler=0;std::vector<uint16_t>parent,clean;std::vector<nfs_soft::TextCommand>commands;std::vector<unsigned char>selected;std::vector<NfsLoadSaveChildShape>shapes;};
class Prepared final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const Payload>source_;std::shared_ptr<const nfs_classic_ui::PreparedText>text_;std::vector<unsigned char>rgba_;std::vector<NfsLoadSaveChildShape>shapes_;
 static int current(void*p){auto*s=static_cast<Prepared*>(p);return s&&s->source_&&s->source_->life&&ChildLife::current(s->source_->life.get())&&s->source_->life->content.load(std::memory_order_acquire)==s->source_->serial;}
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
 NfsLoadSaveChildView view(uint64_t request)const noexcept{NfsLoadSaveChildView v{};if(source_->life&&request!=source_->life->request)return v;v.schema=source_->life?2:1;v.request=request;v.content=source_->serial;v.rgba=rgba_.data();v.bytes=rgba_.size();if(text_)v.text=text_->layer();v.shapes=shapes_.data();v.shapeCount=shapes_.size();v.owner=const_cast<Prepared*>(this);v.retain=retain;v.release=release;if(source_->life){v.current=current;v.sourceGame=source_->life->game;v.sourceEpoch=source_->life->epoch;v.parentContent=source_->life->parentContent;}return v;}
};
}

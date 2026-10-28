// PRIVATE immutable Personal Statistics vectors and classic HATTEN labels.
#pragma once
#include "StatsFrame.h"
#include "ClassicTextDisplay.h"
#include "RecordsFont.h"
#include <atomic>
#include <utility>
namespace nfs_stats_ui {
class Prepared final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const Frame>frame_;
 std::shared_ptr<const nfs_classic_ui::PreparedText>text_;
 std::vector<NfsStatsGraphVertex>graph_;std::vector<NfsStatsDrawBatch>batches_;
 Prepared()=default;
 static void retain(void*p)noexcept{static_cast<Prepared*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p)noexcept{auto*s=static_cast<Prepared*>(p);if(s->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
public:
 static std::shared_ptr<const Prepared>create(std::shared_ptr<const Frame>frame,std::shared_ptr<const nfs_soft::FontCatalog>font=nfs_classic_ui::recordsFont())noexcept{
  if(!frame||!font||!frame->serial||!frame->game||!frame->thread||!frame->form||!frame->archive||frame->clean.size()!=Pixels||frame->original.size()!=Pixels||frame->commands.empty()||frame->commands.size()>MaxCommands||frame->primitives.size()>MaxPrimitives||frame->events.size()!=frame->primitives.size()+frame->commands.size())return {};
  try{auto out=std::shared_ptr<Prepared>(new Prepared,[](const Prepared*p){release(const_cast<Prepared*>(p));});auto displayCommands=frame->commands;std::vector<nfs_classic_ui::TextBox>boxes;boxes.reserve(frame->commands.size());for(const auto&c:frame->commands){nfs_classic_ui::TextBox box;box.role=c.role;box.width=float(c.advance);boxes.push_back(box);}out->text_=nfs_classic_ui::PreparedText::createCommands(displayCommands,std::move(font),frame,boxes);if(!out->text_)return {};
   std::vector<std::pair<size_t,size_t>>ranges;ranges.reserve(frame->primitives.size());for(const auto&p:frame->primitives){size_t first=out->graph_.size();if(!append(p,out->graph_))return {};ranges.push_back({first,out->graph_.size()-first});}
   const auto&textRanges=out->text_->ranges();if(textRanges.size()!=frame->commands.size())return {};size_t nextGraph=0,nextText=0;
   for(const auto&e:frame->events){const auto&list=e.text?textRanges:ranges;auto&next=e.text?nextText:nextGraph;if(e.index!=next++||e.index>=list.size())return {};const auto&r=list[e.index];if(!r.second)continue;NfsStatsDrawBatch batch{};batch.kind=e.text?1u:0u;batch.first=r.first;batch.count=r.second;
    if(!out->batches_.empty()&&out->batches_.back().kind==batch.kind&&out->batches_.back().first+out->batches_.back().count==batch.first)out->batches_.back().count+=batch.count;else out->batches_.push_back(batch);
   }
   if(nextGraph!=frame->primitives.size()||nextText!=frame->commands.size()||out->batches_.empty())return {};out->frame_=std::move(frame);return out;
  }catch(...){return {};}
 }
 bool currentFor(const std::shared_ptr<const Frame>&frame)const noexcept{return frame&&frame_.get()==frame.get()&&!frame_.owner_before(frame)&&!frame.owner_before(frame_)&&text_&&frame_->original.size()==Pixels&&frame_->clean.size()==Pixels;}
 NfsStatsUiView view(uint64_t request)const noexcept{
  NfsStatsUiView v{};if(!request||!currentFor(frame_))return v;v.schema=1;v.width=640;v.height=480;v.request=request;v.content=frame_->serial;v.backdrop=frame_->backdrop;v.original=frame_->original.data();v.clean=frame_->clean.data();v.pixels=Pixels;v.graph=graph_.empty()?nullptr:graph_.data();v.graphVertices=graph_.size();v.text=text_->layer();v.batches=batches_.data();v.batchCount=batches_.size();v.owner=const_cast<Prepared*>(this);v.retain=retain;v.release=release;return v;
 }
 const std::shared_ptr<const Frame>&source()const noexcept{return frame_;}
};
}

// SPDX-License-Identifier: MIT
#pragma once
#include "OpponentInitialSidebarProof.h"
#include "OpponentCaptionGameHooks.inc"
#include "Main343OriginGameHooks.inc"
#include "ChildStaticSceneHistory.h"
#include "ContinuousNativeUploadHooks.inc"
namespace nfs_opponent_initial_sidebar_live {
namespace child=nfs_opponent_initial_sidebar;namespace bg=nfs_background_scene_cpu;
struct State {
 child::Store opponents;
 std::shared_ptr<const child::Proof>pending;std::shared_ptr<const bg::SourceCatalog>catalog;
 bg::Workspace workspace;bg::Plane actual;uintptr_t game=0,thread=0;uint32_t form=0,descriptor=0;uint64_t epoch=0,next=0;unsigned depth=0;bool invalid=false;
 void clear()noexcept{opponents.clear();pending.reset();catalog.reset();}
};
inline State*&slot(){struct Storage{State*p=nullptr;~Storage(){delete p;}};static thread_local Storage storage;return storage.p;}
inline State*current(Game&g){auto*s=slot();return s&&!s->invalid&&s->depth==1&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->thread==nfs_logo_publisher::threadToken()?s:nullptr;}
struct OwnerScope {State*s=nullptr;bool entered=false;
 explicit OwnerScope(Game&g)noexcept{const uint32_t descriptor=uint32_t(g.eax);uint32_t form=descriptor==uint32_t(guest_address(off_4D3204))?child::OpponentForm:0;
  if(!form||!nfs_logo_publisher::enabled())return;auto*&p=slot();if(!p)p=new(std::nothrow)State;if(!p)return;s=p;entered=true;if(s->depth++){s->invalid=true;s->clear();return;}s->clear();s->invalid=false;if(s->next==UINT64_MAX){s->invalid=true;return;}s->epoch=++s->next;s->form=form;s->descriptor=descriptor;s->game=reinterpret_cast<uintptr_t>(&g);s->thread=nfs_logo_publisher::threadToken();s->catalog=nfs_background_scene_runtime::acquireCatalog();
 }
 ~OwnerScope(){if(!entered)return;if(s->depth)--s->depth;if(!s->depth){s->clear();s->game=s->thread=0;s->form=s->descriptor=0;}}
};
inline void completeInitialCaption(Game&g)noexcept{
 auto*s=current(g);if(!s||!s->catalog||!s->epoch||uint32_t(g.esi)!=s->descriptor)return;s->pending.reset();s->opponents.clear();
 auto caption=nfs_opponent_caption::acquireCurrentProof(g);
 if(!caption||!caption->current()||caption->evidence().route!=nfs_opponent_caption_source::Route::InitialMain||!caption->origin())return;
 s->actual.valid=0;uint32_t error=0;nfs_logo_publisher::plane(s->actual,error);if(error||!bg::plane(s->actual,343))return;
 child::Binding b;b.form=s->form;b.formGuest=s->descriptor;b.archive=caption->origin()->archive();b.game=s->game;b.thread=s->thread;b.epoch=s->epoch;b.captionEvent={s->epoch,1,0};
 s->pending=s->opponents.accept(caption->origin(),caption,b,s->actual,s->workspace,s->catalog);
}
inline std::shared_ptr<const child::Proof>acquire(Game&g,uint32_t source,const std::vector<uint8_t>&raw)noexcept{
 auto*s=current(g);
 auto refuse=[&](unsigned reason,uint64_t index=0,uint64_t expected=0,uint64_t actual=0)->std::shared_ptr<const child::Proof>{
  const int saved=errno;const char*trace=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");static thread_local unsigned lines[10]{};
  if(trace&&trace[0]=='1'&&!trace[1]&&reason<10&&lines[reason]++<4){
   const auto history=nfs_child_static_scene::acquire(reinterpret_cast<uintptr_t>(&g),nfs_logo_publisher::threadToken(),uint32_t(guest_address(off_4D3204)),nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire));
   const bool historyRawMatch=history&&history->sidebarRaw&&*history->sidebarRaw==raw;
   size_t historyDiff=SIZE_MAX;unsigned historyExpected=0,historyActual=0;
   if(history&&history->sidebarRaw){const auto&before=*history->sidebarRaw;const size_t n=std::min(before.size(),raw.size());for(size_t k=0;k<n;++k)if(before[k]!=raw[k]){historyDiff=k;historyExpected=before[k];historyActual=raw[k];break;}if(historyDiff==SIZE_MAX&&before.size()!=raw.size())historyDiff=n;}

   std::fprintf(stderr,"OPPONENT INITIAL343 refused reason=%u index=%llu expected=%llu actual=%llu source=%u rawBytes=%zu initialCurrent=%u historyCurrent=%u historyRawMatch=%u historyEpoch=%llu historyEvent=%llu historyCopy=%llu historyGroup=%llu historyBytes=%zu historyFirstDiffByte=%llu historyExpected=%u historyActual=%u\n",reason,(unsigned long long)index,(unsigned long long)expected,(unsigned long long)actual,source,raw.size(),unsigned(s&&s->pending&&s->pending->current()),unsigned(bool(history)),unsigned(historyRawMatch),(unsigned long long)(history?history->domain.ownerEpoch:0),(unsigned long long)(history?history->domain.uploadEvent:0),(unsigned long long)(history?history->domain.copySerial:0),(unsigned long long)(history?history->domain.groupSerial:0),history&&history->sidebarRaw?history->sidebarRaw->size():0,(unsigned long long)historyDiff,historyExpected,historyActual);
  }errno=saved;return {};
 };
 if(!s||!s->pending||!s->pending->current()||!s->catalog)return refuse(1);const auto p=s->pending;
 if(raw.size()!=16+bg::Side*2||p->binding().form!=s->form||p->binding().formGuest!=s->descriptor||p->binding().epoch!=s->epoch||source!=p->post().target.w[8])return refuse(2,0,p->post().target.w[8],source);
 try{nfs::GuestCopyTransaction tx;nfs_logo_capture::Target target{};nfs_logo_publisher::target(tx,target,p->post().target.owner);
  if(!nfs_main_sidebar_origin::sameRetainedTarget(target,p->post().target)){
   const auto&old=p->post().target;for(size_t k=0;k<target.w.size();++k)if(target.w[k]!=old.w[k])return refuse(3,1000+k,old.w[k],target.w[k]);
   for(size_t k=0;k<target.header.size();++k)if(target.header[k]!=old.header[k])return refuse(3,2000+k,old.header[k],target.header[k]);
   for(size_t k=0;k<target.x.size();++k)if(target.x[k]!=old.x[k])return refuse(3,3000+k,old.x[k],target.x[k]);
   for(size_t k=0;k<target.y.size();++k)if(target.y[k]!=old.y[k])return refuse(3,4000+k,old.y[k],target.y[k]);
   if(target.owner!=old.owner)return refuse(3,5000,old.owner,target.owner);
   for(size_t k=0;k<target.ownerWords.size();++k)if(target.ownerWords[k]!=old.ownerWords[k])return refuse(3,6000+k,old.ownerWords[k],target.ownerWords[k]);
   for(size_t k=1;k<5;++k)if(target.ownerState[k]!=old.ownerState[k])return refuse(3,7000+k,old.ownerState[k],target.ownerState[k]);
   return refuse(3);
  }
  const uint32_t archive=tx.load<uint32_t>(dword_4D40B8);if(archive!=p->binding().archive)return refuse(4,0,p->binding().archive,archive);
  for(unsigned k=0;k<4;++k){const uint32_t actual=uint32_t(raw[4*k])|(uint32_t(raw[4*k+1])<<8)|(uint32_t(raw[4*k+2])<<16)|(uint32_t(raw[4*k+3])<<24);if(actual!=target.header[k])return refuse(5,k,target.header[k],actual);}
  for(unsigned k=0;k<bg::Side;++k){const uint16_t actual=bg::le16(raw.data()+16+2*k);if(actual!=p->post().pixels[k])return refuse(6,k,p->post().pixels[k],actual);}
  return p->current()?p:refuse(7);
 }catch(...){return refuse(8);}
}
} // namespace nfs_opponent_initial_sidebar_live

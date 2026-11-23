// PRIVATE dedicated Credits handler40A0B0; no original content/input/timing changes.
#pragma once
#include "CreditsPrepare.h"
#include "CreditsFullFrame.h"
#include "CreditsWholePrepare.h"
#include "CreditsCacheSource.h"
#include "hd/BackgroundSceneQueueAdapter.inc"
#include "MainThreadBridge.h"
#include "GuestCopyTransaction.h"
#include "hd/ContinuousLogoGameHooks.inc"
#include "hd/SmallLogoPrepare.h"
#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cstdio>
namespace nfs_credits_ui_live {
inline bool enabled()noexcept{const char*p=std::getenv("NFS_PRIVATE_CREDITS_UI");return p&&p[0]=='1'&&!p[1];}
inline uintptr_t token()noexcept{static thread_local unsigned char p;return reinterpret_cast<uintptr_t>(&p);}
inline std::atomic<uintptr_t>&ownerThread(){static std::atomic<uintptr_t>p{0};return p;}
struct State {uintptr_t game=0;unsigned depth=0;bool active=false,invalid=false,stringOpen=false;uint64_t request=0,serial=0;unsigned staticCount=0,bitmapDepth=0;uint32_t locale=0;bool staticFailed=false;std::array<nfs_credits_ui::StaticDraw,4>staticDraws{};std::shared_ptr<const nfs_credits_ui::SceneSource>scene;nfs_credits_ui::FrameAssembly whole;std::shared_ptr<const nfs_credits_ui::WholeFrame>published;unsigned frameTraceLines=0;uint64_t nativeBaseRequest=0,nativePartnerRequest=0;std::shared_ptr<const std::vector<uint16_t>>lastStaticPixels;nfs_logo_capture::Target lastStaticTarget{};std::array<std::shared_ptr<std::atomic<uint64_t>>,2>cacheLive{};std::array<std::shared_ptr<const nfs_credits_ui::CacheSource>,2>caches{};std::array<float,4>clip{{0,0,640,480}};std::shared_ptr<nfs_credits_ui::SourceLease>source;std::shared_ptr<nfs_credits_ui::Payload>working;nfs_soft::TextCommand command{};uint32_t color=0,sourceSize=0;};
inline State&storage(){static thread_local State s;return s;}
inline State*current(Game&g)noexcept{auto&s=storage();return ownerThread().load(std::memory_order_acquire)==token()&&s.game==reinterpret_cast<uintptr_t>(&g)&&s.depth==1?&s:nullptr;}
struct Errno {int saved=errno;~Errno(){errno=saved;}};
inline void cancelFrame(Game&,State&s)noexcept{Errno e;const uint64_t request=s.request;s.request=0;s.active=false;s.stringOpen=false;s.working.reset();if(request&&nfs_background_scene_live::activeRequest()==request)nfs_background_scene_live::activeRequest()=0;if(request)try{(void)nfs::enqueue_owned_renderer([request](){return int32_t(nfs_private_credits_ui_cancel(request));});}catch(...){}}
// Called after the original backdrop draw, when the handler has opened credits.qfs.
inline bool bindSource(Game&g,State&s)noexcept{
 try{uint32_t archive=0;std::vector<unsigned char>copy;
  {nfs::GuestCopyTransaction tx;archive=tx.load<uint32_t>(dword_4D40B8);
   if(!archive||uint64_t(archive)+NFS_CREDITS_ARCHIVE_BYTES>UINT64_C(0x100000000))return false;
   if(s.source&&s.source->current(s.game,archive))return true;
   // Reject a replacement archive inside one Credits owner, including same-address
   // reuse after the explicit free hooks have retired the prior lease.
   if(s.source)return false;
  }
  copy.resize(NFS_CREDITS_ARCHIVE_BYTES);
  {nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=archive)return false;
   std::memcpy(copy.data(),nfs_host_address(archive,copy.size()),copy.size());}
  auto identity=nfs_credits_ui::ArchiveIdentity::create(copy);if(!identity)return false;
  // Registration and hashing must not run while holding the guest-copy mutex.
  auto lease=nfs_credits_ui::SourceLease::create(s.game,archive,std::move(identity));if(!lease)return false;
  {nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=archive){lease->retire();return false;}}
  s.source=std::move(lease);return true;
 }catch(...){return false;}
}
inline bool wholeMode()noexcept;
inline std::shared_ptr<const std::vector<uint16_t>>staticPlane(const nfs_logo_capture::Target&);
// One-shot static source collector. All guest reads are bounded snapshots; no
// original draw, register, callback or timing operation is suppressed.
inline nfs_logo_capture::Target staticTarget(){nfs_logo_capture::Target t;nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t);if(!nfs_small_logo_live::target(t,640)||t.w[3]||t.w[4]||t.w[5]!=640||t.w[6]!=480)throw 1;return t;}
inline bool staticResourceCurrent(uint32_t archive,unsigned index){
 if(index>=NFS_CREDITS_RESOURCE_COUNT)return false;const auto&r=nfs_credits_sources[index];std::vector<unsigned char>bytes(r.span);
 {nfs::GuestCopyTransaction tx;std::memcpy(bytes.data(),nfs_host_address(archive+r.offset,r.span),r.span);}
 unsigned char sha[32];return nfs_texture_sha256(bytes.data(),bytes.size(),sha)&&!std::memcmp(sha,r.sha256,32);
}
struct StaticBitmapScope {
 Game&g;State*s=nullptr;nfs_logo_capture::Target target{};nfs_credits_ui::StaticDraw draw{};uint32_t archive=0,source=0;unsigned slot=0;int unwind=0;bool entered=false;
 explicit StaticBitmapScope(Game&x)noexcept:g(x){Errno e;s=current(g);if(!s||s->staticFailed||s->scene)return;
  try{uint32_t currentArchive=0;{nfs::GuestCopyTransaction tx;currentArchive=tx.load<uint32_t>(dword_4D40B8);}
   const uint32_t address=uint32_t(g.eax);if(!currentArchive||address<currentArchive||uint64_t(address)>=uint64_t(currentArchive)+NFS_CREDITS_ARCHIVE_BYTES)return;
   if(s->bitmapDepth||s->staticCount>=4||!bindSource(g,*s))throw 1;
   archive=currentArchive;source=address;{nfs::GuestCopyTransaction tx;s->locale=tx.load<uint8_t>(byte_512DFB);}
   if(s->locale>=6)throw 1;slot=s->staticCount;const unsigned recipe[4]={5,8,0,nfs_credits_title_source_indices[s->locale]};draw.sourceIndex=recipe[slot];
   if(source!=archive+nfs_credits_sources[draw.sourceIndex].offset||!staticResourceCurrent(archive,draw.sourceIndex))throw 1;
   // Direct low-level routine receives final placement after all mode/hotspot adjustments.
   draw.x=int32_t(g.edx);draw.y=int32_t(g.ebx);target=staticTarget();draw.target=target.w;s->bitmapDepth=1;unwind=nfs_logo_publisher::exceptions();entered=true;
  }catch(...){s->staticFailed=true;s->scene.reset();}
 }
 ~StaticBitmapScope()noexcept{if(!entered||!s)return;Errno e;s->bitmapDepth=0;if(s!=current(g)||s->staticFailed)return;
  try{if(nfs_logo_publisher::exceptions()>unwind||s->staticCount!=slot||!s->source||!s->source->current(s->game,archive)||!nfs_logo_capture::same(target,staticTarget())||!staticResourceCurrent(archive,draw.sourceIndex))throw 1;
   {nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D40B8)!=archive||tx.load<uint8_t>(byte_512DFB)!=s->locale)throw 1;}
   s->staticDraws[slot]=draw;++s->staticCount;if(wholeMode()){s->lastStaticTarget=target;s->lastStaticPixels=staticPlane(target);}
   if(s->staticCount==4){s->scene=nfs_credits_ui::SceneSource::create(s->source,s->game,archive,s->locale,s->staticDraws);if(!s->scene)throw 1;}
   const char*trace=std::getenv("NFS_PRIVATE_CREDITS_SOURCE_TRACE");if(trace&&trace[0]=='1'&&!trace[1])std::fprintf(stderr,"CREDITS SOURCE resource=%u slot=%u position=%d,%d completed=%u locale=%u\n",draw.sourceIndex,slot,draw.x,draw.y,s->staticCount,s->locale);
  }catch(...){s->staticFailed=true;s->scene.reset();}
 }
};
inline bool wholeMode()noexcept{const char*p=std::getenv("NFS_PRIVATE_CREDITS_WHOLE");return p&&p[0]=='1'&&!p[1]&&nfs_credits_art_generation();}
inline std::shared_ptr<const std::vector<uint16_t>>staticPlane(const nfs_logo_capture::Target&t){auto p=std::make_shared<std::vector<uint16_t>>(640*480);nfs::GuestCopyTransaction tx;std::memcpy(p->data(),nfs_host_address(t.w[8]+16,p->size()*2),p->size()*2);return p;}
inline bool sameSourceTarget(const nfs_logo_capture::Target&a,const nfs_logo_capture::Target&b)noexcept{return a.w==b.w&&a.header==b.header&&a.x==b.x&&a.y==b.y&&a.owner==b.owner&&a.ownerWords==b.ownerWords;}
inline nfs_logo_capture::Target anchoredTarget(const nfs_logo_capture::Target&anchor){nfs_logo_capture::Target t;nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t,anchor.owner);if(!nfs_small_logo_live::target(t,640)||!sameSourceTarget(t,anchor))throw 1;return t;}
inline bool cacheDescriptors(Game&,State&s,const nfs_credits_ui::CacheSource&cache)noexcept{try{if(!cache.current()||!s.source)return false;const auto&addresses=cache.addresses();const auto&descriptors=cache.descriptors();nfs::GuestCopyTransaction tx;if(!s.source->current(s.game,tx.load<uint32_t>(dword_4D40B8))||tx.load<uint8_t>(byte_512DFB)!=s.locale)return false;
 for(unsigned i=0;i<300;++i){const uint32_t d=tx.load<uint32_t>(dword_4EB74C+cache.slot()*1200+4*i);if(d!=descriptors[i])return false;uint32_t w[7];for(unsigned j=0;j<7;++j)w[j]=tx.load<uint32_t>(d+4*j);if(w[0]!=3||w[1]!=3||w[2]!=3||w[3]!=11||w[5]!=addresses[i]||w[6]!=3)return false;}return true;}catch(...){return false;}}
struct CacheUploadScope {
 Game&g;State*s=nullptr;unsigned slot=0;int unwind=0;std::shared_ptr<const std::vector<uint16_t>>pixels;bool entered=false;
 explicit CacheUploadScope(Game&x)noexcept:g(x){Errno e;s=current(g);if(!s||!wholeMode())return;slot=uint32_t(g.edx);if(slot>1)return;
  if(s->cacheLive[slot])s->cacheLive[slot]->store(0,std::memory_order_release);s->caches[slot].reset();
  try{if(s->staticFailed||s->staticCount!=(slot?4u:2u)||!s->lastStaticPixels||uint32_t(g.eax)!=s->lastStaticTarget.w[8]||!bindSource(g,*s))return;const auto target=anchoredTarget(s->lastStaticTarget);pixels=staticPlane(target);if(*pixels!=*s->lastStaticPixels)return;if(!s->cacheLive[slot])s->cacheLive[slot]=std::make_shared<std::atomic<uint64_t>>(0);unwind=nfs_logo_publisher::exceptions();entered=true;}catch(...){entered=false;}
 }
 ~CacheUploadScope()noexcept{if(!entered||!s)return;Errno e;try{if(s!=current(g)||s->invalid||s->staticFailed||nfs_logo_publisher::exceptions()>unwind||!s->source||*staticPlane(anchoredTarget(s->lastStaticTarget))!=*pixels)throw 1;
  std::array<uint32_t,300>descriptors{},addresses{};uint32_t archive=0;{nfs::GuestCopyTransaction tx;archive=tx.load<uint32_t>(dword_4D40B8);if(!s->source->current(s->game,archive)||tx.load<uint8_t>(byte_512DFB)!=s->locale)throw 1;for(unsigned i=0;i<300;++i){const uint32_t d=tx.load<uint32_t>(dword_4EB74C+1200*slot+4*i);if(!d||(d&3)||uint64_t(d)+28>UINT64_C(0x100000000))throw 1;uint32_t w[7];for(unsigned j=0;j<7;++j)w[j]=tx.load<uint32_t>(d+4*j);if(w[0]!=3||w[1]!=3||w[2]!=3||w[3]!=11||w[6]!=3)throw 1;descriptors[i]=d;addresses[i]=w[5];}}
  const auto serial=nfs_background_scene_live::nextRequest();if(!serial)throw 1;s->cacheLive[slot]->store(serial,std::memory_order_release);s->caches[slot]=nfs_credits_ui::CacheSource::create(s->source,s->game,archive,slot,s->locale,serial,s->cacheLive[slot],pixels,descriptors,addresses,s->staticDraws);if(!s->caches[slot])throw 1;
  if(std::getenv("NFS_PRIVATE_CREDITS_SOURCE_TRACE"))std::fprintf(stderr,"CREDITS CACHE source-ready slot=%u content=%llu\n",slot,(unsigned long long)serial);
 }catch(...){if(s->cacheLive[slot])s->cacheLive[slot]->store(0,std::memory_order_release);s->caches[slot].reset();}}
};
inline void beginFrame(Game&g)noexcept{Errno e;auto*s=current(g);if(!s||s->invalid||wholeMode())return;try{if(s->active||nfs_background_scene_live::activeRequest())return;if(!bindSource(g,*s)){s->invalid=true;return;}if(s->serial==UINT64_MAX){s->invalid=true;return;}auto p=std::make_shared<nfs_credits_ui::Payload>();p->game=s->game;p->thread=token();p->form=uint32_t(guest_address(off_4D2C80));p->serial=++s->serial;{nfs::GuestCopyTransaction tx;p->archive=tx.load<uint32_t>(dword_4D40B8);if(!p->archive)return;if(s->scene&&tx.load<uint8_t>(byte_512DFB)!=s->scene->locale()){s->invalid=true;return;}}p->source=s->source;p->scene=s->scene;if(!p->source->current(p->game,p->archive))return;const uint64_t request=nfs_background_scene_live::nextRequest();if(!request)return;nfs::check_worker_stop();auto result=nfs::enqueue_owned_renderer([request](){return int32_t(nfs_private_credits_ui_begin(request));});if(result!=nfs::OwnedRendererAcceptance::Consumed)return;s->working=std::move(p);s->request=request;s->active=true;s->stringOpen=false;nfs_background_scene_live::activeRequest()=request;}catch(...){cancelFrame(g,*s);}}
inline void finishFrame(Game&g)noexcept{Errno e;auto*s=current(g);if(!s||!s->active)return;try{if(s->invalid||s->stringOpen||!s->working||!s->request||nfs_background_scene_live::activeRequest()!=s->request){cancelFrame(g,*s);return;}uint32_t archive,locale;{nfs::GuestCopyTransaction tx;archive=tx.load<uint32_t>(dword_4D40B8);locale=tx.load<uint8_t>(byte_512DFB);}if(s->working->scene&&locale!=s->working->scene->locale()){cancelFrame(g,*s);return;}if(archive!=s->working->archive||!s->working->source||!s->working->source->current(s->game,archive)){cancelFrame(g,*s);return;}std::shared_ptr<const nfs_credits_ui::Payload>payload=s->working;auto prepared=nfs_credits_ui::Prepared::create(std::move(payload));if(!prepared){cancelFrame(g,*s);return;}const uint64_t request=s->request;const auto keep=prepared;nfs::check_worker_stop();(void)nfs::enqueue_owned_renderer([keep,request](){const auto v=keep->view(request);if(!v.request)return int32_t(nfs_private_credits_ui_cancel(request));return int32_t(nfs_private_credits_ui_finish(&v));});nfs_background_scene_live::activeRequest()=0;s->request=0;s->active=false;s->working.reset();}catch(...){cancelFrame(g,*s);}}
struct OwnerScope {Game&g;State*s=nullptr;bool entered=false;explicit OwnerScope(Game&v)noexcept:g(v){if(!enabled()||uint32_t(g.eax)!=uint32_t(guest_address(off_4D2C80)))return;Errno e;uintptr_t expected=0;if(!ownerThread().compare_exchange_strong(expected,token())&&expected!=token())return;s=&storage();if(s->depth++){s->invalid=true;entered=true;return;}s->game=reinterpret_cast<uintptr_t>(&g);s->active=s->invalid=s->stringOpen=false;s->request=0;if(s->source)s->source->retire();s->source.reset();s->scene.reset();s->whole.abort();s->published.reset();s->frameTraceLines=0;s->nativeBaseRequest=s->nativePartnerRequest=0;s->lastStaticPixels.reset();for(auto&live:s->cacheLive)if(live)live->store(0);s->caches={};s->staticCount=s->bitmapDepth=0;s->staticFailed=false;s->staticDraws={};s->working.reset();s->clip={{0,0,640,480}};entered=true;try{nfs::GuestCopyTransaction tx;if(tx.load<uint16_t>(uint32_t(guest_address(off_4D2C80))+8)!=0x12)s->invalid=true;}catch(...){s->invalid=true;}}
 ~OwnerScope(){if(!entered||!s)return;Errno e;if(s->active)cancelFrame(g,*s);if(s->depth&&!--s->depth){if(s->source)s->source->retire();s->source.reset();s->scene.reset();s->whole.abort();s->published.reset();for(auto&live:s->cacheLive)if(live)live->store(0);s->caches={};s->lastStaticPixels.reset();try{(void)nfs::enqueue_owned_renderer([](){return int32_t(nfs_private_credits_cache_reset());});}catch(...){}s->game=0;s->working.reset();auto expected=token();ownerThread().compare_exchange_strong(expected,0);}}
};
// Both original archive deallocation entries receive the allocation in eax.
inline void retire(Game&g,uint32_t archive)noexcept{Errno e;auto*s=current(g);if(!s||!s->source||!s->source->current(s->game,archive))return;s->source->retire();s->invalid=true;s->whole.abort();s->published.reset();if(s->active)cancelFrame(g,*s);}
// 427B30 source backgrounds are retained. A mid-scroll logo/backdrop draw first
// publishes accumulated HD text, then a fresh snapshot starts after that draw.
struct BackdropScope {
 Game&g;State*s=nullptr;bool owned=false,base=false,partner=false;int unwind=0;uint64_t nativeRequest=0;std::shared_ptr<const nfs_credits_ui::CacheSource>cache;
 explicit BackdropScope(Game&v)noexcept:g(v){Errno e;s=current(g);if(!s||s->invalid)return;owned=true;
  // Preserve the established native text fragments; the independent whole-frame
  // collector survives the mid-scroll partner draw instead of losing earlier text.
  if(s->active)finishFrame(g);
  try{const uint32_t kind=uint32_t(g.eax);int32_t opacity=0;{nfs::GuestCopyTransaction tx;opacity=tx.load<int32_t>(uint32_t(g.esp)+4);}
   if(!s->scene||!s->scene->current()||uint32_t(g.edx)||uint32_t(g.ecx)!=32||opacity!= -1)throw 1;
   if(kind==1){if(uint32_t(g.ebx))throw 1;s->whole.abort();s->published.reset();s->nativeBaseRequest=s->nativePartnerRequest=0;base=true;}
   else if(kind==0){nfs_credits_ui::PartnerPlacement p;p.x=int32_t(g.edx);p.y=int32_t(g.ebx);p.clip=s->clip;if(!s->whole.beginPartner(p))throw 1;partner=true;}
   else throw 1;unwind=nfs_logo_publisher::exceptions();
   if(wholeMode()){
    if(nfs_background_scene_live::activeRequest()||!s->caches[kind]||!cacheDescriptors(g,*s,*s->caches[kind]))throw 1;cache=s->caches[kind];NfsCreditsCacheDrawView draw{};draw.cache=cache->view();draw.request=nfs_background_scene_live::nextRequest();draw.x=int32_t(g.edx);draw.y=int32_t(g.ebx);draw.step=32;std::copy(s->clip.begin(),s->clip.end(),draw.clip);if(!draw.request)throw 1;
    nfs::check_worker_stop();auto keep=cache;auto result=nfs::enqueue_owned_renderer([keep,draw](){return int32_t(nfs_private_credits_cache_begin(&draw));});if(result!=nfs::OwnedRendererAcceptance::Consumed)throw 1;nativeRequest=draw.request;nfs_background_scene_live::activeRequest()=nativeRequest;if(base)s->nativeBaseRequest=nativeRequest;else s->nativePartnerRequest=nativeRequest;
   }
  }catch(...){s->whole.abort();base=partner=false;}
 }
 ~BackdropScope()noexcept{if(!owned)return;Errno e;
  if(nativeRequest){bool valid=s==current(g)&&!s->invalid&&nfs_logo_publisher::exceptions()<=unwind&&cache&&cacheDescriptors(g,*s,*cache);if(nfs_background_scene_live::activeRequest()==nativeRequest)try{const uint64_t request=nativeRequest;auto keep=cache;(void)nfs::enqueue_owned_renderer([keep,request,valid](){return int32_t(valid?nfs_private_credits_cache_finish(request):nfs_private_credits_cache_cancel(request));});}catch(...){valid=false;}if(nfs_background_scene_live::activeRequest()==nativeRequest)nfs_background_scene_live::activeRequest()=0;if(!valid){s->whole.abort();base=partner=false;}}
  if(s==current(g)&&!s->invalid&&nfs_logo_publisher::exceptions()<=unwind){if(base)(void)s->whole.begin(s->scene);else if(partner)(void)s->whole.finishPartner();}else if(s)s->whole.abort();beginFrame(g);}
};
struct SwapScope {explicit SwapScope(Game&g)noexcept{Errno e;auto*s=current(g);const bool open=s&&s->stringOpen;finishFrame(g);if(!s||s!=current(g))return;
 if(open||s->invalid||s->stringOpen||!s->scene){s->whole.abort();s->published.reset();return;}
 bool matching=false;try{nfs::GuestCopyTransaction tx;const auto archive=tx.load<uint32_t>(dword_4D40B8);matching=s->source&&s->source->current(s->game,archive)&&tx.load<uint8_t>(byte_512DFB)==s->scene->locale();}catch(...){matching=false;}
 if(!matching){s->whole.abort();s->published.reset();return;}
 s->published=s->whole.seal();
 if(wholeMode()&&s->published&&s->nativeBaseRequest)try{auto prepared=nfs_credits_ui::PreparedWhole::create(s->published);if(prepared){const auto request=nfs_background_scene_live::nextRequest();auto view=prepared->view(request);view.nativeBaseRequest=s->nativeBaseRequest;view.nativePartnerRequest=s->nativePartnerRequest;auto keep=prepared;(void)nfs::enqueue_owned_renderer([keep,view](){return int32_t(nfs_private_credits_whole_submit(&view));});}}catch(...){}
 const char*trace=std::getenv("NFS_PRIVATE_CREDITS_SOURCE_TRACE");
 if(s->published&&trace&&trace[0]=='1'&&!trace[1]&&(s->frameTraceLines<8||(s->frameTraceLines==8&&s->published->partner()))){++s->frameTraceLines;const auto&p=*s->published;std::fprintf(stderr,"CREDITS WHOLE frame=%llu commands=%zu events=%zu partner=%u empty=%u\n",(unsigned long long)p.serial(),p.commands().size(),p.events().size(),p.partner()?1u:0u,p.commands().empty()?1u:0u);}
}};
struct ClipScope {explicit ClipScope(Game&g)noexcept{Errno e;auto*s=current(g);if(!s||s->invalid)return;try{nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);std::array<float,4>c{};for(unsigned k=0;k<4;++k)c[k]=float(tx.load<int32_t>(sp+4+4*k));if(c[0]<0||c[1]<0||c[2]>640||c[3]>480||c[0]>c[2]||c[1]>c[3]){s->invalid=true;return;}s->clip=c;}catch(...){s->invalid=true;}}};
struct StringScope {Game&g;State*s=nullptr;bool entered=false;explicit StringScope(Game&v)noexcept:g(v){Errno e;s=current(g);if(!s||(!s->active&&!s->whole.active())||s->invalid)return;try{if(s->stringOpen||(s->active&&(!s->working||s->working->commands.size()>=128))){s->invalid=true;return;}nfs::GuestCopyTransaction tx;if(!tx.load<uint32_t>(dword_4D20A0)){s->whole.abort();return;}nfs_soft::TextCommand c{};for(unsigned k=0;k<96;++k){if(uint64_t(uint32_t(g.eax))+k>UINT32_MAX)throw 1;uint8_t ch=tx.load<uint8_t>(uint32_t(g.eax)+k);if(!ch){c.length=k;break;}if(k==95||(ch<32)||(ch>126&&ch!=0x99&&ch!=0xa9&&ch!=0xe0)||ch==96||ch==123||ch==124)throw 1;c.text[k]=ch;}if(!c.length)return;c.sourceSize=tx.load<uint32_t>(dword_4E54AC);if(c.sourceSize!=16&&c.sourceSize!=18&&c.sourceSize!=24&&c.sourceSize!=36)throw 1;s->color=tx.load<uint32_t>(dword_4D20A4)|0xff000000u;s->sourceSize=c.sourceSize;c.x=int32_t(g.edx);c.y=int32_t(g.ebx);c.role=c.sourceSize>=24?0:1;c.alignment=0;c.clip={s->clip[0],s->clip[1],s->clip[2],s->clip[3]};const uint32_t color=s->color;c.color=uint16_t(((color>>8)&0xf800)|((color>>5)&0x7e0)|((color>>3)&31));s->command=c;s->stringOpen=true;entered=true;}catch(...){s->invalid=true;}}
 ~StringScope(){if(entered&&s==current(g)&&s->stringOpen){s->invalid=true;s->stringOpen=false;}}
};
inline void hardwareEnd(Game&g,int32_t endX)noexcept{Errno e;auto*s=current(g);if(!s||(!s->active&&!s->whole.active())||s->invalid||!s->stringOpen)return;try{nfs::GuestCopyTransaction tx;if(!tx.load<uint32_t>(dword_4D20A0)||tx.load<uint32_t>(dword_4E54AC)!=s->sourceSize||(tx.load<uint32_t>(dword_4D20A4)|0xff000000u)!=s->color)throw 1;int64_t width=int64_t(endX)-s->command.x;if(width<=0||width>4096)throw 1;s->command.advance=int32_t(width);if(s->whole.active()&&!s->whole.append(s->command,s->color))throw 1;if(s->active){s->working->commands.push_back(s->command);s->working->argb.push_back(s->color);}s->stringOpen=false;}catch(...){s->invalid=true;}}
}

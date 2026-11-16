// SPDX-License-Identifier: MIT
#pragma once
#include "LocationSceneLiveGameHooks.inc"
#include "BackgroundSceneQueueAdapter.inc"
#include "MainThreadBridge.h"
#include "ChildStaticSceneHistory.h"
#include "../MenuResponsiveDebug.h"
#include <stdexcept>
namespace nfs_location_scene_live {
struct Intent{std::shared_ptr<const bg::Prepared>owned;uint64_t serial=0;bool beginConsumed=false,finishAttempted=false,finishConsumed=false;};
// Main and Location share one renderer/compositor serial and active owner path.
inline uint64_t nextRequest()noexcept{return nfs_background_scene_live::nextRequest();}
inline uint64_t&activeRequest()noexcept{return nfs_background_scene_live::activeRequest();}
// Default-off first-refusal source diagnostics only; no retained-frame authority.
inline void queueRefusal(unsigned code,const State*s,const nfs_logo_publisher::Collector*c,const Intent*i=nullptr)noexcept{
 const int saved=errno;const char*flag=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");if(!flag||flag[0]!='1'||flag[1]){errno=saved;return;}
 uint64_t bits=0;if(s){bits|=1;bits|=uint64_t(s->invalid)<<1;bits|=uint64_t(bool(s->published))<<2;bits|=uint64_t(s->baseVerified)<<3;bits|=uint64_t(bool(s->caption))<<4;bits|=uint64_t(bool(s->captionProof))<<5;bits|=uint64_t(bool(s->preview))<<6;bits|=uint64_t(s->scene.complete)<<7;bits|=uint64_t(s->scene.error)<<8;}
 if(c){bits|=uint64_t(1)<<9;bits|=uint64_t(c->invalid)<<10;bits|=uint64_t(c->prepared)<<11;bits|=uint64_t(c->bound)<<12;bits|=uint64_t(c->copySourceVerified)<<13;bits|=uint64_t(c->group.complete)<<14;bits|=uint64_t(c->copy.complete)<<15;bits|=uint64_t(c->groupActive)<<16;bits|=uint64_t(c->copyActive)<<17;bits|=uint64_t(c->uploaded==300)<<18;}
 NfsMenuResponsiveDebugState responsive{};nfs_menu_responsive_debug_state(&responsive);const auto*caption=nfs_location_caption::state();
 struct Key{uint64_t epoch,responsiveEpoch,bits;unsigned code,stop,hits;};static thread_local Key keys[64]{};static thread_local unsigned used;const uint64_t epoch=s?s->ownerEpoch:0;const unsigned stop=c?unsigned(c->stop):0;unsigned k;
 for(k=0;k<used;++k)if(keys[k].epoch==epoch&&keys[k].responsiveEpoch==responsive.sourceEpoch&&keys[k].bits==bits&&keys[k].code==code&&keys[k].stop==stop)break;
 if(k==used){if(used==64){errno=saved;return;}keys[k]={epoch,responsive.sourceEpoch,bits,code,stop,0};++used;}if(keys[k].hits++>=2){errno=saved;return;}
 std::fprintf(stderr,"LOCATION QUEUE refusal=%u flags=%llu form=4d2a20 ownerEpoch=%llu stateReason=%u iteration=%llu phase=%u mainDepth=%u invocation=%u group=%llu copy=%llu stateUpload=%llu liveUpload=%llu captionSerial=%llu captionLifetime=%llu previewSerial=%llu previewLifetime=%llu collectorRevision=%llu collectorGroup=%llu collectorCopy=%llu collectorUpload=%llu collectorDepth=%u collectorInvocation=%u collectorStop=%u groupDraws=%u intentSerial=%llu activeRequest=%llu content=%llu nextContent=%llu captionOwnerPresent=%u captionOwnerDepth=%u captionOwnerLifetime=%llu captionOwnerSerial=%llu captionOwnerFailure=%u captionOwnerRecording=%u responsiveSchema=%u responsiveForm=%x responsiveEpoch=%llu responsiveX=%u responsiveY=%u shownEpoch=%llu shownRevision=%llu\n",code,(unsigned long long)bits,(unsigned long long)epoch,s?unsigned(s->reason):0,(unsigned long long)(s?s->iteration:0),s?unsigned(s->phase):0,s?s->mainDepth:0,s?s->invocationDepth:0,(unsigned long long)(s?s->groupSerial:0),(unsigned long long)(s?s->copySerial:0),(unsigned long long)(s?s->nativeUploadEvent:0),(unsigned long long)nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire),(unsigned long long)(s?s->captionSerial:0),(unsigned long long)(s?s->captionLifetime:0),(unsigned long long)(s?s->previewSerial:0),(unsigned long long)(s?s->previewLifetime:0),(unsigned long long)(c?c->iterationRevision:0),(unsigned long long)(c?c->group.serial:0),(unsigned long long)(c?c->copy.serial:0),(unsigned long long)(c?c->nativeUploadEvent:0),c?c->depth:0,c?c->invocationDepth:0,stop,c?c->group.draws:0,(unsigned long long)(i?i->serial:0),(unsigned long long)activeRequest(),(unsigned long long)(s&&s->published?s->published->contentRevision:0),(unsigned long long)(s?s->nextContent:0),unsigned(bool(caption)),caption?caption->ownerDepth:0,(unsigned long long)(caption?caption->lifetime:0),(unsigned long long)(caption?caption->serial:0),caption?unsigned(caption->failure):0,unsigned(caption&&caption->recording),responsive.schema,responsive.sourceForm,(unsigned long long)responsive.sourceEpoch,responsive.sourceX,responsive.sourceY,(unsigned long long)responsive.shownEpoch,(unsigned long long)responsive.shownRevision);errno=saved;
}
struct Builder{State*s=nullptr;nfs_logo_publisher::Collector*c=nullptr;std::shared_ptr<const bg::Prepared>owned;};
inline bool sourceSnapshot(void*context,const nfs_logo_publisher::OwnedPacket&packet)noexcept{auto&b=*static_cast<Builder*>(context);unsigned reason=0;
 if(!b.s)reason=20;else if(!b.c)reason=21;else if(b.s->invalid)reason=22;else if(!b.s->published)reason=23;else if(&packet!=&b.c->packet)reason=24;else if(!packet.prepared.valid)reason=25;else if(b.s->copySerial!=b.c->copy.serial)reason=26;else if(b.s->groupSerial!=b.c->group.serial)reason=27;else if(b.s->nativeUploadEvent!=b.c->nativeUploadEvent)reason=28;else if(b.s->published->original!=packet.originalCopyPost565)reason=29;else if(b.s->published->keys!=b.c->rendererAddresses)reason=30;
 if(reason){queueRefusal(reason,b.s,b.c);return false;}b.owned=b.s->published;return true;
}
inline void cancel(Intent&i)noexcept{if(activeRequest()==i.serial)activeRequest()=0;i.owned.reset();i.serial=0;i.beginConsumed=false;}
inline void enqueueBegin(const std::shared_ptr<const bg::Prepared>&owned,Intent&i,State&source){if(!enabled()||!owned||!owned->valid||activeRequest()||i.serial||i.beginConsumed)return;const uint64_t serial=nextRequest();if(!serial)return;NfsMenuCachedSnapshot responsive{};const int priorErrno=errno;try{nfs_menu_cached_snapshot(NFS_MENU_CACHED_LOCATION,uint32_t(guest_address(off_4D2A20)),source.ownerEpoch,source.game,serial,owned->contentRevision,&responsive);}catch(...){nfs_menu_pointer_metadata_failure();}errno=priorErrno;i.owned=owned;i.serial=serial;nfs::check_worker_stop();const auto receipt=nfs::enqueue_owned_renderer([owned,serial,responsive](){auto view=owned->view();view.scene.serial=serial;view.responsive=responsive;return int32_t(nfs_private_location_scene_begin(&view));});if(receipt==nfs::OwnedRendererAcceptance::UseExistingOwnerPath){i.owned.reset();i.serial=0;return;}i.beginConsumed=true;activeRequest()=serial;nfs::check_worker_stop();}
inline std::shared_ptr<const bg::Prepared> retainedAfterAbort(Game&g,State&s)noexcept{
 const auto&frame=s.retainedFrame;
 if(s.invalid||!s.baseVerified||!s.iteration||s.abortIteration!=s.iteration
     ||s.retainedBase!=s.base.event.sequence||!s.retainedUpload
     ||s.retainedUpload!=nfs_logo_publisher::uploadEvents().load(std::memory_order_acquire)
     ||!frame||!frame->valid||!frame->captionWord||!frame->captionWord->current()
     ||frame->preview!=s.preview||!previewCurrent(s)||!archiveCurrent(s)
     ||uint32_t(g.esi)!=uint32_t(guest_address(off_4D2A20))
     ||uint32_t(g.eax)!=1||g.edx||g.ebx||uint32_t(g.ecx)!=32)return {};
 try{
  nfs::GuestCopyTransaction tx;
  if(tx.load<uint32_t>(uint32_t(g.esp))!=UINT32_MAX)return {};
  bg::Target target{};
  nfs_logo_publisher::target(tx,target,s.retainedTarget.owner);
  if(!nfs_main_sidebar_origin::sameRetainedTarget(target,s.retainedTarget))return {};
  const auto bytes=frame->original.size()*sizeof(uint16_t);
  const auto at=target.w[8];
  nfs_logo_publisher::checked(at,16+bytes);
  // The verified copy can change the CPU surface before an interrupted upload.
  // The renderer still matches every GPU tile against the retained frame.
  if(!s.abortAfterCopy&&std::memcmp(nfs_host_address(at+16,bytes),frame->original.data(),bytes))return {};
  for(unsigned i=0;i<300;++i){
   const auto descriptor=tx.load<uint32_t>(dword_4EB74C+1200+4*i);
   std::array<uint32_t,7>words{};
   if(!descriptor)return {};
   nfs_logo_publisher::descriptor(tx,descriptor,words);
   if(words[0]!=3||words[1]!=3||words[2]!=3||words[3]!=11||words[6]!=3||words[5]!=frame->keys[i])return {};
  }
  return frame;
 }catch(...){return {};}
}
inline void begin(Game&g,Intent&i){auto*s=current(g);auto*c=nfs_logo_publisher::store();unsigned refusal=0;if(!s)refusal=1;else if(s->invalid)refusal=2;else if(!s->published){if(auto retained=retainedAfterAbort(g,*s)){enqueueBegin(retained,i,*s);return;}refusal=3;}else if(!c)refusal=4;else if(activeRequest())refusal=5;else if(i.serial)refusal=6;else if(i.beginConsumed)refusal=7;if(refusal){queueRefusal(refusal,s,c,&i);return;}if(!s->published->captionWord)refusal=8;else if(!s->published->captionWord->current())refusal=9;else if(s->published->captionWord->packet()!=s->caption)refusal=10;else if(!captionCurrent(g,*s,c->copy))refusal=11;else if(!previewCurrent(*s))refusal=12;if(refusal){queueRefusal(refusal,s,c,&i);s->fail(Reason::Missing);return;}if(!archiveCurrent(*s)){queueRefusal(13,s,c,&i);s->fail(Reason::Source,true);return;}Builder b{s,c,{}};if(!nfs_logo_publisher::publishNativeEntry(g,sourceSnapshot,&b,0)){queueRefusal(14,s,c,&i);return;}if(b.owned){NfsChildExitPreludeView v{};v.family=NFS_CHILD_EXIT_LOCATION;v.form=0x4d2a20;v.location=b.owned->view();nfs_child_static_scene::Domain d{s->game,s->thread,uint32_t(guest_address(off_4D2A20)),s->ownerEpoch,s->archive,s->nativeUploadEvent,s->groupSerial,s->copySerial,s->catalogue};(void)nfs_child_static_scene::remember(d,b.owned,v,c->copy);}enqueueBegin(b.owned,i,*s);
 if(i.beginConsumed&&b.owned){
  s->retainedFrame=b.owned;
  s->retainedTarget=c->copy.post.target;
  s->retainedBase=s->base.event.sequence;
  s->retainedUpload=s->nativeUploadEvent;
 }
}
inline void finish(Intent&i){if(!i.beginConsumed||i.finishAttempted)return;if(!i.serial||activeRequest()!=i.serial){cancel(i);return;}nfs::check_worker_stop();i.finishAttempted=true;const auto owned=i.owned;const uint64_t serial=i.serial;const auto receipt=nfs::enqueue_owned_renderer([owned,serial](){if(!owned||!owned->preview||!owned->preview->artworkCurrent())return int32_t(nfs_private_location_scene_finish(0));return int32_t(nfs_private_location_scene_finish(serial));});if(receipt!=nfs::OwnedRendererAcceptance::Consumed)throw std::logic_error("background scene finish changed renderer owner");i.finishConsumed=true;activeRequest()=0;nfs::check_worker_stop();}
} // namespace nfs_location_scene_live

// SPDX-License-Identifier: MIT
#pragma once
#include "hd/ContinuousLogoGameHooks.inc"
#include "hd/TrackBadgeSourceCatalog.h"
#include "hd/TrackBadgeArtProvider.h"
#include "hd/TrackBadgeAnimationAbi.h"
#include "MainThreadBridge.h"
#include "GuestCopyTransaction.h"
#include "TextureDigest.h"
#include <cerrno>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <array>
#include <vector>
#include <cstring>
#include <cmath>
namespace nfs_track_badge_live {
inline void descriptor(nfs::GuestCopyTransaction&tx,uint32_t at,std::array<uint32_t,7>&out){if(!at)throw 1;(void)nfs_host_address(at,28);for(unsigned k=0;k<7;++k)out[k]=tx.load<uint32_t>(at+4*k);}
struct Errno {int value=errno;~Errno(){errno=value;}};
struct Art {NfsTrackBadgeArtView view{};~Art(){if(view.owner&&view.release)view.release(view.owner);}bool current()const{return view.generation&&view.generation==nfs_track_badge_art_generation();}static std::shared_ptr<const Art>acquire(unsigned id){auto a=std::make_shared<Art>();if(!nfs_track_badge_art_acquire(id,&a->view)||a->view.schema!=1||a->view.count!=1||!a->current())return {};return a;}};
struct Proof {Game*game=nullptr;uintptr_t thread=0;uint64_t epoch=0,serial=0;unsigned owner=0,index=0;uint32_t archive=0,descriptor=0,key=0;std::array<uint32_t,7>descriptorWords{};std::array<uint16_t,65536>words{};std::vector<unsigned char>source;std::shared_ptr<const Art>art;};
struct Build {Game*game=nullptr;std::shared_ptr<Proof>p;uint32_t bitmap=0,window=0,converted=0;int x=0,y=0;unsigned phase=0;bool bad=false,retired=false;const char*reason="incomplete";std::vector<uint32_t>stage;};
struct State {Game*game=nullptr;uintptr_t thread=0;uint64_t epoch=0,serial=0;unsigned owner=0,track=UINT32_MAX;Build*building=nullptr;bool mesh=false;std::shared_ptr<const Proof>published,drawing;};
inline State&state(){static thread_local State s;return s;}
inline void trace(const Build&b,const char*status,const char*reason)noexcept{if(!std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE"))return;static std::atomic<unsigned>lines{0};if(lines.fetch_add(1,std::memory_order_relaxed)>=128)return;unsigned index=b.p&&b.phase?b.p->index:UINT32_MAX;unsigned art=b.p&&b.p->art?b.p->art->view.images[0].sourceIndex:UINT32_MAX;std::fprintf(stderr,"TRACK BADGE source %s serial=%llu phase=%u sourceIndex=%u catalogIndex=%u selectedTrack=%u reason=%s\n",status,(unsigned long long)(b.p?b.p->serial:0),b.phase,art,index,state().track,reason);}

inline bool sourceCurrent(const Proof&p){auto&s=nfs_track_badge_sources[p.index];nfs::GuestCopyTransaction tx;if(!p.archive||tx.load<uint32_t>(dword_4D40B8)!=p.archive||tx.load<uint32_t>(p.archive)!=0x49504853||tx.load<uint32_t>(p.archive+4)!=s.archiveBytes||tx.load<uint32_t>(p.archive+8)!=s.entries||uint64_t(p.archive)+s.archiveBytes>UINT64_C(0x100000000)||p.source.size()!=s.span)return false;unsigned found=0;for(unsigned k=0;k<s.entries;++k){uint32_t at=p.archive+16+8*k;if(!std::memcmp(nfs_host_address(at,4),s.name,4)){if(tx.load<uint32_t>(at+4)!=s.offset)return false;++found;}}return found==1&&!std::memcmp(p.source.data(),nfs_host_address(p.archive+s.offset,s.span),s.span);}
inline bool current(Game&g,const Proof&p){auto&s=state();return s.owner&&s.owner==p.owner&&s.game==&g&&p.game==&g&&s.thread==p.thread&&p.thread==nfs_logo_publisher::threadToken()&&s.epoch==p.epoch&&s.track==nfs_track_badge_sources[p.index].track&&p.art&&p.art->current()&&sourceCurrent(p);}
struct OwnerScope {bool entered=false;explicit OwnerScope(Game&g,unsigned owner)noexcept{Errno guard;auto&s=state();if(!nfs_track_badge_art_generation())return;if(s.owner||s.epoch==UINT64_MAX){s.published.reset();return;}s.game=&g;s.thread=nfs_logo_publisher::threadToken();s.owner=owner;s.track=UINT32_MAX;s.mesh=false;++s.epoch;s.published.reset();s.drawing.reset();entered=true;}~OwnerScope()noexcept{Errno guard;if(!entered)return;auto&s=state();s.published.reset();s.drawing.reset();s.building=nullptr;s.owner=0;s.game=nullptr;}};
// This observes the real lookup input independently of the image name/size.
inline void selected(Game&g)noexcept{Errno guard;auto&s=state();if(!s.owner||s.game!=&g)return;int32_t t=int16_t(uint32_t(g.eax));s.track=t>=0&&t<8?unsigned(t):UINT32_MAX;}
struct BuildScope {Game&g;Build b;bool entered=false;int unwind=0;explicit BuildScope(Game&v)noexcept:g(v){Errno guard;auto&s=state();if(!s.owner||s.game!=&g||!nfs_track_badge_art_generation())return;s.published.reset();if(s.building){s.building->bad=true;return;}try{nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(uint32_t(g.edx))!=0x36636572||tx.load<uint8_t>(uint32_t(g.edx)+4)||uint32_t(g.eax)||tx.load<uint32_t>(uint32_t(g.esp)+4)){trace(b,"refused","build-input-name-eax-or-stackarg");return;}auto p=std::make_shared<Proof>();if(s.serial==UINT64_MAX)return;p->serial=++s.serial;p->game=&g;p->thread=s.thread;p->epoch=s.epoch;p->owner=s.owner;p->archive=tx.load<uint32_t>(dword_4D40B8);b.game=&g;b.p=p;b.x=int32_t(g.ecx);b.y=0;s.building=&b;entered=true;unwind=nfs_logo_publisher::exceptions();}catch(...){}}
 ~BuildScope()noexcept{Errno guard;if(!entered)return;auto&s=state();s.building=nullptr;if(b.bad||b.phase!=4||!b.retired||nfs_logo_publisher::exceptions()>unwind){trace(b,"refused",b.bad?b.reason:!b.retired?"staging-not-retired":"incomplete-or-unwound");return;}try{if(!current(g,*b.p)){trace(b,"refused","owner-source-or-art-stale");return;}auto p=b.p;auto accepted=nfs::enqueue_owned_renderer([p]{NfsTrackBadgeUpload v{p->serial,p->art->view.generation,p->epoch,p->key,p->words.data(),p->art->view.images[0].pixels};return int32_t(nfs_private_track_badge_upload(&v));});if(accepted==nfs::OwnedRendererAcceptance::Consumed&&current(g,*p)){s.published=p;trace(b,"published","exact-source-stage-alpha200-upload");}else trace(b,"refused","renderer-upload-or-owner-refused");}catch(...){trace(b,"refused","publication-exception");}}
};
inline void resolved(Game&g)noexcept{
 Errno guard;auto*b=state().building;if(!b||b->bad)return;
 try{
  if(b->phase||uint32_t(g.edx)!=0x1900||int32_t(g.ebx)!=b->x||int32_t(g.ecx)!=b->y)throw 1;
  auto&p=*b->p;uint32_t bytes=0,entries=0;
  {nfs::GuestCopyTransaction tx;bytes=tx.load<uint32_t>(p.archive+4);entries=tx.load<uint32_t>(p.archive+8);}
  bool matched=false;
  for(unsigned k=0;k<sizeof(nfs_track_badge_sources)/sizeof(*nfs_track_badge_sources);++k){
   const auto&r=nfs_track_badge_sources[k];
   if(r.name[3]!='6'||r.track!=state().track||r.role!=(p.owner==1?0u:1u)||r.archiveBytes!=bytes||r.entries!=entries||uint64_t(p.archive)+r.offset!=uint32_t(g.eax)||r.x!=b->x||r.y!=b->y)continue;
   p.index=k;p.source.resize(r.span);
   {nfs::GuestCopyTransaction tx;std::memcpy(p.source.data(),nfs_host_address(uint32_t(g.eax),r.span),r.span);}
   unsigned char sha[32];if(!nfs_texture_sha256(p.source.data(),p.source.size(),sha)||std::memcmp(sha,r.sha,32))throw 1;
   // Provider acquisition and current() may take their own locks. Neither may
   // execute while a checked guest-copy transaction is alive.
   unsigned id=r.track*2;if(r.role&&r.track==2)id=16;p.art=Art::acquire(id);
   if(!p.art||!current(g,p))throw 1;matched=true;break;
  }
  if(!matched)throw 1;b->phase=1;
 }catch(...){b->reason="resolve-archive-entry-digest-track-flags-or-registration";b->bad=true;}
}
inline uint32_t targetBitmap(){nfs::GuestCopyTransaction tx;uint32_t a=uint32_t(guest_address(dword_4DB240));if(tx.load<uint32_t>(a)!=0x57494e44||tx.load<uint32_t>(a+4)!=256||tx.load<uint32_t>(a+8)!=256||(tx.load<uint32_t>(a+28)&255)!=32||tx.load<uint32_t>(a+40)!=1024)throw 1;for(unsigned k=0;k<4;++k)if(tx.load<uint32_t>(a+12+4*k)!=(k<2?0u:256u))throw 1;uint32_t bitmap=tx.load<uint32_t>(a+32);if(!bitmap||tx.load<uint32_t>(bitmap+4)!=(256u|(256u<<16)))throw 1;return bitmap;}
struct StageScope {Game&g;Build*b=nullptr;int unwind=0;explicit StageScope(Game&v)noexcept:g(v){Errno guard;b=state().building;if(!b||b->bad){b=nullptr;return;}try{auto&p=*b->p;const auto&r=nfs_track_badge_sources[p.index];if(b->phase!=1||uint32_t(g.eax)!=p.archive+r.offset||int32_t(g.edx)!=r.x||int32_t(g.ebx)!=r.y||uint32_t(g.ecx)!=0x100||!current(g,p))throw 1;b->bitmap=targetBitmap();const auto*bits=static_cast<const uint32_t*>(nfs_host_address(b->bitmap+16,262144));for(unsigned k=0;k<65536;++k)if(bits[k])throw 1;unwind=nfs_logo_publisher::exceptions();}catch(...){b->reason="source-draw-input-window-clip-or-zero-clear";b->bad=true;b=nullptr;}}
 ~StageScope()noexcept{Errno guard;if(!b)return;try{if(targetBitmap()!=b->bitmap||nfs_logo_publisher::exceptions()>unwind||!current(g,*b->p))throw 1;b->stage.resize(65536);std::memcpy(b->stage.data(),nfs_host_address(b->bitmap+16,262144),262144);b->phase=2;}catch(...){b->reason="source-draw-output-window-or-owner";b->bad=true;}}
};
struct ConvertScope {Game&g;Build*b=nullptr;int unwind=0;explicit ConvertScope(Game&v)noexcept:g(v){Errno guard;b=state().building;if(!b||b->bad){b=nullptr;return;}try{if(b->phase!=2||uint32_t(g.eax)!=b->bitmap||uint32_t(g.edx)!=200||b->stage.size()!=65536||std::memcmp(b->stage.data(),nfs_host_address(b->bitmap+16,262144),262144)||!current(g,*b->p))throw 1;unwind=nfs_logo_publisher::exceptions();}catch(...){b->reason="alpha200-conversion-input-stage";b->bad=true;b=nullptr;}}
 ~ConvertScope()noexcept{Errno guard;if(!b)return;try{if(nfs_logo_publisher::exceptions()>unwind||!current(g,*b->p))throw 1;std::memcpy(b->p->words.data(),nfs_host_address(b->bitmap+16,131072),131072);for(unsigned k=0;k<65536;++k){uint32_t w=b->stage[k];uint16_t expected=(w>>24)<200?0:uint16_t(0x8000|((w>>9)&0x7c00)|((w>>6)&0x3e0)|((w>>3)&31));if(b->p->words[k]!=expected)throw 1;}b->converted=b->bitmap+16;b->phase=3;}catch(...){b->reason="alpha200-conversion-output-words";b->bad=true;}}
};
struct UploadScope {
 Game&g;Build*b=nullptr;int unwind=0;
 explicit UploadScope(Game&v)noexcept:g(v){
  Errno guard;b=state().building;if(!b||b->bad){b=nullptr;return;}
  try{
   auto&p=*b->p;
   {nfs::GuestCopyTransaction tx;uint32_t sp=uint32_t(g.esp);p.descriptor=tx.load<uint32_t>(sp+4);
    if(b->phase!=3||p.descriptor!=tx.load<uint32_t>(dword_4D4E44)||tx.load<uint32_t>(sp+8)!=b->converted||tx.load<uint32_t>(sp+12)||std::memcmp(p.words.data(),nfs_host_address(b->converted,131072),131072))throw 1;
    descriptor(tx,p.descriptor,p.descriptorWords);const auto&w=p.descriptorWords;if(w[0]||w[1]||w[2]!=3||w[3]!=11||w[6]!=3)throw 1;
   }
   if(!current(g,p))throw 1;unwind=nfs_logo_publisher::exceptions();
  }catch(...){b->reason="descriptor-upload-input-payload-lod-format";b->bad=true;b=nullptr;}
 }
 ~UploadScope()noexcept{
  Errno guard;if(!b)return;
  try{
   auto&p=*b->p;std::array<uint32_t,7>w{};
   {nfs::GuestCopyTransaction tx;descriptor(tx,p.descriptor,w);for(unsigned k=0;k<7;++k)if(k!=4&&w[k]!=p.descriptorWords[k])throw 1;
    if(w[4]!=b->converted||std::memcmp(p.words.data(),nfs_host_address(b->converted,131072),131072))throw 1;
   }
   if(nfs_logo_publisher::exceptions()>unwind||!current(g,p))throw 1;
   p.key=w[5];p.descriptorWords=w;b->phase=4;
  }catch(...){b->reason="descriptor-upload-output-key-payload";b->bad=true;}
 }
};
inline void retired(Game&g)noexcept{Errno guard;auto*b=state().building;if(!b||b->bad||b->phase!=4)return;try{nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(uint32_t(g.eax)+32)!=b->bitmap)throw 1;b->window=uint32_t(g.eax);b->retired=true;}catch(...){b->reason="staging-window-retirement";b->bad=true;}}
struct DrawScope {
 Game&g;std::shared_ptr<const Proof>p;bool active=false;
 explicit DrawScope(Game&v)noexcept:g(v){
  Errno guard;auto&s=state();p=s.published;if(!p)return;
  try{
   if(!current(g,*p))return;
   {nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(dword_4D4E44)!=p->descriptor)return;std::array<uint32_t,7>w{};descriptor(tx,p->descriptor,w);if(w!=p->descriptorWords)return;}
   if(!current(g,*p))return;
   auto keep=p;active=nfs::enqueue_owned_renderer([keep]{return int32_t(nfs_private_track_badge_begin(keep->serial,keep->epoch));})==nfs::OwnedRendererAcceptance::Consumed;
   if(active)s.drawing=p;
  }catch(...){}
 }
 ~DrawScope()noexcept{Errno guard;if(!active)return;state().drawing.reset();try{auto keep=p;bool valid=current(g,*p);nfs::enqueue_owned_renderer([keep,valid]{return int32_t(nfs_private_track_badge_end(valid?keep->serial:0));});}catch(...){}}
};
struct MeshScope {
 bool entered=false;
 explicit MeshScope(Game&g)noexcept{
  Errno guard;auto&s=state();auto p=s.drawing;if(!p)return;
  try{
   if(s.mesh||!current(g,*p))throw 1;
   {nfs::GuestCopyTransaction tx;uint32_t sp=uint32_t(g.esp),indices=uint32_t(guest_address(dword_4D4D0C));
    if(tx.load<uint32_t>(sp+4)!=1||tx.load<uint32_t>(sp+8)!=uint32_t(guest_address(flt_4D4C8C))||tx.load<uint32_t>(sp+12)!=indices)throw 1;
    for(unsigned k=0;k<4;++k)if(tx.load<uint32_t>(indices+4*k)!=k)throw 1;
   }
   if(!current(g,*p))throw 1;s.mesh=true;entered=true;
  }catch(...){try{nfs::enqueue_owned_renderer([]{return int32_t(nfs_private_track_badge_end(0));});}catch(...){}}
 }
 ~MeshScope()noexcept{if(entered)state().mesh=false;}
};
inline void quad(Game&g)noexcept{
 Errno guard;auto p=state().drawing;if(!p||!state().mesh)return;
 try{
  if(!current(g,*p))throw 1;
  auto vertices=std::make_shared<std::array<float,60>>();
  {nfs::GuestCopyTransaction tx;uint32_t sp=uint32_t(g.esp),a=tx.load<uint32_t>(sp),b=tx.load<uint32_t>(sp+4),c=tx.load<uint32_t>(sp+8);
   if(uint64_t(a)+240>UINT64_C(0x100000000)||b!=a+60||c!=a+120)throw 1;
   std::memcpy(vertices->data(),nfs_host_address(a,240),240);
  }
  if(!current(g,*p))throw 1;
  nfs::enqueue_owned_renderer([p,vertices]{return int32_t(nfs_private_track_badge_quad(p->serial,vertices->data()));});
 }catch(...){try{nfs::enqueue_owned_renderer([]{return int32_t(nfs_private_track_badge_end(0));});}catch(...){}}
}
}




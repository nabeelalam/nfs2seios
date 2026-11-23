// SPDX-License-Identifier: MIT
// Include at the END of VictoryGameHooks.inc, outside its namespace, after State.
// Original calls/stores are observed, never replaced. No freed QFS/staging reads.
#pragma once
#include "VictoryLogoLayerAbi.h"
#include "VictoryLogoSourceCatalog.h"
#include "hd/TrackBadgeSourceCatalog.h"
#include "hd/TrackBadgeArtProvider.h"
#include "hd/TrackBadgeAnimationAbi.h"
#include "hd/BackgroundSceneQueueAdapter.inc"
#include "MainThreadBridge.h"
#include "TextureDigest.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>
namespace nfs_victory_ui_live {
inline void logoTrace(const char*event,const char*reason,unsigned car,uint64_t serial=0)noexcept{
 const int saved=errno;const char*on=std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");static thread_local unsigned count=0;
 if(on&&on[0]=='1'&&!on[1]&&count++<128)std::fprintf(stderr,"VICTORY LOGO source=%s reason=%s car=%u serial=%llu\n",event,reason,car,(unsigned long long)serial);
 errno=saved;
}
inline bool logoDigestPresent(const unsigned char*sha)noexcept{unsigned v=0;for(unsigned k=0;k<32;++k)v|=sha[k];return v!=0;}
inline void logoDescriptor(nfs::GuestCopyTransaction&tx,uint32_t at,std::array<uint32_t,7>&out){
 if(!at||(at&3)||uint64_t(at)+28>UINT64_C(0x100000000))throw 1;
 for(unsigned k=0;k<7;++k)out[k]=tx.load<uint32_t>(at+4*k);
}
struct LogoArt {
 NfsTrackBadgeArtView view{};
 ~LogoArt(){if(view.owner&&view.release)view.release(view.owner);}
 bool current()const noexcept{return view.generation&&view.generation==nfs_track_badge_art_generation();}
 static std::shared_ptr<const LogoArt>acquire(unsigned sourceCar){
  if(sourceCar>=12)return {};auto p=std::make_shared<LogoArt>();
  if(!nfs_track_badge_art_acquire(18+sourceCar,&p->view))return {};
  const auto&v=p->view;const auto&i=v.images[0];unsigned char sha[32];
  if(v.schema!=1||v.count!=1||!v.owner||!v.retain||!v.release||!p->current()||
     i.sourceIndex!=18+sourceCar||i.format!=NFS_TRACK_BADGE_RGBA_PREMULTIPLIED||
     i.width!=1024||i.height!=1024||i.bytes!=1024u*1024u*4u||!i.pixels||
     !nfs_texture_sha256(i.pixels,i.bytes,sha)||std::memcmp(sha,i.sha,32))return {};
  return p;
 }
};
struct LogoProof {
 Game*game=nullptr;uintptr_t thread=0;uint64_t epoch=0,serial=0;
 unsigned car=UINT32_MAX,sourceCar=UINT32_MAX,sourceIndex=UINT32_MAX;
 uint32_t descriptor=0,address=0;
 // descriptorWords[4] is compared as an opaque VALUE after staging retirement.
 // It is never dereferenced outside LogoUploadScope.
 std::array<uint32_t,7>descriptorWords{};
 std::vector<unsigned char>sourceBytes;
 std::vector<uint16_t>words;
 std::shared_ptr<const LogoArt>art;
};
struct LogoBuild {
 Game*game=nullptr;std::shared_ptr<LogoProof>proof;
 uint32_t archive=0,bitmap=0,staging=0;unsigned uploads=0;
 bool bad=false,uploaded=false,registered=false;const char*reason="incomplete";
};
inline void logoBuildFailed(LogoBuild&b,const char*why)noexcept{b.bad=true;b.reason=why;}
inline const NfsTrackBadgeSource*logoRawSource(unsigned sourceCar,unsigned*index=nullptr)noexcept{
 const NfsTrackBadgeSource*out=nullptr;
 for(unsigned k=0;k<sizeof(nfs_track_badge_sources)/sizeof(*nfs_track_badge_sources);++k){const auto&r=nfs_track_badge_sources[k];
  if(r.role==2&&r.track==sourceCar){if(out)return nullptr;out=&r;if(index)*index=k;}}
 return out;
}
// Called only from the original winning-car selection inside4159E0.
inline void logoSelected(Game&g)noexcept{CpuErrno preserve(g);auto*s=current(g);if(!s)return;
 try{const int car=int16_t(uint32_t(g.ecx));if(s->logoBuilding)logoBuildFailed(*s->logoBuilding,"selection-during-build");
  s->logo.reset();s->logoCar=car>=0&&car<12?unsigned(car):UINT32_MAX;
  logoTrace("selected",s->logoCar<12?"finite-winner":"unsupported-winner",s->logoCar);
 }catch(...){s->logo.reset();s->logoCar=UINT32_MAX;}
}
// This function is legal ONLY while4280B0 owns the still-live transient QFS.
inline bool logoBuildCurrent(Game&g,const LogoBuild&b){
 auto*s=current(g);const auto p=b.proof;
 if(!s||b.bad||s->logoBuilding!=&b||b.game!=&g||!p||p->game!=&g||p->thread!=threadToken()||
    p->epoch!=s->epoch||s->logoCar!=p->car||p->car>=12||p->sourceCar>=12||
    !s->ready||s->capturing||!p->art||!p->art->current())return false;
 const auto*r=logoRawSource(p->sourceCar);if(!r||p->sourceBytes.size()!=r->span||
    uint64_t(b.archive)+r->archiveBytes>UINT64_C(0x100000000)||b.bitmap!=b.archive+r->offset)return false;
 nfs::GuestCopyTransaction tx;
 if(tx.load<uint32_t>(dword_4D4208)!=b.archive||tx.load<uint32_t>(b.archive)!=0x49504853u||
    tx.load<uint32_t>(b.archive+4)!=r->archiveBytes||tx.load<uint32_t>(b.archive+8)!=r->entries)return false;
 unsigned found=0;for(unsigned k=0;k<r->entries;++k){const uint32_t at=b.archive+16+8*k;
  if(!std::memcmp(nfs_host_address(at,4),"car6",4)&&tx.load<uint32_t>(at+4)==r->offset)++found;}
 return found==1&&!std::memcmp(nfs_host_address(b.bitmap,r->span),p->sourceBytes.data(),r->span);
}
struct LogoBuildScope {
 Game&g;State*s=nullptr;LogoBuild b;bool entered=false;int unwind=0;
 explicit LogoBuildScope(Game&v)noexcept:g(v){CpuErrno preserve(g);s=current(g);
  if(!s||!s->ready||s->capturing||s->logoCar>=12)return;
  s->logo.reset();if(s->logoBuilding){logoBuildFailed(*s->logoBuilding,"nested-builder");return;}
  try{const unsigned car=s->logoCar;const auto&pin=nfs_victory_logo_source_pins[car];
   if(pin.sourceCar!=(car==2?1u:car)||!logoDigestPresent(pin.wordsSha256))throw 1;
   unsigned sourceIndex=UINT32_MAX;const auto*r=logoRawSource(pin.sourceCar,&sourceIndex);
   if(!r||std::memcmp(r->name,"car6",4)||r->y||r->offset>r->archiveBytes||r->span>r->archiveBytes-r->offset)throw 1;
   auto p=std::make_shared<LogoProof>();p->game=&g;p->thread=threadToken();p->epoch=s->epoch;
   p->car=car;p->sourceCar=pin.sourceCar;p->sourceIndex=sourceIndex;p->serial=nfs_background_scene_live::nextRequest();if(!p->serial)throw 1;
   b.game=&g;b.proof=p;
   {nfs::GuestCopyTransaction tx;b.archive=tx.load<uint32_t>(dword_4D4208);b.bitmap=uint32_t(g.edx);
    if(!b.archive||uint64_t(b.archive)+r->archiveBytes>UINT64_C(0x100000000)||
       uint32_t(g.eax)||int32_t(g.ecx)!=r->x||uint32_t(g.ebx)||
       tx.load<uint32_t>(uint32_t(g.esp)+4)||b.bitmap!=b.archive+r->offset||
       tx.load<uint32_t>(b.archive)!=0x49504853u||tx.load<uint32_t>(b.archive+4)!=r->archiveBytes||
       tx.load<uint32_t>(b.archive+8)!=r->entries)throw 1;
    p->sourceBytes.resize(r->span);std::memcpy(p->sourceBytes.data(),nfs_host_address(b.bitmap,r->span),r->span);}
   unsigned char sha[32];if(!nfs_texture_sha256(p->sourceBytes.data(),p->sourceBytes.size(),sha)||std::memcmp(sha,r->sha,32))throw 1;
   // Provider acquisition must occur outside any checked guest-copy transaction.
   p->art=LogoArt::acquire(p->sourceCar);if(!p->art)throw 1;
   s->logoBuilding=&b;entered=true;unwind=nfs_logo_publisher::exceptions();
   if(!logoBuildCurrent(g,b))throw 1;
  }catch(...){if(s&&s->logoBuilding==&b)s->logoBuilding=nullptr;entered=false;b.bad=true;logoTrace("build-refused","finite-source-input-or-art",s?s->logoCar:UINT32_MAX);}
 }
 ~LogoBuildScope()noexcept{CpuErrno preserve(g);if(!entered)return;
  try{bool valid=s==current(g)&&s->logoBuilding==&b&&!b.bad&&b.uploads==1&&b.uploaded&&b.registered&&
    nfs_logo_publisher::exceptions()==unwind&&logoBuildCurrent(g,b);
   if(valid){nfs::GuestCopyTransaction tx;std::array<uint32_t,7>w{};logoDescriptor(tx,b.proof->descriptor,w);
    valid=tx.load<uint32_t>(dword_4D4E44)==b.proof->descriptor&&w==b.proof->descriptorWords;}
   if(s->logoBuilding==&b)s->logoBuilding=nullptr;
   if(valid){s->logo=std::shared_ptr<const LogoProof>(b.proof);logoTrace("published","finite-source-and-pinned-native-upload",b.proof->car,b.proof->serial);}
   else logoTrace("build-refused",b.bad?b.reason:"source-owner-upload-or-unwind",b.proof?b.proof->car:UINT32_MAX);
  }catch(...){if(s&&s->logoBuilding==&b)s->logoBuilding=nullptr;logoTrace("build-refused","publication-exception",b.proof?b.proof->car:UINT32_MAX);}
 }
};
struct LogoUploadScope {
 Game&g;LogoBuild*b=nullptr;std::array<uint32_t,7>before{};bool entered=false;int unwind=0;
 explicit LogoUploadScope(Game&v)noexcept:g(v){CpuErrno preserve(g);auto*s=current(g);if(!s||!s->logoBuilding)return;b=s->logoBuilding;
  try{if(!logoBuildCurrent(g,*b)||++b->uploads!=1)throw 1;auto&p=*b->proof;
   {nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp);p.descriptor=tx.load<uint32_t>(sp+4);b->staging=tx.load<uint32_t>(sp+8);
    if(!b->staging||tx.load<uint32_t>(sp+12)||tx.load<uint32_t>(dword_4D4E44)!=p.descriptor)throw 1;
    logoDescriptor(tx,p.descriptor,before);if(before[0]||before[1]||before[2]!=3||before[3]!=11||before[6]!=3)throw 1;
    p.words.resize(65536);std::memcpy(p.words.data(),nfs_host_address(b->staging,131072),131072);}
   unsigned char sha[32];if(!nfs_texture_sha256(reinterpret_cast<const unsigned char*>(p.words.data()),131072,sha)||
      std::memcmp(sha,nfs_victory_logo_source_pins[p.car].wordsSha256,32))throw 1;
   entered=true;unwind=nfs_logo_publisher::exceptions();
  }catch(...){if(b)logoBuildFailed(*b,"upload-input-or-pinned-words");entered=false;}
 }
 ~LogoUploadScope()noexcept{CpuErrno preserve(g);if(!entered||!b)return;
  try{if(b->bad||nfs_logo_publisher::exceptions()!=unwind||!logoBuildCurrent(g,*b))throw 1;auto&p=*b->proof;
   {nfs::GuestCopyTransaction tx;std::array<uint32_t,7>after{};logoDescriptor(tx,p.descriptor,after);
    if(tx.load<uint32_t>(dword_4D4E44)!=p.descriptor||after[4]!=b->staging||uint32_t(g.eax)!=p.descriptor)throw 1;
    for(unsigned k=0;k<7;++k)if(k!=4&&after[k]!=before[k])throw 1;
    if(std::memcmp(p.words.data(),nfs_host_address(b->staging,131072),131072))throw 1;
    p.address=after[5];if((p.address&3)||p.address>0x200000u-131072u)throw 1;p.descriptorWords=after;}
   // The original upload witness is still the most recent native upload here.
   // The queued const proof owns both original words and authored art until consumed.
   std::shared_ptr<const LogoProof>keep=b->proof;
   const auto accepted=nfs::enqueue_owned_renderer([keep]{NfsTrackBadgeUpload v{keep->serial,keep->art->view.generation,keep->epoch,keep->address,keep->words.data(),keep->art->view.images[0].pixels};return int32_t(nfs_private_track_badge_upload(&v));});
   if(accepted!=nfs::OwnedRendererAcceptance::Consumed)throw 1;
   b->uploaded=b->registered=true;
  }catch(...){logoBuildFailed(*b,"upload-return-bytes-descriptor-or-registration");}
 }
};
// After the builder returns, check only immutable proof/art and the LIVE descriptor.
// No archive bitmap or staging address is dereferenced here or in any tail method.
inline bool logoProofCurrent(Game&g,const std::shared_ptr<const LogoProof>&p){
 auto*s=current(g);if(!s||!p||s->logo!=p||s->logoCar!=p->car||p->game!=&g||p->thread!=threadToken()||
    p->epoch!=s->epoch||p->car>=12||!p->serial||p->words.size()!=65536||!p->art||!p->art->current())return false;
 nfs::GuestCopyTransaction tx;std::array<uint32_t,7>w{};logoDescriptor(tx,p->descriptor,w);
 return tx.load<uint32_t>(dword_4D4E44)==p->descriptor&&w==p->descriptorWords;
}
struct LogoTailScope {
 Game&g;State*s=nullptr;std::shared_ptr<const Frame>frame;std::shared_ptr<const LogoProof>proof;
 uint64_t request=0,parentRequest=0;bool active=false,bad=false,quadSent=false;int unwind=0;
 bool currentSource(){auto*now=current(g);if(!s||now!=s||s->logoTail!=this||s->capturing||s->presenting||s->string||s->word||
    s->published!=frame||s->lastNativeFrame!=frame||s->lastNativeRequest!=parentRequest||
    !frame||frame->data().epoch!=s->epoch||!frame->data().serial||!frame->data().artwork||
    !proof||frame->data().artwork->image()!=proof->car+1||!logoProofCurrent(g,proof))return false;
  const auto&d=frame->data();return nfs_logo_capture::same(d.target,target())&&d.original==plane(d.target);
 }
 explicit LogoTailScope(Game&v)noexcept:g(v){CpuErrno preserve(g);s=current(g);if(!s)return;
  if(s->logoTail){s->logoTail->bad=true;return;}
  try{frame=s->published;proof=s->logo;parentRequest=s->lastNativeRequest;
   if(!parentRequest||!frame||s->lastNativeFrame!=frame||s->logoMesh)return;
   // Install only our lexical pointer while validating this exact parent frame.
   s->logoTail=this;
   if(!currentSource())throw 1;request=nfs_background_scene_live::nextRequest();if(!request)throw 1;
   auto keep=proof;auto parent=frame;const uint64_t id=request,parentId=parentRequest,content=frame->data().serial;
   const auto accepted=nfs::enqueue_owned_renderer([keep,parent,id,parentId,content]{(void)parent;NfsVictoryLogoTail v{};
    v.schema=1;v.image=keep->car+1;v.address=keep->address;v.request=id;v.parentRequest=parentId;v.content=content;
    v.sourceSerial=keep->serial;v.sourceEpoch=keep->epoch;v.artGeneration=keep->art->view.generation;v.original=keep->words.data();
    return int32_t(nfs_private_victory_logo_begin(&v));});
   if(accepted!=nfs::OwnedRendererAcceptance::Consumed)throw 1;active=true;unwind=nfs_logo_publisher::exceptions();
  }catch(...){bad=true;if(s&&s->logoTail==this)s->logoTail=nullptr;logoTrace("tail-refused","parent-source-or-owned-queue",proof?proof->car:UINT32_MAX);}
 }
 ~LogoTailScope()noexcept{CpuErrno preserve(g);if(!active)return;
  bool valid=false;try{valid=!bad&&quadSent&&s==current(g)&&!s->logoMesh&&nfs_logo_publisher::exceptions()==unwind&&currentSource();}catch(...){valid=false;}
  if(s&&s->logoTail==this){s->logoTail=nullptr;s->logoMesh=false;}
  try{auto keep=proof;auto parent=frame;const uint64_t id=request;
   (void)nfs::enqueue_owned_renderer([keep,parent,id,valid]{(void)keep;(void)parent;return int32_t(nfs_private_victory_logo_end(id,valid?1:0));});
  }catch(...){}
 }
};
struct LogoMeshScope {
 Game&g;LogoTailScope*tail=nullptr;bool entered=false;
 explicit LogoMeshScope(Game&v)noexcept:g(v){CpuErrno preserve(g);auto*s=current(g);if(!s||!s->logoTail)return;tail=s->logoTail;
  try{if(!tail->active||tail->bad||tail->quadSent||s->logoMesh||!tail->currentSource())throw 1;
   {nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp),indices=uint32_t(guest_address(dword_4D4D0C));
    if(tx.load<uint32_t>(sp+4)!=1||tx.load<uint32_t>(sp+8)!=uint32_t(guest_address(flt_4D4C8C))||tx.load<uint32_t>(sp+12)!=indices)throw 1;
    for(unsigned k=0;k<4;++k)if(tx.load<uint32_t>(indices+4*k)!=k)throw 1;}
   s->logoMesh=true;entered=true;
  }catch(...){tail->bad=true;}
 }
 ~LogoMeshScope()noexcept{CpuErrno preserve(g);if(!entered)return;auto*s=current(g);if(s&&s->logoTail==tail)s->logoMesh=false;}
};
inline void logoQuad(Game&g)noexcept{CpuErrno preserve(g);auto*s=current(g);if(!s||!s->logoTail)return;auto*t=s->logoTail;
 try{if(!s->logoMesh||!t->active||t->bad||t->quadSent||!t->currentSource())throw 1;
  auto vertices=std::make_shared<std::array<float,60>>();
  {nfs::GuestCopyTransaction tx;const uint32_t sp=uint32_t(g.esp),a=tx.load<uint32_t>(sp),b=tx.load<uint32_t>(sp+4),c=tx.load<uint32_t>(sp+8);
   if(!a||uint64_t(a)+240>UINT64_C(0x100000000)||b!=a+60||c!=a+120)throw 1;
   std::memcpy(vertices->data(),nfs_host_address(a,240),240);}
  for(unsigned k=0;k<60;++k){const unsigned field=k%15;if((field==0||field==1||field==3||field==4||field==5||field==7||field==8||field==9||field==10||field==11)&&!std::isfinite((*vertices)[k]))throw 1;}
  auto keep=t->proof;auto parent=t->frame;const uint64_t id=t->request;
  const auto accepted=nfs::enqueue_owned_renderer([keep,parent,vertices,id]{(void)keep;(void)parent;return int32_t(nfs_private_victory_logo_quad(id,vertices->data()));});
  if(accepted!=nfs::OwnedRendererAcceptance::Consumed)throw 1;t->quadSent=true;
 }catch(...){t->bad=true;}
}
} // namespace nfs_victory_ui_live
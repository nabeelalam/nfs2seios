// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerPreviewGameHooks.inc"
#include "PlayerPhotoPrepare.h"
#include "PlayerPhotoAnimationAbi.h"
#include "PlayerGraphPhotoCapture.inc"
#include "PlayerGraphSourceProof.h"
#include "PlayerGraphArtwork.h"
#include "MainThreadBridge.h"
#include <array>
#include <cstdio>
#include <cstdlib>
namespace nfs_player_photo_live {
namespace live=nfs_player_scene_live;
struct Errno {int v=errno;~Errno(){errno=v;}};
struct Plane {uint32_t owner=0,bitmap=0,bits=0,w=0,h=0;std::vector<unsigned char>bytes;};
struct Tile {uint32_t descriptor=0,key=0,format=0;std::array<uint32_t,7>words{};std::array<uint16_t,1024>original{};std::vector<unsigned char>rgba;};
struct Proof {uintptr_t game=0,thread=0;uint64_t epoch=0,serial=0;uint32_t form=0,archive=0,car=0;bool graph=false;std::shared_ptr<const nfs_player_graph_source::Packet>graphSource;std::vector<unsigned char>source;std::shared_ptr<const nfs_player_photo::Art>art;std::array<Tile,100>tiles;};
struct Build {Game*g=nullptr;std::shared_ptr<Proof>p;Plane plane;std::array<Plane,2>graphPlanes;unsigned phase=0,uploads=0,stage=0;int x=0,y=0,photoX=0,photoY=0;uint32_t crop=0,converted=0;bool inside=false,resolved=false,cropped=false,convertedReady=false,bad=false;std::array<uint16_t,1024>convertedWords{};};
inline Build*&building(){static thread_local Build*p=nullptr;return p;}
inline std::shared_ptr<const Proof>&published(){static thread_local std::shared_ptr<const Proof>p;return p;}
inline uint64_t&serial(){static thread_local uint64_t n=0;return n;}
inline bool enabled(){return nfs_player_photo_art_generation()!=0;}
// Original loc_40C8F8 tests the previous cached car. With graph ON, the
// first standard-to-bonus change otherwise skips the existing photo builder.
// Admit only that stale-standard-cache case at the exact original branch;
// use the original 40C57B/40D1B0 path, allocations, uploads and draw proofs.
inline bool needsBonusPhotoChange(Game&g)noexcept{
 Errno e;if(!enabled())return false;
 try{const auto form=uint32_t(g.esi);uint32_t selected=0;
  if(form==uint32_t(guest_address(off_4D3050)))selected=uint32_t(guest_address(byte_512DBF));
  else if(form==uint32_t(guest_address(off_4D3100)))selected=uint32_t(guest_address(byte_512DDA));else return false;
  nfs::GuestCopyTransaction tx;const auto car=tx.load<uint8_t>(selected);const auto cached=tx.load<int16_t>(dword_4E5928);
  return tx.load<uint8_t>(byte_512DD1)==1&&car>=NFS_PLAYER_PHOTO_STANDARD_COUNT&&car<NFS_PLAYER_PHOTO_SOURCE_COUNT&&cached>=0&&cached<NFS_PLAYER_PHOTO_STANDARD_COUNT;
 }catch(...){return false;}
}
inline bool contextCurrent(Game&g,const Proof&p){
 auto*l=live::current(g);return l&&!l->invalid&&p.car<NFS_PLAYER_PHOTO_SOURCE_COUNT&&p.game==reinterpret_cast<uintptr_t>(&g)&&p.thread==nfs_logo_publisher::threadToken()&&l->ownerEpoch==p.epoch&&l->form==p.form&&p.art&&p.art->current();
}
// Only caller-owned read leases may enter this helper. It does not acquire a
// second lease or validate the original temporarily restored selected byte.
inline bool graphSourceCurrent(nfs::GuestCopyTransaction&tx,const Proof&p){
 if(!p.graph||!p.graphSource||p.car>=NFS_PLAYER_PHOTO_STANDARD_COUNT||tx.load<uint8_t>(byte_512DD1)!=1)return false;
 const auto&d=p.graphSource->domain();const auto&raw=p.graphSource->sources();
 const uint32_t formGuest=p.form==nfs_player_preview::Player1Form?uint32_t(guest_address(off_4D3050)):uint32_t(guest_address(off_4D3100));
 if(d.game!=p.game||d.thread!=p.thread||d.epoch!=p.epoch||d.form!=p.form||d.formGuest!=formGuest||d.archive!=p.archive||d.car!=p.car||tx.load<uint32_t>(dword_4D40B8)!=p.archive)return false;
 const uint32_t arrays[]={uint32_t(guest_address(dword_4D236C)),uint32_t(guest_address(dword_4D2330)),uint32_t(guest_address(dword_4D23E4)),uint32_t(guest_address(dword_4D23A8))};
 for(unsigned k=0;k<4;++k)if(tx.load<uint32_t>(arrays[k]+4*p.car)!=d.statistics[k])return false;
 for(unsigned k=0;k<5;++k){const auto&id=nfs_player_graph_source::catalog[k];if(uint64_t(p.archive)+id.offset+id.span>UINT64_C(0x100000000)||raw[k].size()!=id.span||std::memcmp(raw[k].data(),nfs_host_address(p.archive+id.offset,id.span),id.span))return false;}
 return true;
}
inline bool current(Game&g,const Proof&p){
 if(!contextCurrent(g,p))return false;
 try{nfs::GuestCopyTransaction tx;
  if(tx.load<uint32_t>(dword_4D40B8)!=p.archive||tx.load<uint8_t>(p.form==nfs_player_preview::Player1Form?byte_512DBF:byte_512DDA)!=p.car)return false;
  if(p.graph)return graphSourceCurrent(tx,p);
  if(p.car<NFS_PLAYER_PHOTO_STANDARD_COUNT&&tx.load<uint8_t>(byte_512DD1))return false;
  const auto&source=nfs_player_photo_sources[p.car];if(uint64_t(p.archive)+source.offset+source.span>UINT64_C(0x100000000))return false;
  return p.source.size()==source.span&&!std::memcmp(p.source.data(),nfs_host_address(p.archive+source.offset,source.span),source.span);
 }catch(...){return false;}
}
// Pending graph observations can only feed the original crop/upload collector.
// They cannot be published, drawn or borrowed as a qualified photo receipt.
inline bool buildingCurrent(Game&g,const Proof&p){
 if(!p.graph)return current(g,p);
 if(!contextCurrent(g,p)||p.car>=NFS_PLAYER_PHOTO_STANDARD_COUNT)return false;
 try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(dword_4D40B8)==p.archive&&tx.load<uint8_t>(byte_512DD1)==1&&tx.load<uint8_t>(p.form==nfs_player_preview::Player1Form?byte_512DBF:byte_512DDA)==p.car;}catch(...){return false;}
}
inline void graphTiles(Proof&p){
 const auto rgba=nfs_player_graph_art::canvas(*p.graphSource);if(rgba.size()!=size_t(1536)*640*4)throw 1;
 for(unsigned k=0;k<100;++k){auto&t=p.tiles[k];const unsigned tile=k%50,col=tile/5,row=tile%5;t.rgba.resize(128*128*4);
  for(unsigned y=0;y<128;++y)for(unsigned x=0;x<128;++x){const unsigned X=col*128+x,Y=row*128+y;auto*out=t.rgba.data()+4*(y*128+x);if(X<1536&&Y<640)std::memcpy(out,rgba.data()+4*(Y*1536+X),4);else{const unsigned w=t.original[(y/4)*32+x/4];if(t.format==11){out[0]=((w>>10)&31)*255/31;out[1]=((w>>5)&31)*255/31;out[2]=(w&31)*255/31;out[3]=(w>>15)*255;}else{out[0]=((w>>8)&15)*17;out[1]=((w>>4)&15)*17;out[2]=(w&15)*17;out[3]=(w>>12)*17;}}}
 }
}
inline Plane plane(Game&g){nfs::GuestCopyTransaction tx;Plane p;p.owner=tx.load<uint32_t>(uint32_t(g.esp)+4);if(!p.owner)throw 1;const auto globals=uint32_t(guest_address(dword_4DB240));const uint32_t magic=tx.load<uint32_t>(globals);p.w=tx.load<uint32_t>(globals+4);p.h=tx.load<uint32_t>(globals+8);p.bits=tx.load<uint32_t>(globals+28)&255;p.bitmap=tx.load<uint32_t>(globals+32);if(magic!=0x57494e44||p.w!=384||p.h!=160||(p.bits!=16&&p.bits!=32)||tx.load<uint32_t>(p.owner+32)!=p.bitmap||tx.load<uint32_t>(globals+40)!=p.w*(p.bits/8))throw 1;for(unsigned k=0;k<4;++k)if(tx.load<uint32_t>(globals+12+4*k)!=(k<2?0:(k==2?p.w:p.h)))throw 1;for(unsigned k:{0u,1u,2u,7u,8u,10u})if(tx.load<uint32_t>(p.owner+4*k)!=tx.load<uint32_t>(globals+4*k))throw 1;const auto wh=tx.load<uint32_t>(p.bitmap+4);if(wh!=(p.w|(p.h<<16)))throw 1;size_t n=size_t(p.w)*p.h*(p.bits/8);p.bytes.resize(n);std::memcpy(p.bytes.data(),nfs_host_address(p.bitmap+16,n),n);return p;}
struct BuildScope {
 Game&g;Build b;bool entered=false;int unwind=0;
 explicit BuildScope(Game&v)noexcept:g(v){
  Errno e;published().reset();if(!enabled()||building())return;auto*l=live::current(g);
  if(!l||l->invalid||(uint32_t(g.eax)!=uint32_t(guest_address(off_4D3050))&&uint32_t(g.eax)!=uint32_t(guest_address(off_4D3100))))return;
  try{
   unsigned car=0,graph=0;uint32_t archive=0;{nfs::GuestCopyTransaction tx;car=tx.load<uint8_t>(l->form==nfs_player_preview::Player1Form?byte_512DBF:byte_512DDA);graph=tx.load<uint8_t>(byte_512DD1);archive=tx.load<uint32_t>(dword_4D40B8);}
   if(car>=NFS_PLAYER_PHOTO_SOURCE_COUNT||serial()==UINT64_MAX)return;
   const bool graphSource=car<NFS_PLAYER_PHOTO_STANDARD_COUNT&&graph==1;
   if(car<NFS_PLAYER_PHOTO_STANDARD_COUNT&&graph&&!graphSource)return;
   auto p=std::make_shared<Proof>();p->game=reinterpret_cast<uintptr_t>(&g);p->thread=nfs_logo_publisher::threadToken();p->epoch=l->ownerEpoch;p->form=l->form;p->archive=archive;p->car=car;p->graph=graphSource;p->serial=++serial();p->art=nfs_player_photo::Art::acquire(car);if(!p->art)return;
   if(!graphSource){const auto&source=nfs_player_photo_sources[car];if(uint64_t(archive)+source.offset+source.span>UINT64_C(0x100000000))return;p->source.resize(source.span);{nfs::GuestCopyTransaction tx;std::memcpy(p->source.data(),nfs_host_address(archive+source.offset,source.span),source.span);}unsigned char sha[32];if(!nfs_texture_sha256(p->source.data(),p->source.size(),sha)||std::memcmp(sha,source.sha,32))return;}
   if(!buildingCurrent(g,*p))return;
   b.g=&g;b.p=p;building()=&b;entered=true;unwind=nfs_logo_publisher::exceptions();
  }catch(...){}
 }
 ~BuildScope()noexcept{
  Errno e;if(!entered)return;building()=nullptr;
  struct CaptureCleanup{bool graph;~CaptureCleanup(){if(graph&&!nfs_player_graph_capture::privateEnabled())nfs_player_graph_capture::captured().reset();}}cleanup{b.p->graph};
  try{
   if(b.bad||b.inside||b.phase!=2||b.uploads!=100||nfs_logo_publisher::exceptions()>unwind)throw 1;
   if(b.p->graph){
    const auto observed=nfs_player_graph_capture::captured();if(!observed)throw 1;
     nfs_player_graph_source::Failure qualificationFailure;auto source=nfs_player_graph_source::verify(*observed,&qualificationFailure);if(!source){if(nfs_player_graph_capture::privateEnabled())std::fprintf(stderr,"GRAPH SOURCE qualification refused reason=%u\n",unsigned(qualificationFailure));throw 1;}const auto&d=source->domain();
    if(d.game!=b.p->game||d.thread!=b.p->thread||d.epoch!=b.p->epoch||d.form!=b.p->form||d.archive!=b.p->archive||d.car!=b.p->car)throw 1;
    for(unsigned phase=0;phase<2;++phase){const auto&actual=b.graphPlanes[phase];const auto&verified=source->canvases()[phase];if(actual.bits!=verified.bits||actual.w!=384||actual.h!=160||actual.bitmap!=verified.words[8]||actual.bytes!=verified.pixels)throw 1;}
    b.p->graphSource=std::move(source);if(!current(g,*b.p))throw 1;graphTiles(*b.p);
   }
   if(!current(g,*b.p))throw 1;
   auto p=b.p;auto receipt=nfs::enqueue_owned_renderer([p]{std::array<NfsPlayerPhotoTile,100>t{};for(unsigned k=0;k<100;++k){t[k].address=p->tiles[k].key;t[k].format=p->tiles[k].format;t[k].original=p->tiles[k].original.data();t[k].rgba=p->tiles[k].rgba.data();}NfsPlayerPhotoAnimation v{p->serial,p->art->view.generation,100,t.data()};return int32_t(nfs_private_player_photo_upload(&v));});
   if(receipt==nfs::OwnedRendererAcceptance::Consumed&&current(g,*p)){published()=p;static thread_local unsigned lines=0;if(std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE")&&lines++<128)std::fprintf(stderr,"PLAYER PHOTO source published serial=%llu car=%u layers=2 tiles=100 graph=%u\n",(unsigned long long)p->serial,p->car,unsigned(p->graph));}
  }catch(...){static thread_local unsigned lines=0;if(std::getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE")&&lines++<128)std::fprintf(stderr,"PLAYER PHOTO source refused phase=%u uploads=%u invalid=%u stage=%u graph=%u\n",b.phase,b.uploads,unsigned(b.bad),b.stage,unsigned(b.p->graph));}
 }
};
struct PhotoScope {Game&g;Build*b=nullptr;int unwind=0;explicit PhotoScope(Game&v)noexcept:g(v){Errno e;b=building();if(!b)return;if(b->bad||b->inside||b->phase>=2||b->uploads!=b->phase*50){b->bad=true;b=nullptr;return;}b->stage=1;b->inside=true;b->resolved=false;b->cropped=b->convertedReady=false;unwind=nfs_logo_publisher::exceptions();}
 ~PhotoScope()noexcept{Errno e;if(!b)return;try{if((!b->p->graph&&!b->resolved)||nfs_logo_publisher::exceptions()>unwind||!buildingCurrent(g,*b->p))throw 1;b->stage=3;b->plane=plane(g);if(b->plane.bits!=(b->phase?16:32))throw 1;
 if(b->p->graph){auto*c=nfs_player_graph_capture::active();if(!c||c->bad||c->g!=&g||c->inPhoto||c->photos!=b->phase+1||!c->capture)throw 1;const auto&captured=c->capture->photos[b->phase].after;if(captured.bits!=b->plane.bits||captured.words[8]!=b->plane.bitmap||captured.pixels!=b->plane.bytes)throw 1;b->graphPlanes[b->phase]=b->plane;}
 ++b->phase;}catch(...){b->bad=true;}b->inside=false;}
};
inline void resolved(Game&g)noexcept{
 Errno e;auto*b=building();if(!b||b->bad||!b->inside||b->p->graph)return;
 try{
  auto&p=*b->p;auto&s=nfs_player_photo_sources[p.car];b->stage=2;
  {nfs::GuestCopyTransaction tx;
   if(b->resolved||uint32_t(g.eax)!=p.archive+s.offset||uint32_t(g.edx)!=0x3000||tx.load<uint16_t>(dword_4E5928)!=p.car)throw 1;
  }
  // current() takes its own checked read lease; never nest that mutex.
  if(!current(g,p))throw 1;
  b->photoX=int32_t(g.ebx);b->photoY=int32_t(g.ecx);if(b->photoX!=0||b->photoY!=0)throw 1;b->resolved=true;
 }catch(...){b->bad=true;}
}
struct CropScope {Game&g;Build*b=nullptr;uint32_t bitmap=0;int x=0,y=0;explicit CropScope(Game&v)noexcept:g(v){Errno e;b=building();if(!b||b->bad||b->inside||!b->phase){b=nullptr;return;}b->stage=4;bitmap=uint32_t(g.eax);x=int32_t(g.edx);y=int32_t(g.ebx);b->cropped=b->convertedReady=false;}
 ~CropScope()noexcept{Errno e;if(!b)return;try{const auto&q=b->plane;if(x<0||y<0||x+32>int(q.w)||y+32>int(q.h)||b->uploads!=(b->phase-1)*50+unsigned(x/32)*5+unsigned(y/32)||x%32||y%32||!buildingCurrent(g,*b->p))throw 1;nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(bitmap+4)!=(32u|(32u<<16)))throw 1;const auto pos=tx.load<uint32_t>(bitmap+12);if((int32_t(pos<<20)>>20)!=x||(int32_t(pos<<4)>>20)!=y)throw 1;const unsigned bytes=q.bits/8;auto*src=static_cast<const unsigned char*>(nfs_host_address(bitmap+16,32*32*bytes));for(unsigned row=0;row<32;++row)if(std::memcmp(src+row*32*bytes,q.bytes.data()+((y+row)*q.w+x)*bytes,32*bytes))throw 1;b->crop=bitmap;b->x=x;b->y=y;b->cropped=true;}catch(...){b->bad=true;}}
};
struct ConvertScope {Game&g;Build*b=nullptr;uint32_t input=0;bool allocated=false;explicit ConvertScope(Game&v,bool allocation)noexcept:g(v),allocated(allocation){Errno e;b=building();if(!b||b->bad||!b->cropped){b=nullptr;return;}b->stage=5;input=uint32_t(g.eax);if(input!=b->crop||allocated!=(b->phase==1)){b->bad=true;b=nullptr;}}
 ~ConvertScope()noexcept{Errno e;if(!b)return;try{nfs::GuestCopyTransaction tx;uint32_t out=allocated?uint32_t(g.eax):input;if(tx.load<uint32_t>(out+4)!=(32u|(32u<<16)))throw 1;b->converted=out+16;std::memcpy(b->convertedWords.data(),nfs_host_address(b->converted,2048),2048);b->convertedReady=true;}catch(...){b->bad=true;}}
};
struct UploadScope {Game&g;Build*b=nullptr;Tile tile;uint32_t payload=0;int unwind=0;explicit UploadScope(Game&v)noexcept:g(v){Errno e;b=building();if(!b||b->bad||!b->convertedReady||b->uploads>=100){b=nullptr;return;}try{b->stage=6;nfs::GuestCopyTransaction tx;uint32_t sp=uint32_t(g.esp);tile.descriptor=tx.load<uint32_t>(sp);payload=tx.load<uint32_t>(sp+4);if(tx.load<uint32_t>(sp+8)||payload!=b->converted)throw 1;uint32_t table=uint32_t(guest_address(b->phase==1?dword_4EB5BC:dword_4EB684));if(tile.descriptor!=tx.load<uint32_t>(table+4*(b->uploads%50)))throw 1;nfs_logo_publisher::descriptor(tx,tile.descriptor,tile.words);if(tile.words[0]!=3||tile.words[1]!=3||tile.words[2]!=3||(tile.words[3]!=11&&tile.words[3]!=12)||tile.words[6]!=3)throw 1;tile.original=b->convertedWords;if(std::memcmp(tile.original.data(),nfs_host_address(payload,2048),2048))throw 1;tile.format=tile.words[3];unwind=nfs_logo_publisher::exceptions();}catch(...){b->bad=true;b=nullptr;}}
 ~UploadScope()noexcept{Errno e;if(!b)return;try{nfs::GuestCopyTransaction tx;std::array<uint32_t,7>after{};nfs_logo_publisher::descriptor(tx,tile.descriptor,after);for(unsigned k=0;k<7;++k)if(k!=4&&after[k]!=tile.words[k])throw 1;if(after[4]!=payload||std::memcmp(tile.original.data(),nfs_host_address(payload,2048),2048)||nfs_logo_publisher::exceptions()>unwind)throw 1;tile.key=after[5];for(unsigned k=0;k<b->uploads;++k)if(tile.key==b->p->tiles[k].key)throw 1;tile.words=after;if(!b->p->graph){tile.rgba.resize(128*128*4);const auto&art=b->p->art->view.images[0];for(unsigned y=0;y<128;++y)for(unsigned x=0;x<128;++x){int X=b->x*4+x,Y=b->y*4+y;auto*d=tile.rgba.data()+4*(y*128+x);if(X>=0&&Y>=0&&X<int(art.width)&&Y<int(art.height))std::memcpy(d,art.pixels+4*(Y*art.width+X),4);else{unsigned w=tile.original[(y/4)*32+x/4];if(tile.format==11){d[0]=((w>>10)&31)*255/31;d[1]=((w>>5)&31)*255/31;d[2]=(w&31)*255/31;d[3]=(w>>15)*255;}else{d[0]=((w>>8)&15)*17;d[1]=((w>>4)&15)*17;d[2]=(w&15)*17;d[3]=(w>>12)*17;}}}}b->p->tiles[b->uploads++]=std::move(tile);b->convertedReady=b->cropped=false;}catch(...){b->bad=true;}}
};
struct DrawScope {
 Game&g;std::shared_ptr<const Proof>p;bool accepted=false;
 explicit DrawScope(Game&v)noexcept:g(v){
  Errno e;p=published();if(!p||!current(g,*p))return;
  try{
   {nfs::GuestCopyTransaction tx;
    for(unsigned k=0;k<100;++k){uint32_t table=uint32_t(guest_address(k<50?dword_4EB5BC:dword_4EB684));if(tx.load<uint32_t>(table+4*(k%50))!=p->tiles[k].descriptor)throw 1;
     std::array<uint32_t,7>w{};nfs_logo_publisher::descriptor(tx,p->tiles[k].descriptor,w);if(w[5]!=p->tiles[k].key)throw 1;for(unsigned j=0;j<7;++j)if(j!=4&&w[j]!=p->tiles[k].words[j])throw 1;
    }
   }
   if(!current(g,*p))return;
   auto keep=p;int x=int32_t(g.eax),y=int32_t(g.edx);if(x< -1024||x>1024||y!=316)return;
   accepted=nfs::enqueue_owned_renderer([keep,x,y]{return int32_t(nfs_private_player_photo_begin(keep->serial,x,y));})==nfs::OwnedRendererAcceptance::Consumed;
  }catch(...){}
 }
 ~DrawScope()noexcept{Errno e;if(!accepted)return;try{auto keep=p;bool okay=current(g,*p);nfs::enqueue_owned_renderer([keep,okay]{return int32_t(nfs_private_player_photo_end(okay?keep->serial:0));});}catch(...){}}
};
}

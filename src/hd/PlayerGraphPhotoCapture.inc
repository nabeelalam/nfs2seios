// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerPreviewGameHooks.inc"
#include "PlayerPhotoArtProvider.h"
#include "PlayerGraphCanvasReplay.h"
#include <array>
#include <memory>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cstdlib>

// Private observations only. Nothing here verifies replay, uploads textures,
// creates a rendering receipt, or grants access to a retained guest address.
namespace nfs_player_graph_capture {
constexpr unsigned Width=384,Height=160,MaxDraws=5,MaxRows=160;
constexpr size_t MaxSource=16+640*480+3+16+256*4;
struct Errno {int saved=errno;~Errno(){errno=saved;}};
struct Canvas {
 std::array<uint32_t,14>words{};std::array<uint32_t,4>header{};
 std::array<uint32_t,Width>x{};std::array<uint32_t,Height>y{};
 unsigned bits=0;std::vector<uint8_t>pixels;
};
struct Row {
 std::array<uint32_t,4>args{};uint32_t callback=0,format=0,converter=0,forward=0,pattern=0,palette=0;
 std::array<uint32_t,256>paletteBefore{},paletteAfter{};
 bool sourceSpanKnown=false,indexedPalette=false,wrappedIndexed32=false;
 std::vector<uint8_t>sourceBefore,sourceAfter,before,after;
};
struct Draw {
 std::array<uint8_t,4>name{};std::array<uint32_t,4>args{};
 std::array<uint32_t,9>tableBefore{},tableAfter{};
 std::vector<uint8_t>sourceBefore,sourceAfter;std::vector<Row>rows;
};
struct Photo {
 Canvas before,after;uint32_t rawColor=0,packedColor=0,fillColor=0;bool colorObserved=false,fillObserved=false;
 std::array<uint32_t,4>fillArgs{};std::array<uint32_t,9>fillTableBefore{},fillTableAfter{};
 std::vector<Row>fillRows;std::vector<Draw>draws;
 nfs_player_preview::Selection consumed{};bool consumedObserved=false;
};
struct Capture {
 uintptr_t game=0,thread=0;uint64_t epoch=0;uint32_t form=0,formGuest=0,archive=0,car=0;
 std::array<Photo,2>photos;
};
struct State {
 Game*g=nullptr;nfs_player_scene_live::State*owner=nullptr;std::shared_ptr<Capture>capture;
 unsigned photos=0,resolves=0;bool bad=false,inPhoto=false,inName=false,inFill=false,inRow=false,inColor=false;
 Draw*draw=nullptr;std::array<uint8_t,4>name{};
};
inline State*&active()noexcept{static thread_local State*p=nullptr;return p;}
inline std::shared_ptr<const Capture>&captured()noexcept{static thread_local std::shared_ptr<const Capture>p;return p;}
inline bool privateEnabled()noexcept{const char*p=std::getenv("NFS_PRIVATE_GRAPH_SOURCE_CAPTURE");return p&&p[0]=='1'&&!p[1];}
inline bool enabled()noexcept{return privateEnabled()||nfs_player_photo_art_generation()!=0;}
inline void fail(State*s)noexcept{if(s)s->bad=true;}
inline bool span(uint32_t p,size_t n)noexcept{return p&&n&&uint64_t(p)+n<=UINT64_C(0x100000000);}
inline void bytes(uint32_t p,std::vector<uint8_t>&v){if(!span(p,v.size()))throw 1;nfs::GuestCopyTransaction tx;std::memcpy(v.data(),nfs_host_address(p,v.size()),v.size());}
inline void table(std::array<uint32_t,9>&out,uint32_t at=uint32_t(guest_address(off_4DAD88))){nfs::GuestCopyTransaction tx;for(unsigned k=0;k<9;++k)out[k]=tx.load<uint32_t>(at+4*k);}
inline void metadata(nfs::GuestCopyTransaction&tx,Canvas&out){
 const uint32_t at=uint32_t(guest_address(dword_4DB240));for(unsigned k=0;k<14;++k)out.words[k]=tx.load<uint32_t>(at+4*k);
 out.bits=out.words[7]&255;if(out.words[0]!=0x57494e44||out.words[1]!=Width||out.words[2]!=Height||(out.bits!=16&&out.bits!=32)||out.words[3]||out.words[4]||out.words[5]!=Width||out.words[6]!=Height||out.words[10]!=Width*(out.bits/8)||!span(out.words[8],16+Width*Height*(out.bits/8))||!span(out.words[12],Width*4)||!span(out.words[11],Height*4))throw 1;
 for(unsigned k=0;k<4;++k)out.header[k]=tx.load<uint32_t>(out.words[8]+4*k);
 if(out.header[1]!=(Width|(Height<<16)))throw 1;
 for(unsigned k=0;k<Width;++k)out.x[k]=tx.load<uint32_t>(out.words[12]+4*k);
 for(unsigned k=0;k<Height;++k)out.y[k]=tx.load<uint32_t>(out.words[11]+4*k);
}
inline bool same(const Canvas&a,const Canvas&b)noexcept{return a.words==b.words&&a.header==b.header&&a.x==b.x&&a.y==b.y&&a.bits==b.bits;}
inline void plane(Canvas&out){
 {nfs::GuestCopyTransaction tx;metadata(tx,out);}
 out.pixels.resize(Width*Height*(out.bits/8)); // Never allocate under a guest lease.
 Canvas after;{nfs::GuestCopyTransaction tx;metadata(tx,after);if(!same(out,after))throw 1;std::memcpy(out.pixels.data(),nfs_host_address(out.words[8]+16,out.pixels.size()),out.pixels.size());metadata(tx,after);if(!same(out,after))throw 1;}
}
inline bool bound(State&s)noexcept{
 auto*l=nfs_player_scene_live::current(*s.g);const auto&p=*s.capture;
 if(s.bad||l!=s.owner||!l||l->invalid||!l->on||l->ownerEpoch!=p.epoch||l->form!=p.form||l->formGuest!=p.formGuest||l->game!=p.game||l->thread!=p.thread||l->archive!=p.archive)return false;
 try{nfs::GuestCopyTransaction tx;return tx.load<uint32_t>(dword_4D40B8)==p.archive&&tx.load<uint8_t>(byte_512DD1)==1&&tx.load<uint8_t>(p.form==0x4d3050?byte_512DBF:byte_512DDA)==p.car;}catch(...){return false;}
}
inline State*current(Game&g)noexcept{auto*s=active();if(!s||s->g!=&g||!s->inPhoto||s->bad)return nullptr;if(!bound(*s)){fail(s);return nullptr;}return s;}
inline Photo&photo(State&s){return s.capture->photos[s.photos];}
inline void selection(State&s){nfs_player_preview::Selection value;if(!nfs_player_preview_live::selection(*s.owner,value)||value.selectedCar!=s.capture->car||value.cachedCarIndex!=value.selectedCar||value.graphEnabled!=1)throw 1;auto&p=photo(s);if(p.consumedObserved&&!(p.consumed==value))throw 1;p.consumed=value;p.consumedObserved=true;}
inline void report(const Capture&c)noexcept{
 if(!privateEnabled())return;
 static thread_local unsigned reports=0;if(reports++>=8)return;
 for(unsigned k=0;k<2;++k){
  const auto&p=c.photos[k];unsigned char sha[32]{};
  if(!nfs_texture_sha256(p.after.pixels.data(),p.after.pixels.size(),sha))return;
  const auto*f=p.fillRows.empty()?nullptr:&p.fillRows.front();
  std::fprintf(stderr,"GRAPH SOURCE CAPTURE observation-only form=%x car=%u phase=%u bits=%u fillRows=%zu callback=%x format=%u converter=%x draws=%zu sha=",c.form,c.car,k,p.after.bits,p.fillRows.size(),f?f->callback:0,f?f->format:0,f?f->converter:0,p.draws.size());
  for(auto v:sha)std::fprintf(stderr,"%02x",v);std::fprintf(stderr," fillTable=");
  for(auto v:p.fillTableBefore)std::fprintf(stderr,"%x,",v);std::fprintf(stderr,"\n");
  for(const auto&d:p.draws){
   const auto*r=d.rows.empty()?nullptr:&d.rows.front();
   std::fprintf(stderr,"GRAPH SOURCE CAPTURE asset=%.4s rows=%zu callback=%x format=%u converter=%x forward=%x sourceSpanKnown=%u table=",reinterpret_cast<const char*>(d.name.data()),d.rows.size(),r?r->callback:0,r?r->format:0,r?r->converter:0,r?r->forward:0,r?unsigned(r->sourceSpanKnown):0);
   for(auto v:d.tableBefore)std::fprintf(stderr,"%x,",v);std::fprintf(stderr," after=");for(auto v:d.tableAfter)std::fprintf(stderr,"%x,",v);std::fprintf(stderr,"\n");
  }
 }
}
struct BuildScope {
 State state;bool entered=false;int unwind=0;
 explicit BuildScope(Game&g)noexcept{Errno e;if(!enabled())return;if(active()){fail(active());return;}captured().reset();auto*l=nfs_player_scene_live::current(g);if(!l||l->invalid||!l->on||!l->ownerEpoch||l->mainDepth!=1||(l->form!=0x4d3050&&l->form!=0x4d3100)||uint32_t(g.eax)!=l->formGuest)return;
  try{auto p=std::make_shared<Capture>();p->game=reinterpret_cast<uintptr_t>(&g);p->thread=nfs_logo_publisher::threadToken();p->epoch=l->ownerEpoch;p->form=l->form;p->formGuest=l->formGuest;{nfs::GuestCopyTransaction tx;p->archive=tx.load<uint32_t>(dword_4D40B8);p->car=tx.load<uint8_t>(l->form==0x4d3050?byte_512DBF:byte_512DDA);if(!p->archive||p->archive!=l->archive||p->car>=12||tx.load<uint8_t>(byte_512DD1)!=1)return;}state.g=&g;state.owner=l;state.capture=std::move(p);if(!bound(state))return;active()=&state;entered=true;unwind=nfs_logo_publisher::exceptions();}catch(...){}}
 ~BuildScope()noexcept{Errno e;if(!entered)return;active()=nullptr;if(state.bad||state.inPhoto||state.inName||state.inRow||state.inColor||state.photos!=2||nfs_logo_publisher::exceptions()>unwind||!bound(state)){static thread_local unsigned refusals=0;if(privateEnabled()&&refusals++<16)std::fprintf(stderr,"GRAPH SOURCE CAPTURE incomplete photos=%u bad=%u inPhoto=%u inName=%u inRow=%u\n",state.photos,unsigned(state.bad),unsigned(state.inPhoto),unsigned(state.inName),unsigned(state.inRow));return;}captured()=state.capture;if(!privateEnabled())return;report(*state.capture);
 const auto replay=nfs_player_graph_replay::replay(*state.capture);
 bool embedded32=true;unsigned palettes=0;
 for(const auto&d:state.capture->photos[0].draws){if(d.sourceBefore.size()<16){embedded32=false;break;}const uint32_t offset=nfs_player_graph_replay::le(d.sourceBefore.data(),4)>>8;if(uint64_t(offset)+16>d.sourceBefore.size()){embedded32=false;break;}const unsigned count=nfs_player_graph_replay::le(d.sourceBefore.data()+offset+4,2);if(!count||count>256||uint64_t(offset)+16+4*count>d.sourceBefore.size()){embedded32=false;break;}for(const auto&r:d.rows){if(!r.sourceSpanKnown){embedded32=false;continue;}++palettes;for(unsigned k=0;k<count;++k)if(r.paletteBefore[k]!=nfs_player_graph_replay::le(d.sourceBefore.data()+offset+16+4*k,4))embedded32=false;}}
 std::fprintf(stderr,"GRAPH CANVAS REPLAY observation-only matched=%u failure=%u phase=%u draw=%u row=%u embedded32PaletteEqual=%u paletteRows=%u sourceAuthenticated=%u tableTailsAuthenticated=%u\n",unsigned(replay.matched),unsigned(replay.failure),replay.phase,replay.draw,replay.row,unsigned(embedded32),palettes,unsigned(replay.sourceAuthenticated),unsigned(replay.dispatchTableTailsAuthenticated));}
};
struct PhotoScope {
 State*s=nullptr;int unwind=0;
 explicit PhotoScope(Game&g)noexcept{Errno e;s=active();if(!s||s->g!=&g)return;if(s->bad||s->inPhoto||s->photos>=2||!bound(*s)){fail(s);s=nullptr;return;}try{auto&p=photo(*s);p.draws.reserve(MaxDraws);p.fillRows.reserve(MaxRows);plane(p.before);if(p.before.bits!=(s->photos?16u:32u))throw 1;s->inPhoto=true;unwind=nfs_logo_publisher::exceptions();}catch(...){fail(s);s=nullptr;}}
 ~PhotoScope()noexcept{Errno e;if(!s)return;try{auto&p=photo(*s);if(s->bad||s->inName||s->inFill||s->inRow||s->inColor||s->draw||nfs_logo_publisher::exceptions()>unwind||!bound(*s)||!p.colorObserved||!p.fillObserved||!p.consumedObserved||p.draws.size()!=5)throw 1;selection(*s);plane(p.after);if(!same(p.before,p.after))throw 1;++s->photos;}catch(...){fail(s);}s->inPhoto=false;}
};
struct ColorScope {
 Game&g;State*s=nullptr;int unwind=0;
 explicit ColorScope(Game&v)noexcept:g(v){Errno e;s=current(g);if(!s)return;auto&p=photo(*s);if(p.colorObserved||s->inColor||s->inFill||s->inName||uint32_t(g.eax)!=0xff000000u){fail(s);s=nullptr;return;}p.rawColor=uint32_t(g.eax);s->inColor=true;unwind=nfs_logo_publisher::exceptions();}
 ~ColorScope()noexcept{Errno e;if(!s)return;s->inColor=false;if(nfs_logo_publisher::exceptions()>unwind||!bound(*s)){fail(s);return;}photo(*s).packedColor=uint32_t(g.eax);photo(*s).colorObserved=true;}
};
struct FillScope {
 State*s=nullptr;int unwind=0;
 explicit FillScope(Game&g)noexcept{Errno e;s=current(g);if(!s)return;auto&p=photo(*s);if(!p.colorObserved||p.fillObserved||s->inFill||s->inName||s->inRow){fail(s);s=nullptr;return;}try{p.fillArgs={{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};if(p.fillArgs!=std::array<uint32_t,4>{{0,0,280,128}})throw 1;{nfs::GuestCopyTransaction tx;p.fillColor=tx.load<uint32_t>(uint32_t(g.esp));}table(p.fillTableBefore,uint32_t(guest_address(off_4DB648)));s->inFill=true;unwind=nfs_logo_publisher::exceptions();}catch(...){fail(s);s=nullptr;}}
 ~FillScope()noexcept{Errno e;if(!s)return;try{if(s->inRow||nfs_logo_publisher::exceptions()>unwind||!bound(*s)||photo(*s).fillRows.size()!=128)throw 1;table(photo(*s).fillTableAfter,uint32_t(guest_address(off_4DB648)));photo(*s).fillObserved=true;}catch(...){fail(s);}s->inFill=false;}
};
struct NameScope {
 State*s=nullptr;int unwind=0;
 explicit NameScope(Game&g)noexcept{Errno e;s=current(g);if(!s)return;if(s->inName||s->draw||s->inFill||!photo(*s).fillObserved||photo(*s).draws.size()>=5){fail(s);s=nullptr;return;}try{selection(*s);if(!span(uint32_t(g.eax),4))throw 1;{nfs::GuestCopyTransaction tx;for(unsigned k=0;k<4;++k)s->name[k]=tx.load<uint8_t>(uint32_t(g.eax)+k);}static const char names[5][5]={"grph","grpg","grpb","grpr","grpy"};if(std::memcmp(s->name.data(),names[photo(*s).draws.size()],4)||uint32_t(g.edx)!=0x3000)throw 1;s->inName=true;s->resolves=0;unwind=nfs_logo_publisher::exceptions();}catch(...){fail(s);s=nullptr;}}
 ~NameScope()noexcept{Errno e;if(!s)return;try{if(s->draw||s->resolves!=1||nfs_logo_publisher::exceptions()>unwind||!bound(*s))throw 1;selection(*s);}catch(...){fail(s);}s->inName=false;}
};
struct ResolvedScope {
 State*s=nullptr;Draw*d=nullptr;int unwind=0;
 explicit ResolvedScope(Game&g)noexcept{Errno e;s=current(g);if(!s)return;if(!s->inName||s->draw||s->inFill||s->resolves++){fail(s);s=nullptr;return;}try{photo(*s).draws.emplace_back();d=&photo(*s).draws.back();d->rows.reserve(MaxRows);d->name=s->name;d->args={{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};size_t size=0;{nfs::GuestCopyTransaction tx;if(!span(d->args[0],16))throw 1;const auto first=tx.load<uint32_t>(d->args[0]),wh=tx.load<uint32_t>(d->args[0]+4),w=wh&65535,h=wh>>16,offset=first>>8;if((first&255)!=0x7b||!w||w>640||!h||h>480||offset<16+uint64_t(w)*h||offset-(16+uint64_t(w)*h)>3||!span(d->args[0],size_t(offset)+16))throw 1;const uint32_t pal=d->args[0]+offset;const unsigned count=tx.load<uint16_t>(pal+4);if((tx.load<uint32_t>(pal)&255)!=0x2a||!count||count>256)throw 1;size=size_t(offset)+16+4*count;if(size>MaxSource||!span(d->args[0],size))throw 1;}d->sourceBefore.resize(size);d->sourceAfter.resize(size);bytes(d->args[0],d->sourceBefore);table(d->tableBefore);s->draw=d;unwind=nfs_logo_publisher::exceptions();}catch(...){fail(s);d=nullptr;}}
 ~ResolvedScope()noexcept{Errno e;if(!d)return;try{if(s->inRow||nfs_logo_publisher::exceptions()>unwind||!bound(*s))throw 1;bytes(d->args[0],d->sourceAfter);table(d->tableAfter);if(d->sourceBefore!=d->sourceAfter)throw 1;}catch(...){fail(s);}s->draw=nullptr;}
};
struct RowScope {
 State*s=nullptr;Row row;bool fill=false;int unwind=0;
 explicit RowScope(Game&g)noexcept{
  Errno e;s=current(g);if(!s||(!s->inFill&&!s->draw)){s=nullptr;return;}
  if(s->inRow){fail(s);s=nullptr;return;}
  try{
   fill=s->inFill;auto&rows=fill?photo(*s).fillRows:s->draw->rows;if(rows.size()>=MaxRows)throw 1;
   row.args={{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};
   const auto&canvas=photo(*s).before;const size_t n=size_t(row.args[2])*(canvas.bits/8);
   if(!row.args[2]||row.args[2]>Width||row.args[1]<uint64_t(canvas.words[8])+16||uint64_t(row.args[1])+n>uint64_t(canvas.words[8])+16+canvas.pixels.size())throw 1;
   Canvas now;
   {nfs::GuestCopyTransaction tx;metadata(tx,now);if(!same(now,canvas)||!span(uint32_t(g.esp),0x3c))throw 1;
    row.callback=tx.load<uint32_t>(uint32_t(g.esp)+0x38);row.format=tx.load<uint8_t>(byte_4DB25C);
    row.converter=tx.load<uint32_t>(dword_571264);row.forward=tx.load<uint32_t>(dword_571260);row.pattern=tx.load<uint32_t>(dword_564140);row.palette=tx.load<uint32_t>(dword_5636EC);
   }
   row.before.resize(n);row.after.resize(n);
   // Exact known dispatch source strides; unknown dispatches collect only
   // metadata/destination bytes, explicitly marked sourceSpanKnown=false.
   row.wrappedIndexed32=!fill&&row.callback==0xf000211c&&row.converter==0xf0002218&&row.format==32;
   const unsigned sourceStride=fill?0:(row.callback==0xf0001850||row.wrappedIndexed32)?1:row.callback==0xf000184c?4:0;
   row.sourceSpanKnown=fill||sourceStride!=0;row.indexedPalette=!fill&&(row.callback==0xf0001850||row.wrappedIndexed32);
   if(row.wrappedIndexed32){nfs::GuestCopyTransaction tx;row.palette=tx.load<uint32_t>(uint32_t(guest_address(off_4DD044)));}
   if(sourceStride){row.sourceBefore.resize(size_t(row.args[2])*sourceStride);row.sourceAfter.resize(row.sourceBefore.size());}
   {nfs::GuestCopyTransaction tx;metadata(tx,now);if(!same(now,canvas)||tx.load<uint32_t>(uint32_t(g.esp)+0x38)!=row.callback)throw 1;
    std::memcpy(row.before.data(),nfs_host_address(row.args[1],n),n);
    if(sourceStride){if(!span(row.args[0],row.sourceBefore.size()))throw 1;std::memcpy(row.sourceBefore.data(),nfs_host_address(row.args[0],row.sourceBefore.size()),row.sourceBefore.size());}
    if(row.indexedPalette){if(!span(row.palette,1024))throw 1;for(unsigned k=0;k<256;++k)row.paletteBefore[k]=tx.load<uint32_t>(row.palette+4*k);}
   }
   s->inRow=true;unwind=nfs_logo_publisher::exceptions();
  }catch(...){fail(s);s=nullptr;}
 }
 ~RowScope()noexcept{
  Errno e;if(!s)return;
  try{
   if(nfs_logo_publisher::exceptions()>unwind||!bound(*s))throw 1;Canvas now;
   {nfs::GuestCopyTransaction tx;metadata(tx,now);if(!same(now,photo(*s).before))throw 1;
    std::memcpy(row.after.data(),nfs_host_address(row.args[1],row.after.size()),row.after.size());
    if(!row.sourceAfter.empty())std::memcpy(row.sourceAfter.data(),nfs_host_address(row.args[0],row.sourceAfter.size()),row.sourceAfter.size());
    if(row.wrappedIndexed32&&(tx.load<uint32_t>(dword_571260)!=row.forward||tx.load<uint32_t>(dword_571264)!=row.converter||tx.load<uint32_t>(uint32_t(guest_address(off_4DD044)))!=row.palette))throw 1;
    if(row.indexedPalette)for(unsigned k=0;k<256;++k)row.paletteAfter[k]=tx.load<uint32_t>(row.palette+4*k);
   }
   auto&rows=fill?photo(*s).fillRows:s->draw->rows;rows.push_back(std::move(row));
  }catch(...){fail(s);}s->inRow=false;
 }
};
} // namespace nfs_player_graph_capture

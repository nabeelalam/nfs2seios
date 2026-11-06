// SPDX-License-Identifier: MIT
#pragma once
#include "SlideshowGameBridge.inc"
#include "ResultsTextDisplay.h"
#include "hd/SlideshowSurfaceBinding.h"
#include "ResultsFont.h"
#include "hd/ContinuousLogoGameHooks.inc"
#include "hd/SmallLogoPrepare.h"
#include "FunctionIds.h"
namespace nfs_slideshow_text {
constexpr unsigned Pixels=640*480;
struct Measurement {bool ready=false;uint32_t text=0;int width=0,anchor=0;unsigned alignment=0;};
struct Payload {
 nfs_logo_capture::Target target{};
 std::vector<uint16_t>baseline,original,clean;
 std::vector<nfs_soft::TextCommand>commands;
 uint64_t epoch=0;
};
struct State {
 Game*game=nullptr;bool failed=false,capturing=false,string=false,word=false;
 unsigned resourceDepth=0;uint32_t textAddress=0,metricAddress=0,fontAddress=0;
 struct Store{uint32_t index;uint16_t before,after;};std::vector<Store>stores;
 std::array<int16_t,256>advances{};std::array<bool,256>known{};
 std::array<uint8_t,96>textBefore{};Measurement measurement{};nfs_soft::TextCommand command{};
 std::shared_ptr<Payload>building;std::shared_ptr<const Payload>published;
 std::shared_ptr<const nfs_results_text::PreparedText>display;
 void invalidate()noexcept{failed=true;capturing=string=word=false;building.reset();published.reset();display.reset();stores.clear();}
};
inline State&state(){static thread_local State value;return value;}
inline State*current(Game&g){auto&s=state();auto&b=nfs_slideshow_live::bridge();return !s.failed&&s.game==&g&&b.game==&g&&b.form?&s:nullptr;}
using CpuErrno=nfs_slideshow_live::Preserve;
template<class F>inline void safe(Game&g,F&&f)noexcept{CpuErrno preserve(g);try{f();}catch(...){state().invalidate();}}
inline nfs_logo_capture::Target target(){nfs_logo_capture::Target t;nfs::GuestCopyTransaction tx;nfs_logo_publisher::target(tx,t);if(!nfs_small_logo_live::target(t,640)||t.w[3]||t.w[4]||t.w[5]!=640||t.w[6]!=480)throw 1;return t;}
inline std::vector<uint16_t>plane(const nfs_logo_capture::Target&t){std::vector<uint16_t>v(Pixels);nfs::GuestCopyTransaction tx;std::memcpy(v.data(),nfs_host_address(t.w[8]+16,Pixels*2),Pixels*2);return v;}
inline void metricTable(State&s,uint32_t font,uint32_t table,unsigned count){
 if(s.metricAddress==table&&s.fontAddress==font)return;if(!font||!table||!count||count>4096)throw 1;
 s.known.fill(false);s.advances.fill(0);nfs::GuestCopyTransaction tx;
 for(unsigned k=0;k<count;++k){uint32_t at=table+11*k;unsigned code=tx.load<uint16_t>(at);if(code<256){if(s.known[code])throw 1;s.known[code]=true;s.advances[code]=int8_t(tx.load<uint8_t>(at+8));}}
 s.metricAddress=table;s.fontAddress=font;
}
struct ResourceScope {
 Game&g;State*s=nullptr;Measurement old{};bool entered=false;
 explicit ResourceScope(Game&x)noexcept:g(x){s=current(g);if(!s||!s->capturing)return;old=s->measurement;s->measurement={};++s->resourceDepth;entered=true;}
 ~ResourceScope()noexcept{if(entered&&s&&s==current(g)){s->measurement=old;if(s->resourceDepth)--s->resourceDepth;}}
};
inline void measured(Game&g,uint32_t text,int width,unsigned alignment)noexcept{
 auto*s=current(g);if(!s||!s->capturing||!s->resourceDepth||s->string)return;
 safe(g,[&]{if(width<=0||width>4096||(alignment!=1&&alignment!=2))throw 1;s->measurement={true,text,width,int(g.esi),alignment};});
}
struct StringScope {
 Game&g;State*s=nullptr;bool entered=false;
 explicit StringScope(Game&x)noexcept:g(x){s=current(g);if(!s||!s->capturing)return;safe(g,[&]{
  if(s->string||s->word||s->building->commands.size()>=256)throw 1;
  auto&c=s->command;c={};s->textBefore.fill(0);s->textAddress=uint32_t(g.eax);
  uint32_t font=0,table=0,count=0,size=0,callback=0;
  {nfs::GuestCopyTransaction tx;for(unsigned k=0;k<96;++k){uint8_t value=tx.load<uint8_t>(s->textAddress+k);s->textBefore[k]=value;if(!value){c.length=k;break;}}
   // Original 405CF0 draws no glyphs for an empty string (e.g. a blank name).
   // Skip that command; any unexpected original stores still fail final replay.
   if(!c.length){if(!s->textBefore[0])return;throw 1;}
   if(c.length>95||s->textBefore[c.length])throw 1;
   font=tx.load<uint32_t>(dword_4DD770);table=tx.load<uint32_t>(dword_4DD76C);count=tx.load<uint32_t>(dword_4DD75C);size=tx.load<uint32_t>(dword_4E54AC);callback=tx.load<uint32_t>(dword_4DD77C);
   c.color=uint16_t(tx.load<uint32_t>(dword_4DD714));c.originX=tx.load<int32_t>(dword_4DD6F4);c.originY=tx.load<int32_t>(dword_4DD6F8);
  }
  size&=255;if(callback!=uint32_t(game_id_sub_4A5830)||(size!=13&&size!=14&&size!=16&&size!=18&&size!=24&&size!=36))throw 1;metricTable(*s,font,table,count);
  bool terminal=false;int width=0;for(unsigned k=0;k<c.length;++k){unsigned code=s->textBefore[k];c.text[k]=uint8_t(code);if(code==9){terminal=true;continue;}if(terminal||!s->known[code]||nfs_soft::index(uint8_t(code))<0)throw 1;width+=s->advances[code];}
  if(width<0||width>4096)throw 1;c.sourceSize=size;c.x=g.edx;c.y=g.ebx;c.advance=width;c.clip={0,0,640,480};c.alignment=0;c.role=0;
  const auto&m=s->measurement;if(m.ready&&m.text==s->textAddress){int expected=m.anchor-(m.alignment==2?m.width:m.width/2);if(c.x==expected||c.x==expected+1){c.alignment=m.alignment;c.advance=m.width;}}
  s->string=true;entered=true;
 });}
 ~StringScope()noexcept{if(!entered||!s||s!=current(g))return;safe(g,[&]{
  if(!s->string||s->word||!s->building)throw 1;
  {nfs::GuestCopyTransaction tx;for(unsigned k=0;k<=s->command.length;++k)if(tx.load<uint8_t>(s->textAddress+k)!=s->textBefore[k])throw 1;}
  s->building->commands.push_back(s->command);s->string=false;
 });}
};
struct Word {
 Game&g;State*s=nullptr;uint32_t address=0,index=0;uint16_t before=0,value=0;bool entered=false;
 Word(Game&x,uint32_t at,uint16_t computed)noexcept:g(x){s=current(g);if(!s||!s->capturing||!s->string)return;safe(g,[&]{
  if(s->word||!s->building)throw 1;uint64_t first=uint64_t(s->building->target.w[8])+16;if(at<first||uint64_t(at)+2>first+Pixels*2||(at-first)%2)throw 1;address=at;index=unsigned((at-first)/2);value=computed;
  {nfs::GuestCopyTransaction tx;before=tx.load<uint16_t>(at);}if(s->stores.size()>=1000000)throw 1;s->word=true;entered=true;
 });}
 ~Word()noexcept{if(!entered||!s||s!=current(g))return;safe(g,[&]{uint16_t after;{nfs::GuestCopyTransaction tx;after=tx.load<uint16_t>(address);}if(after!=value)throw 1;s->stores.push_back({index,before,after});s->word=false;});}
};

inline void event(const nfs_slideshow_live::Snapshot&e)noexcept {
 using E=nfs_slideshow_live::Event;
 if(e.event==E::Enter || e.event==E::Exit || e.event==E::ArchiveBefore){state()=State{};return;}
 auto&b=nfs_slideshow_live::bridge();if(!e.game||b.game!=e.game||b.form!=e.form)return;
 if(e.event==E::OverlayBefore){
  state()=State{};state().game=e.game;
  safe(*e.game,[&]{auto&s=state();auto image=b.session.frame();if(!image)throw 1;
   auto p=std::make_shared<Payload>();p->epoch=image->epoch;p->target=target();p->baseline=plane(p->target);
   if(!nfs_slideshow::sourceSurfaceMatches(*image,p->baseline))throw 1;
   s.building=std::move(p);s.capturing=true;
  });return;
 }
 if(e.event==E::OverlayAfter){
  auto*s=current(*e.game);if(!s||!s->capturing)return;
  safe(*e.game,[&]{if(s->string||s->word||!s->building)throw 1;auto p=s->building;
   if(!nfs_logo_capture::same(p->target,target()))throw 1;
   p->original=plane(p->target);p->clean=p->original;
   for(auto i=s->stores.rbegin();i!=s->stores.rend();++i){if(p->clean[i->index]!=i->after)throw 1;p->clean[i->index]=i->before;}
   // Every visible alteration in the controls pass must be accounted for by
   // actual observed text stores. Retain originals when a sprite/other draw exists.
   if(p->clean!=p->baseline)throw 1;
   if(!p->commands.empty()){
    s->display=nfs_results_text::PreparedText::createCommands(p->commands,nfs_results_text::resultsFont(),p);
    if(!s->display)throw 1;
   }
   s->published=p;s->building.reset();s->capturing=false;
  });
 }
}
} // namespace nfs_slideshow_text
namespace nfs_slideshow_live {
inline void observeText(const Snapshot&e)noexcept{nfs_slideshow_text::event(e);}
}

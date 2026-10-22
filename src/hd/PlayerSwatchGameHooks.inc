// SPDX-License-Identifier: MIT
#pragma once
#ifndef NFS_PLAYER_SWATCH_RUNTIME_TEST
#include "PlayerSceneLiveGameHooks.inc"
#endif
#include "PlayerSwatchPacket.h"
// Refresh-only original-operation observer. Body scratch writes never inherit
// cached-image authority. Scopes surround original calls without changing them.
namespace nfs_player_swatch_live {
namespace live=nfs_player_scene_live;namespace sw=nfs_player_swatch;
enum class Site:uint32_t{Refresh=0x40cab0};
enum class Failure{None,Owner,Nested,Order,Read,Target,Source,Color,Row,Unwind,Overflow,Allocation,Verify};
struct State {
 sw::Input input;live::State*owner=nullptr;decltype(live::State::preview)preview;std::shared_ptr<const nfs_logo_capture::Copy>copy;
 uintptr_t game=0,thread=0;uint64_t epoch=0,serial=0;live::bg::Event copyEvent{},baseEvent{},previewEvent{};
 unsigned depth=0,phase=0,converted=0,filled=0;bool invalid=false,callActive=false,rowActive=false;
 Failure failure=Failure::None;sw::Failure verifyFailure=sw::Failure::None;
 uint32_t rowGuest=0,carGuest=0,colorGuest=0,tableGuest=0,raw=0;std::array<uint32_t,3>rgb{};
};
inline std::atomic<uintptr_t>&ownerThread()noexcept{static std::atomic<uintptr_t>v{0};return v;}
inline State*&slot()noexcept{struct Storage{State*p=nullptr;~Storage(){delete p;}};static thread_local Storage s;return s.p;}
inline bool eventSame(const live::bg::Event&a,const live::bg::Event&b)noexcept{return a.ownerEpoch==b.ownerEpoch&&a.sequence==b.sequence&&a.iteration==b.iteration;}
inline void reject(State&s,Failure why)noexcept{s.failure=why;s.invalid=true;s.input.error=1;if(s.owner&&s.owner==live::slot()&&s.owner->on&&s.owner->game==s.game&&s.owner->thread==s.thread){s.owner->clearSwatch();s.owner->published.reset();}}
inline State*current(Game&g)noexcept{if(ownerThread().load(std::memory_order_acquire)!=nfs_logo_publisher::threadToken())return nullptr;auto*s=slot();return s&&s->depth==1&&!s->invalid&&s->game==reinterpret_cast<uintptr_t>(&g)&&s->owner==live::current(g)?s:nullptr;}
inline bool bound(State&s,Game&g)noexcept{auto*l=live::current(g);return l&&l==s.owner&&!l->invalid&&!l->iteration&&l->baseVerified&&l->form==s.input.binding.form&&l->formGuest==s.input.binding.owner&&l->sourceOwner==s.input.binding.owner&&l->preview&&l->preview==s.preview&&l->preview->selection().selectedCar==s.input.binding.car&&eventSame(l->previewEvent,s.previewEvent)&&l->currentCopy==s.copy&&l->ownerEpoch==s.epoch&&l->nextSwatchSerial==s.serial&&eventSame(l->currentCopyEvent,s.copyEvent)&&eventSame(l->base.event,s.baseEvent)&&l->archive==s.input.binding.archive&&live::archiveCurrent(*l);}
inline uint64_t event(State&s)noexcept{auto e=s.owner->event(s.copyEvent.iteration);if(!e.sequence||s.owner->invalid){reject(s,Failure::Overflow);return 0;}return e.sequence;}
inline bool tableNow(State&s,std::array<uint32_t,4>&out)noexcept{try{nfs::GuestCopyTransaction tx;for(unsigned k=0;k<4;++k)out[k]=tx.load<uint32_t>(s.tableGuest+4*k);return true;}catch(...){return false;}}
inline bool selectionCurrent(State&s)noexcept{try{nfs::GuestCopyTransaction tx;return ((tx.load<uint32_t>(s.rowGuest+12)==1)==(s.input.binding.player==1))&&tx.load<uint8_t>(s.carGuest)==s.input.binding.car&&tx.load<uint8_t>(s.colorGuest)==s.input.binding.color;}catch(...){return false;}}
inline bool noTargetAlias(uint32_t at,size_t bytes,const nfs_logo_capture::Target&t)noexcept{return at&&uint64_t(at)+bytes<=UINT64_C(0x100000000)&&!nfs_small_logo_live::overlap(at,bytes,t.w[8],16+sw::Full*2);}
struct Scope {
 Game&g;State*s=nullptr;bool entered=false,claimed=false;int unwind=0;
 Scope(Game&game,Site site)noexcept:g(game){auto*l=live::current(g);if(!l||l->invalid||l->iteration||!l->baseVerified||!l->preview||!l->currentCopy||!l->currentCopy->complete||!l->currentCopyEvent.sequence||l->currentCopyEvent.iteration||site!=Site::Refresh||g.edx!=365||g.ebx!=136)return;
  const uint32_t row=l->form==0x4d3050?uint32_t(guest_address(dword_4D2F80)):l->form==0x4d3100?uint32_t(guest_address(dword_4D2B68)):0;if(!row||uint32_t(g.eax)!=row||!live::archiveCurrent(*l))return;
  uintptr_t expected=0,t=nfs_logo_publisher::threadToken();if(!ownerThread().compare_exchange_strong(expected,t)&&expected!=t)return;claimed=expected==0;
  auto*&p=slot();if(!p)p=new(std::nothrow)State;if(!p){if(claimed)ownerThread().store(0);return;}s=p;if(s->depth){reject(*s,Failure::Nested);return;}
  l->clearSwatch();l->published.reset();s->owner=l;s->preview=l->preview;s->previewEvent=l->previewEvent;s->copy=l->currentCopy;s->copyEvent=l->currentCopyEvent;s->baseEvent=l->base.event;s->game=reinterpret_cast<uintptr_t>(&g);s->thread=t;s->epoch=l->ownerEpoch;s->depth=1;s->phase=s->converted=s->filled=0;s->invalid=s->callActive=s->rowActive=false;s->failure=Failure::None;s->verifyFailure=sw::Failure::None;s->rowGuest=row;s->rgb={};s->raw=0;s->input.complete=s->input.error=0;s->input.events={};s->input.binding={};s->input.pre.valid=s->input.post.valid=0;
  entered=true;unwind=nfs_logo_publisher::exceptions();if(l->nextSwatchSerial==UINT64_MAX){reject(*s,Failure::Overflow);return;}s->serial=++l->nextSwatchSerial;
  auto&b=s->input.binding;b.origin=sw::Origin::Refresh;b.form=l->form;b.epoch=b.lifetime=s->epoch;b.serial=s->serial;b.owner=l->formGuest;b.archive=l->archive;b.player=l->form==0x4d3050?1:2;b.row=b.player==1?sw::Player1Row:sw::Player2Row;b.carAddress=b.player==1?0x512dbf:0x512dda;
  try{for(auto&f:s->input.fills){f.rows.clear();f.rows.reserve(21);f.complete=f.error=0;f.callback=f.converter=f.pattern=0;f.format=0;}s->carGuest=uint32_t(guest_address(b.player==1?byte_512DBF:byte_512DDA));
   {nfs::GuestCopyTransaction tx;if((tx.load<uint32_t>(s->rowGuest+12)==1)!=(b.player==1))throw 1;b.car=tx.load<uint8_t>(s->carGuest);if(b.car>=15)throw 1;s->colorGuest=uint32_t(guest_address(b.player==1?byte_512DC1:byte_512DDC))+b.car;b.color=tx.load<uint8_t>(s->colorGuest);s->tableGuest=uint32_t(guest_address(dword_4D62C8))+16*b.color;}
   b.colorAddress=(b.player==1?0x512dc1:0x512ddc)+b.car;b.tableAddress=sw::ColorTable+16*b.color;
   if(!live::readPlane(s->input.pre)||!sw::plane(s->input.pre)||(!sw::plane(s->copy->post)||!nfs_logo_capture::same(s->input.pre.target,s->copy->post.target)||s->input.pre.pixels!=s->copy->post.pixels)||!noTargetAlias(s->rowGuest,16,s->input.pre.target)||!noTargetAlias(s->carGuest,1,s->input.pre.target)||!noTargetAlias(s->colorGuest,1,s->input.pre.target)||!noTargetAlias(s->tableGuest,16,s->input.pre.target)||!tableNow(*s,b.table)||!sw::color(b.table,s->raw,&s->rgb)||b.car!=l->preview->selection().selectedCar)throw 1;
   s->input.events.enter=event(*s);
  }catch(...){reject(*s,Failure::Source);}
 }
 ~Scope(){if(!entered){if(claimed)ownerThread().store(0);return;}if(nfs_logo_publisher::exceptions()>unwind)reject(*s,Failure::Unwind);
  if(!s->invalid){std::array<uint32_t,4>table{};if(!bound(*s,g)||s->callActive||s->rowActive||s->phase!=3||s->converted!=2||s->filled!=2||!selectionCurrent(*s)||!tableNow(*s,table)||table!=s->input.binding.table||!live::readPlane(s->input.post))reject(*s,Failure::Order);
   else{s->input.events.exit=event(*s);s->input.complete=1;sw::Authority a{s->input.binding,s->input.events};auto packet=sw::createPacket(s->input,a,s->copy->post,s->input.post,&s->verifyFailure);if(!packet||!bound(*s,g))reject(*s,Failure::Verify);else{auto*l=s->owner;l->swatch=std::move(packet);l->swatchAuthority=a;l->swatchCopyPost=s->copy->post;l->actualSwatchPost=s->input.post;l->swatchCopyEvent=s->copyEvent;l->acceptedSwatchCopySerial=s->copy->serial;l->swatchEvent={s->epoch,s->input.events.exit,s->copyEvent.iteration};l->swatchCurrent=true;}}}
  s->depth=0;s->copy.reset();s->preview.reset();if(claimed)ownerThread().store(0);
 }
};
// These scopes are lexical caller scopes, outside the generated call's return
// address adjustment. Output arguments therefore refer to the caller ESP.
struct HsvScope {Game&g;State*s=nullptr;std::array<uint32_t,4>out{};int unwind=0;
 explicit HsvScope(Game&game)noexcept:g(game){s=current(g);if(!s)return;unwind=nfs_logo_publisher::exceptions();if(s->phase||s->callActive||uint32_t(g.eax)!=s->input.binding.color){reject(*s,Failure::Order);s=nullptr;return;}s->callActive=true;try{out={{uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx),0}};nfs::GuestCopyTransaction tx;out[3]=tx.load<uint32_t>(uint32_t(g.esp));for(auto at:out)if(!noTargetAlias(at,4,s->input.pre.target))throw 1;}catch(...){reject(*s,Failure::Read);}}
 ~HsvScope(){if(!s)return;if(!s->invalid){try{if(nfs_logo_publisher::exceptions()>unwind)throw 1;nfs::GuestCopyTransaction tx;for(unsigned k=0;k<4;++k)if(tx.load<uint32_t>(out[k])!=s->input.binding.table[k])throw 1;}catch(...){reject(*s,Failure::Color);}if(!s->invalid){s->input.events.table=event(*s);s->phase=1;}}s->callActive=false;}
};
struct RgbScope {Game&g;State*s=nullptr;std::array<uint32_t,3>out{};int unwind=0;
 explicit RgbScope(Game&game)noexcept:g(game){s=current(g);if(!s)return;unwind=nfs_logo_publisher::exceptions();if(s->phase!=1||s->callActive||uint32_t(g.eax)!=s->input.binding.table[0]||uint32_t(g.edx)!=s->input.binding.table[1]||uint32_t(g.ebx)!=s->input.binding.table[2]){reject(*s,Failure::Order);s=nullptr;return;}s->callActive=true;try{nfs::GuestCopyTransaction tx;out={{uint32_t(g.ecx),tx.load<uint32_t>(uint32_t(g.esp)),tx.load<uint32_t>(uint32_t(g.esp)+4)}};for(auto at:out)if(!noTargetAlias(at,4,s->input.pre.target))throw 1;}catch(...){reject(*s,Failure::Read);}}
 ~RgbScope(){if(!s)return;if(!s->invalid){try{if(nfs_logo_publisher::exceptions()>unwind)throw 1;nfs::GuestCopyTransaction tx;for(unsigned k=0;k<3;++k)if((s->input.rgb[k]=tx.load<uint32_t>(out[k]))!=s->rgb[k])throw 1;}catch(...){reject(*s,Failure::Color);}if(!s->invalid){s->input.events.rgb=event(*s);s->phase=2;}}s->callActive=false;}
};
struct ShadeScope {Game&g;State*s=nullptr;uint32_t out=0;int unwind=0;
 explicit ShadeScope(Game&game)noexcept:g(game){s=current(g);if(!s)return;unwind=nfs_logo_publisher::exceptions();if(s->phase!=2||s->callActive||uint32_t(g.edx)!=s->input.binding.table[3]){reject(*s,Failure::Order);s=nullptr;return;}s->callActive=true;out=uint32_t(g.ecx);try{if(!noTargetAlias(uint32_t(g.eax),12,s->input.pre.target)||!noTargetAlias(uint32_t(g.ebx),12,s->input.pre.target)||!noTargetAlias(out,4,s->input.pre.target))throw 1;nfs::GuestCopyTransaction tx;for(unsigned k=0;k<3;++k){if(tx.load<uint32_t>(uint32_t(g.eax)+4*k)!=(k==1?210u:0u)||tx.load<uint32_t>(uint32_t(g.ebx)+4*k)!=s->rgb[k])throw 1;}}catch(...){reject(*s,Failure::Read);}}
 ~ShadeScope(){if(!s)return;if(!s->invalid){try{if(nfs_logo_publisher::exceptions()>unwind)throw 1;nfs::GuestCopyTransaction tx;const uint32_t raw=0xff000000u|(uint32_t(tx.load<uint8_t>(out))<<16)|(uint32_t(tx.load<uint8_t>(out+1))<<8)|tx.load<uint8_t>(out+2);if(raw!=s->raw)throw 1;s->input.rawColor=raw;}catch(...){reject(*s,Failure::Color);}if(!s->invalid){s->input.events.shade=event(*s);s->phase=3;}}s->callActive=false;}
};
struct ColorScope {Game&g;State*s=nullptr;uint32_t raw=0;int unwind=0;
 explicit ColorScope(Game&game)noexcept:g(game){s=current(g);if(!s)return;unwind=nfs_logo_publisher::exceptions();if(s->phase!=3||s->callActive||s->converted>=2||s->filled!=s->converted){reject(*s,Failure::Order);s=nullptr;return;}raw=s->converted?s->raw:0xff000000u;if(uint32_t(g.eax)!=raw){reject(*s,Failure::Color);s=nullptr;return;}s->callActive=true;}
 ~ColorScope(){if(!s)return;if(!s->invalid){if(nfs_logo_publisher::exceptions()>unwind||uint32_t(g.eax)!=sw::pack(raw))reject(*s,Failure::Color);else ++s->converted;}s->callActive=false;}
};
struct FillScope {Game&g;State*s=nullptr;sw::Fill*f=nullptr;int unwind=0;
 explicit FillScope(Game&game)noexcept:g(game){s=current(g);if(!s)return;unwind=nfs_logo_publisher::exceptions();if(s->phase!=3||s->callActive||s->filled>=2||s->converted!=s->filled+1){reject(*s,Failure::Order);s=nullptr;return;}s->callActive=true;f=&s->input.fills[s->filled];f->call={{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};f->rawColor=s->filled?s->raw:0xff000000u;try{nfs::GuestCopyTransaction tx;if(tx.load<uint32_t>(uint32_t(g.esp))!=sw::pack(f->rawColor))throw 1;f->format=tx.load<uint8_t>(byte_4DB25C);for(unsigned k=0;k<9;++k)f->tableBefore[k]=tx.load<uint32_t>(uint32_t(guest_address(off_4DB648))+4*k);}catch(...){reject(*s,Failure::Read);}if(!s->invalid&&(!live::targetNow(f->before)||!nfs_logo_capture::same(f->before,s->input.pre.target)||f->call!=std::array<uint32_t,4>{{s->filled?366u:367u,s->filled?141u:142u,84,21}}))reject(*s,Failure::Target);}
 ~FillScope(){if(!s||!f)return;if(!s->invalid){try{if(nfs_logo_publisher::exceptions()>unwind||s->rowActive)throw 1;{nfs::GuestCopyTransaction tx;f->pattern=tx.load<uint32_t>(dword_564140);for(unsigned k=0;k<9;++k)f->tableAfter[k]=tx.load<uint32_t>(uint32_t(guest_address(off_4DB648))+4*k);}if(!live::targetNow(f->after)||!nfs_logo_capture::same(f->before,f->after))throw 1;f->complete=1;if(s->filled)s->input.events.color=event(*s);else s->input.events.shadow=event(*s);++s->filled;}catch(...){reject(*s,Failure::Row);}}s->callActive=false;}
};
struct RowScope {State*s=nullptr;sw::Row*r=nullptr;nfs_logo_capture::Target before{};int unwind=0;
 explicit RowScope(Game&g)noexcept{s=current(g);if(!s||!s->callActive||s->phase!=3||s->filled>=2||s->converted!=s->filled+1)return;if(s->rowActive){reject(*s,Failure::Nested);return;}auto&f=s->input.fills[s->filled];try{if(f.rows.size()>=21)throw 1;f.rows.emplace_back();r=&f.rows.back();r->args={{uint32_t(g.eax),uint32_t(g.edx),uint32_t(g.ebx),uint32_t(g.ecx)}};unwind=nfs_logo_publisher::exceptions();{nfs::GuestCopyTransaction tx;r->callback=tx.load<uint32_t>(uint32_t(g.esp)+0x38);r->format=tx.load<uint8_t>(byte_4DB25C);r->converter=tx.load<uint32_t>(dword_571264);r->pattern=tx.load<uint32_t>(dword_564140);}
 if(r->callback!=sw::FillCallback||r->format!=16||r->converter||!r->args[2]||r->args[2]>84||!live::targetNow(before)||!nfs_logo_capture::same(before,s->input.pre.target)||r->args[1]<before.w[8]+16||((r->args[1]-before.w[8]-16)&1)||uint64_t(r->args[1])+2*r->args[2]>uint64_t(before.w[8])+16+sw::Full*2)throw 1;
 r->before.resize(r->args[2]);r->after.resize(r->args[2]);{nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(r->args[1],2*r->args[2]);std::memcpy(r->before.data(),nfs_host_address(r->args[1],2*r->args[2]),2*r->args[2]);}s->rowActive=true;
 }catch(...){reject(*s,Failure::Row);r=nullptr;}}
 ~RowScope(){if(!r)return;if(!s->invalid){try{if(nfs_logo_publisher::exceptions()>unwind)throw 1;{nfs::GuestCopyTransaction tx;nfs_logo_publisher::checked(r->args[1],2*r->args[2]);std::memcpy(r->after.data(),nfs_host_address(r->args[1],2*r->args[2]),2*r->args[2]);}nfs_logo_capture::Target after;if(!live::targetNow(after)||!nfs_logo_capture::same(before,after))throw 1;auto&f=s->input.fills[s->filled];if(f.rows.size()==1){f.callback=r->callback;f.converter=r->converter;}if(f.callback!=r->callback||f.converter!=r->converter)throw 1;}catch(...){reject(*s,Failure::Row);}}s->rowActive=false;}
};
} // namespace nfs_player_swatch_live

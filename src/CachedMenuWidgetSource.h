// SPDX-License-Identifier: MIT
#pragma once
#include "Game.h"
#include "BSS.h"
#include "DATA.h"
#include "MenuResponsiveCached.h"
#include "MenuResponsivePointerHooks.h"
#include "GuestCopyTransaction.h"
#include <array>
#include <cerrno>
#include <exception>
#include <stdint.h>
/* Exact original producer/consumer observations only. No guest stores. */
namespace nfs_menu_cached_widget_source {
constexpr unsigned Capacity=40,ItemCapacity=64;
struct Row {NfsMenuCachedWidget value{};uint32_t address=0;};
struct Snapshot {
 uint32_t tag=0,descriptor=0,header0=0,header1=0,cursorX=0,cursorY=0,count=0,itemCount=0,opponentAddress=0,opponentValid=0;
 uintptr_t game=0;std::array<Row,Capacity> rows{};
};
struct State {uint64_t ownerGeneration=0;unsigned depth=0;bool invalid=false,valid=false;Snapshot producer;};
inline State& state(){static thread_local State s;return s;}
inline void invalidate()noexcept {
 state().valid=false;
 try{nfs_menu_cached_widgets(0,0,0,0,nullptr,0,0,0);}catch(...){nfs_menu_pointer_metadata_failure();}
}
inline uint32_t tag(uint32_t descriptor){
 if(descriptor==uint32_t(guest_address(off_4D2790)))return 0x4d2790u;
 if(descriptor==uint32_t(guest_address(off_4D3A88)))return 0x4d3a88u;
 return 0;
}
inline bool capture(Game& g,uint32_t descriptor,Snapshot& out){
 out={};out.tag=tag(descriptor);if(!out.tag)return false;
 out.descriptor=descriptor;out.game=reinterpret_cast<uintptr_t>(&g);
 {nfs::GuestCopyTransaction copy;
  out.header0=copy.load<uint32_t>(descriptor);out.header1=copy.load<uint32_t>(descriptor+4);
  out.cursorX=copy.load<uint32_t>(dword_4D4C7C);out.cursorY=copy.load<uint32_t>(dword_4D4C80);
  // Same original slot5 descriptor resolution as the checked Main observer.
  // The32-byte descriptor is guest metadata; native startAddress is word5.
  const uint32_t opponent=copy.load<uint32_t>(dword_4EB5A4+5*4);
  if(opponent&&!(opponent&3u)&&opponent<=UINT32_MAX-31u){
   struct NativeTextureDescriptor {uint32_t words[8];};
   const auto native=copy.load<NativeTextureDescriptor>(opponent);
   out.opponentAddress=native.words[5];
   out.opponentValid=!(out.opponentAddress&3u)&&out.opponentAddress<=0x200000u-32768u;
  }
  const int count=copy.load<int16_t>(word_4E8C06);if(count<=0||count>int(Capacity)||out.cursorX>639||out.cursorY>479)return false;
  std::array<uint32_t,ItemCapacity> items{};unsigned itemCount=0;bool terminated=false;
  for(;itemCount<ItemCapacity;++itemCount){const uint32_t p=copy.load<uint32_t>(descriptor+12+4*itemCount);if(!p){terminated=true;break;}items[itemCount]=p;}
  if(!terminated)return false;out.count=unsigned(count);out.itemCount=itemCount;
  for(unsigned i=0;i<out.count;++i){auto& row=out.rows[i];auto& w=row.value;
   const int id=copy.load<int16_t>(dword_4E89B6+2+2*i);
   if(id<0||unsigned(id)>=itemCount||(i&&unsigned(id)<=out.rows[i-1].value.itemId))return false;
   w.itemId=uint32_t(id);w.itemCount=itemCount;row.address=items[unsigned(id)];w.itemType=copy.load<uint8_t>(row.address);
   if(w.itemType!=1&&w.itemType!=2)return false;
   w.itemFlags=copy.load<uint8_t>(row.address+(w.itemType==1?2:8));
   // Consumer423CA0 retests only type2's disabled bit. Type1 presence is selectable.
   w.enabled=w.itemType==2?uint32_t(!(w.itemFlags&1u)):1u;
   w.left=copy.load<int16_t>(dword_4E8A56+2+2*i);w.top=copy.load<int16_t>(dword_4E8AA6+2+2*i);
   w.right=copy.load<int16_t>(dword_4E8A06+2+2*i);w.bottom=copy.load<int16_t>(dword_4E8964+4+2*i);
   if(w.left>=w.right||w.top>=w.bottom)return false;
  }
 }
 return true;
}
inline bool sameGeometry(const Snapshot&a,const Snapshot&b){
 if(a.tag!=b.tag||a.descriptor!=b.descriptor||a.game!=b.game||a.header0!=b.header0||a.header1!=b.header1||a.count!=b.count||a.itemCount!=b.itemCount)return false;
 for(unsigned i=0;i<a.count;++i){const auto& x=a.rows[i];const auto& y=b.rows[i];
  if(x.address!=y.address||x.value.itemId!=y.value.itemId||x.value.itemType!=y.value.itemType||
     x.value.left!=y.value.left||x.value.top!=y.value.top||x.value.right!=y.value.right||x.value.bottom!=y.value.bottom)return false;
 }
 return true;
}
inline void publish(const Snapshot&s){
 std::array<NfsMenuCachedWidget,Capacity> rows{};for(unsigned i=0;i<s.count;++i)rows[i]=s.rows[i].value;
 // capture() has ended its transaction BEFORE invoking the host metadata mutex.
 nfs_menu_cached_widgets(s.tag,s.descriptor,s.cursorX,s.cursorY,rows.data(),s.count,s.opponentAddress,s.opponentValid);
}
struct ProducerScope {
 Game& game;uint32_t descriptor;uint64_t ownerGeneration=0;bool outer=false,unwinding=false;
 explicit ProducerScope(Game&g):game(g),descriptor(uint32_t(g.eax)),unwinding(std::uncaught_exception()){
  const int saved=errno;auto&s=state();outer=s.depth==0;ownerGeneration=s.ownerGeneration;++s.depth;
  if(outer)s.invalid=false;else s.invalid=true;invalidate();errno=saved;
 }
 ~ProducerScope()noexcept{
  const int saved=errno;auto&s=state();if(s.depth)--s.depth;
  if(outer)try{
   if(s.depth||s.invalid||s.ownerGeneration!=ownerGeneration||std::uncaught_exception()!=unwinding){invalidate();errno=saved;return;}
   Snapshot fresh;if(!capture(game,descriptor,fresh)){invalidate();errno=saved;return;}
   s.producer=fresh;s.valid=true;publish(fresh);
  }catch(...){invalidate();}
  errno=saved;
 }
 ProducerScope(const ProducerScope&)=delete;
 ProducerScope&operator=(const ProducerScope&)=delete;
};
inline void consumer(Game&g,uint32_t descriptor)noexcept{
 const int saved=errno;try{auto&s=state();Snapshot current;
  if(s.depth||s.invalid||!s.valid||!capture(g,descriptor,current)||!sameGeometry(s.producer,current)){invalidate();errno=saved;return;}
  publish(current);
 }catch(...){invalidate();}errno=saved;
}
inline void ownerBoundary()noexcept{const int saved=errno;auto&s=state();++s.ownerGeneration;if(s.depth)s.invalid=true;invalidate();errno=saved;}
inline void retire()noexcept{const int saved=errno;invalidate();errno=saved;}
}

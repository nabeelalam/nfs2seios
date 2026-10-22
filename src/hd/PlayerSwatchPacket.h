// SPDX-License-Identifier: MIT
#pragma once
#include "SmallLogoPrepare.h"
#include <memory>
#include <vector>
// Pure owned replay. Authority is a separately acquired CURRENT source witness,
// never values copied from the submitted candidate. No guest reads or GPU work.
namespace nfs_player_swatch {
using nfs_logo_capture::Target;using nfs_logo_capture::Plane;
constexpr size_t Full=640*480;constexpr uint32_t FillCallback=0xf0001aa8;
constexpr uint32_t Player1Row=0x4d2f80,Player2Row=0x4d2b68,Player2BodyRow=0x4d309c,ColorTable=0x4d62c8;
enum class Origin{Refresh,Body};
// Address-like fields below are canonical original-source identity tags, not
// relocated guest pointers. A live caller must separately validate relocation,
// readable spans and aliasing before constructing independently owned records.
struct Binding {
 Origin origin=Origin::Refresh;uint32_t form=0;
 uint64_t epoch=0,serial=0,lifetime=0;uint32_t owner=0,archive=0,row=0,player=0,car=0,color=0;
 uint32_t carAddress=0,colorAddress=0,tableAddress=0;
 std::array<uint32_t,4>table{};
};
inline bool same(const Binding&a,const Binding&b)noexcept{return a.origin==b.origin&&a.form==b.form&&a.epoch==b.epoch&&a.serial==b.serial&&a.lifetime==b.lifetime&&a.owner==b.owner&&a.archive==b.archive&&a.row==b.row&&a.player==b.player&&a.car==b.car&&a.color==b.color&&a.carAddress==b.carAddress&&a.colorAddress==b.colorAddress&&a.tableAddress==b.tableAddress&&a.table==b.table;}
// Independently witnessed completed original calls, in one owner epoch.
struct Events{uint64_t enter=0,table=0,rgb=0,shade=0,shadow=0,color=0,exit=0;};
inline bool same(const Events&a,const Events&b)noexcept{return a.enter==b.enter&&a.table==b.table&&a.rgb==b.rgb&&a.shade==b.shade&&a.shadow==b.shadow&&a.color==b.color&&a.exit==b.exit;}
struct Authority{Binding binding;Events events;};
inline bool valid(const Authority&a)noexcept{const auto&b=a.binding;const auto&e=a.events;
 if(!b.epoch||!b.serial||!b.lifetime||!b.owner||!b.archive||b.car>255||b.color>255)return false;
 if(b.origin!=Origin::Refresh&&b.origin!=Origin::Body)return false;
 if(b.player==1){if(b.form!=0x4d3050||b.row!=Player1Row||b.carAddress!=0x512dbf||b.colorAddress!=0x512dc1+b.car)return false;}
 else if(b.player==2){if(b.form!=0x4d3100||b.row!=(b.origin==Origin::Refresh?Player2Row:Player2BodyRow)||b.carAddress!=0x512dda||b.colorAddress!=0x512ddc+b.car)return false;}else return false;
 if(b.tableAddress!=ColorTable+16*b.color)return false;
 for(auto v:b.table)if(v>255)return false;
 return e.enter&&e.enter<e.table&&e.table<e.rgb&&e.rgb<e.shade&&e.shade<e.shadow&&e.shadow<e.color&&e.color<e.exit;
}
// Exact restricted original table domain, not a standard HSV approximation.
// Memory.cpp has111 complete four-word entries, every word0..255 (D includes0
// and values>127).458670 fetches h,s,v,D.457FA8 uses sextants and16.16 products;
// its only x87 conversion is integer6*h-sector*256, exactly representable.
//4584E0 fixed lighting[0,210,0] has brightness82, no extra colored-light branch.
inline bool color(const std::array<uint32_t,4>&t,uint32_t&raw,std::array<uint32_t,3>*rgbOut=nullptr)noexcept{
 for(auto v:t)if(v>255)return false;
 const uint32_t h=t[0],s=t[1],v=t[2],d=t[3],sector=(6*h)>>8,f=((6*h)&255)<<8;
 // Original SHR16 followed by ADC includes discarded bit15 (round to nearest).
 const uint32_t p=(v*(256-s))>>8,q=(v*(256-(((s*f)+32768u)>>16)))>>8,u=(v*(256-(((s*(65536-f))+32768u)>>16)))>>8;
 std::array<uint32_t,3>rgb{};
 switch(sector){case 0:rgb={{v,u,p}};break;case 1:rgb={{q,v,p}};break;case 2:rgb={{p,v,u}};break;case 3:rgb={{p,q,v}};break;case 4:rgb={{u,p,v}};break;case 5:rgb={{v,p,q}};break;default:return false;}
 // Zero saturation bypasses sextants in457FA8; the result is the same gray.
 if(!s)rgb={{v,v,v}};if(rgbOut)*rgbOut=rgb;
 for(auto&c:rgb){if(d>=82)c=(c*82)/d;else c+=((std::min(c*4u,255u)-c)*(82-d))/(127-d);c&=255;}
 raw=0xff000000u|(rgb[0]<<16)|(rgb[1]<<8)|rgb[2];return true;
}
inline uint16_t pack(uint32_t raw)noexcept{return uint16_t(((raw&0xf80000u)>>8)+((raw&0xfc00u)>>5)+((raw>>3)&31));}
// pattern is the original dword564140 GLOBAL consumed by f0001aa8; the
// callback ignores source-pointer contents.488620 sets it even for zero rows.
struct Row{uint32_t callback=0,format=0,converter=0,pattern=0;std::array<uint32_t,4>args{};std::vector<uint16_t>before,after;};
struct Fill{Target before{},after{};std::array<uint32_t,4>call{};uint32_t rawColor=0,pattern=0,callback=0,format=0,converter=0,complete=0,error=0;std::array<uint32_t,9>tableBefore{},tableAfter{};std::vector<Row>rows;};
struct Input{Binding binding;Events events;uint32_t complete=0,error=0;std::array<uint32_t,3>rgb{};uint32_t rawColor=0;Plane pre{},post{};std::array<Fill,2>fills;};
enum class Failure{None,Binding,Events,Color,Target,Fill,Rows,Pixels,Current,Allocation};
// Full640 storage is required, with a valid selected clip (including empty).
// The independently authenticated base remains mandatory for unwritten pixels.
inline bool plane(const Plane&p)noexcept{return p.valid==1&&p.width==640&&p.height==480&&nfs_small_logo_live::target(p.target,640)&&p.target.header[3]==0;}
struct Rect{uint32_t left=366,top=141,width=85,height=22;};
class Packet;
inline std::shared_ptr<const Packet>createPacket(const Input&,const Authority&,const Plane&,const Plane&,Failure*failure=nullptr)noexcept;
class Packet final{
 Binding binding_;Events events_;Target target_;std::vector<uint16_t>post_,pixels_;std::vector<uint8_t>coverage_;uint32_t raw_=0;
 Packet()=default;friend std::shared_ptr<const Packet>createPacket(const Input&,const Authority&,const Plane&,const Plane&,Failure*)noexcept;
public:
 const Binding&binding()const noexcept{return binding_;}const Target&target()const noexcept{return target_;}
 uint64_t epoch()const noexcept{return binding_.epoch;}uint64_t serial()const noexcept{return binding_.serial;}uint64_t lifetime()const noexcept{return binding_.lifetime;}uint32_t archive()const noexcept{return binding_.archive;}uint32_t player()const noexcept{return binding_.player;}uint32_t form()const noexcept{return binding_.form;}Origin origin()const noexcept{return binding_.origin;}
 Rect bounds()const noexcept{return {};}uint16_t packedColor()const noexcept{return pack(raw_);}uint32_t rawColor()const noexcept{return raw_;}const std::vector<uint16_t>&post()const noexcept{return post_;}
 // Cropped85x22 overlay at366,141. Coverage includes black/unchanged writes.
 const std::vector<uint16_t>&pixels()const noexcept{return pixels_;}const std::vector<uint8_t>&coverage()const noexcept{return coverage_;}
 bool matches(const Authority&a,const Target&t,const uint16_t*post,size_t n)const noexcept{return valid(a)&&same(binding_,a.binding)&&same(events_,a.events)&&nfs_logo_capture::same(target_,t)&&post&&n==Full&&!std::memcmp(post_.data(),post,n*2);}
 bool disjoint(const uint8_t*other,size_t n)const noexcept{if(!other||n!=Full)return false;for(unsigned y=0;y<22;++y)for(unsigned x=0;x<85;++x)if(coverage_[y*85+x]&&other[(y+141)*640+x+366])return false;return true;}
};
inline std::shared_ptr<const Packet>createPacket(const Input&i,const Authority&a,const Plane&base,const Plane&current,Failure*failure)noexcept{
 auto reject=[&](Failure f)->std::shared_ptr<const Packet>{if(failure)*failure=f;return {};};if(failure)*failure=Failure::None;
 if(!valid(a)||!same(i.binding,a.binding)||i.complete!=1||i.error)return reject(Failure::Binding);
 if(!same(i.events,a.events))return reject(Failure::Events);
 uint32_t raw=0;std::array<uint32_t,3>rgb{};if(!color(a.binding.table,raw,&rgb)||i.rawColor!=raw||i.rgb!=rgb)return reject(Failure::Color);
 if(!plane(base)||!plane(i.pre)||!plane(i.post)||!plane(current)||!nfs_logo_capture::same(base.target,i.pre.target)||!nfs_logo_capture::same(base.target,i.post.target)||!nfs_logo_capture::same(base.target,current.target)||base.pixels!=i.pre.pixels)return reject(Failure::Target);
 const auto&t=base.target;if(nfs_small_logo_live::overlap(a.binding.tableAddress,16,t.w[8],16+Full*2)||nfs_small_logo_live::overlap(a.binding.row,16,t.w[8],16+Full*2))return reject(Failure::Target);
 try{
  std::vector<uint16_t>replay(base.pixels.begin(),base.pixels.end());std::vector<uint8_t>coverage(Full);
  const std::array<uint32_t,9>table{{0xf0001aa0,0xf0001aa4,0xf0001aa8,0xf0001aa8,0xf0001aac,0xf0001ab0,0,0,0}};
  for(unsigned k=0;k<2;++k){const auto&f=i.fills[k];const uint32_t x=k?366:367,y=k?141:142,rawFill=k?raw:0xff000000u;const uint16_t pixel=pack(rawFill);const uint32_t pattern=uint32_t(pixel)|(uint32_t(pixel)<<16);
   // Selected clipping is half-open. A completed fill can invoke no row callback;
   // that is an authenticated no-write event, not a fabricated full swatch.
   const uint32_t left=std::max(x,t.w[3]),top=std::max(y,t.w[4]),right=std::min(x+84,t.w[5]),bottom=std::min(y+21,t.w[6]);
   const uint32_t rows=left<right&&top<bottom?bottom-top:0,width=rows?right-left:0;
   if(!nfs_logo_capture::same(f.before,t)||!nfs_logo_capture::same(f.after,t)||f.call!=std::array<uint32_t,4>{{x,y,84,21}}||f.rawColor!=rawFill||f.pattern!=pattern||f.callback!=(rows?FillCallback:0)||f.format!=16||f.converter||f.complete!=1||f.error||f.tableBefore!=table||f.tableAfter!=table||f.rows.size()!=rows)return reject(Failure::Fill);
   for(unsigned yy=0;yy<rows;++yy){const auto&r=f.rows[yy];const uint32_t dest=t.w[8]+16+((top+yy)*640+left)*2;
    if(r.callback!=FillCallback||r.format!=16||r.converter||r.pattern!=pattern||r.args[1]!=dest||r.args[2]!=width||r.before.size()!=width||r.after.size()!=width)return reject(Failure::Rows);
    for(unsigned xx=0;xx<width;++xx){const size_t at=(top+yy)*640+left+xx;if(r.before[xx]!=replay[at]||r.after[xx]!=pixel)return reject(Failure::Pixels);replay[at]=pixel;coverage[at]=255;}
   }
  }
  if(!std::equal(replay.begin(),replay.end(),i.post.pixels.begin()))return reject(Failure::Pixels);
  if(i.post.pixels!=current.pixels)return reject(Failure::Current);
  std::shared_ptr<Packet>p(new Packet);p->binding_=a.binding;p->events_=a.events;p->target_=t;p->raw_=raw;p->post_=std::move(replay);p->pixels_.reserve(85*22);p->coverage_.reserve(85*22);
  for(unsigned y=141;y<163;++y)for(unsigned x=366;x<451;++x){p->pixels_.push_back(p->post_[y*640+x]);p->coverage_.push_back(coverage[y*640+x]);}return p;
 }catch(...){return reject(Failure::Allocation);}
}
} // namespace nfs_player_swatch
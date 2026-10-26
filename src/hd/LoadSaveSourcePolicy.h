// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include <array>
#include <memory>
#include <vector>
#include <string>
// Pure owned Load/Save evidence. No guest reads, live issuer, renderer activation,
// file actions or input. Caller-supplied profiles are consistency assumptions,
// not authenticated original fonts; no production profile is registered here.
namespace nfs_loadsave_source {
constexpr uint32_t FormSymbol=0x4d39a8,HandlerSymbol=0x40938c,FormType=9;
constexpr size_t Words=8,Commands=16,Pixels=640*480,MaxFontBytes=262144;
inline const char*text(size_t i)noexcept{static const char*const v[]={"save tournament","load tournament","delete tournament","save replay","view replay","delete replay","DONE","LOAD/SAVE"};return i<Words?v[i]:"";}
inline uint32_t resource(size_t i)noexcept{return i<7?0x179+uint32_t(i):i==7?0x178:0;}
inline uint32_t descriptorSymbol(size_t i)noexcept{static const uint32_t v[]={0x4d3908,0x4d391c,0x4d3930,0x4d3944,0x4d3958,0x4d396c,0x4d3980,0x4d3994};return i<Words?v[i]:0;}
inline int top(size_t i)noexcept{static const int v[]={144,170,196,222,248,274,317,105};return i<Words?v[i]:-1;}
enum class Style:unsigned {normal,selected,disabled,heading};
inline uint16_t rgb565(uint32_t c)noexcept{return uint16_t(((c>>8)&0xf800)|((c>>5)&0x7e0)|((c>>3)&31));}
inline uint16_t color(Style s)noexcept{return rgb565(s==Style::heading?0x08f0f0:s==Style::disabled?0x484040:s==Style::selected?0xffffff:0x908080);}
struct Owner {
 uint32_t formSymbol=0,handlerSymbol=0,type=0,runtimeForm=0,archive=0,targetOwner=0;
 uintptr_t game=0,thread=0;uint64_t epoch=0,lifetime=0,serial=0,context=0;
 unsigned depth=0;bool nested=false,retired=false,unwound=false;
 bool valid()const noexcept{return formSymbol==FormSymbol&&handlerSymbol==HandlerSymbol&&type==FormType&&runtimeForm&&archive&&targetOwner&&game&&thread&&epoch&&lifetime&&serial&&context&&depth==1&&!nested&&!retired&&!unwound;}
 bool operator==(const Owner&o)const noexcept{return formSymbol==o.formSymbol&&handlerSymbol==o.handlerSymbol&&type==o.type&&runtimeForm==o.runtimeForm&&archive==o.archive&&targetOwner==o.targetOwner&&game==o.game&&thread==o.thread&&epoch==o.epoch&&lifetime==o.lifetime&&serial==o.serial&&context==o.context&&depth==o.depth&&nested==o.nested&&retired==o.retired&&unwound==o.unwound;}
};
struct Row {uint32_t symbol=0,descriptor=0,textGuest=0,descriptorWord=0;unsigned inputCategory=0,flags=0;};
struct Authority {Owner owner{};uint32_t table=0;uint16_t selected=0;std::array<Row,Words>rows{};};
inline bool authority(const Authority&a)noexcept{
 if(!a.owner.valid()||!a.table||a.selected>=7)return false; // Only the seven action rows are selectable.
 for(size_t i=0;i<Words;++i){const auto&r=a.rows[i];if(r.symbol!=descriptorSymbol(i)||!r.descriptor||!r.textGuest||r.descriptorWord!=(resource(i)<<16|(i==7?0u:2u))||r.inputCategory!=(i==7?0u:2u)||r.flags>1||(r.flags&&i!=0&&i!=3))return false;for(size_t j=0;j<i;++j)if(r.descriptor==a.rows[j].descriptor||r.textGuest==a.rows[j].textGuest)return false;}
 return true;
}
inline Style style(const Authority&a,size_t i)noexcept{return i==7?Style::heading:a.rows[i].flags?Style::disabled:a.selected==i?Style::selected:Style::normal;}
struct Glyph {uint32_t offset=0,width=0,height=0,pitch=0,advance=0;};
// Owned original-source profile and palette bytes. A later audited live adapter
// must authenticate these against an independently supported source identity.
struct Profile {
 std::vector<uint8_t>fontBytes;std::array<Glyph,96>glyphs{};
 std::array<std::array<uint32_t,16>,8>palettes{}; // 2*Style + shadow(0)/foreground(1)
 std::array<unsigned,Words>roles{};uint64_t identity=0;
 std::shared_ptr<const nfs_soft::FontCatalog>displayFont;
 bool valid()const noexcept{
  if(!identity||!displayFont||fontBytes.empty()||fontBytes.size()>MaxFontBytes)return false;
  for(auto role:roles)if(role>1)return false;
  for(const auto&g:glyphs)if(!g.width||g.width>64||!g.height||g.height>64||g.pitch!=(g.width+1)/2||!g.advance||g.advance>64||uint64_t(g.offset)+uint64_t(g.pitch)*g.height>fontBytes.size())return false;
  return true;
 }
};
// No captured exact Load/Save source font/state has yet been admitted. This
// explicit barrier is not a live authorization or a caller-set readiness flag.
inline bool supportedOriginalProfile(const Profile&)noexcept{return false;}
struct Snapshot {
 Owner owner{};nfs_soft::SurfaceIdentity surface{};uint64_t profileIdentity=0;
 uint32_t font=0,palette=0,callbackSymbol=0,mode=0,format=0,flags=0;
 int originX=0,originY=0,size=0;std::vector<uint8_t>fontBytes;
 bool valid(const Profile&p,const Owner&o)const noexcept{return owner==o&&owner.valid()&&surface.valid()&&surface.lifetime==o.lifetime&&surface.geometry[1]==640&&surface.geometry[2]==480&&!surface.geometry[3]&&!surface.geometry[4]&&surface.geometry[5]==640&&surface.geometry[6]==480&&profileIdentity==p.identity&&font&&palette&&callbackSymbol==0x4a5830&&!mode&&format==16&&!flags&&!originX&&!originY&&size==18&&fontBytes==p.fontBytes;}
};
// Exact source callback arithmetic from opaque/unclipped 16-bit4A5830 branch.
// Palette entries below0x10000000 perform no store and are represented by omission.
inline uint16_t blend(uint16_t before,uint32_t p)noexcept{
 if(p<0xfc000000u){uint32_t packed=(((uint32_t(before)<<16)|(before&255u))&0xf800001fu)+((before&0x7e0u)<<8);uint64_t product=uint64_t(packed)*(255u-(p>>24));uint32_t low=uint32_t(product);p+=uint32_t(product>>32)<<16;p+=(low>>5)&255u;p+=(low>>11)&0xff00u;}
 return uint16_t(((p&0xf80000u)>>8)+((p>>3)&31u)+((p&0xfc00u)>>5));
}
struct Store {uint32_t index=0;uint16_t before=0,computed=0,after=0;};
struct GlyphUse {uint8_t code=0;int x=0,y=0;std::vector<Store>stores;};
struct String {
 size_t word=Words;bool shadow=false;uint32_t descriptor=0,textGuest=0;unsigned resourcePolicy=0;
 int measuredAdvance=0;bool resolved=false,alignmentObserved=false,measured=false;
 Snapshot before{},after{};std::array<uint32_t,16>paletteWords{};std::vector<GlyphUse>glyphs;
};
struct Evidence {Authority entry{},exit{};Snapshot post{};std::vector<String>strings;};
inline bool sameAuthority(const Authority&a,const Authority&b)noexcept{
 if(!(a.owner==b.owner)||a.table!=b.table||a.selected!=b.selected)return false;
 for(size_t i=0;i<Words;++i){const auto&x=a.rows[i];const auto&y=b.rows[i];if(x.symbol!=y.symbol||x.descriptor!=y.descriptor||x.textGuest!=y.textGuest||x.descriptorWord!=y.descriptorWord||x.inputCategory!=y.inputCategory||x.flags!=y.flags)return false;}return true;
}
// Historical pure verification only: caller must prove current source ownership,
// full bitmap header/LUTs, source profile provenance and GPU upload independently.
inline bool consistent(const std::shared_ptr<const nfs_soft::FramePacket>&packet,const Evidence&e,const Profile&p,const Authority&current,const uint16_t*currentPlane,size_t count)noexcept{
 if(!p.valid()||!authority(current)||!sameAuthority(e.entry,current)||!sameAuthority(e.entry,e.exit)||!e.post.valid(p,current.owner)||!packet||packet->font()!=p.displayFont||packet->serial()!=current.owner.serial||!(packet->surface()==e.post.surface)||packet->pixels()!=Pixels||packet->commands()!=Commands||packet->records().size()!=Commands||e.strings.size()!=Commands||!packet->writes()||!packet->matches(e.post.surface,current.owner.serial,currentPlane,count))return false;
 try{
  auto replay=packet->clean();if(replay.size()!=Pixels)return false;size_t writes=0,vertices=0;
  for(size_t k=0;k<Commands;++k){size_t i=k/2;bool shadow=k%2==0;const auto&s=e.strings[k];const auto&r=packet->records()[k];const auto&c=r.command;size_t len=std::strlen(text(i));
   if(s.word!=i||s.shadow!=shadow||s.descriptor!=current.rows[i].descriptor||s.textGuest!=current.rows[i].textGuest||s.resourcePolicy!=4||!s.resolved||!s.alignmentObserved||!s.measured||!s.before.valid(p,current.owner)||!s.after.valid(p,current.owner)||!(s.before.surface==e.post.surface)||!(s.after.surface==e.post.surface)||s.before.font!=e.post.font||s.after.font!=e.post.font||s.before.palette!=e.post.palette||s.after.palette!=e.post.palette||s.paletteWords!=p.palettes[2*unsigned(style(current,i))+!shadow]||s.glyphs.size()!=len||c.length!=len||std::memcmp(c.text.data(),text(i),len)||c.role!=p.roles[i]||c.alignment!=2||c.sourceSize!=18||c.originX||c.originY||c.clip.left||c.clip.top||c.clip.right!=640||c.clip.bottom!=480||c.y!=top(i)+int(shadow)||c.color!=(shadow?uint16_t(1):color(style(current,i)))||r.vertexFirst!=vertices||!r.vertexCount||r.vertexCount%6)return false;
   for(size_t n=len;n<c.text.size();++n)if(c.text[n])return false;
   int advance=0;for(unsigned char code:std::string(text(i))){int at=nfs_soft::index(code);if(at<0)return false;advance+=int(p.glyphs[size_t(at)].advance);}
   if(s.measuredAdvance!=advance||c.advance!=advance||c.x!=426-advance+int(shadow))return false;
   int pen=c.x;for(size_t n=0;n<len;++n){const auto&use=s.glyphs[n];if(use.code!=uint8_t(text(i)[n])||use.x!=pen||use.y!=c.y)return false;int at=nfs_soft::index(use.code);if(at<0)return false;const auto&g=p.glyphs[size_t(at)];if(pen<0||uint64_t(pen)+g.width>640||c.y<0||uint64_t(c.y)+g.height>480)return false;size_t store=0;
    for(unsigned y=0;y<g.height;++y)for(unsigned x=0;x<g.width;++x){uint8_t byte=p.fontBytes[g.offset+y*g.pitch+x/2];unsigned nibble=x%2?byte&15:byte>>4;uint32_t palette=s.paletteWords[nibble];if(palette<0x10000000u)continue;if(store>=use.stores.size())return false;const auto&w=use.stores[store++];size_t index=(size_t(c.y)+y)*640+size_t(pen)+x;uint16_t computed=blend(replay[index],palette);if(w.index!=index||w.before!=replay[index]||w.computed!=computed||w.after!=computed)return false;replay[index]=computed;if(++writes>nfs_soft::FramePacketBuilder::maxWrites)return false;}
    if(store!=use.stores.size())return false;
    pen+=int(g.advance);
   }
   vertices+=r.vertexCount;if(vertices>packet->vertices().size())return false;
  }
  return writes==packet->writes()&&vertices==packet->vertices().size()&&replay==packet->original();
 }catch(...){return false;}
}
} // namespace nfs_loadsave_source

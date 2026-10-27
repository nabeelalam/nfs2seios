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
// Original FFN record: code:u16, w/h:u8, source x/y:u16,
// advance/bearingX/bearingY:i8. Source bytes retain the shared atlas.
struct Glyph {uint16_t code=0;uint32_t sourceX=0,sourceY=0,width=0,height=0;int advance=0,bearingX=0,bearingY=0;uint32_t recordOffset=0;};
// Owned original-source profile and palette bytes. A later audited live adapter
// must authenticate these against an independently supported source identity.
struct Profile {
 std::vector<uint8_t>fontBytes,glyphRecords;std::array<Glyph,96>glyphs{};
 uint32_t atlasWidth=0,atlasHeight=0,atlasStride=0,kerningCount=0;
 std::array<std::array<uint32_t,16>,8>palettes{};
 std::array<bool,8>paletteObserved{}; // Unobserved styles remain declined.
 std::array<unsigned,Words>roles{};uint64_t identity=0;
 std::shared_ptr<const nfs_soft::FontCatalog>displayFont;
 bool valid()const noexcept{
  if(glyphRecords.empty()||glyphRecords.size()%11||glyphRecords.size()>4096*11||!identity||!displayFont||!atlasWidth||atlasWidth>4096||!atlasHeight||atlasHeight>4096||atlasStride!=(atlasWidth+1)/2||kerningCount||fontBytes.size()!=16+uint64_t(atlasStride)*atlasHeight||fontBytes.size()>MaxFontBytes)return false;
  auto word=[&](size_t at){return uint32_t(fontBytes[at])|uint32_t(fontBytes[at+1])<<8|uint32_t(fontBytes[at+2])<<16|uint32_t(fontBytes[at+3])<<24;};
  if(word(0)!=0x7a||word(4)!=(atlasWidth|(atlasHeight<<16))||word(8)||word(12))return false;
  for(auto role:roles)if(role>1)return false;
  for(size_t i=0;i<glyphs.size();++i){const auto&g=glyphs[i];if(!g.code)continue;uint64_t rounded=(uint64_t(g.width)+1)/2*2;
   if(g.code!=i+32||!g.width||g.width>64||!g.height||g.height>64||g.advance<=0||g.advance>127||g.bearingX< -128||g.bearingX>127||g.bearingY< -128||g.bearingY>127||g.sourceX%2||g.sourceX+rounded>atlasWidth||uint64_t(g.sourceY)+g.height>atlasHeight)return false;
   if(g.recordOffset%11||uint64_t(g.recordOffset)+11>glyphRecords.size())return false;
   const auto*r=glyphRecords.data()+g.recordOffset;
   if((uint16_t(r[0])|uint16_t(r[1])<<8)!=g.code||r[2]!=g.width||r[3]!=g.height||(uint32_t(r[4])|uint32_t(r[5])<<8)!=g.sourceX||(uint32_t(r[6])|uint32_t(r[7])<<8)!=g.sourceY||int(int8_t(r[8]))!=g.advance||int(int8_t(r[9]))!=g.bearingX||int(int8_t(r[10]))!=g.bearingY)return false;
  }return true;
 }
};
// Actual source captures inform this model; no production registration exists. This
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
 if(p<0xfc000000u){uint32_t packedColor=(((uint32_t(before)<<16)|(before&255u))&0xf800001fu)+((before&0x7e0u)<<8);uint64_t product=uint64_t(packedColor)*(255u-(p>>24));uint32_t low=uint32_t(product);p+=uint32_t(product>>32)<<16;p+=(low>>5)&255u;p+=(low>>11)&0xff00u;}
 return uint16_t(((p&0xf80000u)>>8)+((p>>3)&31u)+((p&0xfc00u)>>5));
}
struct Store {uint32_t index=0;uint16_t before=0,computed=0,after=0;
 uint32_t sourceOffset=0,paletteValue=0;uint8_t sourceByte=0,sourceParity=0;
};
struct GlyphUse {uint8_t code=0;size_t textIndex=0;int x=0,y=0;
 uint32_t sourceX=0,sourceY=0,width=0,height=0,stride=0;std::vector<Store>stores;
};
// Both retained slots stay protected even when the source selector alternates.
struct Target {
 uint32_t owner=0;uint64_t lifetime=0;std::array<uint32_t,14>descriptor{};
 std::array<uint32_t,4>header{};std::vector<uint32_t>x,y;
 bool valid(unsigned width)const noexcept{
  if(!owner||!lifetime||descriptor[0]!=0x57494e44||descriptor[1]!=width||descriptor[2]!=480||descriptor[3]||descriptor[4]||descriptor[5]!=width||descriptor[6]!=480||(descriptor[7]&255)!=16||!descriptor[8]||descriptor[10]!=2*width||!descriptor[11]||!descriptor[12]||descriptor[13]||header!=std::array<uint32_t,4>{{0x78,width|(480u<<16),0,0}}||x.size()!=width||y.size()!=480)return false;
  for(size_t i=0;i<x.size();++i)if(x[i]!=2*i)return false;
  for(size_t i=0;i<y.size();++i)if(y[i]!=16+2*width*i)return false;
  return true;
 }
 bool operator==(const Target&o)const noexcept{return owner==o.owner&&lifetime==o.lifetime&&descriptor==o.descriptor&&header==o.header&&x==o.x&&y==o.y;}
};
struct Targets {
 Target full{};std::array<Target,2>slots{};unsigned selected=2;uint32_t selectedOwner=0;
 bool valid(const Owner&o)const noexcept{return selected<2&&full.valid(640)&&full.owner==o.targetOwner&&full.lifetime==o.lifetime&&slots[0].valid(343)&&slots[1].valid(343)&&slots[0].owner!=slots[1].owner&&full.owner!=slots[0].owner&&full.owner!=slots[1].owner&&slots[0].descriptor[8]!=slots[1].descriptor[8]&&selectedOwner==slots[selected].owner;}
 bool resourcesEqual(const Targets&o)const noexcept{return full==o.full&&slots==o.slots;}
};
struct String {
 size_t word=Words;bool shadow=false;uint32_t descriptor=0,textGuest=0;unsigned resourcePolicy=0;
 int measuredAdvance=0;bool resolved=false,alignmentObserved=false,measured=false;
 Snapshot before{},after{};std::array<uint32_t,16>paletteWords{};std::vector<GlyphUse>glyphs;
};
struct Evidence {Authority entry{},exit{};Snapshot post{};std::vector<String>strings;
 Targets targetsBefore{},targetsAfter{};
 std::vector<uint16_t>fullBeforeCopy,retainedCopy,fullAfterCopy;
};
inline bool sameAuthority(const Authority&a,const Authority&b)noexcept{
 if(!(a.owner==b.owner)||a.table!=b.table||a.selected!=b.selected)return false;
 for(size_t i=0;i<Words;++i){const auto&x=a.rows[i];const auto&y=b.rows[i];if(x.symbol!=y.symbol||x.descriptor!=y.descriptor||x.textGuest!=y.textGuest||x.descriptorWord!=y.descriptorWord||x.inputCategory!=y.inputCategory||x.flags!=y.flags)return false;}return true;
}
// Historical pure verification only: caller must prove current source ownership,
// full bitmap header/LUTs, source profile provenance and GPU upload independently.
inline bool consistent(const std::shared_ptr<const nfs_soft::FramePacket>&packet,const Evidence&e,const Profile&p,const Authority&current,const uint16_t*currentPlane,size_t count)noexcept{
 if(!p.valid()||!e.targetsBefore.valid(current.owner)||!e.targetsAfter.valid(current.owner)||!e.targetsBefore.resourcesEqual(e.targetsAfter)||!authority(current)||!sameAuthority(e.entry,current)||!sameAuthority(e.entry,e.exit)||!e.post.valid(p,current.owner)||!packet||packet->font()!=p.displayFont||packet->serial()!=current.owner.serial||!(packet->surface()==e.post.surface)||packet->pixels()!=Pixels||packet->commands()!=Commands||packet->records().size()!=Commands||e.strings.size()!=Commands||!packet->writes()||!packet->matches(e.post.surface,current.owner.serial,currentPlane,count))return false;
 try{
  auto replay=packet->clean();if(replay.size()!=Pixels||e.fullBeforeCopy.size()!=Pixels||e.fullAfterCopy!=replay||e.retainedCopy.size()!=343*480)return false;
  auto copied=e.fullBeforeCopy;for(size_t y=0;y<480;++y)for(size_t x=0;x<343;++x){auto v=e.retainedCopy[y*343+x];if(v)copied[y*640+x]=v;}if(copied!=replay)return false;
  for(size_t j=0;j<14;++j)if(e.targetsBefore.full.descriptor[j]!=e.post.surface.geometry[j])return false;
  size_t writes=0,vertices=0;
  for(size_t k=0;k<Commands;++k){size_t i=k/2;bool shadow=k%2==0;const auto&s=e.strings[k];const auto&r=packet->records()[k];const auto&c=r.command;size_t len=std::strlen(text(i));size_t callbacks=0;for(size_t n=0;n<len;++n)if(uint8_t(text(i)[n])>32)++callbacks;size_t paletteIndex=2*unsigned(style(current,i))+!shadow;
   if(s.word!=i||s.shadow!=shadow||s.descriptor!=current.rows[i].descriptor||s.textGuest!=current.rows[i].textGuest||s.resourcePolicy!=4||!s.resolved||!s.alignmentObserved||!s.measured||!s.before.valid(p,current.owner)||!s.after.valid(p,current.owner)||!(s.before.surface==e.post.surface)||!(s.after.surface==e.post.surface)||s.before.font!=e.post.font||s.after.font!=e.post.font||s.before.palette!=e.post.palette||s.after.palette!=e.post.palette||!p.paletteObserved[paletteIndex]||s.paletteWords!=p.palettes[paletteIndex]||s.glyphs.size()!=callbacks||c.length!=len||std::memcmp(c.text.data(),text(i),len)||c.role!=p.roles[i]||c.alignment!=2||c.sourceSize!=18||c.originX||c.originY||c.clip.left||c.clip.top||c.clip.right!=640||c.clip.bottom!=480||c.y!=top(i)+int(shadow)||c.color!=(shadow?uint16_t(1):color(style(current,i)))||r.vertexFirst!=vertices||!r.vertexCount||r.vertexCount%6)return false;
   for(size_t n=len;n<c.text.size();++n)if(c.text[n])return false;
   int advance=0;for(unsigned char code:std::string(text(i))){int at=nfs_soft::index(code);if(at<0)return false;if(p.glyphs[size_t(at)].code!=code)return false;advance+=int(p.glyphs[size_t(at)].advance);}
   if(s.measuredAdvance!=advance||c.advance!=advance||c.x!=426-advance+int(shadow))return false;
   int pen=c.x;size_t callback=0;for(size_t n=0;n<len;++n){uint8_t code=uint8_t(text(i)[n]);int at=nfs_soft::index(code);if(at<0)return false;const auto&g=p.glyphs[size_t(at)];
    if(code==32){pen+=g.advance;continue;}
    const auto&use=s.glyphs[callback++];int dx=pen+g.bearingX,dy=c.y+g.bearingY;unsigned rounded=(g.width+1)/2*2;
    if(use.code!=code||use.textIndex!=n||use.x!=dx||use.y!=dy||use.sourceX!=g.sourceX||use.sourceY!=g.sourceY||use.width!=g.width||use.height!=g.height||use.stride!=p.atlasStride||dx<0||dy<0||uint64_t(dx)+rounded>640||uint64_t(dy)+g.height>480)return false;
    size_t store=0;for(unsigned y=0;y<g.height;++y)for(unsigned x=0;x<rounded;++x){uint32_t offset=16+(g.sourceY+y)*p.atlasStride+(g.sourceX+x)/2;uint8_t byte=p.fontBytes[offset];unsigned parity=(g.sourceX+x)%2,nibble=parity?byte&15:byte>>4;uint32_t palette=s.paletteWords[nibble];if(palette<0x10000000u)continue;if(store>=use.stores.size())return false;const auto&w=use.stores[store++];size_t index=(size_t(dy)+y)*640+size_t(dx)+x;uint16_t computed=blend(replay[index],palette);
     if(w.index!=index||w.before!=replay[index]||w.computed!=computed||w.after!=computed||w.sourceOffset!=offset||w.sourceParity!=parity||w.sourceByte!=byte||w.paletteValue!=palette)return false;
     replay[index]=computed;if(++writes>nfs_soft::FramePacketBuilder::maxWrites)return false;
    }
    if(store!=use.stores.size())return false;
    pen+=g.advance;
   }
   vertices+=r.vertexCount;if(vertices>packet->vertices().size())return false;
  }
  return writes==packet->writes()&&vertices==packet->vertices().size()&&replay==packet->original();
 }catch(...){return false;}
}
} // namespace nfs_loadsave_source

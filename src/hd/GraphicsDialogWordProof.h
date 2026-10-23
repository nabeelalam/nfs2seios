// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include "OriginalUiRecords.h"
namespace nfs_graphics_text {struct TailScope;}
namespace nfs_graphics_dialog_word {
constexpr unsigned Words=8,Commands=32;
inline const char*text(unsigned i)noexcept{static const char*const names[]={"car detail (F2)","brightness (F10)","view distance (F3)","horizon (F4)","reset","DONE","CANCEL","GRAPHICS"};return i<Words?names[i]:"";}
inline unsigned width(unsigned i)noexcept{static const unsigned v[]={90,105,120,76,33,35,51,68};return i<Words?v[i]:0;}
inline unsigned left(unsigned i)noexcept{return i==6?466:i<Words?426-width(i):0;}
inline unsigned top(unsigned i)noexcept{static const unsigned v[]={144,170,196,222,274,317,317,105};return i<Words?v[i]:0;}
inline unsigned record(unsigned i)noexcept{static const unsigned v[]={0,6,12,18,24,26,28,30};return i<Words?v[i]:Commands;}
inline unsigned vertices(unsigned i)noexcept{static const unsigned v[]={78,90,96,66,30,24,36,48};return i<Words?v[i]:0;}
inline unsigned role(unsigned i)noexcept{return i<5?1:0;}
// Recorded full-text alignment remains its original capture hint. Resource
// policy below is independently observed from the original table and measure.
inline unsigned alignment(unsigned i)noexcept{return i==6?0:2;}
inline unsigned resource(unsigned i)noexcept{static const unsigned v[]={0x162,0x164,0x166,0x167,0x169,0x16a,0x16b,0x161};return i<Words?v[i]:0;}
inline unsigned anchor(unsigned i)noexcept{return i==6?466:426;}
inline unsigned resourceAlignment(unsigned i)noexcept{return i==6?3:4;}
inline unsigned measureKind(unsigned i)noexcept{return resourceAlignment(i)%3==1?2:resourceAlignment(i)%3==2?1:0;}
struct Snapshot {nfs_logo_capture::Target target{};nfs_soft::SurfaceIdentity identity{};uint32_t archive=0,font=0,callback=0,palette=0,color=0,mode=0,flags=0,format=0;int32_t originX=0,originY=0,size=0;std::array<uint32_t,16>paletteWords{};std::array<uint8_t,16>fontHeader{};};
struct Authority {uint32_t archive=0,selector=0,entry=0,entryWord=0,table=0,selectedIndex=0;std::array<uint32_t,Words>descriptors{},words{};std::array<std::array<uint8_t,12>,Words>headers{};std::array<uint32_t,Words>texts{};bool operator==(const Authority&o)const noexcept{return archive==o.archive&&selector==o.selector&&entry==o.entry&&entryWord==o.entryWord&&table==o.table&&selectedIndex==o.selectedIndex&&descriptors==o.descriptors&&words==o.words&&headers==o.headers&&texts==o.texts;}};
struct Resource {uint32_t descriptor=0,descriptorWord=0,textGuest=0,inputCategory=0;int32_t anchor=0,y=0,measuredWidth=0;unsigned strings=0,measureKind=0,resourcePolicy=0;bool alignmentObserved=false,resolved=false,measured=false,complete=false;Snapshot before{},resolvedState{},measure{},after{};};
struct Evidence {uint32_t form=0;uintptr_t game=0,thread=0;uint64_t serial=0,lifetime=0;Authority authority{};Snapshot post{};std::array<Resource,Words>resources{};};
inline bool command(const nfs_soft::TextCommand&c,unsigned i,unsigned shadow)noexcept{
 const auto length=std::strlen(text(i));if(c.length!=length||std::memcmp(c.text.data(),text(i),length)||c.role!=role(i)||c.alignment!=alignment(i)||c.sourceSize!=18||c.x!=int(left(i)+shadow)||c.y!=int(top(i)+shadow)||c.advance!=int(width(i))||c.originX||c.originY||c.clip.left||c.clip.top||c.clip.right!=640||c.clip.bottom!=480)return false;for(size_t k=length;k<c.text.size();++k)if(c.text[k])return false;
 return shadow?c.color==1:i==7?c.color==3998:(c.color==37904||c.color==65535);
}
inline bool consistent(const std::shared_ptr<const nfs_soft::FramePacket>&p,const Evidence&e)noexcept{
 if(!p||!e.form||!e.game||!e.thread||!e.serial||!e.lifetime||p->serial()!=e.serial||p->surface().lifetime!=e.lifetime||p->pixels()!=640*480||p->commands()!=Commands||p->records().size()!=Commands||!p->writes()||e.authority.archive!=e.post.archive||!(p->surface()==e.post.identity))return false;
 size_t end=0;for(const auto&r:p->records()){if(r.vertexFirst!=end||!r.vertexCount||r.vertexCount%6||r.command.sourceSize!=18||r.command.originX||r.command.originY||r.command.clip.left||r.command.clip.top||r.command.clip.right!=640||r.command.clip.bottom!=480)return false;end+=r.vertexCount;if(end>p->vertices().size())return false;}if(end!=p->vertices().size())return false;
 for(unsigned i=0;i<Words;++i){const auto&r=e.resources[i];if(!r.complete||!r.resolved||!r.alignmentObserved||r.resourcePolicy!=resourceAlignment(i)||r.measured!=bool(measureKind(i))||r.measureKind!=measureKind(i)||r.strings!=2||r.descriptor!=e.authority.descriptors[i]||r.descriptorWord!=e.authority.words[i]||(r.descriptorWord>>16)!=resource(i)||r.textGuest!=e.authority.texts[i]||r.anchor!=int(anchor(i))||r.y!=int(top(i))||r.measuredWidth!=int(measureKind(i)?width(i):0)||r.inputCategory>2)return false;for(const auto*s:{&r.before,&r.resolvedState,&r.after})if(s->archive!=e.authority.archive||!(s->identity==p->surface()))return false;if(r.measured&&(r.measure.archive!=e.authority.archive||!(r.measure.identity==p->surface())))return false;for(unsigned j=0;j<2;++j){const auto&c=p->records()[record(i)+j];if(!command(c.command,i,j==0)||c.vertexCount!=vertices(i))return false;}}
 return true;
}
class Proof final {std::shared_ptr<const nfs_soft::FramePacket>packet_;Evidence evidence_;Proof(std::shared_ptr<const nfs_soft::FramePacket>p,const Evidence&e):packet_(std::move(p)),evidence_(e){}friend struct ::nfs_graphics_text::TailScope;
public:const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}const Evidence&evidence()const noexcept{return evidence_;}bool current()const noexcept{return consistent(packet_,evidence_);} // Historical consistency; no current guest or GPU authority.
};
}

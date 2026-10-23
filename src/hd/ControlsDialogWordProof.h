// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include "OriginalUiRecords.h"
namespace nfs_controls_text {struct TailScope;}
namespace nfs_controls_dialog_word {
constexpr unsigned Words=5,Commands=16,VertexCount=696;
// Source draw order differs from authored image order. Ellipses remain separate.
inline const char*text(unsigned i)noexcept{static const char*const v[]={"setup","steering sensitivity","force feedback","DONE","CONTROLS"};return i<Words?v[i]:"";}
inline unsigned artId(unsigned i)noexcept{static const unsigned v[]={2,3,4,5,1};return i<Words?v[i]:0;}
inline unsigned resource(unsigned i)noexcept{static const unsigned v[]={0x148,0x149,0x14a,0x14b,0x145};return i<Words?v[i]:0;}
inline unsigned width(unsigned i)noexcept{static const unsigned v[]={36,123,96,35,68};return i<Words?v[i]:0;}
inline unsigned left(unsigned i)noexcept{return i<Words?416-(i==4?width(i)/2:width(i)):0;}
inline unsigned top(unsigned i)noexcept{static const unsigned v[]={170,196,224,267,105};return i<Words?v[i]:0;}
inline unsigned record(unsigned i)noexcept{static const unsigned v[]={0,4,8,12,14};return i<Words?v[i]:Commands;}
inline unsigned vertices(unsigned i)noexcept{static const unsigned v[]={30,114,78,24,48};return i<Words?v[i]:0;}
inline unsigned role(unsigned i)noexcept{return i<3?1:0;}
inline unsigned alignment(unsigned i)noexcept{return i==4?1:2;}
inline unsigned resourceAlignment(unsigned i)noexcept{return i==4?5:4;}
inline unsigned measureKind(unsigned i)noexcept{return alignment(i);}
inline unsigned anchor(unsigned i)noexcept{return i<Words?416:0;}
inline unsigned descriptorFlags(unsigned i)noexcept{return i==2?0x1502:i<2?2:0;}
inline bool supportedSelection(unsigned selected)noexcept{return selected<4;}
struct Snapshot {nfs_logo_capture::Target target{};nfs_soft::SurfaceIdentity identity{};uint32_t archive=0,font=0,callback=0,palette=0,color=0,mode=0,flags=0,format=0;int32_t originX=0,originY=0,size=0;std::array<uint32_t,16>paletteWords{};std::array<uint8_t,16>fontHeader{};};
// These compiled guest identities are supplied by the actual Controls collector
// from guest_address(symbol), never inferred from resource numbers or PC offsets.
struct SourceIdentity {
 uint32_t form=0,parentMenu=0,parentDescriptor=0,parentEntry=0,defaultList=0,glyphCallback=0;
 std::array<uint32_t,Words>descriptors{};std::array<uint32_t,3>children{};
 bool operator==(const SourceIdentity&o)const noexcept{return form==o.form&&parentMenu==o.parentMenu&&parentDescriptor==o.parentDescriptor&&parentEntry==o.parentEntry&&defaultList==o.defaultList&&glyphCallback==o.glyphCallback&&descriptors==o.descriptors&&children==o.children;}
 bool valid()const noexcept{if(!form||!parentMenu||!parentDescriptor||!parentEntry||!defaultList||!glyphCallback)return false;for(unsigned i=0;i<Words;++i){if(!descriptors[i])return false;for(unsigned j=0;j<i;++j)if(descriptors[i]==descriptors[j])return false;}for(auto c:children)if(!c)return false;return true;}
};
struct Authority {
 uint32_t archive=0,table=0,selector=0,entry=0,entryWord=0,parentMenu=0,parentDescriptor=0,parentMenuChild=0,selectedIndex=0,terminator=1;
 std::array<uint32_t,5>parentWords{};
 std::array<uint32_t,Words>descriptors{},texts{};
 std::array<std::array<uint32_t,5>,Words>descriptorWords{};
 std::array<std::array<uint8_t,12>,Words>headers{};
 bool operator==(const Authority&o)const noexcept{return archive==o.archive&&table==o.table&&selector==o.selector&&entry==o.entry&&entryWord==o.entryWord&&parentMenu==o.parentMenu&&parentDescriptor==o.parentDescriptor&&parentMenuChild==o.parentMenuChild&&selectedIndex==o.selectedIndex&&terminator==o.terminator&&parentWords==o.parentWords&&descriptors==o.descriptors&&texts==o.texts&&descriptorWords==o.descriptorWords&&headers==o.headers;}
};
struct Resource {uint32_t descriptor=0,descriptorWord=0,textGuest=0,inputCategory=0;int32_t anchor=0,y=0,measuredWidth=0;unsigned strings=0,measureKind=0,resourcePolicy=0;bool alignmentObserved=false,resolved=false,measured=false,complete=false;std::array<uint8_t,96>resolvedText{};Snapshot before{},resolvedState{},measure{},after{};};
struct Evidence {SourceIdentity source{};uint32_t form=0;uintptr_t game=0,thread=0;uint64_t serial=0,lifetime=0;Authority authority{};Snapshot post{};std::array<Resource,Words>resources{};};
inline uint32_t le32(const uint8_t*p)noexcept{return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
inline int16_t le16(const uint8_t*p)noexcept{return int16_t(uint16_t(p[0])|(uint16_t(p[1])<<8));}
inline bool exactText(const nfs_soft::TextCommand&c,const char*name)noexcept{const size_t n=std::strlen(name);if(c.length!=n||std::memcmp(c.text.data(),name,n))return false;for(size_t k=n;k<c.text.size();++k)if(c.text[k])return false;return true;}
inline bool box(const nfs_soft::TextCommand&c)noexcept{return c.sourceSize==18&&!c.originX&&!c.originY&&!c.clip.left&&!c.clip.top&&c.clip.right==640&&c.clip.bottom==480;}
inline uint16_t foreground(unsigned i,unsigned selected)noexcept{return i==4?3998:i==selected?65535:37904;}
inline bool command(const nfs_soft::TextCommand&c,unsigned i,bool shadow,unsigned selected)noexcept{return i<Words&&supportedSelection(selected)&&exactText(c,text(i))&&box(c)&&c.role==role(i)&&c.alignment==alignment(i)&&c.x==int(left(i)+unsigned(shadow))&&c.y==int(top(i)+unsigned(shadow))&&c.advance==int(width(i))&&c.color==(shadow?1:foreground(i,selected));}
inline bool ellipsis(const nfs_soft::TextCommand&c,unsigned row,bool shadow,unsigned selected)noexcept{return row<3&&supportedSelection(selected)&&exactText(c,"...")&&box(c)&&c.role==1&&c.alignment==2&&c.x==416+int(shadow)&&c.y==int(top(row)+unsigned(shadow))&&c.advance==12&&c.color==(shadow?1:foreground(row,selected));}
inline bool sameSnapshot(const Snapshot&a,const Snapshot&b)noexcept{return nfs_logo_capture::same(a.target,b.target)&&a.identity==b.identity&&a.archive==b.archive&&a.font==b.font&&a.callback==b.callback&&a.palette==b.palette&&a.color==b.color&&a.mode==b.mode&&a.flags==b.flags&&a.format==b.format&&a.originX==b.originX&&a.originY==b.originY&&a.size==b.size&&a.paletteWords==b.paletteWords&&a.fontHeader==b.fontHeader;}
inline bool sameFontCore(const Snapshot&a,const Snapshot&b)noexcept{return a.font==b.font&&a.callback==b.callback&&a.mode==b.mode&&a.flags==b.flags&&a.format==b.format&&a.originX==b.originX&&a.originY==b.originY&&a.size==b.size&&a.fontHeader==b.fontHeader;}
inline bool snapshotValid(const Snapshot&s,const Evidence&e)noexcept{
 if(!s.identity.valid()||s.identity.geometry[1]!=640||s.identity.lifetime!=e.lifetime||s.archive!=e.authority.archive||!s.font||!s.palette||s.font!=e.post.font||s.palette!=e.post.palette||s.fontHeader!=e.post.fontHeader||s.callback!=e.source.glyphCallback||s.mode||s.flags||s.format!=16||s.size!=18||s.originX||s.originY||s.color>65535||!s.target.owner||s.target.ownerState[3]!=s.target.owner)return false;
 const auto&t=s.target;if(t.w[1]!=640||t.w[2]!=480||t.w[3]||t.w[4]||t.w[5]!=640||t.w[6]!=480||t.w[7]!=0x7810||t.w[10]!=1280||t.w[13]||t.header!=std::array<uint32_t,4>{{0x78,640u|(480u<<16),0,0}})return false;
 for(unsigned k=0;k<14;++k)if(s.identity.geometry[k]!=t.w[k])return false;
 for(unsigned k=0;k<640;++k)if(t.x[k]!=2*k)return false;for(unsigned k=0;k<480;++k)if(t.y[k]!=16+1280*k)return false;
 for(unsigned k:{0u,1u,2u,7u,8u,10u,11u,12u,13u})if(t.ownerWords[k]!=t.w[k])return false;
 return nfs_logo_capture::same(t,e.post.target)&&s.identity==e.post.identity;
}
inline bool authorityValid(const Evidence&e)noexcept{
 const auto&s=e.source;const auto&a=e.authority;
 if(!s.valid()||e.form!=s.form||!a.archive||!a.table||a.selector!=6||a.entry!=s.parentEntry||a.entryWord!=0x00070002||a.parentMenu!=s.parentMenu||a.parentMenuChild!=s.parentMenu||a.parentDescriptor!=s.parentDescriptor||a.parentWords!=std::array<uint32_t,5>{{0x00810002,s.form,2,0,s.defaultList}}||!supportedSelection(a.selectedIndex)||a.terminator||a.descriptors!=s.descriptors)return false;
 for(unsigned i=0;i<Words;++i){const auto&w=a.descriptorWords[i];const uint32_t child=i<3?s.children[i]:i==3?UINT32_MAX:0;
  if(w!=std::array<uint32_t,5>{{(resource(i)<<16)|(i==4?0u:2u),child,descriptorFlags(i),0,i==4?0:s.defaultList}})return false;
  const auto&h=a.headers[i];if(h[0]!=18||h[1]!=(i==4?5:1)||h[2]!=(i==4?5:2)||h[3]!=resourceAlignment(i)||le16(h.data()+4)!=416||le16(h.data()+6)!=int(top(i)))return false;
  if(uint64_t(a.table)+12*resource(i)+12>UINT64_C(0x100000000)||!a.texts[i]||uint64_t(a.table)+le32(h.data()+8)!=a.texts[i])return false;
 }
 return true;
}
inline bool consistent(const std::shared_ptr<const nfs_soft::FramePacket>&p,const Evidence&e)noexcept{
 if(!p||!e.game||!e.thread||!e.serial||!e.lifetime||!authorityValid(e)||p->serial()!=e.serial||p->surface().lifetime!=e.lifetime||p->pixels()!=640*480||p->commands()!=Commands||p->records().size()!=Commands||p->vertices().size()!=VertexCount||!p->writes()||!(p->surface()==e.post.identity)||!snapshotValid(e.post,e)||e.post.color!=3998)return false;
 size_t end=0;for(unsigned n=0;n<Commands;++n){const auto&r=p->records()[n];if(r.vertexFirst!=end||!r.vertexCount||r.vertexCount%6)return false;end+=r.vertexCount;if(end>p->vertices().size())return false;}if(end!=VertexCount)return false;
 for(unsigned i=0;i<Words;++i){const auto&r=e.resources[i];const auto&selected=e.authority.selectedIndex;
  if(!r.complete||!r.resolved||!r.measured||!r.alignmentObserved||r.resourcePolicy!=resourceAlignment(i)||r.measureKind!=measureKind(i)||r.strings!=2||r.descriptor!=e.source.descriptors[i]||r.descriptorWord!=e.authority.descriptorWords[i][0]||r.textGuest!=e.authority.texts[i]||r.anchor!=416||r.y!=int(top(i))||r.measuredWidth!=int(width(i))||r.inputCategory!=(i<4&&i==selected?1u:0u)||r.after.color!=foreground(i,selected))return false;
  const auto name=text(i);const size_t length=std::strlen(name);if(std::memcmp(r.resolvedText.data(),name,length))return false;for(size_t k=length;k<r.resolvedText.size();++k)if(r.resolvedText[k])return false;
  for(const auto*s:{&r.before,&r.resolvedState,&r.measure,&r.after})if(!snapshotValid(*s,e))return false;
  if(!sameFontCore(r.resolvedState,r.measure)||!sameFontCore(r.measure,r.after))return false;
  for(unsigned j=0;j<2;++j){const auto&c=p->records()[record(i)+j];if(!command(c.command,i,j==0,selected)||c.vertexCount!=vertices(i))return false;}
  if(i<3)for(unsigned j=0;j<2;++j){const auto&c=p->records()[record(i)+2+j];if(!ellipsis(c.command,i,j==0,selected)||c.vertexCount!=18)return false;}
 }
 return true;
}
struct Current {SourceIdentity source{};uint32_t form=0;uintptr_t game=0,thread=0;uint64_t serial=0,lifetime=0;Authority authority{};Snapshot post{};std::shared_ptr<const nfs_soft::FramePacket>packet;const uint16_t*original565=nullptr;size_t planeWords=0;};
inline bool matchesCurrent(const std::shared_ptr<const nfs_soft::FramePacket>&p,const Evidence&e,const Current&c)noexcept{return consistent(p,e)&&c.packet==p&&c.source==e.source&&c.form==e.form&&c.game==e.game&&c.thread==e.thread&&c.serial==e.serial&&c.lifetime==e.lifetime&&c.authority==e.authority&&sameSnapshot(c.post,e.post)&&p->matches(c.post.identity,c.serial,c.original565,c.planeWords);}
class Proof final {std::shared_ptr<const nfs_soft::FramePacket>packet_;Evidence evidence_;Proof(std::shared_ptr<const nfs_soft::FramePacket>p,const Evidence&e):packet_(std::move(p)),evidence_(e){}friend struct ::nfs_controls_text::TailScope;
public:const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}const Evidence&evidence()const noexcept{return evidence_;}bool current()const noexcept{return consistent(packet_,evidence_);} // Historical consistency only; live acquisition must independently re-read Current.
};
} // namespace nfs_controls_dialog_word

// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundSceneRuntimeAbi.h"
#include "SmallLogoPrepare.h"
#include <vector>
#include <limits>
namespace nfs_background_scene_cpu {
using nfs_logo_capture::Target;using nfs_logo_capture::Plane;
constexpr unsigned Full=307200,Side=164640;constexpr uint32_t MainForm=0x405f61;
struct Identity{uint32_t span,width,height,paletteOffset;std::array<uint8_t,16>header;std::array<uint8_t,32>sha;};
// Trusted compile-time caller data. Validation proves bounds/completeness, not provenance.
struct SourceCatalog {
 std::array<Identity,13> identities{};
 std::array<uint32_t,13> roles{};
 std::array<std::array<uint8_t,32>,2> mapSha{};
 std::array<uint32_t,2> mapRoles{};
 nfs_small_logo_live::SourceCatalog logos;
};
inline bool validCatalog(const SourceCatalog&c) noexcept {
 if(!nfs_small_logo_live::validCatalog(c.logos))return false;
 for(unsigned k=0;k<13;++k){const auto&i=c.identities[k];bool nonzero=false;for(auto b:i.sha)nonzero|=b!=0;
  if(c.roles[k]!=k||!nonzero||!i.width||i.width>343||!i.height||i.height>480||i.span<16||i.span>165696||uint64_t(16)+uint64_t(i.width)*i.height>i.paletteOffset||uint64_t(i.paletteOffset)+1040>i.span||nfs_small_logo_live::le32(i.header.data()+4)!=(i.width|(i.height<<16))||(i.header[0]&127)!=0x7b||(k==0&&(i.width!=343||i.height!=480)))return false;
  for(unsigned j=0;j<k;++j)if(i.sha==c.identities[j].sha)return false;
 }
 for(unsigned k=0;k<2;++k){bool nonzero=false;for(auto b:c.mapSha[k])nonzero|=b!=0;if(c.mapRoles[k]!=k||!nonzero)return false;}
 return c.mapSha[0]!=c.mapSha[1];
}
// ownerEpoch is menu/form/archive lifetime; iteration is the lazy40BC20 redraw epoch (base has0).
struct Event{uint64_t ownerEpoch=0,sequence=0,iteration=0;};
struct Row{uint32_t callback=0,converter=0,unit=0;std::array<uint32_t,4>args{};std::vector<uint8_t>payload;};
struct Draw{uint32_t role=UINT32_MAX,complete=0,error=0,callback=0,callbackGlobal=0,format=0,paletteGuest=0,paletteValid=0;std::array<uint32_t,4>args{};Target before{},after{};std::array<uint32_t,9>tableBefore{},tableAfter{};std::vector<uint8_t>sourceBefore,sourceAfter;std::array<uint32_t,256>paletteBefore{},paletteAfter{};std::vector<Row>rows;};
struct Base{Event event;uint32_t mapRole=UINT32_MAX,form=0;std::vector<uint8_t>sourceBefore,sourceAfter;Plane post;};
struct Scene{Event event;uint32_t form=0,complete=0,error=0,tailKind=0,tailPredicate=0,tailSymbol=0;Plane preSide,postSide,postWords,tailPre,tailPost;std::vector<Draw>draws;};
// Every event must come from one synchronous original-owner event sequencer in a FUTURE hook.
// Old independent TLS/capture serials cannot populate this binding legitimately.
struct JoinedInput{const Base&base;const Scene&scene;const nfs_logo_capture::Group&logos;const nfs_logo_capture::Copy&copy;Event groupEvent,tailEvent,copyEvent,uploadEvent;Target currentTarget;uint64_t serial=0,contentRevision=0;std::array<uint32_t,300>rendererAddress{};};
struct Workspace{std::array<uint16_t,Side>side{};std::array<uint16_t,Full>whole{};nfs_small_logo_live::Workspace logo;};
struct Prepared{bool valid=false;uint32_t mapRole=0,wordCount=0;uint64_t serial=0,contentRevision=0;std::array<uint16_t,Full>original{};std::array<uint32_t,300>keys{};std::array<NfsBackgroundSceneWordLayer,4>words{};std::array<NfsSmallLogoLayer,3>logos{};nfs_small_logo_live::Prepared logoScratch;
 NfsBackgroundSceneFrameView view()const noexcept{NfsBackgroundSceneFrameView v{};if(!valid)return v;v.schema=1;v.width=640;v.height=480;v.mapRole=mapRole;v.wordCount=wordCount;v.tailKind=NFS_BACKGROUND_TAIL_MAIN_SKIP;v.serial=serial;v.contentRevision=contentRevision;v.planeWords=Full;v.original565=original.data();std::copy(keys.begin(),keys.end(),v.rendererAddress);v.panelClip[2]=343;v.panelClip[3]=480;std::copy(words.begin(),words.end(),v.words);std::copy(logos.begin(),logos.end(),v.logos);return v;}};
enum class Failure{None,Binding,Base,Target,Scene,Source,Table,Palette,Rows,Pixels,Logos,Tail,Copy,NativeKeys};
inline uint32_t le32(const uint8_t*p)noexcept{return nfs_small_logo_live::le32(p);}inline uint16_t le16(const uint8_t*p)noexcept{return nfs_small_logo_live::le16(p);}
inline bool fullClip(const Target&t,unsigned width)noexcept{return nfs_small_logo_live::target(t,width)&&t.w[3]==0&&t.w[4]==0&&t.w[5]==width&&t.w[6]==480;}
inline bool plane(const Plane&p,unsigned width)noexcept{return p.valid==1&&p.width==width&&p.height==480&&fullClip(p.target,width);}
inline bool samePlane(const Plane&a,const Plane&b,unsigned width)noexcept{return plane(a,width)&&plane(b,width)&&nfs_logo_capture::same(a.target,b.target)&&std::equal(a.pixels.begin(),a.pixels.begin()+width*480,b.pixels.begin());}
inline bool source(const Draw&d,const Identity&i,const Target&t)noexcept{return d.complete==1&&!d.error&&d.sourceBefore.size()==i.span&&d.sourceAfter==d.sourceBefore&&std::equal(i.header.begin(),i.header.end(),d.sourceBefore.begin())&&nfs_small_logo_live::hashMatches(d.sourceBefore.data(),i.span,i.sha)&&nfs_logo_capture::same(d.before,t)&&nfs_logo_capture::same(d.after,t)&&d.args[0]&&uint64_t(d.args[0])+i.span<=0x100000000ULL&&!nfs_small_logo_live::overlap(d.args[0],i.span,t.w[8],16+Side*2);}
inline Failure side(const Draw&d,const Target&t,Workspace&w,const SourceCatalog&catalog)noexcept{const auto&i=catalog.identities[0];if(d.role||!source(d,i,t)||d.args[1]!=0x400||d.args[2]||d.args[3])return Failure::Source;const std::array<uint32_t,9> table{{0xf0001874,0xf0002124,0xf0001878,0xf0001878,0xf000187c,0xf0001880,0,2,0}};if(d.tableBefore!=table||d.tableAfter!=table)return Failure::Table;if(le32(d.sourceBefore.data()+i.paletteOffset)!=0x2a||(le32(d.sourceBefore.data()+i.paletteOffset+4)&65535)!=256)return Failure::Palette;if(d.rows.size()!=1&&d.rows.size()!=480)return Failure::Rows;
 const bool indexed=d.callback==0xf000221c;
 if(indexed){if(d.callbackGlobal!=0||d.format!=16||!d.paletteGuest||d.paletteValid!=1||d.paletteBefore!=d.paletteAfter)return Failure::Palette;for(unsigned k=0;k<256;++k)if(d.paletteBefore[k]!=uint32_t(nfs_small_logo_live::pack(le32(d.sourceBefore.data()+i.paletteOffset+16+k*4))))return Failure::Palette;}
 for(unsigned j=0;j<d.rows.size();++j){const auto&r=d.rows[j];unsigned n=d.rows.size()==1?Side:343,start=d.rows.size()==1?0:343*j;const uint32_t dest=t.w[8]+16+2*start;
  if(indexed){if(r.unit!=1||r.callback!=0xf000221c||r.converter||r.args[0]!=d.args[0]+16+start||r.args[1]!=dest||r.args[2]!=n||r.payload.size()!=n||std::memcmp(r.payload.data(),d.sourceBefore.data()+16+start,n))return Failure::Rows;}
  else if(!((r.unit==1&&r.callback==0xf0001878&&r.args[2]==n)||(r.unit==2&&r.callback==0xf0002124&&r.args[2]==2*n))||r.args[1]!=dest||!r.args[0]||uint64_t(r.args[0])+2*n>0x100000000ULL||r.payload.size()!=2*n)return Failure::Rows;
  for(unsigned k=0;k<n;++k){uint8_t index=d.sourceBefore[16+start+k];uint16_t expected=nfs_small_logo_live::pack(le32(d.sourceBefore.data()+i.paletteOffset+16+index*4));if(indexed){if(d.paletteBefore[r.payload[k]]!=expected)return Failure::Pixels;}else if(le16(r.payload.data()+2*k)!=expected)return Failure::Pixels;w.side[start+k]=expected;}}

 return Failure::None;}
inline Failure word(const Draw&d,const Target&t,Workspace&w,NfsBackgroundSceneWordLayer&out,const SourceCatalog&catalog)noexcept{if(d.role<1||d.role>12)return Failure::Source;const auto&i=catalog.identities[d.role];if(!source(d,i,t))return Failure::Source;const uint32_t mode=d.args[1]&~511u,opacity=d.args[1]&511u;if(mode!=0x1c00&&mode!=0x2000)return Failure::Source;if(d.callback!=0xf0001818||d.callbackGlobal!=d.callback||d.format!=0x7b||!d.paletteGuest||d.paletteValid!=1||d.paletteBefore!=d.paletteAfter)return Failure::Palette;
 const uint32_t table[6]={0xf000006c,0xf0001804,0xf0001808,0xf000180c,0xf0001810,0xf0001814};for(unsigned k=0;k<6;++k)if(d.tableBefore[k]!=table[k]||d.tableAfter[k]!=table[k])return Failure::Table;if(d.tableBefore[7]!=3||d.tableAfter[6]!=d.callback||d.tableAfter[7]!=3||d.tableAfter[8]!=0x7b)return Failure::Table;if((le32(d.sourceBefore.data()+i.paletteOffset)&255)!=0x2a||(le32(d.sourceBefore.data()+i.paletteOffset+4)&65535)!=256)return Failure::Palette;
 for(unsigned k=0;k<256;++k){uint32_t raw=le32(d.sourceBefore.data()+i.paletteOffset+16+k*4),effective=raw;if(opacity<255){effective=0;for(unsigned c=0;c<4;++c)effective|=((uint32_t((raw>>(8*c))&255)*opacity)>>8)<<(8*c);}if(d.paletteBefore[k]!=effective)return Failure::Palette;}
 auto s16=[](uint32_t v){return int32_t(int16_t(v&65535));};int x=nfs_small_logo_live::signed12(le32(i.header.data()+12)),y=nfs_small_logo_live::signed12(le32(i.header.data()+12)>>16);if(mode==0x2000){const int64_t xx=int64_t(int32_t(d.args[2]))-s16(le32(i.header.data()+8)),yy=int64_t(int32_t(d.args[3]))-s16(le32(i.header.data()+8)>>16);if(xx<INT32_MIN||xx>INT32_MAX||yy<INT32_MIN||yy>INT32_MAX)return Failure::Source;x=int(xx);y=int(yy);}int l=std::max(x,0),top=std::max(y,0),right=int(std::min<int64_t>(int64_t(x)+i.width,343)),bottom=int(std::min<int64_t>(int64_t(y)+i.height,480));if(l>=right||top>=bottom||d.rows.size()!=unsigned(bottom-top))return Failure::Rows;
 for(int yy=top;yy<bottom;++yy){const auto&r=d.rows[unsigned(yy-top)];unsigned sx=unsigned(l-x),sy=unsigned(yy-y),n=unsigned(right-l);if(r.unit!=1||r.callback!=d.callback||r.args[0]!=d.args[0]+16+sy*i.width+sx||r.args[1]!=t.w[8]+16+yy*686+l*2||r.args[2]!=n||r.payload.size()!=n||std::memcmp(r.payload.data(),d.sourceBefore.data()+16+sy*i.width+sx,n))return Failure::Rows;for(unsigned k=0;k<n;++k){unsigned at=yy*343+l+k;w.side[at]=nfs_small_logo_live::pixel(w.side[at],d.paletteBefore[r.payload[k]],false);}}
 out={};out.sourceRole=d.role;out.scalar256=opacity>=255?256:opacity;out.left=x;out.top=y;out.clip[0]=int32_t(t.w[3]);out.clip[1]=int32_t(t.w[4]);out.clip[2]=int32_t(t.w[5]);out.clip[3]=int32_t(t.w[6]);return Failure::None;}
inline bool retainedBaseTarget(const Target&a,const Target&b)noexcept{return fullClip(a,640)&&fullClip(b,640)&&a.w==b.w&&a.header==b.header&&a.x==b.x&&a.y==b.y&&a.owner==b.owner&&a.ownerWords==b.ownerWords&&a.ownerState[3]==a.owner&&b.ownerState[3]==b.owner;}
inline Failure verifyAndPrepare(const JoinedInput&i,Workspace&w,Prepared&o,const SourceCatalog&catalog)noexcept{o.valid=false;o.wordCount=0;o.words={};if(!validCatalog(catalog))return Failure::Source;const auto&b=i.base;const auto&s=i.scene;const auto&g=i.logos;const auto&c=i.copy;
 const Event ev[6]={b.event,s.event,i.groupEvent,i.tailEvent,i.copyEvent,i.uploadEvent};if(!i.serial||!i.contentRevision||!ev[0].ownerEpoch||!ev[0].sequence||ev[0].iteration)return Failure::Binding;for(unsigned k=1;k<6;++k)if(ev[k].ownerEpoch!=ev[0].ownerEpoch||ev[k].sequence<=ev[k-1].sequence||!ev[k].iteration||ev[k].iteration!=ev[1].iteration)return Failure::Binding;

 if(b.mapRole>1||b.form!=MainForm||b.sourceBefore.size()!=614416||b.sourceBefore!=b.sourceAfter||!nfs_small_logo_live::hashMatches(b.sourceBefore.data(),614416,catalog.mapSha[b.mapRole])||!plane(b.post,640))return Failure::Base;for(unsigned k=0;k<Full;++k)if(b.post.pixels[k]!=le16(b.sourceBefore.data()+16+2*k))return Failure::Base;
 if(s.form!=MainForm||!s.complete||s.error||s.draws.empty()||s.draws.size()>5||!plane(s.preSide,343)||!nfs_logo_capture::same(s.preSide.target,s.postSide.target)||!nfs_logo_capture::same(s.preSide.target,s.postWords.target))return Failure::Scene;
 auto failure=side(s.draws[0],s.preSide.target,w,catalog);if(failure!=Failure::None)return failure;if(!plane(s.postSide,343)||!std::equal(w.side.begin(),w.side.end(),s.postSide.pixels.begin()))return Failure::Pixels;
 for(unsigned k=1;k<s.draws.size();++k){failure=word(s.draws[k],s.preSide.target,w,o.words[k-1],catalog);if(failure!=Failure::None)return failure;}if(!plane(s.postWords,343)||!std::equal(w.side.begin(),w.side.end(),s.postWords.pixels.begin())||!samePlane(s.postWords,g.pre,343))return Failure::Pixels;
 if(nfs_small_logo_live::verifyAndPrepare(g,c,w.logo,o.logoScratch,catalog.logos)!=nfs_small_logo_live::Failure::None)return Failure::Logos;
 if(s.tailKind!=NFS_BACKGROUND_TAIL_MAIN_SKIP||s.tailPredicate!=MainForm||s.tailSymbol!=MainForm||!samePlane(s.tailPre,g.post,343)||!samePlane(s.tailPre,s.tailPost,343))return Failure::Tail;
 if(c.sourceOwner.header[3]||!retainedBaseTarget(b.post.target,c.pre.target)||!nfs_logo_capture::same(i.currentTarget,c.post.target))return Failure::Target;for(unsigned k=0;k<Side;++k)if(le16(c.sourceBefore.data()+16+2*k)!=s.tailPost.pixels[k])return Failure::Copy;
 // Reconstruct complete CURRENT native source from the independently identified map, not old Copy.PRE.
 w.whole=b.post.pixels;const uint32_t key=c.row[0].colorKey;for(unsigned y=0;y<480;++y)for(unsigned x=0;x<343;++x){unsigned at=y*640+x;uint16_t sourceWord=le16(c.sourceBefore.data()+16+2*(y*343+x));w.whole[at]=nfs_small_logo_live::pixel(w.whole[at],nfs_small_logo_live::expanded(sourceWord,key),false);}if(!std::equal(w.whole.begin(),w.whole.end(),c.post.pixels.begin()))return Failure::Pixels;
 for(unsigned k=0;k<300;++k){uint32_t keyAddress=i.rendererAddress[k];if(uint64_t(keyAddress)+2048>0x100000000ULL)return Failure::NativeKeys;for(unsigned j=0;j<k;++j)if(uint64_t(keyAddress)<uint64_t(i.rendererAddress[j])+2048&&uint64_t(i.rendererAddress[j])<uint64_t(keyAddress)+2048)return Failure::NativeKeys;}
 o.mapRole=b.mapRole;o.wordCount=unsigned(s.draws.size()-1);o.serial=i.serial;o.contentRevision=i.contentRevision;o.original=c.post.pixels;o.keys=i.rendererAddress;o.logos=o.logoScratch.layers;o.valid=true;return Failure::None;
}
} // namespace nfs_background_scene_cpu

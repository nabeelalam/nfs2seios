// SPDX-License-Identifier: MIT
#pragma once
#include "hd/BackgroundScenePrepare.h"
#include "hd/SoftwareFramePacket.h"
#include "hd/LoadSaveSourcePolicy.h"
#include <memory>
#include <cstring>
namespace nfs_ls_scene {
namespace bg=nfs_background_scene_cpu;
namespace logo=nfs_small_logo_live;
using Target=nfs_logo_capture::Target;
using Plane=nfs_logo_capture::Plane;
constexpr uint32_t Form=0x4d39a8,Handler=0x40938c,CacheSite=0x40bbe0,CacheStart=0x40bc20,CaptionSite=0x40c110;
constexpr size_t Side=343*480,RawSide=16+Side*2;
// Pure source consistency model ONLY. These values are records to be checked,
// not live permits. No renderer view or live issuer is exposed by this header.
/* NFS_ASSEMBLY_DIAG_BEGIN:1 */struct AssemblyDiagnostic {uint32_t checkpoint,count,codes[4],index;};
inline AssemblyDiagnostic&assemblyDiagnostic()noexcept{static thread_local AssemblyDiagnostic d{0,0,{0,0,0,0},0};return d;}
inline void assemblyReset(uint32_t phase)noexcept{assemblyDiagnostic()={phase,0,{0,0,0,0},0};}
inline void assemblyCheckpoint(uint32_t code)noexcept{assemblyDiagnostic().checkpoint=code;}
inline void assemblyIndex(uint32_t index)noexcept{assemblyDiagnostic().index=index;}
inline bool assemblyGate(uint32_t code,bool failed)noexcept{if(failed){auto&d=assemblyDiagnostic();if(d.count<4)d.codes[d.count++]=code;}return failed;}
/* NFS_ASSEMBLY_DIAG_END:1 */struct Owner {
 uint32_t form=0,handler=0,type=0,archive=0;
 uintptr_t game=0,thread=0;uint64_t epoch=0;
 bool valid()const noexcept{return form==Form&&handler==Handler&&type==9&&archive&&game&&thread&&epoch;}
 bool operator==(const Owner&x)const noexcept{return form==x.form&&handler==x.handler&&type==x.type&&archive==x.archive&&game==x.game&&thread==x.thread&&epoch==x.epoch;}
};
inline bool retained(const Target&a,const Target&b)noexcept{
 if(a.w!=b.w||a.header!=b.header||a.x!=b.x||a.y!=b.y||a.owner!=b.owner||a.ownerWords!=b.ownerWords)return false;
 for(unsigned k=1;k<5;++k)if(a.ownerState[k]!=b.ownerState[k])return false;
 return true;
}
struct Slots {
 std::array<Target,2>target{};std::array<uint64_t,2>lifetime{};unsigned selected=2;
 bool valid()const noexcept{return selected<2&&lifetime[0]&&lifetime[1]&&bg::fullClip(target[0],343)&&bg::fullClip(target[1],343)&&target[0].owner!=target[1].owner&&target[0].w[8]!=target[1].w[8]&&!logo::overlap(target[0].w[8],RawSide,target[1].w[8],RawSide);}
 bool sameCatalog(const Slots&x)const noexcept{return valid()&&x.valid()&&lifetime==x.lifetime&&retained(target[0],x.target[0])&&retained(target[1],x.target[1]);}
};
struct Directory {
 uint32_t address=0;std::array<uint32_t,4>header{};std::vector<uint8_t>bytes;
 bool valid()const noexcept{
  const uint64_t count=header[2],size=header[1];
  if(!address||header[0]!=0x49504853||!count||count>4096||size<16+count*8||size>64*1024*1024||uint64_t(address)+size>UINT64_C(0x100000000)||bytes.size()!=count*8)return false;
  for(unsigned k=0;k<count;++k){auto off=bg::le32(bytes.data()+k*8+4);if(off<16+count*8||off>=size)return false;}
  return true;
 }
 bool operator==(const Directory&x)const noexcept{return address==x.address&&header==x.header&&bytes==x.bytes;}
 bool resolves(uint32_t name,uint32_t source,size_t span)const noexcept{
  if(!valid()||source<address||!span)return false;uint32_t off=source-address,found=0;
  for(unsigned k=0;k<header[2];++k){auto p=bytes.data()+8*k;auto at=bg::le32(p+4);if(bg::le32(p)==name){if(++found!=1||at!=off)return false;}if(at>off&&uint64_t(off)+span>at)return false;}
  return found==1&&uint64_t(off)+span<=header[1];
 }
};
class Trusted final {
 bg::SourceCatalog catalog_;std::array<uint32_t,13>names_{};std::array<uint32_t,5>logoNames_{};
 Trusted(const bg::SourceCatalog&c,const std::array<uint32_t,13>&n,const std::array<uint32_t,5>&l):catalog_(c),names_(n),logoNames_(l){}
public:
 // The runtime must obtain this input from the independently trusted immutable
 // program catalog. This validates structure; it does not establish provenance.
 static std::shared_ptr<const Trusted>create(const bg::SourceCatalog&c,const std::array<uint32_t,13>&n,const std::array<uint32_t,5>&l)noexcept{
  if(!bg::validCatalog(c))return {};for(auto v:n)if(!v)return {};for(auto v:l)if(!v)return {};
  try{return std::shared_ptr<const Trusted>(new Trusted(c,n,l));}catch(...){return {};}
 }
 const bg::SourceCatalog&catalog()const noexcept{return catalog_;}
 bool binds(const Directory&d,const bg::Scene&s,const nfs_logo_capture::Group&g)const noexcept{
  if(!d.valid())return false;for(const auto&v:s.draws){if(v.role>12||!d.resolves(names_[v.role],v.args[0],catalog_.identities[v.role].span))return false;}
  for(unsigned k=0;k<3;++k){const auto&v=g.draw[k];if(v.role>4||!d.resolves(logoNames_[v.role],v.args[0],catalog_.logos.identities[v.role].span))return false;}return true;
 }
};
struct Caption {
 uint32_t site=0,form=0,incomingEbp=0,archive=0,selector=0,entry=0,entryWord=0,textGuest=0;
 uint32_t font=0,callback=0,palette=0,mode=0,flags=0,format=0;int32_t measured=0,anchor=0;
 uint64_t serial=0,lifetime=0;std::array<unsigned char,32>originalFontSha{};std::array<uint32_t,16>paletteWords{};std::vector<nfs_loadsave_source::GlyphUse>glyphs;std::shared_ptr<const nfs_soft::FramePacket>packet;
 // Complete original callback/store evidence is replayed independently below.
};
// No supported live profile registration is supplied here. These owned inputs
// are explicit consistency assumptions until the exact lexical adapter joins
// them to the trusted source font/catalog in the actual LoadSave cache tail.
struct CaptionProfile {uint32_t glyphCallback=0;std::array<unsigned char,32>fontSha{};nfs_loadsave_source::Profile source;
 bool valid()const noexcept{return glyphCallback&&source.valid()&&source.paletteObserved[1]&&logo::hashMatches(source.fontBytes.data(),uint32_t(source.fontBytes.size()),fontSha);}
};
inline bool caption(const Caption&c,const CaptionProfile&profile,const Owner&o,const Target&t,const Plane&pre,const std::vector<uint8_t>&raw)noexcept{
 if(!bg::plane(pre,343)||!retained(t,pre.target)||!profile.valid()||c.originalFontSha!=profile.fontSha||c.site!=CaptionSite||c.form!=Form||c.incomingEbp!=Form||c.archive!=o.archive||c.selector!=6||!c.entry||!c.entryWord||!c.textGuest||!c.font||c.callback!=profile.glyphCallback||!c.palette||c.mode||c.flags||c.format!=16||c.measured!=48||c.anchor!=168||!c.serial||!c.lifetime||!c.packet||raw.size()!=RawSide)return false;
 const auto&p=*c.packet;if(p.font()!=profile.source.displayFont||c.paletteWords!=profile.source.palettes[1]||c.glyphs.size()!=7||p.pixels()!=Side||p.serial()!=c.serial||p.surface().lifetime!=c.lifetime||p.commands()!=1||p.records().size()!=1||!p.writes()||p.clean().size()!=Side||p.original().size()!=Side||!std::equal(p.clean().begin(),p.clean().end(),pre.pixels.begin()))return false;
 const auto&w=p.surface().geometry;if(!p.surface().valid()||!std::equal(t.w.begin(),t.w.end(),w.begin()))return false;
 if(w[14]!=t.x[0]||w[15]!=t.x[1]||w[16]!=t.x[342]||w[17]!=t.y[0]||w[18]!=t.y[1]||w[19]!=t.y[479])return false;
 const auto&v=p.records()[0].command;
 if(v.length!=7||std::memcmp(v.text.data(),"options",7)||v.role!=1||v.alignment!=2||v.sourceSize!=18||v.x!=120||v.y!=364||v.originX||v.originY||v.advance!=48||v.color!=65535||v.clip.left||v.clip.top||v.clip.right!=343||v.clip.bottom!=480)return false;
 // Reconstruct every original nibble callback/store, signed bearing, even
 // rounded width and advance; final-plane equality rejects hidden stores.
 try{auto replay=p.clean();int pen=v.x;size_t writes=0;
  for(unsigned n=0;n<7;++n){const unsigned code=uint8_t("options"[n]);const auto&g=profile.source.glyphs[code-32];const auto&use=c.glyphs[n];const unsigned rounded=(g.width+1)/2*2;const int dx=pen+g.bearingX,dy=v.y+g.bearingY;
   if(g.code!=code||use.code!=code||use.textIndex!=n||use.x!=dx||use.y!=dy||use.sourceX!=g.sourceX||use.sourceY!=g.sourceY||use.width!=g.width||use.height!=g.height||use.stride!=profile.source.atlasStride||dx<0||dy<0||uint64_t(dx)+rounded>343||uint64_t(dy)+g.height>480)return false;
   size_t atStore=0;for(unsigned y=0;y<g.height;++y)for(unsigned x=0;x<rounded;++x){const uint32_t offset=16+(g.sourceY+y)*profile.source.atlasStride+(g.sourceX+x)/2;const uint8_t byte=profile.source.fontBytes[offset],parity=uint8_t((g.sourceX+x)%2);const uint32_t value=c.paletteWords[parity?byte&15:byte>>4];if(value<0x10000000)continue;
    if(atStore>=use.stores.size())return false;const auto&w=use.stores[atStore++];const unsigned index=(dy+y)*343+dx+x;const auto expected=nfs_loadsave_source::blend(replay[index],value);
    if(w.index!=index||w.before!=replay[index]||w.computed!=expected||w.after!=expected||w.sourceOffset!=offset||w.sourceByte!=byte||w.sourceParity!=parity||w.paletteValue!=value)return false;replay[index]=expected;++writes;
   }if(atStore!=use.stores.size())return false;pen+=g.advance;
  }if(pen!=168||writes!=p.writes()||replay!=p.original())return false;
 }catch(...){return false;}
 for(unsigned k=0;k<4;++k)if(bg::le32(raw.data()+4*k)!=t.header[k])return false;
 for(unsigned k=0;k<Side;++k)if(bg::le16(raw.data()+16+2*k)!=p.original()[k])return false;
 return true;
}
struct CacheInput {
 Owner owner;Slots before,after;Directory archiveBefore,archiveAfter;
 uint64_t revision=0;std::array<uint64_t,5>sequence{};uint32_t callsite=0,start=0;
 const bg::Scene&scene;const nfs_logo_capture::Group&logos;const Caption&caption;
 // Both are stability snapshots AFTER caption completion, not the pre-caption plane.
 const std::vector<uint8_t>&rawBefore;const std::vector<uint8_t>&rawAfter;
};
class Cache final {
 Owner owner_;Slots slots_;unsigned slot_=2;uint64_t revision_=0;Directory archive_;
 std::vector<uint8_t>raw_;std::shared_ptr<const Trusted>trusted_;std::shared_ptr<const nfs_soft::FramePacket>caption_;
 std::array<NfsBackgroundSceneWordLayer,4>words_{};unsigned wordCount_=0;
 nfs_logo_capture::Group logos_;Cache()=default;
public:
 const std::vector<uint8_t>&raw()const noexcept{return raw_;}const Owner&owner()const noexcept{return owner_;}
 unsigned slot()const noexcept{return slot_;}unsigned wordCount()const noexcept{return wordCount_;}
 const auto&words()const noexcept{return words_;}const auto&logos()const noexcept{return logos_;}
 const std::shared_ptr<const Trusted>&trusted()const noexcept{return trusted_;}
 const Slots&slots()const noexcept{return slots_;}const Directory&directory()const noexcept{return archive_;}
 static std::shared_ptr<const Cache>create(const CacheInput&i,std::shared_ptr<const Trusted>trusted,const CaptionProfile&profile)noexcept{
  if(!trusted||!i.owner.valid()||!i.revision||i.callsite!=CacheSite||i.start!=CacheStart||!i.before.sameCatalog(i.after)||i.after.selected!=1-i.before.selected||i.archiveBefore.address!=i.owner.archive||!i.archiveBefore.valid()||!(i.archiveBefore==i.archiveAfter)||i.caption.lifetime!=i.after.lifetime[i.after.selected]||i.rawBefore!=i.rawAfter||i.rawBefore.size()!=RawSide)return {};
  for(unsigned k=0;k<5;++k)if(!i.sequence[k]||(k&&i.sequence[k]<=i.sequence[k-1]))return {};
  const auto&s=i.scene;const auto&t=i.after.target[i.after.selected];const auto&g=i.logos;
  // Source drawing and its glyph/store snapshots precede the original selector flip.
  // Keep those exact PRE-flip identities separate from the selected POST-flip slot.
  const auto&producer=i.before.target[i.after.selected];
  const uint32_t beforeWord=i.before.target[0].ownerState[0],afterWord=i.after.target[0].ownerState[0];
  if(beforeWord!=i.before.target[1].ownerState[0]||afterWord!=i.after.target[1].ownerState[0]||
     (beforeWord>>16)!=i.before.selected||(afterWord>>16)!=i.after.selected||(beforeWord&65535u)!=(afterWord&65535u)||
     !nfs_logo_capture::same(producer,s.preSide.target)||!nfs_logo_capture::same(producer,s.postSide.target)||
     !nfs_logo_capture::same(producer,s.postWords.target)||!nfs_logo_capture::same(producer,g.pre.target)||
     !nfs_logo_capture::same(producer,g.post.target))return {};

  if(s.form!=Form||!s.complete||s.error||s.draws.empty()||s.draws.size()>5||!bg::plane(s.preSide,343)||!retained(t,s.preSide.target)||!retained(t,s.postWords.target)||!retained(t,g.pre.target)||!bg::samePlane(s.postWords,g.pre,343)||g.thread!=i.owner.thread||!trusted->binds(i.archiveBefore,s,g))return {};
  try{auto work=std::unique_ptr<bg::Workspace>(new bg::Workspace);if(bg::side(s.draws[0],producer,*work,trusted->catalog())!=bg::Failure::None||!bg::plane(s.postSide,343)||!retained(t,s.postSide.target)||!std::equal(work->side.begin(),work->side.end(),s.postSide.pixels.begin()))return {};
   std::array<NfsBackgroundSceneWordLayer,4>words{};for(unsigned k=1;k<s.draws.size();++k)if(bg::word(s.draws[k],producer,*work,words[k-1],trusted->catalog())!=bg::Failure::None)return {};
   if(!std::equal(work->side.begin(),work->side.end(),s.postWords.pixels.begin())||logo::verifyGroup(g,work->logo,trusted->catalog().logos)!=logo::Failure::None||!retained(t,g.post.target)||!nfs_ls_scene::caption(i.caption,profile,i.owner,t,g.post,i.rawBefore))return {};
   auto p=std::shared_ptr<Cache>(new Cache);p->owner_=i.owner;p->slots_=i.after;p->slot_=i.after.selected;p->revision_=i.revision;p->archive_=i.archiveAfter;p->raw_=i.rawAfter;p->trusted_=std::move(trusted);p->caption_=i.caption.packet;p->words_=words;p->wordCount_=unsigned(s.draws.size()-1);p->logos_=g;return p;
  }catch(...){return {};}
 }
 bool matches(const Owner&o,const Slots&s,const Directory&d,uint64_t revision,const uint8_t*raw,size_t bytes)const noexcept{return o.valid()&&o==owner_&&s.sameCatalog(slots_)&&s.selected==slot_&&d==archive_&&revision==revision_&&raw&&bytes==raw_.size()&&!std::memcmp(raw,raw_.data(),bytes);}
};

struct CopyRow {uint32_t callback=0;std::array<uint32_t,3>args{};std::vector<uint8_t>source;std::vector<uint16_t>before,after;};
struct FullDraw {
 std::array<uint32_t,4>args{};Target before{},after{};std::array<uint32_t,9>tableBefore{},tableAfter{};
 std::vector<uint8_t>sourceBefore,sourceAfter;std::vector<CopyRow>rows;
};
struct SceneInput {
 Owner owner;Slots slots;Directory directory;Target full;
 uint64_t cacheRevision=0,serial=0;uint32_t mapName=0,mapRole=2;
 const FullDraw&reset;const FullDraw&copy;const std::vector<uint16_t>&pre;
 const std::vector<uint16_t>&postReset;const std::vector<uint16_t>&clean;
};
class OwnedScene final {
 std::shared_ptr<const Cache>cache_;std::shared_ptr<const Trusted>trusted_;
 std::vector<uint16_t>clean_;Owner owner_;Target full_;uint64_t serial_=0;unsigned mapRole_=2;
 OwnedScene()=default;
 static bool replay(const FullDraw&d,const Target&t,bool side,std::vector<uint16_t>&pixels)noexcept{
/* NFS_ASSEMBLY_DIAG_BEGIN:104 */nfs_ls_scene::assemblyCheckpoint(side?11000u:10000u);/* NFS_ASSEMBLY_DIAG_END:104 */  const unsigned width=side?343:640,cb=side?0xf00019cc:0xf0001878;
  const size_t bytes=16+width*480*2;
  if(/* NFS_ASSEMBLY_DIAG_BEGIN:2 */nfs_ls_scene::assemblyGate(1001,/* NFS_ASSEMBLY_DIAG_END:2 */!retained(d.before,t)/* NFS_ASSEMBLY_DIAG_BEGIN:3 */)/* NFS_ASSEMBLY_DIAG_END:3 */||/* NFS_ASSEMBLY_DIAG_BEGIN:4 */nfs_ls_scene::assemblyGate(1002,/* NFS_ASSEMBLY_DIAG_END:4 */!retained(d.after,t)/* NFS_ASSEMBLY_DIAG_BEGIN:5 */)/* NFS_ASSEMBLY_DIAG_END:5 */||/* NFS_ASSEMBLY_DIAG_BEGIN:6 */nfs_ls_scene::assemblyGate(1003,/* NFS_ASSEMBLY_DIAG_END:6 */!d.args[0]/* NFS_ASSEMBLY_DIAG_BEGIN:7 */)/* NFS_ASSEMBLY_DIAG_END:7 */||/* NFS_ASSEMBLY_DIAG_BEGIN:8 */nfs_ls_scene::assemblyGate(1004,/* NFS_ASSEMBLY_DIAG_END:8 */uint64_t(d.args[0])+bytes>UINT64_C(0x100000000)/* NFS_ASSEMBLY_DIAG_BEGIN:9 */)/* NFS_ASSEMBLY_DIAG_END:9 */||/* NFS_ASSEMBLY_DIAG_BEGIN:10 */nfs_ls_scene::assemblyGate(1005,/* NFS_ASSEMBLY_DIAG_END:10 */logo::overlap(d.args[0],bytes,t.w[8],614416)/* NFS_ASSEMBLY_DIAG_BEGIN:11 */)/* NFS_ASSEMBLY_DIAG_END:11 */||/* NFS_ASSEMBLY_DIAG_BEGIN:12 */nfs_ls_scene::assemblyGate(1006,/* NFS_ASSEMBLY_DIAG_END:12 */d.args[1]!=(side?0x1000u:0x400u)/* NFS_ASSEMBLY_DIAG_BEGIN:13 */)/* NFS_ASSEMBLY_DIAG_END:13 */||/* NFS_ASSEMBLY_DIAG_BEGIN:14 */nfs_ls_scene::assemblyGate(1007,/* NFS_ASSEMBLY_DIAG_END:14 */d.args[2]/* NFS_ASSEMBLY_DIAG_BEGIN:15 */)/* NFS_ASSEMBLY_DIAG_END:15 */||/* NFS_ASSEMBLY_DIAG_BEGIN:16 */nfs_ls_scene::assemblyGate(1008,/* NFS_ASSEMBLY_DIAG_END:16 */d.args[3]/* NFS_ASSEMBLY_DIAG_BEGIN:17 */)/* NFS_ASSEMBLY_DIAG_END:17 */||/* NFS_ASSEMBLY_DIAG_BEGIN:18 */nfs_ls_scene::assemblyGate(1009,/* NFS_ASSEMBLY_DIAG_END:18 */d.sourceBefore.size()!=bytes/* NFS_ASSEMBLY_DIAG_BEGIN:19 */)/* NFS_ASSEMBLY_DIAG_END:19 */||/* NFS_ASSEMBLY_DIAG_BEGIN:20 */nfs_ls_scene::assemblyGate(1010,/* NFS_ASSEMBLY_DIAG_END:20 */d.sourceBefore!=d.sourceAfter/* NFS_ASSEMBLY_DIAG_BEGIN:21 */)/* NFS_ASSEMBLY_DIAG_END:21 */||/* NFS_ASSEMBLY_DIAG_BEGIN:22 */nfs_ls_scene::assemblyGate(1011,/* NFS_ASSEMBLY_DIAG_END:22 */d.tableBefore!=d.tableAfter/* NFS_ASSEMBLY_DIAG_BEGIN:23 */)/* NFS_ASSEMBLY_DIAG_END:23 */||/* NFS_ASSEMBLY_DIAG_BEGIN:24 */nfs_ls_scene::assemblyGate(1012,/* NFS_ASSEMBLY_DIAG_END:24 */d.tableBefore[3]!=cb/* NFS_ASSEMBLY_DIAG_BEGIN:25 */)/* NFS_ASSEMBLY_DIAG_END:25 */||/* NFS_ASSEMBLY_DIAG_BEGIN:26 */nfs_ls_scene::assemblyGate(1013,/* NFS_ASSEMBLY_DIAG_END:26 */d.tableBefore[6]/* NFS_ASSEMBLY_DIAG_BEGIN:27 */)/* NFS_ASSEMBLY_DIAG_END:27 */||/* NFS_ASSEMBLY_DIAG_BEGIN:28 */nfs_ls_scene::assemblyGate(1014,/* NFS_ASSEMBLY_DIAG_END:28 */d.tableBefore[8]/* NFS_ASSEMBLY_DIAG_BEGIN:29 */)/* NFS_ASSEMBLY_DIAG_END:29 */||/* NFS_ASSEMBLY_DIAG_BEGIN:30 */nfs_ls_scene::assemblyGate(1015,/* NFS_ASSEMBLY_DIAG_END:30 */(!side&&d.tableBefore[7]!=2)/* NFS_ASSEMBLY_DIAG_BEGIN:31 */)/* NFS_ASSEMBLY_DIAG_END:31 */)return false;
  if(/* NFS_ASSEMBLY_DIAG_BEGIN:32 */nfs_ls_scene::assemblyGate(1016,/* NFS_ASSEMBLY_DIAG_END:32 */bg::le32(d.sourceBefore.data())!=0x78/* NFS_ASSEMBLY_DIAG_BEGIN:33 */)/* NFS_ASSEMBLY_DIAG_END:33 */||/* NFS_ASSEMBLY_DIAG_BEGIN:34 */nfs_ls_scene::assemblyGate(1017,/* NFS_ASSEMBLY_DIAG_END:34 */bg::le32(d.sourceBefore.data()+4)!=(width|(480u<<16))/* NFS_ASSEMBLY_DIAG_BEGIN:35 */)/* NFS_ASSEMBLY_DIAG_END:35 */||/* NFS_ASSEMBLY_DIAG_BEGIN:36 */nfs_ls_scene::assemblyGate(1018,/* NFS_ASSEMBLY_DIAG_END:36 */bg::le32(d.sourceBefore.data()+8)/* NFS_ASSEMBLY_DIAG_BEGIN:37 */)/* NFS_ASSEMBLY_DIAG_END:37 */||/* NFS_ASSEMBLY_DIAG_BEGIN:38 */nfs_ls_scene::assemblyGate(1019,/* NFS_ASSEMBLY_DIAG_END:38 */bg::le32(d.sourceBefore.data()+12)/* NFS_ASSEMBLY_DIAG_BEGIN:39 */)/* NFS_ASSEMBLY_DIAG_END:39 */)return false;
  const bool flat=!side&&d.rows.size()==1;if(/* NFS_ASSEMBLY_DIAG_BEGIN:40 */nfs_ls_scene::assemblyGate(1020,/* NFS_ASSEMBLY_DIAG_END:40 */!flat&&d.rows.size()!=480/* NFS_ASSEMBLY_DIAG_BEGIN:41 */)/* NFS_ASSEMBLY_DIAG_END:41 */)return false;
  for(unsigned y=0;y<(flat?1u:480u);++y){const auto&r=d.rows[y];const unsigned n=flat?640*480:width;
   if(/* NFS_ASSEMBLY_DIAG_BEGIN:42 */nfs_ls_scene::assemblyGate(1021,/* NFS_ASSEMBLY_DIAG_END:42 */r.callback!=cb/* NFS_ASSEMBLY_DIAG_BEGIN:43 */)/* NFS_ASSEMBLY_DIAG_END:43 */||/* NFS_ASSEMBLY_DIAG_BEGIN:44 */nfs_ls_scene::assemblyGate(1022,/* NFS_ASSEMBLY_DIAG_END:44 */r.args[0]!=d.args[0]+16+y*width*2/* NFS_ASSEMBLY_DIAG_BEGIN:45 */)/* NFS_ASSEMBLY_DIAG_END:45 */||/* NFS_ASSEMBLY_DIAG_BEGIN:46 */nfs_ls_scene::assemblyGate(1023,/* NFS_ASSEMBLY_DIAG_END:46 */r.args[1]!=t.w[8]+16+y*1280/* NFS_ASSEMBLY_DIAG_BEGIN:47 */)/* NFS_ASSEMBLY_DIAG_END:47 */||/* NFS_ASSEMBLY_DIAG_BEGIN:48 */nfs_ls_scene::assemblyGate(1024,/* NFS_ASSEMBLY_DIAG_END:48 */r.args[2]!=n/* NFS_ASSEMBLY_DIAG_BEGIN:49 */)/* NFS_ASSEMBLY_DIAG_END:49 */||/* NFS_ASSEMBLY_DIAG_BEGIN:50 */nfs_ls_scene::assemblyGate(1025,/* NFS_ASSEMBLY_DIAG_END:50 */r.source.size()!=n*2/* NFS_ASSEMBLY_DIAG_BEGIN:51 */)/* NFS_ASSEMBLY_DIAG_END:51 */||/* NFS_ASSEMBLY_DIAG_BEGIN:52 */nfs_ls_scene::assemblyGate(1026,/* NFS_ASSEMBLY_DIAG_END:52 */r.before.size()!=n/* NFS_ASSEMBLY_DIAG_BEGIN:53 */)/* NFS_ASSEMBLY_DIAG_END:53 */||/* NFS_ASSEMBLY_DIAG_BEGIN:54 */nfs_ls_scene::assemblyGate(1027,/* NFS_ASSEMBLY_DIAG_END:54 */r.after.size()!=n/* NFS_ASSEMBLY_DIAG_BEGIN:55 */)/* NFS_ASSEMBLY_DIAG_END:55 */||/* NFS_ASSEMBLY_DIAG_BEGIN:56 */nfs_ls_scene::assemblyGate(1028,/* NFS_ASSEMBLY_DIAG_END:56 */std::memcmp(r.source.data(),d.sourceBefore.data()+16+y*width*2,n*2)/* NFS_ASSEMBLY_DIAG_BEGIN:57 */)/* NFS_ASSEMBLY_DIAG_END:57 */)return false;
   for(unsigned x=0;x<n;++x){const size_t at=flat?x:y*640+x;const uint16_t value=bg::le16(r.source.data()+2*x),expected=side&&!value?pixels[at]:value;/* NFS_ASSEMBLY_DIAG_BEGIN:106 */nfs_ls_scene::assemblyIndex(uint32_t(at));/* NFS_ASSEMBLY_DIAG_END:106 */if(/* NFS_ASSEMBLY_DIAG_BEGIN:58 */nfs_ls_scene::assemblyGate(1029,/* NFS_ASSEMBLY_DIAG_END:58 */r.before[x]!=pixels[at]/* NFS_ASSEMBLY_DIAG_BEGIN:59 */)/* NFS_ASSEMBLY_DIAG_END:59 */||/* NFS_ASSEMBLY_DIAG_BEGIN:60 */nfs_ls_scene::assemblyGate(1030,/* NFS_ASSEMBLY_DIAG_END:60 */r.after[x]!=expected/* NFS_ASSEMBLY_DIAG_BEGIN:61 */)/* NFS_ASSEMBLY_DIAG_END:61 */)return false;pixels[at]=expected;}
  }return true;
 }
public:
 const auto&clean()const noexcept{return clean_;}const auto&cache()const noexcept{return cache_;}unsigned mapRole()const noexcept{return mapRole_;}
 static std::shared_ptr<const OwnedScene>create(const SceneInput&i,std::shared_ptr<const Cache>cache,std::shared_ptr<const Trusted>trusted)noexcept{/* NFS_ASSEMBLY_DIAG_BEGIN:105 */nfs_ls_scene::assemblyCheckpoint(20000);/* NFS_ASSEMBLY_DIAG_END:105 */
  if(/* NFS_ASSEMBLY_DIAG_BEGIN:62 */nfs_ls_scene::assemblyGate(2001,/* NFS_ASSEMBLY_DIAG_END:62 */!cache/* NFS_ASSEMBLY_DIAG_BEGIN:63 */)/* NFS_ASSEMBLY_DIAG_END:63 */||/* NFS_ASSEMBLY_DIAG_BEGIN:64 */nfs_ls_scene::assemblyGate(2002,/* NFS_ASSEMBLY_DIAG_END:64 */!trusted/* NFS_ASSEMBLY_DIAG_BEGIN:65 */)/* NFS_ASSEMBLY_DIAG_END:65 */||/* NFS_ASSEMBLY_DIAG_BEGIN:66 */nfs_ls_scene::assemblyGate(2003,/* NFS_ASSEMBLY_DIAG_END:66 */cache->trusted().get()!=trusted.get()/* NFS_ASSEMBLY_DIAG_BEGIN:67 */)/* NFS_ASSEMBLY_DIAG_END:67 */||/* NFS_ASSEMBLY_DIAG_BEGIN:68 */nfs_ls_scene::assemblyGate(2004,/* NFS_ASSEMBLY_DIAG_END:68 */cache->trusted().owner_before(trusted)/* NFS_ASSEMBLY_DIAG_BEGIN:69 */)/* NFS_ASSEMBLY_DIAG_END:69 */||/* NFS_ASSEMBLY_DIAG_BEGIN:70 */nfs_ls_scene::assemblyGate(2005,/* NFS_ASSEMBLY_DIAG_END:70 */trusted.owner_before(cache->trusted())/* NFS_ASSEMBLY_DIAG_BEGIN:71 */)/* NFS_ASSEMBLY_DIAG_END:71 */||/* NFS_ASSEMBLY_DIAG_BEGIN:72 */nfs_ls_scene::assemblyGate(2006,/* NFS_ASSEMBLY_DIAG_END:72 */!i.owner.valid()/* NFS_ASSEMBLY_DIAG_BEGIN:73 */)/* NFS_ASSEMBLY_DIAG_END:73 */||/* NFS_ASSEMBLY_DIAG_BEGIN:74 */nfs_ls_scene::assemblyGate(2007,/* NFS_ASSEMBLY_DIAG_END:74 */!i.serial/* NFS_ASSEMBLY_DIAG_BEGIN:75 */)/* NFS_ASSEMBLY_DIAG_END:75 */||/* NFS_ASSEMBLY_DIAG_BEGIN:76 */nfs_ls_scene::assemblyGate(2008,/* NFS_ASSEMBLY_DIAG_END:76 */i.mapRole>1/* NFS_ASSEMBLY_DIAG_BEGIN:77 */)/* NFS_ASSEMBLY_DIAG_END:77 */||/* NFS_ASSEMBLY_DIAG_BEGIN:78 */nfs_ls_scene::assemblyGate(2009,/* NFS_ASSEMBLY_DIAG_END:78 */!i.mapName/* NFS_ASSEMBLY_DIAG_BEGIN:79 */)/* NFS_ASSEMBLY_DIAG_END:79 */||/* NFS_ASSEMBLY_DIAG_BEGIN:80 */nfs_ls_scene::assemblyGate(2010,/* NFS_ASSEMBLY_DIAG_END:80 */!bg::fullClip(i.full,640)/* NFS_ASSEMBLY_DIAG_BEGIN:81 */)/* NFS_ASSEMBLY_DIAG_END:81 */||/* NFS_ASSEMBLY_DIAG_BEGIN:82 */nfs_ls_scene::assemblyGate(2011,/* NFS_ASSEMBLY_DIAG_END:82 */i.pre.size()!=bg::Full/* NFS_ASSEMBLY_DIAG_BEGIN:83 */)/* NFS_ASSEMBLY_DIAG_END:83 */||/* NFS_ASSEMBLY_DIAG_BEGIN:84 */nfs_ls_scene::assemblyGate(2012,/* NFS_ASSEMBLY_DIAG_END:84 */i.postReset.size()!=bg::Full/* NFS_ASSEMBLY_DIAG_BEGIN:85 */)/* NFS_ASSEMBLY_DIAG_END:85 */||/* NFS_ASSEMBLY_DIAG_BEGIN:86 */nfs_ls_scene::assemblyGate(2013,/* NFS_ASSEMBLY_DIAG_END:86 */i.clean.size()!=bg::Full/* NFS_ASSEMBLY_DIAG_BEGIN:87 */)/* NFS_ASSEMBLY_DIAG_END:87 */||/* NFS_ASSEMBLY_DIAG_BEGIN:88 */nfs_ls_scene::assemblyGate(2014,/* NFS_ASSEMBLY_DIAG_END:88 */!cache->matches(i.owner,i.slots,i.directory,i.cacheRevision,i.copy.sourceBefore.data(),i.copy.sourceBefore.size())/* NFS_ASSEMBLY_DIAG_BEGIN:89 */)/* NFS_ASSEMBLY_DIAG_END:89 */||/* NFS_ASSEMBLY_DIAG_BEGIN:90 */nfs_ls_scene::assemblyGate(2015,/* NFS_ASSEMBLY_DIAG_END:90 */i.copy.args[0]!=i.slots.target[i.slots.selected].w[8]/* NFS_ASSEMBLY_DIAG_BEGIN:91 */)/* NFS_ASSEMBLY_DIAG_END:91 */||/* NFS_ASSEMBLY_DIAG_BEGIN:92 */nfs_ls_scene::assemblyGate(2016,/* NFS_ASSEMBLY_DIAG_END:92 */!i.directory.resolves(i.mapName,i.reset.args[0],614416)/* NFS_ASSEMBLY_DIAG_BEGIN:93 */)/* NFS_ASSEMBLY_DIAG_END:93 */||/* NFS_ASSEMBLY_DIAG_BEGIN:94 */nfs_ls_scene::assemblyGate(2017,/* NFS_ASSEMBLY_DIAG_END:94 */!logo::hashMatches(i.reset.sourceBefore.data(),i.reset.sourceBefore.size(),trusted->catalog().mapSha[i.mapRole])/* NFS_ASSEMBLY_DIAG_BEGIN:95 */)/* NFS_ASSEMBLY_DIAG_END:95 */)return {};
  try{auto pixels=i.pre;if(/* NFS_ASSEMBLY_DIAG_BEGIN:96 */nfs_ls_scene::assemblyGate(2018,/* NFS_ASSEMBLY_DIAG_END:96 */!replay(i.reset,i.full,false,pixels)/* NFS_ASSEMBLY_DIAG_BEGIN:97 */)/* NFS_ASSEMBLY_DIAG_END:97 */||/* NFS_ASSEMBLY_DIAG_BEGIN:98 */nfs_ls_scene::assemblyGate(2019,/* NFS_ASSEMBLY_DIAG_END:98 */pixels!=i.postReset/* NFS_ASSEMBLY_DIAG_BEGIN:99 */)/* NFS_ASSEMBLY_DIAG_END:99 */||/* NFS_ASSEMBLY_DIAG_BEGIN:100 */nfs_ls_scene::assemblyGate(2020,/* NFS_ASSEMBLY_DIAG_END:100 */!replay(i.copy,i.full,true,pixels)/* NFS_ASSEMBLY_DIAG_BEGIN:101 */)/* NFS_ASSEMBLY_DIAG_END:101 */||/* NFS_ASSEMBLY_DIAG_BEGIN:102 */nfs_ls_scene::assemblyGate(2021,/* NFS_ASSEMBLY_DIAG_END:102 */pixels!=i.clean/* NFS_ASSEMBLY_DIAG_BEGIN:103 */)/* NFS_ASSEMBLY_DIAG_END:103 */)return {};auto p=std::shared_ptr<OwnedScene>(new OwnedScene);p->cache_=std::move(cache);p->trusted_=std::move(trusted);p->clean_=std::move(pixels);p->owner_=i.owner;p->full_=i.full;p->serial_=i.serial;p->mapRole_=i.mapRole;return p;}catch(...){return {};}
 }
};
// Bounded owner-local cache retention. It is not a live source issuer. A future
// source adapter supplies only lexical-site records and invalidates every
// overlapping/retired source or target, owner unwind, archive/context change.
class Collector final {
 Owner owner_;std::array<std::shared_ptr<const Cache>,2>cache_{};bool open_=false;uint64_t revision_=0;
public:
 bool enter(const Owner&o)noexcept{if(open_||!o.valid()){cancel();return false;}owner_=o;open_=true;return true;}
 void cancel()noexcept{open_=false;cache_={};owner_={};revision_=0;}
 bool publish(const CacheInput&i,std::shared_ptr<const Trusted>t,const CaptionProfile&p)noexcept{
  if(!open_||!(i.owner==owner_)||i.revision<=revision_){cancel();return false;}auto c=Cache::create(i,std::move(t),p);if(!c){cancel();return false;}revision_=i.revision;cache_[c->slot()]=std::move(c);return true;
 }
 std::shared_ptr<const Cache>current(const Owner&o,const Slots&s,const Directory&d,uint64_t revision,const uint8_t*raw,size_t bytes)noexcept{
  if(!open_||s.selected>1||!(o==owner_)){cancel();return {};}auto p=cache_[s.selected];if(!p)return {};if(!p->matches(o,s,d,revision,raw,bytes)){cancel();return {};}return p;
 }
 bool has(unsigned slot)const noexcept{return open_&&slot<2&&bool(cache_[slot]);}
 void retire(uint32_t address,size_t bytes)noexcept{
  if(!address||!bytes||uint64_t(address)+bytes>UINT64_C(0x100000000)){cancel();return;}
  for(auto&p:cache_)if(p){
   const auto&d=p->directory();if(logo::overlap(address,bytes,d.address,d.header[1])){cancel();return;}
   for(const auto&t:p->slots().target)if(logo::overlap(address,bytes,t.owner,56)||logo::overlap(address,bytes,t.w[8],RawSide)||logo::overlap(address,bytes,t.w[11],480*4)||logo::overlap(address,bytes,t.w[12],343*4)){cancel();return;}
  }
 }
};
} // namespace nfs_ls_scene

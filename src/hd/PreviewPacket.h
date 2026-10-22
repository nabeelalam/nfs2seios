// SPDX-License-Identifier: MIT
#pragma once
#include "SmallLogoPrepare.h"
#include "OriginalPreviewLayerAbi.h"
#include <memory>
#include <vector>
// Pure replay verifier, not a source of guest-read or publication authority.
// The live caller must bind every record to one current original owner/archive
// and completed call sequence before passing coherent owned observations here.
// No recorder, guest-pointer dereference, fallback or filesystem loader exists.
namespace nfs_location_preview {
using nfs_logo_capture::Target;using nfs_logo_capture::Plane;
namespace original=nfs_small_logo_live;
constexpr size_t Full=640*480,MaxDraws=3,MaxRows=480,MaxSource=16+Full+3+16+256*4;
constexpr uint32_t LocationForm=0x404abd,FillCallback=0xf0001aa8,IndexedCallback=0xf0001850;
struct Row {
 uint32_t callback=0,format=0,converter=0,pattern=0,paletteGuest=0;
 std::array<uint32_t,256>paletteBefore{},paletteAfter{};std::array<uint32_t,4>args{}; // Original EAX,EDX,EBX,ECX at the callback.
 std::vector<uint8_t>source; // Exact indexed bytes; empty for solid fill.
 std::vector<uint16_t>before,after; // Complete contiguous destination row.
};
struct Fill {
 Target before{},after{};std::array<uint32_t,4>call{}; // x,y,width,height at488620.
 uint32_t rawColor=0,pattern=0,callback=0,format=0,converter=0,complete=0,error=0;
 std::array<uint32_t,9>tableBefore{},tableAfter{};std::vector<Row>rows;
};
struct Draw {
 Target before{},after{};std::array<uint32_t,4>args{}; // resolved source,mode,x,y.
 std::array<uint8_t,4>name{};std::vector<uint8_t>sourceBefore,sourceAfter;
 std::array<unsigned char,32>sha{};
 std::array<uint32_t,256>paletteBefore{},paletteAfter{};
 std::array<uint32_t,9>tableBefore{},tableAfter{};
 uint32_t callback=0,paletteGuest=0,format=0,converter=0,complete=0,error=0;
 std::vector<Row>rows;
};
struct Input {
 uint64_t lifetime=0,serial=0;uint32_t archive=0,form=0,complete=0,error=0,fillCalls=0,drawCalls=0;
 Plane pre{},post{};Fill fill;std::vector<Draw>draws;
};
struct Rect {uint32_t left=0,top=0,width=0,height=0;};
struct SourceMetadata {
 std::array<uint8_t,4>name{};std::array<unsigned char,32>sha{};
 uint32_t sourceAddress=0,width=0,height=0,paletteOffset=0,paletteCount=0;
};
enum class Failure {None,Binding,Base,Target,Fill,Source,Palette,Rows,Pixels,Bounds,Allocation};
inline bool planeValid(const Plane&p)noexcept{return p.valid==1&&p.width==640&&p.height==480&&original::target(p.target,640)&&p.target.w[3]==0&&p.target.w[4]==0&&p.target.w[5]==640&&p.target.w[6]==480&&p.target.header[3]==0;}
// Canonical LE digest, independent of host padding/endian. Startup/refresh only.
inline bool wordsSha(const uint16_t*words,size_t count,std::array<unsigned char,32>&out)noexcept{
 if(!words||!count||count>Full)return false;
 try{std::vector<uint8_t>bytes(count*2);for(size_t k=0;k<count;++k){bytes[k*2]=uint8_t(words[k]);bytes[k*2+1]=uint8_t(words[k]>>8);}return nfs_texture_sha256(bytes.data(),bytes.size(),out.data())!=0;}catch(...){return false;}
}
class Packet;
inline std::shared_ptr<const Packet>createPacket(const Input&,const Plane&,Failure*failure=nullptr)noexcept;
class Packet final {
 Target target_;uint64_t lifetime_=0,serial_=0;uint32_t archive_=0;
 Rect rect_;std::vector<uint16_t>post_,pixels_;std::vector<uint8_t>coverage_;std::array<unsigned char,32>sha_{},coverageSha_{},baseSha_{};
 std::vector<SourceMetadata>sources_;
 Packet()=default;
 friend std::shared_ptr<const Packet>createPacket(const Input&,const Plane&,Failure*)noexcept;
public:
 const Target&target()const noexcept{return target_;}uint64_t lifetime()const noexcept{return lifetime_;}uint64_t serial()const noexcept{return serial_;}uint32_t archive()const noexcept{return archive_;}
 const Rect&bounds()const noexcept{return rect_;}const std::vector<uint16_t>&post()const noexcept{return post_;}const std::vector<uint16_t>&pixels()const noexcept{return pixels_;}
 // Coverage describes callback writes, even when a write kept the same value.
 // Partial-alpha writes retain exact original RGB565 blending; uncovered holes
 // must remain transparent when composited over the authored background.
 const std::vector<uint8_t>&coverage()const noexcept{return coverage_;}
 const std::array<unsigned char,32>&sha256()const noexcept{return sha_;}
 const std::array<unsigned char,32>&coverageSha256()const noexcept{return coverageSha_;}
 // Borrowed view remains valid only while this immutable Packet is retained.
 NfsOriginalPreviewLayerView view()const noexcept{NfsOriginalPreviewLayerView v{};v.schema=NFS_ORIGINAL_PREVIEW_LAYER_SCHEMA;v.left=int32_t(rect_.left);v.top=int32_t(rect_.top);v.width=rect_.width;v.height=rect_.height;v.pixels565=pixels_.data();v.pixelCount=pixels_.size();std::copy(sha_.begin(),sha_.end(),v.sha256);v.coverage=coverage_.data();v.coverageCount=coverage_.size();std::copy(coverageSha_.begin(),coverageSha_.end(),v.coverageSha256);return v;}const std::vector<SourceMetadata>&sources()const noexcept{return sources_;}
 bool baseMatches(const Plane&base)const noexcept{std::array<unsigned char,32>sha{};return planeValid(base)&&nfs_logo_capture::same(target_,base.target)&&wordsSha(base.pixels.data(),Full,sha)&&sha==baseSha_;}
 // Accepted serial/lifetime/archive must come from CURRENT original-owner state,
 // never from this candidate itself; addresses are correlation, not read authority.
 bool matches(const Target&t,uint64_t serial,uint64_t lifetime,uint32_t archive,const uint16_t*post,size_t count)const noexcept{return serial==serial_&&lifetime==lifetime_&&archive==archive_&&nfs_logo_capture::same(t,target_)&&post&&count==Full&&!std::memcmp(post_.data(),post,Full*2);}
};
inline std::shared_ptr<const Packet>createPacket(const Input&i,const Plane&base,Failure*failure)noexcept{
 auto reject=[&](Failure f)->std::shared_ptr<const Packet>{if(failure)*failure=f;return {};};
 if(failure)*failure=Failure::None;
 if(!i.lifetime||!i.serial||!i.archive||i.form!=LocationForm||i.complete!=1||i.error||i.fillCalls!=1||i.drawCalls!=i.draws.size()||i.draws.empty()||i.draws.size()>MaxDraws)return reject(Failure::Binding);
 if(!planeValid(base)||!planeValid(i.pre)||!nfs_logo_capture::same(base.target,i.pre.target)||base.pixels!=i.pre.pixels)return reject(Failure::Base);
 if(!planeValid(i.post)||!nfs_logo_capture::same(i.pre.target,i.post.target))return reject(Failure::Target);
 try{
  std::vector<uint16_t>replay(base.pixels.begin(),base.pixels.end());std::vector<uint8_t>coverage(Full);const auto&t=i.pre.target;const uint64_t destination=uint64_t(t.w[8])+16;
  int left=640,top=480,right=0,bottom=0;
  auto include=[&](int l,int y,int r,int b){if(l<r&&y<b){left=std::min(left,l);top=std::min(top,y);right=std::max(right,r);bottom=std::max(bottom,b);}};
  auto same=[&](const Target&a,const Target&b){return nfs_logo_capture::same(a,t)&&nfs_logo_capture::same(b,t);};
  const auto&f=i.fill;const std::array<uint32_t,4>fillCall{{211,316,280,128}};
  const std::array<uint32_t,9>fillTable{{0xf0001aa0,0xf0001aa4,0xf0001aa8,0xf0001aa8,0xf0001aac,0xf0001ab0,0,0,0}};
  if(!same(f.before,f.after)||f.complete!=1||f.error||f.call!=fillCall||f.rawColor!=0xff000000u||f.pattern||f.callback!=FillCallback||f.format!=16||f.converter||f.tableBefore!=fillTable||f.tableAfter!=fillTable||f.rows.size()!=128)return reject(Failure::Fill);
  include(211,316,491,444);
  for(unsigned y=0;y<128;++y){const auto&r=f.rows[y];const uint32_t dest=uint32_t(destination+uint64_t((316+y)*640+211)*2);
   if(r.callback!=FillCallback||r.format!=f.format||r.converter!=f.converter||r.pattern!=f.pattern||r.args[1]!=dest||r.args[2]!=280||!r.source.empty()||r.before.size()!=280||r.after.size()!=280)return reject(Failure::Rows);
   for(unsigned x=0;x<280;++x){const size_t at=(316+y)*640+211+x;if(r.before[x]!=replay[at]||r.after[x]!=0)return reject(Failure::Pixels);replay[at]=0;coverage[at]=255;}
  }
  std::vector<SourceMetadata>sources;sources.reserve(i.draws.size());
  for(const auto&d:i.draws){
   if(!same(d.before,d.after)||d.complete!=1||d.error||d.args[1]!=0x3000||!d.args[0]||d.sourceBefore.size()<32||d.sourceBefore.size()>MaxSource||d.sourceBefore!=d.sourceAfter||uint64_t(d.args[0])+d.sourceBefore.size()>0x100000000ULL||original::overlap(d.args[0],uint32_t(d.sourceBefore.size()),t.w[8],16+Full*2)||!original::hashMatches(d.sourceBefore.data(),uint32_t(d.sourceBefore.size()),d.sha))return reject(Failure::Source);
   for(uint8_t ch:d.name)if(ch<32||ch>126)return reject(Failure::Source);
   const auto&bytes=d.sourceBefore;const uint32_t first=original::le32(bytes.data()),wh=original::le32(bytes.data()+4),width=wh&65535,height=wh>>16,offset=first>>8;
   const uint64_t imageEnd=16+uint64_t(width)*height;
   if((first&127)!=0x7b||!width||width>640||!height||height>480||imageEnd>offset||offset-imageEnd>3||uint64_t(offset)+16>bytes.size())return reject(Failure::Source);
   const uint32_t paletteHeader=original::le32(bytes.data()+offset),count=original::le32(bytes.data()+offset+4)&65535;
   if((paletteHeader&255)!=0x2a||!count||count>256||uint64_t(offset)+16+count*4>bytes.size()||bytes.size()-(uint64_t(offset)+16+count*4)>3)return reject(Failure::Palette);
   if(d.callback!=IndexedCallback||d.format!=16||d.converter||!d.paletteGuest||uint64_t(d.paletteGuest)+1024>0x100000000ULL||original::overlap(d.paletteGuest,1024,t.w[8],16+Full*2)||d.paletteBefore!=d.paletteAfter||!original::tables(d.tableBefore,d.tableAfter,true,IndexedCallback,0x7b))return reject(Failure::Palette);
   for(unsigned k=0;k<count;++k){const uint32_t raw=original::le32(bytes.data()+offset+16+k*4);if(d.paletteBefore[k]!=((raw&0xff000000u)|original::pack(raw)))return reject(Failure::Palette);}
   if(count<256&&d.paletteBefore[255])return reject(Failure::Palette);
   const int64_t x=int32_t(d.args[2]),y=int32_t(d.args[3]);const int l=int(std::max<int64_t>(0,std::min<int64_t>(640,x))),up=int(std::max<int64_t>(0,std::min<int64_t>(480,y))),r=int(std::max<int64_t>(0,std::min<int64_t>(640,x+width))),down=int(std::max<int64_t>(0,std::min<int64_t>(480,y+height)));
   const unsigned rows=l<r&&up<down?unsigned(down-up):0;
   if(d.rows.size()!=rows||d.rows.size()>MaxRows)return reject(Failure::Rows);
   for(unsigned row=0;row<rows;++row){const auto&observed=d.rows[row];const unsigned yy=unsigned(up)+row,n=unsigned(r-l),sx=unsigned(int64_t(l)-x),sy=unsigned(int64_t(yy)-y);const uint32_t source=d.args[0]+16+sy*width+sx,dest=uint32_t(destination+uint64_t(yy*640+l)*2);
    if(observed.callback!=IndexedCallback||observed.format!=d.format||observed.converter!=d.converter||observed.paletteGuest!=d.paletteGuest||observed.paletteBefore!=d.paletteBefore||observed.paletteAfter!=d.paletteAfter||observed.args[0]!=source||observed.args[1]!=dest||observed.args[2]!=n||observed.source.size()!=n||observed.before.size()!=n||observed.after.size()!=n||std::memcmp(observed.source.data(),bytes.data()+16+sy*width+sx,n))return reject(Failure::Rows);
    for(unsigned k=0;k<n;++k){const unsigned index=observed.source[k];if(index>=count&&index!=255)return reject(Failure::Palette);const size_t at=yy*640+l+k;const uint16_t expected=original::pixel(replay[at],d.paletteBefore[index],true);if(observed.before[k]!=replay[at]||observed.after[k]!=expected)return reject(Failure::Pixels);replay[at]=expected;if(d.paletteBefore[index]>=0x10000000u){coverage[at]=255;include(l+int(k),int(yy),l+int(k)+1,int(yy)+1);}}
   }
   SourceMetadata metadata;metadata.name=d.name;metadata.sha=d.sha;metadata.sourceAddress=d.args[0];metadata.width=width;metadata.height=height;metadata.paletteOffset=offset;metadata.paletteCount=count;sources.push_back(metadata);
  }
  if(replay.size()!=Full||!std::equal(replay.begin(),replay.end(),i.post.pixels.begin()))return reject(Failure::Pixels);
  if(left<0||top<0||right>640||bottom>480||left>=right||top>=bottom)return reject(Failure::Bounds);
  for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)if((x<unsigned(left)||x>=unsigned(right)||y<unsigned(top)||y>=unsigned(bottom))&&replay[y*640+x]!=base.pixels[y*640+x])return reject(Failure::Bounds);
  auto packet=std::shared_ptr<Packet>(new Packet);packet->target_=t;packet->lifetime_=i.lifetime;packet->serial_=i.serial;packet->archive_=i.archive;packet->rect_={uint32_t(left),uint32_t(top),uint32_t(right-left),uint32_t(bottom-top)};packet->post_=std::move(replay);packet->sources_=std::move(sources);packet->pixels_.resize(size_t(right-left)*(bottom-top));packet->coverage_.resize(packet->pixels_.size());
  for(int y=top;y<bottom;++y){std::copy_n(packet->post_.data()+y*640+left,right-left,packet->pixels_.data()+size_t(y-top)*(right-left));std::copy_n(coverage.data()+y*640+left,right-left,packet->coverage_.data()+size_t(y-top)*(right-left));}
  if(!wordsSha(packet->pixels_.data(),packet->pixels_.size(),packet->sha_)||!wordsSha(base.pixels.data(),Full,packet->baseSha_)||!nfs_texture_sha256(packet->coverage_.data(),packet->coverage_.size(),packet->coverageSha_.data()))return reject(Failure::Allocation);
  return packet;
 }catch(...){return reject(Failure::Allocation);}
}
} // namespace nfs_location_preview

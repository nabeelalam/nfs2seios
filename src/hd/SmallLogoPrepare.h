// SPDX-License-Identifier: MIT
#pragma once
#include "OriginalUiRecords.h"
#include "SmallLogoLayerAbi.h"
#include "TextureDigest.h"
#include <algorithm>
namespace nfs_small_logo_live {
using nfs_logo_capture::Target;
using nfs_logo_capture::Group;
using nfs_logo_capture::Copy;
constexpr unsigned SourcePixels=343*480,DestinationPixels=640*480;
enum class Failure {None,Group,Target,Source,Palette,Table,Row,GroupPixels,Copy,CopyPixels,Union};
struct Prepared {
 bool valid=false;
 uint32_t width=0,height=0,layerCount=0;
 std::array<uint16_t,DestinationPixels> cleanCopyPost565{};
 std::array<uint8_t,DestinationPixels> copyAffected{},groupCleanupAffected{};
 std::array<NfsSmallLogoLayer,3> layers{};
};
struct Workspace {
 std::array<uint16_t,SourcePixels> groupReplay{},cleanSource{};
 std::array<uint16_t,DestinationPixels> copyReplay{};
 std::array<uint8_t,SourcePixels> groupUnion{};
};
struct SourceIdentity {uint32_t span,width,height,paletteOffset,paletteCount,maxIndex;std::array<uint32_t,4> header;std::array<unsigned char,32> sha;};
// Trusted caller-owned catalogue; no filesystem loader or built-in original identities.
struct SourceCatalog {
 std::array<SourceIdentity,5> identities{};
 std::array<uint32_t,5> roles{};
};
inline bool validCatalog(const SourceCatalog& c) noexcept {
 for(unsigned k=0;k<5;++k){const auto&i=c.identities[k];bool nonzero=false;for(auto b:i.sha)nonzero|=b!=0;
  if(c.roles[k]!=k||!nonzero||!i.width||i.width>343||!i.height||i.height>480||i.span>nfs_logo_capture::MaxSource||i.span<16||!i.paletteCount||i.paletteCount>256||i.maxIndex>=i.paletteCount||uint64_t(16)+uint64_t(i.width)*i.height>i.paletteOffset||uint64_t(i.paletteOffset)+16+4*i.paletteCount>i.span||(i.header[0]&255)!=0x7b||i.header[1]!=(i.width|(i.height<<16)))return false;
  for(unsigned j=0;j<k;++j)if(i.sha==c.identities[j].sha)return false;
 }return true;
}
inline uint16_t le16(const uint8_t*p) noexcept {return uint16_t(p[0])|uint16_t(uint16_t(p[1])<<8);}
inline uint32_t le32(const uint8_t*p) noexcept {return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
inline int signed12(uint32_t x) noexcept {x&=4095;return (x&2048)?int(x)-4096:int(x);}
inline uint16_t pack(uint32_t c) noexcept {return uint16_t(((c>>8)&0xf800)|((c>>5)&0x7e0)|((c>>3)&31));}
// Original 483288/482BD4/483070 arithmetic, source-reviewed against replay-v1.
inline uint16_t pixel(uint16_t old,uint32_t c,bool full) noexcept {
 if(c<0x10000000)return old;
 if(c>=0xfc000000)return full?uint16_t(c):pack(c);
 const uint32_t spread=(old&31u)|((old&0x7e0u)<<8)|((old&0xf800u)<<16);
 const uint64_t p=uint64_t(spread)*(255-(c>>24));
 if(full)return uint16_t(c+(((p>>32)<<8)&0xf800)+((p>>16)&0x7e0)+((p>>8)&31));
 return pack(uint32_t(c+((p>>32)<<16)+((p>>5)&255)+((p>>11)&0xff00)));
}
inline uint32_t expanded(uint16_t w,uint32_t key) noexcept {return w==key?0:0xff000000u|((w&0xf800u)<<8)|((w&0x7e0u)<<5)|((w&31u)<<3);}
inline bool hashMatches(const uint8_t*p,uint32_t n,const std::array<unsigned char,32>&h) noexcept {std::array<unsigned char,32>a{};return nfs_texture_sha256(p,n,a.data())&&a==h;}
inline bool overlap(uint32_t a,uint32_t an,uint32_t b,uint32_t bn) noexcept {return uint64_t(a)<uint64_t(b)+bn&&uint64_t(b)<uint64_t(a)+an;}
inline bool target(const Target&t,unsigned width) noexcept {
 const auto&w=t.w;
 if(w[1]!=width||w[2]!=480||w[7]!=0x7810||w[10]!=width*2||w[13]||!w[8]||!w[11]||!w[12]||!t.owner)return false;
 if((t.header[0]&127)!=0x78||t.header[1]!=(width|(480u<<16)))return false;
 if(w[3]>w[5]||w[5]>width||w[4]>w[6]||w[6]>480)return false;
 for(unsigned i=0;i<width;++i)if(t.x[i]!=2*i)return false;
 for(unsigned i=0;i<480;++i)if(t.y[i]!=16+i*width*2)return false;
 for(unsigned i:{0u,1u,2u,7u,8u,10u,11u,12u,13u})if(t.ownerWords[i]!=w[i])return false;
 return uint64_t(w[8])+16+width*480*2<=0x100000000ULL;
}
inline bool tables(const std::array<uint32_t,9>&a,const std::array<uint32_t,9>&b,bool full,uint32_t callback,uint32_t format) noexcept {
 static constexpr uint32_t f[6]={4026531948u,4026538044u,4026538048u,4026538052u,4026538056u,4026538060u};
 static constexpr uint32_t s[6]={4026531948u,4026537988u,4026537992u,4026537996u,4026538000u,4026538004u};
 for(unsigned i=0;i<6;++i)if(a[i]!=(full?f[i]:s[i])||b[i]!=a[i])return false;
 return a[7]==3&&b[7]==3&&b[6]==callback&&b[8]==format;
}
// No Store/sample flags, file I/O, allocation, guest reads or retained input pointers.
// Caller supplies coherent immutable native-owned records and nonaliasing workspace/output.
inline Failure verifyAndPrepare(const Group&g,const Copy&c,Workspace&ws,Prepared&out,const SourceCatalog&catalog) noexcept {
 out.valid=false;out.width=out.height=out.layerCount=0;
 if(!validCatalog(catalog))return Failure::Source;
 if(!g.serial||!g.thread||g.kind!=1||g.error||g.complete!=1||g.stage!=2||g.draws!=3)return Failure::Group;
 if(g.pre.valid!=1||g.post.valid!=1||g.pre.width!=343||g.post.width!=343||g.pre.height!=480||g.post.height!=480||!nfs_logo_capture::same(g.pre.target,g.post.target)||!target(g.pre.target,343))return Failure::Target;
 if(g.draw[0].role<1||g.draw[0].role>4||g.draw[1].role!=g.draw[0].role%4+1||g.draw[2].role!=0)return Failure::Group;
 std::copy_n(g.pre.pixels.begin(),SourcePixels,ws.groupReplay.begin());ws.groupUnion.fill(0);
 for(unsigned i=0;i<3;++i){
  const auto&d=g.draw[i];if(d.role>=5)return Failure::Source;const auto&a=catalog.identities[d.role];
  if(d.error||d.complete!=1||d.resolves!=1||!nfs_logo_capture::same(d.before,g.pre.target)||!nfs_logo_capture::same(d.after,g.pre.target))return Failure::Group;
  if(d.span!=a.span||d.sourceHeader!=a.header||d.shaBefore!=a.sha||d.shaAfter!=a.sha||std::memcmp(d.sourceBefore.data(),d.sourceAfter.data(),a.span)||!hashMatches(d.sourceBefore.data(),a.span,a.sha))return Failure::Source;
  for(unsigned j=0;j<4;++j)if(le32(d.sourceBefore.data()+4*j)!=a.header[j])return Failure::Source;
  const bool full=i!=1;const uint32_t opacity=d.args[1]&511,cb=full?0xf0001850u:0xf0001818u;
  bool coefficient=false;for(unsigned j=0;j<80;++j)if(opacity==(j*255u)/80u)coefficient=true;
  if((full&&d.args[1]!=0x2800)||(!full&&((d.args[1]&~511u)!=0x1c00||!coefficient))||!d.args[0]||d.args[2]||d.args[3]||uint64_t(d.args[0])+d.span>0x100000000ULL)return Failure::Source;
  if(d.callback!=cb||d.callbackGlobal!=cb||d.callbackFormat!=0x7b||!d.paletteGuest||d.paletteValid!=1||d.paletteCount!=a.paletteCount||d.paletteFirst!=d.paletteLast)return Failure::Palette;
  if(!tables(d.tableBefore,d.tableAfter,full,cb,0x7b))return Failure::Table;
  for(unsigned j=0;j<a.paletteCount;++j){uint32_t p=le32(d.sourceBefore.data()+a.paletteOffset+16+4*j),v=0;if(full)v=(p&0xff000000u)|pack(p);else for(unsigned k=0;k<4;++k)v|=((((p>>(8*k))&255)*opacity)>>8)<<(8*k);if(d.paletteFirst[j]!=v)return Failure::Palette;}
  if(d.paletteFirst[255])return Failure::Palette;for(unsigned j=a.paletteCount;j<255;++j)if(d.paletteFirst[j])return Failure::Palette;
  unsigned maxIndex=0;for(unsigned j=0;j<a.width*a.height;++j){unsigned v=d.sourceBefore[16+j];if(v>=a.paletteCount)return Failure::Palette;maxIndex=std::max(maxIndex,v);}if(maxIndex!=a.maxIndex||d.actualMaxIndex!=a.maxIndex)return Failure::Palette;
  const int x=signed12(a.header[3]),y=signed12(a.header[3]>>16);const auto&w=g.pre.target.w;
  const int l=std::max(x,int(w[3])),t=std::max(y,int(w[4])),r=std::min(x+int(a.width),int(w[5])),b=std::min(y+int(a.height),int(w[6]));
  if(l>=r||t>=b||d.rows!=unsigned(b-t)||d.rows>480||overlap(d.args[0],d.span,w[8],16+SourcePixels*2))return Failure::Row;
  for(int yy=t;yy<b;++yy){const auto&row=d.row[unsigned(yy-t)];const uint32_t sx=unsigned(l-x),sy=unsigned(yy-y),n=unsigned(r-l);const uint64_t dest=uint64_t(w[8])+g.pre.target.y[unsigned(yy)]+g.pre.target.x[unsigned(l)],src=uint64_t(d.args[0])+16+sy*a.width+sx;
   if(row.callback!=cb||row.args[0]!=src||row.args[1]!=dest||row.args[2]!=n||dest+2*n>0x100000000ULL)return Failure::Row;
   for(unsigned k=0;k<n;++k){const unsigned at=unsigned(yy)*343+unsigned(l)+k;ws.groupUnion[at]=1;ws.groupReplay[at]=pixel(ws.groupReplay[at],d.paletteFirst[d.sourceBefore[16+sy*a.width+sx+k]],full);}
  }
 }
 if(!std::equal(ws.groupReplay.begin(),ws.groupReplay.end(),g.post.pixels.begin()))return Failure::GroupPixels;
 if(!c.serial||c.thread!=g.thread||c.linkedGroup!=g.serial||c.error||c.complete!=1||c.span!=16+SourcePixels*2||c.pre.valid!=1||c.post.valid!=1||c.pre.width!=640||c.post.width!=640||c.pre.height!=480||c.post.height!=480||!nfs_logo_capture::same(c.pre.target,c.post.target)||!target(c.pre.target,640)||!target(c.sourceOwner,343))return Failure::Copy;
 const auto&st=c.sourceOwner;const auto&gt=g.post.target;const auto&dw=c.pre.target.w;
 if(c.args[0]!=st.w[8]||c.args[1]!=0x2800||c.args[2]||c.args[3]||st.owner!=gt.owner||st.x!=gt.x||st.y!=gt.y||st.w[8]!=gt.w[8])return Failure::Copy;
 if(c.shaBefore!=c.shaAfter||std::memcmp(c.sourceBefore.data(),c.sourceAfter.data(),c.span)||!hashMatches(c.sourceBefore.data(),c.span,c.shaBefore)||overlap(c.args[0],c.span,dw[8],16+DestinationPixels*2))return Failure::Copy;
 for(unsigned j=0;j<4;++j)if(le32(c.sourceBefore.data()+4*j)!=st.header[j])return Failure::Copy;
 if(!tables(c.tableBefore,c.tableAfter,true,0,0x7d))return Failure::Table;
 for(unsigned j=0;j<SourcePixels;++j){uint16_t src=le16(c.sourceBefore.data()+16+2*j);if(ws.groupUnion[j]&&src!=g.post.pixels[j])return Failure::Union;ws.cleanSource[j]=ws.groupUnion[j]?g.pre.pixels[j]:src;}
 const int x=signed12(st.header[3]),y=signed12(st.header[3]>>16),l=std::max(x,int(dw[3])),t=std::max(y,int(dw[4])),r=std::min(x+343,int(dw[5])),b=std::min(y+480,int(dw[6]));
 if(l>=r||t>=b||c.rows!=unsigned(b-t)||c.rows>480)return Failure::Row;
 std::copy_n(c.pre.pixels.begin(),DestinationPixels,ws.copyReplay.begin());std::copy_n(c.pre.pixels.begin(),DestinationPixels,out.cleanCopyPost565.begin());out.copyAffected.fill(0);out.groupCleanupAffected.fill(0);
 const uint32_t key=c.row[0].colorKey;
 for(int yy=t;yy<b;++yy){const auto&row=c.row[unsigned(yy-t)];const auto&rr=row.row;const uint32_t n=unsigned(r-l),sx=unsigned(l-x),sy=unsigned(yy-y);const uint64_t dest=uint64_t(dw[8])+c.pre.target.y[unsigned(yy)]+c.pre.target.x[unsigned(l)];
  if(rr.callback!=0xf0001844||row.finalCallback!=rr.callback||row.converter!=0xf0001f44||row.colorKey!=key||row.bytes!=4*n||!rr.args[0]||uint64_t(rr.args[0])+4*n>0x100000000ULL||rr.args[1]!=dest||rr.args[2]!=n||overlap(rr.args[0],4*n,dw[8],16+DestinationPixels*2))return Failure::Row;
  for(unsigned k=0;k<n;++k){const unsigned sourceAt=sy*343+sx+k,at=unsigned(yy)*640+unsigned(l)+k;const uint32_t actual=le32(row.source.data()+4*k),expect=expanded(le16(c.sourceBefore.data()+16+2*sourceAt),key);if(actual!=expect)return Failure::Copy;
   ws.copyReplay[at]=pixel(ws.copyReplay[at],actual,false);out.cleanCopyPost565[at]=pixel(out.cleanCopyPost565[at],expanded(ws.cleanSource[sourceAt],key),false);out.copyAffected[at]=1;out.groupCleanupAffected[at]=ws.groupUnion[sourceAt];
  }
 }
 if(!std::equal(ws.copyReplay.begin(),ws.copyReplay.end(),c.post.pixels.begin()))return Failure::CopyPixels;
 for(unsigned i=0;i<3;++i){const auto&d=g.draw[i];const auto&a=catalog.identities[d.role];auto&layer=out.layers[i];layer.role=d.role;layer.scalar256=i==1?(d.args[1]&511):256;layer.left=float(signed12(a.header[3])+x);layer.top=float(signed12(a.header[3]>>16)+y);layer.right=layer.left+float(a.width);layer.bottom=layer.top+float(a.height);
  layer.clip[0]=float(std::max(l,int(g.pre.target.w[3])+x));layer.clip[1]=float(std::max(t,int(g.pre.target.w[4])+y));layer.clip[2]=float(std::min(r,int(g.pre.target.w[5])+x));layer.clip[3]=float(std::min(b,int(g.pre.target.w[6])+y));
  if(layer.clip[0]>=layer.clip[2]||layer.clip[1]>=layer.clip[3])return Failure::Row;
 }
 out.width=640;out.height=480;out.layerCount=3;out.valid=true;return Failure::None;
}
}

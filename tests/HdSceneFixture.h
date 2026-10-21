#pragma once
// SPDX-License-Identifier: MIT
#include "hd/BackgroundScenePrepare.h"
#include <cstdio>
#include <memory>
#include <stdexcept>
namespace bg=nfs_background_scene_cpu;
namespace logo=nfs_small_logo_live;
namespace rec=nfs_logo_capture;
#define CHECK(x) do { if(!(x)) throw std::runtime_error(#x); } while(0)
static void put32(uint8_t*p,uint32_t v){for(unsigned k=0;k<4;++k)p[k]=uint8_t(v>>(8*k));}
static void put16(uint8_t*p,uint16_t v){p[0]=uint8_t(v);p[1]=uint8_t(v>>8);}
// Independent opaque RGB565 oracle: no production pixel/pack helper is called.
static uint16_t opaque(uint32_t c){return uint16_t((((c>>16)&255)/8)*2048+(((c>>8)&255)/4)*32+(c&255)/8);}
static uint32_t rgba(uint16_t v){return 0xff000000u|uint32_t(v/2048)*524288u|uint32_t((v/32)%64)*1024u|uint32_t(v%32)*8u;}
static std::array<uint8_t,32> digest(const std::vector<uint8_t>&v){std::array<uint8_t,32>a{};CHECK(nfs_texture_sha256(v.data(),v.size(),a.data()));return a;}
static std::vector<uint8_t> indexed(unsigned width,unsigned height,uint32_t color){std::vector<uint8_t>v(16+width*height+1040);put32(v.data(),0x7b);put32(v.data()+4,width|(height<<16));std::fill(v.begin()+16,v.begin()+16+width*height,1);auto*pal=v.data()+16+width*height;put32(pal,0x2a);put32(pal+4,256);put32(pal+20,color);return v;}
static rec::Target target(unsigned width,uint32_t bitmap,uint32_t owner){rec::Target t{};t.w[1]=width;t.w[2]=480;t.w[5]=width;t.w[6]=480;t.w[7]=0x7810;t.w[8]=bitmap;t.w[10]=2*width;t.w[11]=width==640?0x30000:0x32000;t.w[12]=width==640?0x31000:0x33000;t.header[0]=0x78;t.header[1]=width|(480u<<16);for(unsigned k=0;k<width;++k)t.x[k]=2*k;for(unsigned k=0;k<480;++k)t.y[k]=16+2*width*k;t.owner=owner;t.ownerWords=t.w;t.ownerState[1]=0x51000;t.ownerState[3]=0x41000;return t;}
static void plane(rec::Plane&p,const rec::Target&t,unsigned width,uint16_t color){p.target=t;p.width=width;p.height=480;p.valid=1;p.pixels.fill(color);}
static void table(std::array<uint32_t,9>&v,bool full,uint32_t callback,uint32_t format){const uint32_t f[6]={4026531948u,4026538044u,4026538048u,4026538052u,4026538056u,4026538060u},s[6]={4026531948u,4026537988u,4026537992u,4026537996u,4026538000u,4026538004u};for(unsigned k=0;k<6;++k)v[k]=(full?f:s)[k];v[6]=callback;v[7]=3;v[8]=format;}
struct Fixture {
 bg::SourceCatalog catalog;
 bg::Base base;bg::Scene scene;rec::Group group;rec::Copy copy;
 bg::Workspace workspace;bg::Prepared output;
 std::array<uint32_t,300>keys{};
 std::array<std::vector<uint8_t>,5>logoBytes;
 uint16_t sideColor=opaque(0xff2468acu),wordColor=opaque(0xffad6231u),frameColor=opaque(0xffdd8844u);
 Fixture(){
  const auto full=target(640,0x100000,0x41000),side=target(343,0x200000,0x51000);
  base.event={1,1,0};base.form=bg::MainForm;base.mapRole=0;base.sourceBefore.resize(614416);put32(base.sourceBefore.data(),0x78);put32(base.sourceBefore.data()+4,640|(480u<<16));for(unsigned k=0;k<bg::Full;++k)put16(base.sourceBefore.data()+16+2*k,0x2a55);base.sourceAfter=base.sourceBefore;plane(base.post,full,640,0x2a55);catalog.mapSha[0]=digest(base.sourceBefore);auto otherMap=base.sourceBefore;otherMap[16]^=1;catalog.mapSha[1]=digest(otherMap);catalog.mapRoles={{0,1}};
  for(unsigned k=0;k<13;++k){auto bytes=indexed(k?1:343,k?1:480,k?0xffad6231u+k-1:0xff2468acu);auto&i=catalog.identities[k];i.span=bytes.size();i.width=k?1:343;i.height=k?1:480;i.paletteOffset=16+i.width*i.height;std::copy_n(bytes.data(),16,i.header.begin());i.sha=digest(bytes);catalog.roles[k]=k;
   if(k<2){bg::Draw d;d.role=k;d.complete=1;d.args={{0x400000+0x40000*k,k?0x1cffu:0x400u,0,0}};d.before=d.after=side;d.sourceBefore=d.sourceAfter=bytes;d.paletteValid=1;d.paletteGuest=0x80000;d.format=k?0x7b:16;d.callback=k?0xf0001818:0xf000221c;d.callbackGlobal=k?d.callback:0;d.paletteBefore[1]=k?0xffad6231u:sideColor;d.paletteAfter=d.paletteBefore;
    if(k)table(d.tableBefore,false,0,0);else d.tableBefore={{0xf0001874,0xf0002124,0xf0001878,0xf0001878,0xf000187c,0xf0001880,0,2,0}};
    d.tableAfter=d.tableBefore;if(k){d.tableAfter[6]=d.callback;d.tableAfter[8]=0x7b;}
    bg::Row row;row.callback=d.callback;row.unit=1;row.args={{d.args[0]+16,side.w[8]+16,k?1:bg::Side,0}};row.payload.assign(bytes.begin()+16,bytes.begin()+16+(k?1:bg::Side));d.rows.push_back(std::move(row));scene.draws.push_back(std::move(d));
   }
  }
  scene.event={1,2,1};scene.form=bg::MainForm;scene.complete=1;plane(scene.preSide,side,343,0x7777);plane(scene.postSide,side,343,sideColor);scene.postWords=scene.postSide;scene.postWords.pixels[0]=wordColor;
  for(unsigned k=0;k<5;++k){logoBytes[k]=indexed(1,1,k?0xffa02050u+k:0xffdd8844u);auto&i=catalog.logos.identities[k];i.span=logoBytes[k].size();i.width=i.height=1;i.paletteOffset=17;i.paletteCount=256;i.maxIndex=1;for(unsigned z=0;z<4;++z)i.header[z]=bg::le32(logoBytes[k].data()+4*z);i.sha=digest(logoBytes[k]);catalog.logos.roles[k]=k;}
  group.serial=17;group.thread=29;group.kind=1;group.complete=1;group.stage=2;group.draws=3;group.pre=scene.postWords;group.post=group.pre;group.post.pixels[0]=frameColor;
  const unsigned roles[3]={1,2,0};
  for(unsigned k=0;k<3;++k){auto&d=group.draw[k];unsigned role=roles[k];auto&i=catalog.logos.identities[role];d.role=role;d.span=i.span;d.paletteCount=256;d.actualMaxIndex=1;d.complete=d.resolves=d.rows=1;d.args={{0x600000+role*0x10000,k==1?0x1c00u:0x2800u,0,0}};d.sourceHeader=i.header;d.before=d.after=side;d.shaBefore=d.shaAfter=i.sha;std::copy(logoBytes[role].begin(),logoBytes[role].end(),d.sourceBefore.begin());d.sourceAfter=d.sourceBefore;d.callback=d.callbackGlobal=k==1?0xf0001818:0xf0001850;d.callbackFormat=0x7b;d.paletteGuest=0x82000;d.paletteValid=1;uint32_t raw=bg::le32(logoBytes[role].data()+17+20);d.paletteFirst[1]=k==1?0:(0xff000000u|opaque(raw));d.paletteLast=d.paletteFirst;table(d.tableBefore,k!=1,0,0);table(d.tableAfter,k!=1,d.callback,0x7b);d.row[0].callback=d.callback;d.row[0].args={{d.args[0]+16,side.w[8]+16,1,0}};}
  scene.tailKind=NFS_BACKGROUND_TAIL_MAIN_SKIP;scene.tailPredicate=scene.tailSymbol=bg::MainForm;scene.tailPre=scene.tailPost=group.post;
  copy.serial=31;copy.thread=29;copy.linkedGroup=17;copy.complete=1;copy.span=rec::MaxCopySource;copy.rows=480;copy.args={{side.w[8],0x2800,0,0}};copy.pre=copy.post=base.post;copy.sourceOwner=side;for(unsigned z=0;z<4;++z)put32(copy.sourceBefore.data()+4*z,side.header[z]);for(unsigned k=0;k<bg::Side;++k)put16(copy.sourceBefore.data()+16+2*k,group.post.pixels[k]);copy.sourceAfter=copy.sourceBefore;CHECK(nfs_texture_sha256(copy.sourceBefore.data(),copy.span,copy.shaBefore.data()));copy.shaAfter=copy.shaBefore;table(copy.tableBefore,true,0,0);table(copy.tableAfter,true,0,0x7d);
  for(unsigned y=0;y<480;++y){auto&row=copy.row[y];row.row.callback=row.finalCallback=0xf0001844;row.converter=0xf0001f44;row.bytes=1372;row.row.args={{0x900000+y*1372,full.w[8]+16+y*1280,343,0}};for(unsigned x=0;x<343;++x){uint16_t value=group.post.pixels[y*343+x];put32(row.source.data()+4*x,rgba(value));copy.post.pixels[y*640+x]=value;}}
  for(unsigned k=0;k<300;++k)keys[k]=k*2048;
 }
 bg::Failure verify(){bg::JoinedInput input{base,scene,group,copy,{1,3,1},{1,4,1},{1,5,1},{1,6,1},copy.post.target,1,1,keys};return bg::verifyAndPrepare(input,workspace,output,catalog);}
};

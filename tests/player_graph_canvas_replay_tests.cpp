// SPDX-License-Identifier: MIT
#include "../src/hd/PlayerGraphCanvasReplay.h"
#include <cstdio>
#include <cstdlib>
namespace replay=nfs_player_graph_replay;
struct Canvas {std::array<uint32_t,14>words{};std::array<uint32_t,4>header{};std::array<uint32_t,384>x{};std::array<uint32_t,160>y{};unsigned bits=0;std::vector<uint8_t>pixels;};
struct Row {std::array<uint32_t,4>args{};uint32_t callback=0,format=0,converter=0,forward=0,palette=0;bool sourceSpanKnown=true;std::array<uint32_t,256>paletteBefore{},paletteAfter{};std::vector<uint8_t>sourceBefore,sourceAfter,before,after;};
struct Draw {std::array<uint8_t,4>name{};std::array<uint32_t,4>args{};std::array<uint32_t,9>tableBefore{},tableAfter{};std::vector<uint8_t>sourceBefore,sourceAfter;std::vector<Row>rows;};
struct Selection {uint32_t selectedCar=0,cachedCarIndex=0,graphEnabled=1;std::array<uint32_t,4>graphValues{{10,12,12,9}};};
struct Photo {Canvas before,after;uint32_t rawColor=0xff000000,packedColor=0,fillColor=0;bool colorObserved=true,fillObserved=true,consumedObserved=true;std::array<uint32_t,4>fillArgs{{0,0,280,128}};std::array<uint32_t,9>fillTableBefore{},fillTableAfter{};std::vector<Row>fillRows;std::vector<Draw>draws;Selection consumed;};
struct Capture {uintptr_t game=1,thread=2;uint64_t epoch=3;uint32_t form=0x4d3050,formGuest=0x4d3050,archive=400,car=0;std::array<Photo,2>photos;};
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
static void word(std::vector<uint8_t>&v,size_t at,uint32_t n,unsigned bytes=4){replay::put(v.data()+at,n,bytes);}
static Capture fixture(){
 Capture c;const int xs[]={0,8,26,26,-1},ys[]={0,11,41,71,101};const char*names[]={"grph","grpg","grpb","grpr","grpy"};
 for(unsigned phase=0;phase<2;++phase){auto&p=c.photos[phase];const unsigned stride=phase?2:4;const uint32_t black=phase?0:0xff000000,green=phase?0x07e0:0xff00ff00;
  auto&canvas=p.before;canvas.bits=phase?16:32;canvas.words={{0x57494e44,384,160,0,0,384,160,canvas.bits,2000000+phase*300000,0,384*stride,10000,20000,0}};canvas.header={{0x78,384|(160<<16),0,0}};for(unsigned x=0;x<384;++x)canvas.x[x]=x*stride;for(unsigned y=0;y<160;++y)canvas.y[y]=16+y*384*stride;canvas.pixels.resize(384*160*stride);p.after=canvas;p.packedColor=p.fillColor=black;
  p.fillTableBefore=p.fillTableAfter={{0xf0001aa0,0xf0001aa4,0xf0001aa8,0xf0001aa8,0xf0001aac,0xf0001ab0,0,0,0}};
  for(unsigned y=0;y<128;++y){Row r;r.callback=phase?0xf0001aa8:0xf0001ab0;r.format=canvas.bits;r.args={{0,canvas.words[8]+16+y*384*stride,280,0}};r.before.resize(280*stride);r.after.resize(280*stride);for(unsigned x=0;x<280;++x){word(r.after,x*stride,black,stride);word(p.after.pixels,(y*384+x)*stride,black,stride);}p.fillRows.push_back(std::move(r));}
  for(unsigned k=0;k<5;++k){Draw d;std::memcpy(d.name.data(),names[k],4);d.args={{1000000+k*1000,0x3000,uint32_t(xs[k]),uint32_t(ys[k])}};d.tableBefore=d.tableAfter={{0xf000006c,0xf000183c,0xf0001840,0xf0001844,0xf0001848,0xf000184c,0,3,0x7d}};
   d.sourceBefore.resize(42);word(d.sourceBefore,0,(18<<8)|0x7b);word(d.sourceBefore,4,2|(1<<16));d.sourceBefore[16]=d.sourceBefore[17]=1;word(d.sourceBefore,18,0x2a);word(d.sourceBefore,22,2);word(d.sourceBefore,38,0xff00ff00);d.sourceAfter=d.sourceBefore;
   Row r;const unsigned left=unsigned(std::max(0,xs[k])),n=k==4?1:2,sx=k==4?1:0;r.callback=phase?0xf0001850:0xf000211c;r.format=canvas.bits;r.converter=phase?0:0xf0002218;r.forward=phase?0:0xf000184c;r.palette=50000;r.paletteBefore[1]=phase?0xff0007e0:0xff00ff00;r.paletteAfter=r.paletteBefore;r.args={{d.args[0]+16+sx,canvas.words[8]+16+unsigned(ys[k]*384+int(left))*stride,n,0}};r.sourceBefore=r.sourceAfter=std::vector<uint8_t>(n,1);r.before.resize(n*stride);r.after.resize(n*stride);
   for(unsigned x=0;x<n;++x){const auto at=size_t(ys[k]*384+int(left+x))*stride;std::memcpy(r.before.data()+x*stride,p.after.pixels.data()+at,stride);word(r.after,x*stride,green,stride);word(p.after.pixels,at,green,stride);}d.rows.push_back(std::move(r));p.draws.push_back(std::move(d));
  }
 }
 return c;
}
int main(){
 // Independent arithmetic examples, including zero-alpha and modulo channels.
 CHECK(replay::blend32(0xff123456,0x00112233)==0xff123456);
 CHECK(replay::blend32(0x00112233,0x80402010)==0x80402010);
 CHECK(replay::blend32(0xff402010,0x80402010)==0xff5f2f17);
 CHECK(replay::blend32(0xffffffff,0x80ffffff)==0xff7d7d7d);
 auto c=fixture();auto r=replay::replay(c);CHECK(r.matched);CHECK(!r.sourceAuthenticated);CHECK(!r.dispatchTableTailsAuthenticated);
 c.form=0x4d3100;c.formGuest=c.form;CHECK(replay::replay(c).matched);
 for(unsigned fault=0;fault<12;++fault){c=fixture();switch(fault){
  case 0:c.photos[0].after.pixels[0]^=1;break;
  case 1:c.photos[1].draws[2].tableAfter[3]^=1;break;
  case 2:++c.photos[0].consumed.graphValues[1];break;
  case 3:c.photos[0].draws[0].rows[0].sourceBefore[0]=0;break;
  case 4:c.photos[1].draws[0].rows[0].after[0]^=1;break;
  case 5:c.photos[0].draws[2].rows.clear();break;
  case 6:c.photos[1].draws[1].rows[0].paletteBefore[1]^=1;break;
  case 7:c.photos[0].draws[2].name[0]='X';break;
  case 8:c.photos[0].draws[0].rows[0].forward=0;break;
  case 9:c.photos[0].before.x[7]^=1;break;
  case 10:c.photos[1].draws[4].args[2]=0;break;
  case 11:c.photos[0].fillRows[1].args[1]+=4;break;
 }CHECK(!replay::replay(c).matched);}
 std::printf("PASS graph canvas row replay: %u checks; no source-authentication claim\n",checks);
}

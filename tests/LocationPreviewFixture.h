// SPDX-License-Identifier: MIT
#pragma once
#include "hd/PreviewPacket.h"
#include <stdexcept>
namespace nfs_location_preview_test {
namespace pv=nfs_location_preview;
inline void put32(uint8_t*p,uint32_t n){for(unsigned k=0;k<4;++k)p[k]=uint8_t(n>>(8*k));}
// Independent RGB565 arithmetic for this original preview's three palette entries.
inline uint16_t color(unsigned index,uint16_t old){if(!index)return old;if(index==1)return 0xf800;const uint64_t p=(uint64_t(old&31)|uint64_t(old&0x7e0)*256|uint64_t(old&0xf800)*65536)*127;return uint16_t(0x800007e0u+((p>>24)&0xf800)+((p>>16)&0x7e0)+((p>>8)&31));}
struct Fixture {
 pv::Input input;std::vector<uint16_t>oracle;
 explicit Fixture(const pv::Plane&base,uint64_t serial=11,uint64_t lifetime=1,uint32_t archive=0x20000){
  input.pre=base;input.post=base;input.serial=serial;input.lifetime=lifetime;input.archive=archive;input.form=pv::LocationForm;input.complete=1;input.fillCalls=input.drawCalls=1;oracle.assign(base.pixels.begin(),base.pixels.end());auto&fill=input.fill;fill.before=fill.after=base.target;fill.call={{211,316,280,128}};fill.rawColor=0xff000000;fill.callback=pv::FillCallback;fill.format=16;fill.complete=1;fill.tableBefore=fill.tableAfter={{0xf0001aa0,0xf0001aa4,0xf0001aa8,0xf0001aa8,0xf0001aac,0xf0001ab0,0,0,0}};
  for(unsigned y=316;y<444;++y){pv::Row row;row.callback=pv::FillCallback;row.format=16;row.args[1]=base.target.w[8]+16+(y*640+211)*2;row.args[2]=280;for(unsigned x=211;x<491;++x){row.before.push_back(oracle[y*640+x]);row.after.push_back(0);oracle[y*640+x]=0;}fill.rows.push_back(std::move(row));}
  pv::Draw draw;draw.before=draw.after=base.target;draw.args={{0x400000,0x3000,211,316}};draw.name={{'t','r','k','4'}};draw.callback=pv::IndexedCallback;draw.paletteGuest=0x900000;draw.format=16;draw.complete=1;draw.paletteBefore[1]=0xff00f800;draw.paletteBefore[2]=0x800007e0;draw.paletteAfter=draw.paletteBefore;draw.tableBefore={{0xf000006c,0xf000183c,0xf0001840,0xf0001844,0xf0001848,0xf000184c,0,3,0}};draw.tableAfter=draw.tableBefore;draw.tableAfter[6]=pv::IndexedCallback;draw.tableAfter[8]=0x7b;
  const unsigned width=307,height=146,offset=16+width*height+2;draw.sourceBefore.resize(offset+16+12);put32(draw.sourceBefore.data(),(offset<<8)|0x7b);put32(draw.sourceBefore.data()+4,width|(height<<16));put32(draw.sourceBefore.data()+offset,0x2a);put32(draw.sourceBefore.data()+offset+4,3);put32(draw.sourceBefore.data()+offset+20,0xffff0000);put32(draw.sourceBefore.data()+offset+24,0x8000ff00);
  for(unsigned y=0;y<height;++y){pv::Row row;row.callback=pv::IndexedCallback;row.format=16;row.paletteGuest=draw.paletteGuest;row.paletteBefore=row.paletteAfter=draw.paletteBefore;row.args={{draw.args[0]+16+y*width,base.target.w[8]+16+((316+y)*640+211)*2,width,0}};for(unsigned x=0;x<width;++x){const unsigned index=(x+y)%3,at=(316+y)*640+211+x;draw.sourceBefore[16+y*width+x]=uint8_t(index);row.source.push_back(uint8_t(index));row.before.push_back(oracle[at]);oracle[at]=color(index,oracle[at]);row.after.push_back(oracle[at]);}draw.rows.push_back(std::move(row));}
  draw.sourceAfter=draw.sourceBefore;if(!nfs_texture_sha256(draw.sourceBefore.data(),draw.sourceBefore.size(),draw.sha.data()))throw std::runtime_error("preview fixture digest");input.draws.push_back(std::move(draw));std::copy(oracle.begin(),oracle.end(),input.post.pixels.begin());
 }
 std::shared_ptr<const pv::Packet>packet(const pv::Plane&base)const{return pv::createPacket(input,base);}
};
}

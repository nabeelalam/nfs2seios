// SPDX-License-Identifier: MIT
#pragma once
#include "OriginalUiRecords.h"
#include "SoftwareFramePacket.h"
#include "SmallLogoPrepare.h"
namespace nfs_audio_frame {
constexpr size_t Pixels=640*480,MaxRows=480,MaxDraws=13,MaxSource=65536;
using Target=nfs_logo_capture::Target;
struct Binding {
 uint32_t form=0,owner=0,archive=0;uint64_t ownerEpoch=0,pass=0,lifetime=0;nfs_soft::SurfaceIdentity surface{};
 bool valid()const noexcept{return form&&owner&&archive&&ownerEpoch&&pass&&lifetime&&surface.valid()&&surface.lifetime==lifetime&&surface.geometry[1]==640&&surface.geometry[3]==0&&surface.geometry[4]==0&&surface.geometry[5]==640&&surface.geometry[6]==480;}
 bool operator==(const Binding&o)const noexcept{return form==o.form&&owner==o.owner&&archive==o.archive&&ownerEpoch==o.ownerEpoch&&pass==o.pass&&lifetime==o.lifetime&&surface==o.surface;}
};
// Supplied independently by the source-bound runtime. Numeric fields never grant
// guest access; the producer must prove exact lexical form/owner/current pass.
inline bool targetValid(const Target&t,const Binding&b,bool full=true)noexcept{if(!b.valid()||t.owner!=b.owner||!nfs_small_logo_live::target(t,640)||t.header[3])return false;for(unsigned k=0;k<14;++k)if((full||k<3||k>6)&&t.w[k]!=b.surface.geometry[k])return false;return !full||(t.w[3]==0&&t.w[4]==0&&t.w[5]==640&&t.w[6]==480);}
struct Row {uint32_t callback=0,callbackGlobal=0,callbackFormat=0,paletteGuest=0;std::array<uint32_t,4>args{};std::array<uint32_t,256>paletteBefore{},paletteAfter{};std::vector<uint8_t>source;std::vector<uint16_t>before,after;};
struct BackgroundDraw {Target before{},after{};std::array<uint32_t,4>args{};std::array<uint32_t,9>tableBefore{},tableAfter{};uint32_t complete=0,error=0;uint32_t sidebarWord=0,sidebarHandle=0,sidebarTableEntry=0,sidebarBitmap=0;std::vector<uint8_t>sourceBefore,sourceAfter;std::vector<Row>rows;};
struct BackgroundRecord {Binding binding;uint32_t complete=0,error=0;BackgroundDraw reset,sidebar;std::vector<uint16_t>preReset,postReset,preBars;};
struct BarDraw {Target before{},after{};std::array<uint32_t,4>namedArgs{},args{};std::array<uint8_t,4>name{};uint32_t asset=0,sequence=0,complete=0,error=0;std::array<uint8_t,3>values{};int32_t selected=0,held=0;std::vector<uint8_t>sourceBefore,sourceAfter;std::vector<Row>rows;};
struct BarRecord {Binding binding;uint32_t complete=0,error=0;std::vector<uint16_t>pre,post;std::vector<BarDraw>draws;};
struct SourceIdentity {uint32_t span=0,width=0,height=0,paletteOffset=0,paletteCount=0;int32_t hotX=0,hotY=0;std::array<uint32_t,4>header{};std::array<unsigned char,32>sha{};};
inline bool nonzero(const std::array<unsigned char,32>&s)noexcept{for(auto v:s)if(v)return true;return false;}
}

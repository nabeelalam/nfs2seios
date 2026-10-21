// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstdint>
#include <cstring>
// Owned observations only; retained namespace keeps verifier compatibility.
// No recorder, sampling, file export or guest-memory access is provided.
namespace nfs_logo_capture {
constexpr uint32_t MaxDraws=3,MaxRows=480,MaxPixels=640*480,MaxSource=21000,MaxCopySource=16+343*480*2;
struct Target{std::array<uint32_t,14>w{};std::array<uint32_t,4>header{};std::array<uint32_t,640>x{};std::array<uint32_t,480>y{};uint32_t owner=0;std::array<uint32_t,14>ownerWords{};std::array<uint32_t,5>ownerState{};};
struct Plane{Target target;uint32_t width=0,height=0,valid=0;std::array<uint16_t,MaxPixels>pixels{};};
struct Row{uint32_t callback=0;std::array<uint32_t,4>args{};};
struct CopyRow{Row row;uint32_t converter=0,finalCallback=0,colorKey=0,bytes=0;std::array<uint8_t,2560>source{};};
struct Draw{uint32_t role=UINT32_MAX,span=0,paletteCount=0,actualMaxIndex=0,error=0,complete=0,resolves=0,rows=0;std::array<uint32_t,4>args{},sourceHeader{};Target before{},after{};std::array<uint32_t,9>tableBefore{},tableAfter{};std::array<unsigned char,32>shaBefore{},shaAfter{};std::array<uint8_t,MaxSource>sourceBefore{},sourceAfter{};uint32_t callback=0,paletteGuest=0,paletteValid=0,callbackGlobal=0,callbackFormat=0;std::array<uint32_t,256>paletteFirst{},paletteLast{};std::array<Row,MaxRows>row{};};
struct Group{uint64_t serial=0,thread=0;uint32_t kind=0,ownerArg=0,error=0,complete=0,stage=0,draws=0;Plane pre{},post{};std::array<Draw,MaxDraws>draw{};};
struct Copy{uint64_t serial=0,thread=0,linkedGroup=0;uint32_t error=0,complete=0,span=0,rows=0;std::array<uint32_t,4>args{};Plane pre{},post{};Target sourceOwner{};std::array<uint32_t,9>tableBefore{},tableAfter{};std::array<unsigned char,32>shaBefore{},shaAfter{};std::array<uint8_t,MaxCopySource>sourceBefore{},sourceAfter{};std::array<CopyRow,MaxRows>row{};};
inline bool same(const Target&a,const Target&b)noexcept{return a.w==b.w&&a.header==b.header&&a.x==b.x&&a.y==b.y&&a.owner==b.owner&&a.ownerWords==b.ownerWords&&a.ownerState==b.ownerState;}
}

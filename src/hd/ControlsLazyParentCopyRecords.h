// SPDX-License-Identifier: MIT
#pragma once
#include "OriginalUiRecords.h"
#include <vector>
namespace nfs_controls_lazy_parent_copy {
constexpr uint32_t SourceWidth=343, Height=480, MainWidth=640;
constexpr uint32_t SourceBytes=16+SourceWidth*Height*2, PlaneBytes=MainWidth*Height*2;
constexpr uint32_t DispatchId=0xf0001878u, CopyId=0xf0002124u;
// All identities and observations are untrusted numeric data, never authority.
struct Binding {
 uint64_t game=0,thread=0,epoch=0,lifetime=0,parentRevision=0,packetSerial=0,cacheRevision=0;
 uint32_t form=0,archive=0,sourceOwner=0,sourceBitmap=0,mainOwner=0;
 bool valid()const noexcept{return game&&thread&&epoch&&lifetime&&parentRevision&&packetSerial&&cacheRevision&&form&&archive&&sourceOwner&&sourceBitmap&&mainOwner;}
 bool operator==(const Binding&b)const noexcept{return game==b.game&&thread==b.thread&&epoch==b.epoch&&lifetime==b.lifetime&&parentRevision==b.parentRevision&&packetSerial==b.packetSerial&&cacheRevision==b.cacheRevision&&form==b.form&&archive==b.archive&&sourceOwner==b.sourceOwner&&sourceBitmap==b.sourceBitmap&&mainOwner==b.mainOwner;}
};
struct Geometry {int32_t x=0,y=0,left=0,top=0,right=0,bottom=0;
 bool operator==(const Geometry&b)const noexcept{return x==b.x&&y==b.y&&left==b.left&&top==b.top&&right==b.right&&bottom==b.bottom;}};
struct Row {
 uint64_t dispatchBegin=0,copyBegin=0,copyEnd=0,dispatchEnd=0;
 uint32_t dispatch=0,callback=0;
 // EAX, EDX, EBX, ECX: wrapper EBX pixels; final copy EBX bytes.
 std::array<uint32_t,4> dispatchArgs{},copyArgs{},copyReturn{},dispatchReturn{};
 std::vector<uint8_t> sourceBefore,sourceAfter,destinationBefore,destinationAfter;
};
enum class RowKind : uint32_t {Direct=0,Keyed=1};
struct KeyedRow {
 uint64_t rowBegin=0,transformBegin=0,transformEnd=0,forwardBegin=0,forwardEnd=0,rowEnd=0;
 uint32_t wrapper=0,transform=0,forward=0,keyBefore=0,keyAfter=0;
 // Actual wrapper entry ESP; original pushes ECX/ESI then allocates 0x1400.
 uint32_t wrapperEsp=0,temporaryAddress=0,allocationBase=0,allocationBytes=0;
 // EAX, EDX, EBX, ECX; no plain-byte-copy return tuple applies to this route.
 std::array<uint32_t,4> entry{},transformArgs{},forwardArgs{};
 std::vector<uint8_t> sourceBefore,sourceAfter,temporaryBeforeForward,temporaryAfterForward,destinationBefore,destinationAfter;
};
struct Record {
 Binding bindingBefore{},bindingAfter{};
 uint32_t complete=0,error=0,site=0,mode=0;
 std::array<uint32_t,4> originalArgs{};
 uint64_t packetAccepted=0,copyBegin=0,resolveBegin=0,resolveEnd=0,copyEnd=0;
 nfs_logo_capture::Target mainBefore{},mainAfter{},sourceBefore{},sourceAfter{};
 Geometry resolved{};
 std::vector<uint8_t> sidebarBefore,sidebarAfter,mainPlaneBefore,mainPlaneAfter;
 RowKind rowKind=RowKind::Direct;
 uint32_t keyBefore=0,keyAfter=0;
 std::vector<Row> rows;
 std::vector<KeyedRow> keyedRows;
};
// Supplied independently by the live producer from its immutable retained packet.
struct Current {Binding binding{};nfs_logo_capture::Target sidebar{},main{};std::vector<uint8_t> sidebarBytes;};
}

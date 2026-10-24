// SPDX-License-Identifier: MIT
#pragma once
#include "SmallLogoPrepare.h"
#include "SoftwareFramePacket.h"
#include "FeedbackModalGeometry.h"
#include <array>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>
namespace nfs_feedback_backdrop {
constexpr size_t Pixels=640u*480u,MaxSource=17224,MaxRows=480,MaxRowPixels=640,MaxStores=Pixels;
using Target=nfs_logo_capture::Target;
using Plane=std::vector<uint16_t>;
enum class Role:uint32_t {banner=0,emblem=1};
struct SourceIdentity {
 uint64_t providerKey=0;uint32_t tag=0,span=0,width=0,height=0,paletteOffset=0,paletteCount=0,archiveOffset=0;
 int32_t hotX=0,hotY=0;std::array<uint32_t,4>header{},paletteHeader{};std::array<unsigned char,32>sha{},archiveSha{};
};
struct Binding {
 uint32_t owner=0,parent=0,form=0,archive=0,targetOwner=0,titleResource=0,messageResource=0;
 uint64_t ownerEpoch=0,parentEpoch=0,modalEpoch=0,lifetime=0,resourceEpoch=0;
 std::array<uint32_t,2>sources{};std::array<uint64_t,2>providerKeys{};nfs_soft::SurfaceIdentity surface{};
 bool valid()const noexcept {
  if(!owner||!parent||!form||!archive||!targetOwner||!titleResource||!messageResource||titleResource==messageResource||!ownerEpoch||!parentEpoch||!modalEpoch||!lifetime||!resourceEpoch||!sources[0]||!sources[1]||!providerKeys[0]||!providerKeys[1]||providerKeys[0]==providerKeys[1]||!surface.valid()||surface.lifetime!=lifetime)return false;
  const auto&w=surface.geometry;return w[1]==640&&w[3]==0&&w[4]==0&&w[5]==640&&w[6]==480;
 }
 bool operator==(const Binding&o)const noexcept{return owner==o.owner&&parent==o.parent&&form==o.form&&archive==o.archive&&targetOwner==o.targetOwner&&titleResource==o.titleResource&&messageResource==o.messageResource&&ownerEpoch==o.ownerEpoch&&parentEpoch==o.parentEpoch&&modalEpoch==o.modalEpoch&&lifetime==o.lifetime&&resourceEpoch==o.resourceEpoch&&sources==o.sources&&providerKeys==o.providerKeys&&surface==o.surface;}
};
// Supplied by a separately authenticated lexical/source producer. These are copied
// numeric evidence, never a capability to read guests or to admit native artwork.
struct SelectorContract {
 // Raw relocated guest addresses supplied by the authenticated producer. Symbolic
 // roles are original dword571264 / dword571260 / dword5636E8 / byte5635E4;
 // original virtual addresses must never replace the observed ranges below.
 // Generated BSS is byte-addressed: scalar slots can be unaligned even on
 // ARM64. The producer copies safely; exact expected addresses, complete
 // spans and nonaliasing matter here, not an invented scalar alignment rule.
 uint32_t outer=0,convertSlot=0,convert=0,blendSlot=0,blend=0,opacitySlot=0,opacity=0,lutAddress=0;
 std::array<uint8_t,256>lut{};
 bool valid()const noexcept {
  if(outer!=0xf000211c||!convert||!blend||convert==blend||opacity!=180)return false;
  const uint32_t addresses[4]={convertSlot,blendSlot,opacitySlot,lutAddress};
  const unsigned sizes[4]={4,4,4,256};
  for(unsigned i=0;i<4;++i){if(!addresses[i]||uint64_t(addresses[i])+sizes[i]>0x100000000ULL)return false;
   for(unsigned j=0;j<i;++j)if(uint64_t(addresses[i])<uint64_t(addresses[j])+sizes[j]&&uint64_t(addresses[j])<uint64_t(addresses[i])+sizes[i])return false;
  }
  for(unsigned i=0;i<256;++i)if(lut[i]!=(i*180u>>8))return false;return true;
 }
 bool operator==(const SelectorContract&o)const noexcept{return outer==o.outer&&convertSlot==o.convertSlot&&convert==o.convert&&blendSlot==o.blendSlot&&blend==o.blend&&opacitySlot==o.opacitySlot&&opacity==o.opacity&&lutAddress==o.lutAddress&&lut==o.lut;}
};
struct SelectorEvidence {uint32_t complete=0;SelectorContract before{},after{};};
struct Store {uint32_t address=0;uint16_t before=0,computed=0,after=0;};
struct Row {
 // Dispatch is observed at caller4A10BC esp+0x38 before/after its call.
 // This is not the off4DAD64 callback global.
 uint32_t sequence=0,callback=0,callerStackBefore=0,callerStackAfter=0,dispatchSlot=0,dispatchBefore=0,dispatchAfter=0,format=0,sourceAddress=0,destinationAddress=0,count=0,paletteAddress=0;
 std::array<uint32_t,256>paletteBefore{},paletteAfter{};SelectorEvidence selectors{};
 std::vector<uint8_t>source,sourceAfter;Plane before,after;std::vector<Store>stores;
};
struct Scratch {
 uint32_t descriptor=0,source=0,allocated=0,selected=0,fillComplete=0,retired=0;
 std::array<uint32_t,14>words{},retireWords{};std::array<uint32_t,4>header{};std::vector<uint32_t>x,y;
 uint32_t fillColor=0,fillCallback=0,fillDestination=0,fillCount=0,opacity=0;uint16_t fillWord=0;
 std::vector<uint8_t>postFill,afterBlit;std::vector<Row>rows;
};
enum class Step:uint32_t {initialMain,allocateScratch,selectScratch,fillColor,fillScratch,restoreMainAfterScratch,bodyBlit,retireScratch,bannerClip,bannerResolve,bannerDraw,restoreClip,emblemResolve,emblemHotspot,emblemDraw};
struct IndexedDraw {
 Role role=Role::banner;uint32_t sequence=0,complete=0,error=0,archive=0,sourceAddress=0,resolvedAddress=0,namedResolves=0,mode=0;
 std::array<uint8_t,4>name{};int32_t inputX=0,inputY=0,x=0,y=0;bool hotspot=false;
 // Exact source UV window derived from the destination clip, independently checked.
 std::array<uint32_t,4>uv{};Target before{},after{};
 std::vector<uint8_t>sourceBefore,sourceAfter;std::vector<Row>rows;
};
struct Record {
 Binding binding{};uint32_t complete=0,error=0,initialMainRestoreComplete=0;int32_t titleWidth=0,messageWidth=0;
 // initial and initialRestored are captured at the FIRST original main restore,
 // before any scratch allocation. The later restore has its own target/plane.
 Target initial{},restoredMainAfterScratch{},pretextTarget{};Plane before,initialRestored,restored,pretext;
 std::vector<Step>steps;Scratch body{};std::array<IndexedDraw,2>draws{};
};
inline bool nonzero(const std::array<unsigned char,32>&s)noexcept{for(auto x:s)if(x)return true;return false;}
inline bool targetValid(const Target&t,const Binding&b,bool full=true)noexcept{
 if(!b.valid()||t.owner!=b.targetOwner||!nfs_small_logo_live::target(t,640)||t.header[3])return false;
 for(unsigned k=0;k<14;++k)if((full||k<3||k>6)&&t.w[k]!=b.surface.geometry[k])return false;
 return !full||(t.w[3]==0&&t.w[4]==0&&t.w[5]==640&&t.w[6]==480);
}
}

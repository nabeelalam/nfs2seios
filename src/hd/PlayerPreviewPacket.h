// SPDX-License-Identifier: MIT
#pragma once
#include "PreviewPacket.h"
namespace nfs_player_preview {
// Source snapshots must be read independently before/after the exact40CD70 call.
// Pure records do not grant guest-read, original-owner or publication authority.
struct Selection {
 uint32_t selectedCar=0,cachedCarIndex=0,graphEnabled=0;
 std::array<uint32_t,4>graphValues{}; //236C,2330,23E4,23A8 at cached index.
 bool operator==(const Selection&o)const noexcept{return selectedCar==o.selectedCar&&cachedCarIndex==o.cachedCarIndex&&graphEnabled==o.graphEnabled&&graphValues==o.graphValues;}
};
}
#define NFS_PREVIEW_REPLAY_NAMESPACE nfs_player_preview
#define NFS_PREVIEW_REPLAY_MAX_DRAWS 5
#define NFS_PREVIEW_REPLAY_FORM_CONSTANTS constexpr uint32_t Player1Form=0x4D3050,Player2Form=0x4D3100;
#define NFS_PREVIEW_REPLAY_EXTRA_INPUT Selection selectionBefore{},selectionAfter{};
#define NFS_PREVIEW_REPLAY_BINDING_DECLARATION inline bool binding(const Input&)noexcept;
#define NFS_PREVIEW_REPLAY_BINDING(i) binding(i)
#define NFS_PREVIEW_REPLAY_PACKET_FIELDS Selection selection_{};uint32_t form_=0;
#define NFS_PREVIEW_REPLAY_PACKET_ACCESSORS const Selection&selection()const noexcept{return selection_;}uint32_t form()const noexcept{return form_;}
#define NFS_PREVIEW_REPLAY_PACKET_CAPTURE packet->selection_=i.selectionBefore;packet->form_=i.form;
#include "PreviewReplay.inc"
#undef NFS_PREVIEW_REPLAY_PACKET_CAPTURE
#undef NFS_PREVIEW_REPLAY_PACKET_ACCESSORS
#undef NFS_PREVIEW_REPLAY_PACKET_FIELDS
#undef NFS_PREVIEW_REPLAY_BINDING
#undef NFS_PREVIEW_REPLAY_BINDING_DECLARATION
#undef NFS_PREVIEW_REPLAY_EXTRA_INPUT
#undef NFS_PREVIEW_REPLAY_FORM_CONSTANTS
#undef NFS_PREVIEW_REPLAY_MAX_DRAWS
#undef NFS_PREVIEW_REPLAY_NAMESPACE

namespace nfs_player_preview {
// Original off409E30 table, not its off409E70 opponent-class sibling.
inline std::array<uint8_t,4>carName(unsigned index)noexcept{
 static const char names[15][5]={"car1","car9","carc","car7","car8","cara","car4","car6","car3","carb","car5","car2","carf","care","card"};
 std::array<uint8_t,4>out{};if(index<15)std::copy_n(names[index],4,out.begin());return out;
}
inline std::array<uint32_t,4>statistics(unsigned index)noexcept{
 static const uint32_t values[4][15]={
 {10,12,1,11,7,5,9,6,3,4,2,8,0,0,0},
 {12,8,4,11,5,3,10,7,1,6,2,9,0,0,0},
 {12,8,5,2,11,4,3,10,7,6,9,1,0,0,0},
 {9,8,4,6,12,5,1,11,3,10,7,2,0,0,0}};
 std::array<uint32_t,4>out{};if(index<15)for(unsigned k=0;k<4;++k)out[k]=values[k][index];return out;
}
inline bool binding(const Input&i)noexcept{
 if((i.form!=Player1Form&&i.form!=Player2Form)||!(i.selectionBefore==i.selectionAfter))return false;
 const auto&s=i.selectionBefore;if(s.selectedCar>=15||s.cachedCarIndex>=15||s.graphEnabled>255)return false;
 const bool graph=s.graphEnabled&&s.selectedCar<=11;
 if(!graph){if(s.graphValues!=std::array<uint32_t,4>{}||i.draws.size()!=1)return false;const auto&d=i.draws[0];return d.name==carName(s.cachedCarIndex)&&d.args[2]==211&&d.args[3]==316;}
 if(s.graphValues!=statistics(s.cachedCarIndex)||i.draws.size()!=5)return false;
 static const char names[5][5]={"grph","grpg","grpb","grpr","grpy"};
 for(unsigned k=0;k<5;++k){const auto&d=i.draws[k];if(std::memcmp(d.name.data(),names[k],4))return false;
  const int32_t x=k?237-9*(12-int32_t(s.graphValues[k-1])):211;
  const uint32_t y=k?327+30*(k-1):316;
  if(d.args[2]!=uint32_t(x)||d.args[3]!=y)return false;
 }
 return true;
}
} // namespace nfs_player_preview

// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include "OriginalUiRecords.h"
namespace nfs_options_caption{struct TailScope;}namespace nfs_setup_caption{struct TailScope;}
namespace nfs_player1_caption{struct TailScope;}namespace nfs_player2_caption{struct TailScope;}
namespace nfs_compact_caption {
enum class Policy:uint32_t{Player1=0x4d3050,Player2=0x4d3100};
// Only the ordinary incoming-EBP route is currently observed. Initial caller
// authority must be added separately; this artifact cannot attest that route.
enum class Route:uint32_t{OrdinaryEbp=1};
struct Snapshot {
 nfs_logo_capture::Target target{};nfs_soft::SurfaceIdentity identity{};
 uint32_t archive=0,font=0,callback=0,palette=0,color=0,mode=0,flags=0,format=0,selector=0,entry=0,entryWord=0,resource=0;
 int32_t originX=0,originY=0,size=0;std::array<uint32_t,16>paletteWords{};std::array<uint8_t,16>fontHeader{};
};
// Public observation values carry no authority. Only a completed exact lexical
// source observer may construct Proof, after original command and word replay.
struct Evidence {
 Policy policy=Policy::Player1;Route route=Route::OrdinaryEbp;
 // Namespace-local caption owner token; not comparable to publisher thread tokens.
 uintptr_t game=0,policyThread=0;uint32_t formGuest=0,incomingEbp=0,requestedResource=0,inputCategory=0,textGuest=0;
 uint64_t lifetime=0,serial=0,resourceSerial=0;int32_t inputAnchor=0,inputY=0,measureWidth=0,measureAnchor=0;uint32_t measureKind=0;
 nfs_soft::TextCommand command{};Snapshot resolved{},measure{},stringBefore{},stringAfter{};
};
class Proof final {
 std::shared_ptr<const nfs_soft::FramePacket>packet_;Evidence evidence_;
 Proof(std::shared_ptr<const nfs_soft::FramePacket>packet,const Evidence&e):packet_(std::move(packet)),evidence_(e){}
 friend struct nfs_options_caption::TailScope;friend struct nfs_setup_caption::TailScope;
 friend struct nfs_player1_caption::TailScope;friend struct nfs_player2_caption::TailScope;
public:
 const Evidence&evidence()const noexcept{return evidence_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 const std::shared_ptr<const nfs_soft::FontCatalog>&font()const noexcept{return packet_->font();}
 // Immutable HISTORICAL source evidence only. A future join must acquire an
 // independently current owner/archive/storage/serial and actual full plane.
};
} // namespace nfs_compact_caption

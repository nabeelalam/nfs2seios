// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include "OriginalUiRecords.h"
namespace nfs_location_caption {struct TailScope;}
namespace nfs_location_caption_source {
constexpr uint32_t LogicalForm=0x404abd;
struct Snapshot {
 nfs_logo_capture::Target target{};nfs_soft::SurfaceIdentity identity{};
 uint32_t archive=0,font=0,callback=0,palette=0,color=0,mode=0,flags=0,format=0;
 int32_t originX=0,originY=0,size=0;
 std::array<uint32_t,16>paletteWords{};std::array<uint8_t,16>fontHeader{};
};
struct Evidence {
 uint32_t logicalForm=LogicalForm,formGuest=0,textGuest=0;uintptr_t game=0,policyThread=0;
 uint64_t lifetime=0,serial=0;nfs_soft::TextCommand command{};
 Snapshot pre{},stringBefore{},stringAfter{},post{};
};
// Historical lexical source evidence only. Public observations cannot seal it;
// current scene/cache/native upload authority must be joined independently.
class Proof final {
 std::shared_ptr<const nfs_soft::FramePacket>packet_;Evidence evidence_;
 Proof(std::shared_ptr<const nfs_soft::FramePacket>packet,const Evidence&e):packet_(std::move(packet)),evidence_(e){}
 friend struct nfs_location_caption::TailScope;
public:
 const Evidence&evidence()const noexcept{return evidence_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 const std::shared_ptr<const nfs_soft::FontCatalog>&font()const noexcept{return packet_->font();}
 bool current()const noexcept{return bool(packet_);} // Historical immutable seal, not live owner authority.
};
} // namespace nfs_location_caption_source

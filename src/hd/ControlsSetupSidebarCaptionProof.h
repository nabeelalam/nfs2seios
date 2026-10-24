// SPDX-License-Identifier: MIT
#pragma once
#include "NestedSidebarCaptionProof.h"
#include "ControlsDialogWordProof.h"
namespace nfs_controls_setup_sidebar_text {struct TailScope;}
namespace nfs_controls_setup_sidebar_caption {
using Snapshot=nfs_nested_sidebar_caption::Snapshot;
// A complete historical child-authority observation. Mutable row, column and
// binding values are history, never a requirement that user input remain frozen.
struct ChildAuthority {
 nfs_controls_dialog_word::Authority parent{};
 uint32_t form=0,gameType=0,column=0,assignment=0,archive=0,table=0;
 std::array<uint32_t,3>header{};
 std::array<uint32_t,16>descriptors{};
 std::array<std::array<uint32_t,5>,16>words{};
 std::array<std::array<uint32_t,2>,10>bindings{};
};
struct Evidence {
 uint32_t form=0,handler=0,incomingEbp=0,textGuest=0;
 uintptr_t game=0,thread=0;uint64_t serial=0,lifetime=0,ownerEpoch=0;
 ChildAuthority authority{};
 int32_t measureWidth=0,measureAnchor=0;uint32_t measureKind=0;
 nfs_soft::TextCommand command{};Snapshot pre{},measure{},before{},after{},post{};
};
class Proof final {
 std::shared_ptr<const nfs_soft::FramePacket>packet_;Evidence evidence_;
 Proof(std::shared_ptr<const nfs_soft::FramePacket>p,const Evidence&e):packet_(std::move(p)),evidence_(e){}
 friend struct ::nfs_controls_setup_sidebar_text::TailScope;
public:
 const Evidence&evidence()const noexcept{return evidence_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 bool current()const noexcept{return bool(packet_);} // Immutable history only.
};
}

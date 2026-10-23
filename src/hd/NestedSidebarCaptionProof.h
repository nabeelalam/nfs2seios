// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include "OriginalUiRecords.h"
namespace nfs_audio_sidebar_text {struct TailScope;}
namespace nfs_graphics_sidebar_text {struct TailScope;}
namespace nfs_nested_sidebar_caption {
enum class Policy:uint32_t {None=0,Audio=1,Graphics=2};
struct Snapshot {
 nfs_logo_capture::Target target{};nfs_soft::SurfaceIdentity identity{};
 uint32_t archive=0,selector=0,entry=0,entryWord=0,font=0,callback=0,palette=0,color=0,mode=0,flags=0,format=0;
 int32_t originX=0,originY=0,size=0;std::array<uint32_t,16>paletteWords{};std::array<uint8_t,16>fontHeader{};
};
struct Evidence {
 Policy policy=Policy::None;uint32_t form=0,incomingEbp=0,textGuest=0;uintptr_t game=0,thread=0;uint64_t serial=0,lifetime=0;
 int32_t measureWidth=0,measureAnchor=0;uint32_t measureKind=0;nfs_soft::TextCommand command{};
 Snapshot pre{},measure{},before{},after{},post{};
};
class Proof final {
 std::shared_ptr<const nfs_soft::FramePacket>packet_;Evidence evidence_;
 Proof(std::shared_ptr<const nfs_soft::FramePacket>p,const Evidence&e):packet_(std::move(p)),evidence_(e){}
 friend struct ::nfs_audio_sidebar_text::TailScope;friend struct ::nfs_graphics_sidebar_text::TailScope;
public:
 const Evidence&evidence()const noexcept{return evidence_;}
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 bool current()const noexcept{return bool(packet_);} // Immutable history only; current source and renderer owners are independent.
};
struct Binding {
 Policy policy=Policy::None;uint32_t form=0,archive=0;uintptr_t game=0;uint64_t serial=0,lifetime=0;
 bool empty()const noexcept{return policy==Policy::None&&!form&&!archive&&!game&&!serial&&!lifetime;}
};
inline bool exactCommand(const nfs_soft::TextCommand&c)noexcept{
 if(c.length!=7||std::memcmp(c.text.data(),"options",7)||c.role!=1||c.alignment!=2||c.sourceSize!=18||c.x!=120||c.y!=364||c.advance!=48||c.originX||c.originY||c.color!=0xffff||c.clip.left||c.clip.top||c.clip.right!=343||c.clip.bottom!=480)return false;
 for(size_t k=7;k<c.text.size();++k)if(c.text[k])return false;return true;
}
inline bool sameOwner(const nfs_logo_capture::Target&a,const nfs_logo_capture::Target&b)noexcept{
 if(a.w!=b.w||a.header!=b.header||a.x!=b.x||a.y!=b.y||a.owner!=b.owner||a.ownerWords!=b.ownerWords)return false;for(unsigned k=1;k<5;++k)if(a.ownerState[k]!=b.ownerState[k])return false;return true;
}
}

// SPDX-License-Identifier: MIT
#pragma once
#include "FeedbackModalBackdropRecords.h"
#include "ControlsDialogWordProof.h"
namespace nfs_feedback_modal_text {
using Snapshot=nfs_controls_dialog_word::Snapshot;
using Binding=nfs_feedback_backdrop::Binding;
// All addresses/function identities are supplied from compiled symbols by the
// lexical producer. These copied observations grant no guest or native access.
struct SourceIdentity {
 nfs_controls_dialog_word::SourceIdentity controls{};
 uint32_t caller=0,modal=0,stringDraw=0,measure=0,glyph=0;
 bool valid()const noexcept{return controls.valid()&&caller&&modal&&stringDraw&&measure&&glyph==controls.glyphCallback&&caller!=modal&&stringDraw!=measure;}
 bool operator==(const SourceIdentity&o)const noexcept{return controls==o.controls&&caller==o.caller&&modal==o.modal&&stringDraw==o.stringDraw&&measure==o.measure&&glyph==o.glyph;}
};
struct Resource {
 uint32_t id=0,table=0,headerAddress=0,textAddress=0;
 std::array<uint8_t,12>headerBefore{},headerAfter{};
 std::array<uint8_t,96>textBefore{},textAfter{};
 uint32_t alignment=0,colorIndex=0;int32_t anchor=0,y=0,measuredWidth=0;
 // Completed operation counts are observations, not source capabilities.
 unsigned resolves=0,measures=0,strings=0;
 Snapshot before{},resolved{},measured{},stringBefore{},after{};
};
struct Evidence {
 SourceIdentity source{};Binding binding{};
 uintptr_t game=0,thread=0;uint64_t serial=0,fontIdentity=0;
 uint32_t caller=0,modal=0,count=0,feedbackAvailable=1,controlsArchive=0;
 nfs_controls_dialog_word::Authority authority{};
 std::array<Resource,2>resources{};Snapshot post{};
};
inline const char*text(unsigned i)noexcept{return i==0?"force feedback":i==1?"device not found":"";}
inline unsigned resourceId(unsigned i)noexcept{return i==0?0x14a:i==1?0x301:0;}
inline unsigned width(unsigned i)noexcept{return i==0?96:i==1?108:0;}
inline int anchor(unsigned i)noexcept{return i==0?350:320;}
inline int top(unsigned i)noexcept{return i==0?185:225;}
inline uint16_t color(unsigned i)noexcept{return i==0?1:65535;}
inline bool exactBytes(const std::array<uint8_t,96>&v,unsigned i)noexcept{const auto*s=text(i);const size_t n=std::strlen(s);if(i>1||std::memcmp(v.data(),s,n))return false;for(size_t k=n;k<v.size();++k)if(v[k])return false;return true;}
// Original modal policy 2 normalizes to packet alignment 1; packet x is
// already the measured draw-left, not the original modal anchor.
inline bool command(const nfs_soft::TextCommand&c,unsigned i)noexcept{return i<2&&nfs_controls_dialog_word::exactText(c,text(i))&&nfs_controls_dialog_word::box(c)&&c.role==i&&c.alignment==1&&c.x==anchor(i)-int(width(i)/2)&&c.y==top(i)&&c.advance==int(width(i))&&c.color==color(i);}
// Current is independently re-observed by the lexical producer after painting;
// its lifetime must extend to the outer caller after original presentation.
struct Current {Evidence observed{};std::shared_ptr<const nfs_soft::FramePacket>packet;const uint16_t*original565=nullptr;size_t planeWords=0;};
}

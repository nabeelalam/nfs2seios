// SPDX-License-Identifier: MIT
#pragma once
#include "FeedbackModalTextProof.h"
#include "FeedbackModalInvocationGameHooks.inc"
#include <atomic>
// Independent lexical collection only. No source Proof is sealed here: a future
// backdrop replay must still join this same opaque invocation before sealing.
// Surface lifetime belongs to the Controls owner, not the per-popup serial.
namespace nfs_feedback_modal_text_live {
using Snapshot=nfs_feedback_modal_text::Snapshot;
using Resource=nfs_feedback_modal_text::Resource;
using SourceIdentity=nfs_feedback_modal_text::SourceIdentity;
using Authority=nfs_controls_dialog_word::Authority;
struct State;struct ModalScope;
class Pending final {
 std::shared_ptr<const nfs_feedback_modal_live::Invocation>invocation_;
 SourceIdentity source_{};Authority authority_{};uintptr_t game_=0,thread_=0;
 uint64_t epoch_=0;uint32_t array_=0;std::array<Resource,2>resources_{};
 Snapshot post_{};nfs_logo_capture::Target main_{};std::vector<uint16_t>before_;
 std::shared_ptr<const nfs_soft::FramePacket>packet_;
 explicit Pending(const State&);friend struct ModalScope;
public:
 const auto&invocation()const noexcept{return invocation_;}
 const SourceIdentity&source()const noexcept{return source_;}const Authority&authority()const noexcept{return authority_;}
 uintptr_t game()const noexcept{return game_;}uintptr_t thread()const noexcept{return thread_;}uint64_t epoch()const noexcept{return epoch_;}
 uint32_t array()const noexcept{return array_;}const auto&resources()const noexcept{return resources_;}const Snapshot&post()const noexcept{return post_;}
 const auto&mainTarget()const noexcept{return main_;}const auto&beforeMain()const noexcept{return before_;}const auto&packet()const noexcept{return packet_;}
};
inline SourceIdentity source()noexcept{SourceIdentity s;s.controls=nfs_controls_text::dialogSource();s.caller=uint32_t(game_id_sub_410414);s.modal=uint32_t(game_id_sub_402E80);s.stringDraw=uint32_t(game_id_sub_405CF0);s.measure=uint32_t(game_id_sub_482424);s.glyph=uint32_t(game_id_sub_4A5830);return s;}
struct State {
 std::shared_ptr<const nfs_feedback_modal_live::Invocation>invocation;
 Game*game=nullptr;uintptr_t thread=0;uint64_t epoch=0;SourceIdentity compiled{};Authority authority{};
 uint32_t array=0;unsigned modalDepth=0,policyDepth=0,policyIndex=0,commands=0,glyphDepth=0;
 bool failed=false,modalEntered=false,closed=false,recording=false,presented=false;const char*failure="none";
 nfs_controls_text::State scratch;Snapshot post{};nfs_logo_capture::Target main{};
 std::vector<uint16_t>before;std::array<Resource,2>resources{};std::shared_ptr<const Pending>pending;
 explicit State(Game&g):invocation(nfs_feedback_modal_live::activeInvocation(g)),game(&g),thread(nfs_logo_publisher::threadToken()),epoch(invocation?invocation->modalEpoch():0),compiled(source()),scratch(nfs_soft::acquireFontCatalog()),before(640*480){scratch.lifetime=invocation?invocation->lifetime():0;}
 void reject(const char*why)noexcept{if(!failed)failure=why;failed=true;recording=false;scratch.builder.invalidate();pending.reset();}
};
inline State*&active()noexcept{static thread_local State*p=nullptr;return p;}
inline State*current(Game&g)noexcept{auto*s=active();return s&&s->game==&g&&s->thread==nfs_logo_publisher::threadToken()&&s->invocation&&nfs_feedback_modal_live::activeInvocation(g)==s->invocation?s:nullptr;}
inline Pending::Pending(const State&s):invocation_(s.invocation),source_(s.compiled),authority_(s.authority),game_(reinterpret_cast<uintptr_t>(s.game)),thread_(s.thread),epoch_(s.epoch),array_(s.array),resources_(s.resources),post_(s.post),main_(s.main),before_(s.before),packet_(s.scratch.packet){}
inline bool readResources(State&s,bool initial)noexcept{try{
 Authority a;if(!s.invocation||nfs_feedback_modal_live::activeInvocation(*s.game)!=s.invocation||!nfs_controls_text::readDialogAuthority(a)||a.selectedIndex!=2||!(a==s.invocation->authority())||!(source()==s.compiled))return false;
 nfs::GuestCopyTransaction tx;if(tx.load<uint8_t>(byte_512ED3)||!a.table||!s.array||uint64_t(s.array)+8>0x100000000ULL)return false;
 if(!initial&&!(a==s.authority))return false;
 for(unsigned i=0;i<2;++i){auto&r=s.resources[i];Resource observed;observed.id=nfs_feedback_modal_text::resourceId(i);observed.table=a.table;const uint64_t h=uint64_t(a.table)+12*observed.id;if(h+12>0x100000000ULL)return false;observed.headerAddress=uint32_t(h);
  for(unsigned k=0;k<12;++k)observed.headerBefore[k]=tx.load<uint8_t>(observed.headerAddress+k);
  const uint64_t text=uint64_t(a.table)+nfs_controls_dialog_word::le32(observed.headerBefore.data()+8);const auto*name=nfs_feedback_modal_text::text(i);const size_t length=std::strlen(name);
  if(!text||text+length+1>0x100000000ULL||tx.load<uint32_t>(s.array+4*i)!=text)return false;observed.textAddress=uint32_t(text);
  for(size_t k=0;k<=length;++k){const auto c=tx.load<uint8_t>(observed.textAddress+uint32_t(k));if(c!=uint8_t(name[k]))return false;observed.textBefore[k]=c;}
  if(!i&&(observed.headerBefore!=a.headers[2]||observed.textAddress!=a.texts[2]))return false;
  if(initial){r.id=observed.id;r.table=observed.table;r.headerAddress=observed.headerAddress;r.textAddress=observed.textAddress;r.headerBefore=r.headerAfter=observed.headerBefore;r.textBefore=r.textAfter=observed.textBefore;r.resolves=1;}
  else if(r.id!=observed.id||r.table!=observed.table||r.headerAddress!=observed.headerAddress||r.textAddress!=observed.textAddress||r.headerBefore!=observed.headerBefore||r.textBefore!=observed.textBefore)return false;
 }
 if(initial)s.authority=a;return true;
 }catch(...){return false;}}
inline bool readMain(State&s,bool copy)noexcept{try{nfs::GuestCopyTransaction tx;const auto owner=tx.load<uint32_t>(dword_4D4D24);if(!owner)return false;nfs_logo_capture::Target t{},after{};nfs_logo_publisher::target(tx,t,owner);
 // Modal entry may have no ambient selected window. Authenticate the explicit
 // Main descriptor through this invocation, never weaken Controls capture.
 if(!s.invocation||!nfs_feedback_modal_live::mainTarget(t,owner))return false;
 if(copy){if(!nfs_logo_capture::same(t,s.invocation->initialTarget()))return false;s.main=t;nfs_logo_publisher::checked(t.w[8]+16,640*480*2);std::memcpy(s.before.data(),nfs_host_address(t.w[8]+16,640*480*2),640*480*2);if(s.before!=s.invocation->before())return false;}else if(!nfs_logo_capture::same(t,s.main))return false;
 nfs_logo_publisher::target(tx,after,owner);return tx.load<uint32_t>(dword_4D4D24)==owner&&nfs_logo_capture::same(t,after);
 }catch(...){return false;}}
inline bool capture(State&s,Snapshot&out,bool pixels)noexcept{nfs_controls_text::Current c;if(!nfs_controls_text::capture(s.scratch,c,pixels,true)||!nfs_logo_capture::same(c.target,s.main)||c.archive!=s.authority.archive||c.originX||c.originY||!readResources(s,false)||!readMain(s,false))return false;out=nfs_controls_text::dialogSnapshot(c);return true;}
struct ArmScope {
 std::shared_ptr<State>s;int unwind=0;
 explicit ArmScope(Game&g)noexcept{if(auto*prior=active()){prior->reject("nested-arm");return;}try{s=std::make_shared<State>(g);if(!s->epoch||!s->scratch.font||uint32_t(g.eax)!=2){s.reset();return;}s->array=uint32_t(g.edx);if(!readResources(*s,true)||!readMain(*s,true)){s.reset();return;}unwind=nfs_logo_publisher::exceptions();active()=s.get();}catch(...){s.reset();}}
 ~ArmScope(){if(!s)return;if(nfs_logo_publisher::exceptions()>unwind)s->reject("arm-unwind");if(active()==s.get())active()=nullptr;s->closed=true;s->pending.reset();}
};
struct ModalScope {
 State*s=nullptr;int unwind=0;
 explicit ModalScope(Game&g)noexcept{s=current(g);if(!s)return;if(s->failed||s->modalEntered||s->modalDepth||uint32_t(g.eax)!=2||uint32_t(g.edx)!=s->array||!readResources(*s,false)){s->reject("modal-route");s=nullptr;return;}s->modalEntered=true;s->modalDepth=1;unwind=nfs_logo_publisher::exceptions();}
 ~ModalScope(){if(!s)return;s->modalDepth=0;if(s->failed)return;if(nfs_logo_publisher::exceptions()>unwind||s->policyDepth||s->recording||s->glyphDepth||s->commands!=2||!capture(*s,s->post,true)){s->reject("modal-final");return;}s->scratch.packet=s->scratch.builder.finish(s->post.identity,s->scratch.pixels.data(),s->scratch.pixels.size());if(!s->scratch.packet){s->reject("modal-replay");return;}try{s->pending=std::shared_ptr<const Pending>(new Pending(*s));}catch(...){s->reject("pending-allocation");}}
};
struct PolicyScope {
 State*s=nullptr;unsigned index=0;int unwind=0;
 explicit PolicyScope(Game&g)noexcept{s=current(g);if(!s||!s->modalDepth||s->failed){s=nullptr;return;}if(s->policyDepth||s->commands>=2){s->reject("policy-order");s=nullptr;return;}index=s->commands;s->policyIndex=index;auto&r=s->resources[index];try{nfs::GuestCopyTransaction tx;r.alignment=tx.load<uint32_t>(uint32_t(g.esp)+4);r.colorIndex=tx.load<uint32_t>(uint32_t(g.esp)+8);}catch(...){s->reject("policy-stack");s=nullptr;return;}r.anchor=int32_t(g.edx);r.y=int32_t(g.ebx);
 if(uint32_t(g.eax)!=r.textAddress||int32_t(g.ecx)!=18||r.alignment!=2||r.colorIndex!=(index?2u:0u)||r.anchor!=nfs_feedback_modal_text::anchor(index)||r.y!=nfs_feedback_modal_text::top(index)||!capture(*s,r.before,false)){s->reject("policy-identity");s=nullptr;return;}s->policyDepth=1;unwind=nfs_logo_publisher::exceptions();}
 ~PolicyScope(){if(!s)return;s->policyDepth=0;if(s->failed)return;if(nfs_logo_publisher::exceptions()>unwind||s->commands!=index+1||s->resources[index].strings!=1||!readResources(*s,false))s->reject("policy-exit");}
};
inline void selectedFont(Game&g)noexcept{auto*s=current(g);if(!s||s->failed||!s->policyDepth)return;auto&r=s->resources[s->policyIndex];if(r.measures||r.strings||!capture(*s,r.resolved,false))s->reject("policy-font");}
struct MeasureScope {
 State*s=nullptr;unsigned index=0;int unwind=0;
 explicit MeasureScope(Game&g)noexcept{s=current(g);if(!s||s->failed||!s->policyDepth){s=nullptr;return;}index=s->policyIndex;if(uint32_t(g.eax)!=s->resources[index].textAddress||s->resources[index].measures){s->reject("measure-identity");s=nullptr;return;}unwind=nfs_logo_publisher::exceptions();}
 ~MeasureScope(){if(!s||s->failed)return;auto&r=s->resources[index];if(nfs_logo_publisher::exceptions()>unwind||!capture(*s,r.measured,false)){s->reject("measure-state");return;}r.measuredWidth=int32_t(s->game->eax);if(r.measuredWidth!=int(nfs_feedback_modal_text::width(index))){s->reject("measure-width");return;}r.measures=1;}
};
struct StringScope {
 State*s=nullptr;unsigned index=0;int unwind=0;nfs_soft::TextCommand command{};
 explicit StringScope(Game&g)noexcept{s=current(g);if(!s||s->failed||!s->modalDepth){s=nullptr;return;}if(!s->policyDepth||s->recording||s->commands>=2){s->reject("string-order");s=nullptr;return;}index=s->commands;auto&r=s->resources[index];if(s->policyIndex!=index||r.measures!=1||r.strings||uint32_t(g.eax)!=r.textAddress||!capture(*s,r.stringBefore,index==0)){s->reject("string-state");s=nullptr;return;}
 if(!index&&!s->scratch.builder.begin(r.stringBefore.identity,s->epoch,s->scratch.pixels.data(),s->scratch.pixels.size())){s->reject("builder-begin");s=nullptr;return;}
 command.text=r.textBefore;command.length=unsigned(std::strlen(nfs_feedback_modal_text::text(index)));command.role=index;command.alignment=1;command.sourceSize=18;command.x=int32_t(g.edx);command.y=int32_t(g.ebx);command.advance=int(nfs_feedback_modal_text::width(index));command.color=uint16_t(r.stringBefore.color);
 if(!nfs_feedback_modal_text::command(command,index)||!s->scratch.builder.beginString(command)){s->reject("string-command");s=nullptr;return;}s->recording=true;unwind=nfs_logo_publisher::exceptions();}
 ~StringScope(){if(!s)return;s->recording=false;if(s->failed)return;auto&r=s->resources[index];if(nfs_logo_publisher::exceptions()>unwind||s->glyphDepth||!capture(*s,r.after,false)||!nfs_controls_dialog_word::sameSnapshot(r.stringBefore,r.after)){s->reject("string-drift");return;}try{nfs::GuestCopyTransaction tx;const int64_t advance=int64_t(tx.load<int32_t>(dword_4DD6EC))-command.x;if(advance!=command.advance){s->reject("string-advance");return;}}catch(...){s->reject("string-advance-read");return;}if(!s->scratch.builder.endString(command)){s->reject("string-end");return;}r.strings=1;++s->commands;}
};
struct GlyphScope {State*s=nullptr;explicit GlyphScope(Game&g)noexcept{s=current(g);if(!s||!s->recording||s->failed){s=nullptr;return;}if(s->glyphDepth++){s->reject("nested-glyph");}}~GlyphScope(){if(s&&s->glyphDepth)--s->glyphDepth;}};
struct WordCapture {
 State*s=nullptr;uint32_t address=0;size_t index=0;uint16_t before=0,value=0;
 WordCapture(Game&g,uint32_t at,uint16_t computed)noexcept{s=current(g);if(!s||!s->recording||s->failed){s=nullptr;return;}const auto first=uint64_t(s->main.w[8])+16;if(s->glyphDepth!=1||at<first||uint64_t(at)>=first+640*480*2||(at-first)%2){s->reject("word-address");s=nullptr;return;}address=at;index=size_t((at-first)/2);value=computed;try{nfs::GuestCopyTransaction tx;before=tx.load<uint16_t>(at);}catch(...){s->reject("word-before");s=nullptr;}}
 ~WordCapture(){if(!s||s->failed)return;try{nfs::GuestCopyTransaction tx;const auto after=tx.load<uint16_t>(address);if(!s->scratch.builder.word(index,before,value,after))s->reject("word-replay");}catch(...){s->reject("word-after");}}
};
inline std::shared_ptr<const Pending>acquireCurrentPending(Game&g)noexcept{auto*s=current(g);if(!s||s->failed||s->closed||s->modalDepth||!s->pending)return {};Snapshot now;if(!capture(*s,now,true)||!nfs_controls_dialog_word::sameSnapshot(now,s->post)||!s->scratch.packet->matches(now.identity,s->epoch,s->scratch.pixels.data(),s->scratch.pixels.size())){s->reject("current-drift");return {};}return s->pending;}
inline void presented(Game&g)noexcept{auto*s=current(g);if(s)s->presented=bool(acquireCurrentPending(g));}
inline void beforeDescriptorRetire(Game&g,uint32_t descriptor)noexcept{auto*s=current(g);if(s&&(descriptor==s->main.owner||descriptor==s->main.w[8]||descriptor==s->post.font||descriptor==s->post.palette))s->reject("retired");}
inline void beforeOwnerRetire(Game&g,uint32_t owner)noexcept{beforeDescriptorRetire(g,owner);}
}

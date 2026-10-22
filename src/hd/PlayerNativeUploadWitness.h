// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerScenePrepare.h"
#include <atomic>
#include <algorithm>
#include <array>
#include <memory>
// CPU source/upload identity only. No GL/context/resource-generation authority.
namespace nfs_player_native {
using Source=nfs_player_scene_cpu::VerifiedSource;
using SourceBinding=nfs_player_scene_cpu::SourceBinding;
using Plane=nfs_logo_capture::Plane;
struct CurrentBinding {
 SourceBinding source{};
 // sourceOwner is the independently current relocated lexical Player form,
 // distinct from native gameOwner, target.owner and raw upload EAX bitmap.
 uint32_t sourceOwner=0,fullTargetOwner=0,sourceBitmap=0;
};
inline bool sameBinding(const SourceBinding&a,const SourceBinding&b)noexcept{
 return a.form==b.form&&a.formGuest==b.formGuest&&a.sourceOwner==b.sourceOwner&&a.archive==b.archive&&a.gameOwner==b.gameOwner&&a.thread==b.thread&&a.epoch==b.epoch&&a.lifetime==b.lifetime&&a.generation==b.generation&&a.contentSerial==b.contentSerial&&a.copySerial==b.copySerial&&a.kind==b.kind;
}
inline bool bindingMatches(const Source&s,const CurrentBinding&c)noexcept{
 const auto&b=s.binding();return nfs_player_native::sameBinding(b,c.source)&&b.gameOwner&&b.thread&&b.epoch&&b.lifetime&&b.generation&&b.contentSerial&&b.copySerial&&b.archive&&b.formGuest&&b.sourceOwner==b.formGuest&&c.sourceOwner==b.sourceOwner&&c.fullTargetOwner==s.target().owner&&c.sourceBitmap==s.target().w[8]&&c.fullTargetOwner&&c.sourceBitmap&&s.form()==b.form&&(s.player()==1||s.player()==2);
}
inline bool planeMatches(const Source&s,const Plane&p)noexcept{return p.valid==1&&p.width==640&&p.height==480&&nfs_logo_capture::same(p.target,s.target())&&s.original().size()==640u*480u&&std::equal(s.original().begin(),s.original().end(),p.pixels.begin());}
inline uint16_t to1555(uint16_t p)noexcept{const uint16_t rgb=uint16_t(((p&0xf800u)>>1)|((p&0x7c0u)>>1)|(p&31u));return rgb?uint16_t(rgb|0x8000):0;}
inline bool overlap(uint32_t a,uint64_t an,uint32_t b,uint64_t bn)noexcept{return uint64_t(a)<uint64_t(b)+bn&&uint64_t(b)<uint64_t(a)+an;}
struct TileObservation {
 uint32_t descriptorGuest=0,expectedSlotDescriptor=0,payloadGuest=0,tableGuest=0;
 std::array<uint32_t,7>before{},after{};std::array<uint8_t,2048>pixels{};
 bool originalUploadReturned=false;
};
enum class Failure{None,Source,Binding,Overwrite,Order,Payload,Descriptor,Alias,Incomplete,Unwind,Allocation,Retire};
class Witness;
class Proof final {
 struct Validity{std::atomic<bool>valid{true};};
 std::shared_ptr<const Source>source_;std::shared_ptr<Validity>validity_;
 std::array<uint32_t,300>keys_{};const std::atomic<uint64_t>*events_=nullptr;uint64_t event_=0;
 Proof(std::shared_ptr<const Source>s,std::shared_ptr<Validity>v,const std::array<uint32_t,300>&k,const std::atomic<uint64_t>*e,uint64_t n):source_(std::move(s)),validity_(std::move(v)),keys_(k),events_(e),event_(n){}
 friend class Witness;
 bool eventCurrent()const noexcept{return validity_&&validity_->valid.load(std::memory_order_acquire)&&events_&&event_&&event_!=UINT64_MAX&&events_->load(std::memory_order_acquire)==event_;}
public:
 bool current(const CurrentBinding&c)const noexcept{return eventCurrent()&&source_&&bindingMatches(*source_,c)&&eventCurrent();}
 uint64_t uploadEvent(const CurrentBinding&c)const noexcept{return current(c)?event_:0;}
 std::shared_ptr<const Source>source(const CurrentBinding&c)const noexcept{if(!current(c))return {};auto s=source_;return current(c)?s:std::shared_ptr<const Source>{};}
 bool keys(const CurrentBinding&c,std::array<uint32_t,300>&out)const noexcept{out={};if(!current(c))return false;out=keys_;if(!current(c)){out={};return false;}return true;}
 // A pure layout result is not native/GL authorization. The caller must still
 // consume the exact guarded proof at presentation and apply GPU owner guards.
 bool preparedForPresentation(const CurrentBinding&c,const Plane&actual,nfs_player_scene_cpu::Prepared&out)const noexcept{
  out.valid=false;if(!current(c)||!planeMatches(*source_,actual)||!current(c))return false;
  try{if(!nfs_player_scene_cpu::untrustedPreparedLayout(source_,keys_,out))return false;}catch(...){out.valid=false;return false;}
  if(!current(c)){out.valid=false;return false;}return out.valid;
 }
};
class Witness final {
 std::shared_ptr<const Source>source_;std::shared_ptr<Proof::Validity>validity_;std::shared_ptr<const Proof>proof_;
 const std::atomic<uint64_t>*events_=nullptr;uint64_t event_=0;unsigned count_=0;bool active_=false;
 std::array<uint32_t,300>descriptors_{},keys_{};
 Failure failure_=Failure::None;
 bool checked(const CurrentBinding&c)noexcept{if(!source_||!bindingMatches(*source_,c))return reject(Failure::Binding);if(!validity_||!validity_->valid.load(std::memory_order_acquire)||!events_||!event_||event_==UINT64_MAX||events_->load(std::memory_order_acquire)!=event_)return reject(Failure::Overwrite);return true;}
 bool reject(Failure f)noexcept{reset();failure_=f;return false;}
public:
 Witness()=default;Witness(const Witness&)=delete;Witness&operator=(const Witness&)=delete;
 ~Witness(){reset();}
 void reset()noexcept{if(validity_)validity_->valid.store(false,std::memory_order_release);active_=false;count_=0;source_.reset();proof_.reset();validity_.reset();events_=nullptr;event_=0;failure_=Failure::None;}
 Failure failure()const noexcept{return failure_;}
 unsigned uploaded()const noexcept{return count_;}
 // Caller supplies the actual process-global slot1 overwrite counter, whose
 // lifetime must exceed all retained proofs (the original static counter does).
 bool begin(std::shared_ptr<const Source>s,const CurrentBinding&c,const Plane&actualBefore,uint64_t event,const std::atomic<uint64_t>&globalEvents)noexcept{
  reset();if(!s||!bindingMatches(*s,c))return reject(Failure::Binding);if(!planeMatches(*s,actualBefore))return reject(Failure::Source);if(!event||event==UINT64_MAX||globalEvents.load(std::memory_order_acquire)!=event)return reject(Failure::Overwrite);
  try{validity_=std::make_shared<Proof::Validity>();}catch(...){return reject(Failure::Allocation);}source_=std::move(s);events_=&globalEvents;event_=event;active_=true;return checked(c);
 }
 bool tile(const TileObservation&t,const CurrentBinding&c)noexcept{
  if(!active_||count_>=300)return reject(Failure::Order);if(!checked(c))return false;
  if(!t.originalUploadReturned||!t.descriptorGuest||t.descriptorGuest!=t.expectedSlotDescriptor||!t.payloadGuest||t.tableGuest||uint64_t(t.descriptorGuest)+28>UINT64_C(0x100000000)||uint64_t(t.payloadGuest)+2048>UINT64_C(0x100000000))return reject(Failure::Descriptor);
  if(t.before[0]!=3||t.before[1]!=3||t.before[2]!=3||t.before[3]!=11||t.before[6]!=3||t.after[4]!=t.payloadGuest)return reject(Failure::Descriptor);
  for(unsigned k=0;k<7;++k)if(k!=4&&t.before[k]!=t.after[k])return reject(Failure::Descriptor);
  const uint32_t address=t.after[5];if(uint64_t(address)+2048>UINT64_C(0x100000000)||overlap(t.descriptorGuest,28,t.payloadGuest,2048)||overlap(t.descriptorGuest,28,c.sourceBitmap,614416)||overlap(t.payloadGuest,2048,c.sourceBitmap,614416))return reject(Failure::Alias);
  for(unsigned k=0;k<count_;++k)if(overlap(t.descriptorGuest,28,descriptors_[k],28)||overlap(address,2048,keys_[k],2048))return reject(Failure::Alias);
  const unsigned x=(count_/15)*32,y=(count_%15)*32;const auto&pixels=source_->original();for(unsigned yy=0;yy<32;++yy)for(unsigned xx=0;xx<32;++xx){unsigned at=2*(yy*32+xx);uint16_t p=uint16_t(t.pixels[at])|(uint16_t(t.pixels[at+1])<<8);if(p!=to1555(pixels[(y+yy)*640+x+xx]))return reject(Failure::Payload);}
  if(!checked(c))return false;descriptors_[count_]=t.descriptorGuest;keys_[count_]=address;++count_;return true;
 }
 bool finish(const Plane&actualAfter,bool originalUploadReturned,const CurrentBinding&c,bool unwind=false)noexcept{
  if(unwind)return reject(Failure::Unwind);if(!active_||count_!=300||!originalUploadReturned)return reject(Failure::Incomplete);if(!checked(c))return false;if(!planeMatches(*source_,actualAfter))return reject(Failure::Source);
  try{proof_=std::shared_ptr<const Proof>(new Proof(source_,validity_,keys_,events_,event_));}catch(...){return reject(Failure::Allocation);}if(!checked(c))return false;active_=false;return true;
 }
 std::shared_ptr<const Proof>proofForPresentation(const CurrentBinding&c,const Plane&actual)noexcept{
  if(active_||!proof_)return {};if(!checked(c))return {};if(!planeMatches(*source_,actual)){reject(Failure::Source);return {};}auto p=proof_;return p->current(c)?p:std::shared_ptr<const Proof>{};
 }
 // Every unrelated slot1 upload must increment globalEvents before any source
 // match; this notification additionally invalidates retained CPU proof objects.
 void slot1Overwrite()noexcept{reject(Failure::Overwrite);}
 void retire(uint32_t guest)noexcept{if(source_&&guest&&(guest==source_->binding().formGuest||guest==source_->target().owner||guest==source_->target().w[8]))reject(Failure::Retire);}
};
} // namespace nfs_player_native

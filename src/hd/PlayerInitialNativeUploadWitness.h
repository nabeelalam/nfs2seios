// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerInitialSidebarProof.h"
#include <array>
#include <atomic>
#include <memory>
// CPU evidence for the normal Player intro's ORIGINAL slot0 upload only.
// This does not construct a full Player scene, preview, Copy, or GPU authority.
namespace nfs_player_initial_native {
namespace initial=nfs_player_initial_sidebar;namespace bg=nfs_background_scene_cpu;
using Source=initial::Proof;using Plane=bg::Plane;using Catalog=bg::SourceCatalog;
constexpr uint64_t SourceBytes=16+uint64_t(bg::Side)*2,StageBytes=16+uint64_t(bg::Full)*2;
// This function-local static counter has process lifetime and is shared by all
// inline call sites. Every actual slot0 overwrite must increment it BEFORE
// matching an optional source; no counter pointer can outlive caller storage.
inline std::atomic<uint64_t>&uploadEvents()noexcept{static std::atomic<uint64_t>value{0};return value;}
inline uint64_t overwriteEvent()noexcept{auto&events=uploadEvents();uint64_t old=events.load(std::memory_order_acquire);for(;;){if(old==UINT64_MAX)return 0;if(events.compare_exchange_weak(old,old+1,std::memory_order_acq_rel))return old+1==UINT64_MAX?0:old+1;}}
// Independently supplied CURRENT source identity only. The temporary640 owner
// is deliberately absent: original428250 frees it before returning. Its owned
// geometry/pixels below are historical evidence, not a live-address capability.
struct Binding{initial::Binding sidebar{};uint32_t sourceOwner=0,sourceBitmap=0;};
inline bool sameBinding(const Binding&a,const Binding&b)noexcept{return initial::sameBinding(a.sidebar,b.sidebar)&&a.sourceOwner==b.sourceOwner&&a.sourceBitmap==b.sourceBitmap;}
inline bool bindingMatches(const Source&s,const Binding&b,const std::shared_ptr<const Catalog>&catalog)noexcept{return s.current()&&initial::sameBinding(s.binding(),b.sidebar)&&s.catalogMatches(catalog)&&b.sourceOwner&&b.sourceBitmap&&b.sourceOwner==s.post().target.owner&&b.sourceBitmap==s.post().target.w[8]&&s.post().target.header[3]==0;}
inline bool sourceMatches(const Source&s,const Plane&actual)noexcept{return bg::plane(actual,343)&&actual.target.header[3]==0&&nfs_main_sidebar_origin::sameRetainedTarget(s.post().target,actual.target)&&std::equal(s.post().pixels.begin(),s.post().pixels.begin()+bg::Side,actual.pixels.begin());}
inline bool span(uint32_t at,uint64_t bytes)noexcept{return at&&uint64_t(at)+bytes<=UINT64_C(0x100000000);}
inline bool overlap(uint32_t a,uint64_t an,uint32_t b,uint64_t bn)noexcept{return uint64_t(a)<uint64_t(b)+bn&&uint64_t(b)<uint64_t(a)+an;}
inline bool bitmapOrMetadata(uint32_t at,uint64_t bytes,const bg::Target&t,uint64_t imageBytes)noexcept{return overlap(at,bytes,t.w[8],imageBytes)||overlap(at,bytes,t.owner,56)||overlap(at,bytes,t.w[12],uint64_t(t.w[1])*4)||overlap(at,bytes,t.w[11],480*4);}
inline bool targetRanges(const bg::Target&t,uint64_t bytes)noexcept{return span(t.owner,56)&&span(t.w[8],bytes)&&span(t.w[12],uint64_t(t.w[1])*4)&&span(t.w[11],480*4)&&!overlap(t.owner,56,t.w[8],bytes)&&!overlap(t.w[12],uint64_t(t.w[1])*4,t.w[8],bytes)&&!overlap(t.w[11],480*4,t.w[8],bytes);}
// The original clear0 +483470 rawcopy has a complete observable result: each
// row's left343 words is the genuine source and its right297 words is zero.
// No submitted staging PRE is trusted as authority for any published pixel.
inline bool padded(const Source&s,const Plane&p)noexcept{
 if(!bg::plane(p,640)||p.target.header[3]||!targetRanges(p.target,StageBytes)||!targetRanges(s.post().target,SourceBytes))return false;
 const auto&source=s.post().target;if(overlap(source.w[8],SourceBytes,p.target.w[8],StageBytes)||bitmapOrMetadata(p.target.owner,56,source,SourceBytes)||bitmapOrMetadata(source.owner,56,p.target,StageBytes)||bitmapOrMetadata(p.target.w[8],StageBytes,source,SourceBytes)||bitmapOrMetadata(source.w[8],SourceBytes,p.target,StageBytes))return false;
 for(unsigned y=0;y<480;++y)for(unsigned x=0;x<640;++x)if(p.pixels[y*640+x]!=(x<343?s.post().pixels[y*343+x]:0))return false;return true;
}
inline bool sameStage(const Plane&a,const Plane&b)noexcept{return bg::plane(b,640)&&nfs_logo_capture::same(a.target,b.target)&&a.pixels==b.pixels;}
inline uint16_t to1555(uint16_t p)noexcept{const uint16_t rgb=uint16_t(((p&0xf800u)>>1)|((p&0x7c0u)>>1)|(p&31u));return rgb?uint16_t(rgb|0x8000):0;}
struct TileObservation{uint32_t descriptorGuest=0,expectedSlotDescriptor=0,payloadGuest=0,tableGuest=0;std::array<uint32_t,7>before{},after{};std::array<uint8_t,2048>pixels{};bool originalUploadReturned=false;};
enum class Failure{None,Source,Stage,Binding,Overwrite,Order,Descriptor,Alias,Payload,Incomplete,Unwind,Allocation,Retire,Owner};
class Witness;
class Proof final{
 struct Validity{std::atomic<bool>valid{true};};
 std::shared_ptr<const Source>source_;std::shared_ptr<const Plane>stage_;std::shared_ptr<Validity>validity_;Binding binding_{};std::array<uint32_t,300>keys_{};uint64_t event_=0;
 Proof()=default;friend class Witness;
 bool eventCurrent()const noexcept{return validity_&&validity_->valid.load(std::memory_order_acquire)&&event_&&event_!=UINT64_MAX&&uploadEvents().load(std::memory_order_acquire)==event_;}
public:
 Proof(const Proof&)=delete;Proof&operator=(const Proof&)=delete;
 bool current(const Binding&binding,const std::shared_ptr<const Catalog>&catalog)const noexcept{if(!eventCurrent())return false;if(!source_||!sameBinding(binding_,binding)||!bindingMatches(*source_,binding,catalog)){if(validity_)validity_->valid.store(false,std::memory_order_release);return false;}return eventCurrent();}
 uint64_t uploadEvent(const Binding&binding,const std::shared_ptr<const Catalog>&catalog)const noexcept{return current(binding,catalog)?event_:0;}
 std::shared_ptr<const Source>source(const Binding&binding,const std::shared_ptr<const Catalog>&catalog)const noexcept{if(!current(binding,catalog))return {};auto result=source_;return current(binding,catalog)?result:std::shared_ptr<const Source>{};}
 bool keys(const Binding&binding,const std::shared_ptr<const Catalog>&catalog,std::array<uint32_t,300>&out)const noexcept{out={};if(!current(binding,catalog))return false;out=keys_;if(!current(binding,catalog)){out={};return false;}return true;}
 // Owned historical plane, usable after expected temporary640 cleanup. Numeric
 // target addresses describe the completed upload and confer no current lease.
 std::shared_ptr<const Plane>stage(const Binding&binding,const std::shared_ptr<const Catalog>&catalog)const noexcept{return current(binding,catalog)?stage_:std::shared_ptr<const Plane>{};}
};
class Witness final{
 std::shared_ptr<const Source>source_;std::shared_ptr<const Plane>stage_;std::shared_ptr<Proof::Validity>validity_;std::shared_ptr<const Proof>proof_;Binding binding_{};bg::Target sourceBefore_{};
 std::array<uint32_t,300>descriptors_{},payloads_{},keys_{};uint64_t event_=0;unsigned count_=0;bool active_=false;Failure failure_=Failure::None;
 bool reject(Failure failure)noexcept{reset();failure_=failure;return false;}
 bool checked(const Binding&binding,const std::shared_ptr<const Catalog>&catalog)noexcept{if(!source_||!sameBinding(binding_,binding)||!bindingMatches(*source_,binding,catalog))return reject(Failure::Binding);if(!validity_||!validity_->valid.load(std::memory_order_acquire)||!event_||event_==UINT64_MAX||uploadEvents().load(std::memory_order_acquire)!=event_)return reject(Failure::Overwrite);return true;}
 bool aliases(uint32_t at,uint64_t bytes)const noexcept{return bitmapOrMetadata(at,bytes,source_->post().target,SourceBytes)||bitmapOrMetadata(at,bytes,stage_->target,StageBytes);}
public:
 Witness()=default;Witness(const Witness&)=delete;Witness&operator=(const Witness&)=delete;~Witness(){reset();}
 void reset()noexcept{if(validity_)validity_->valid.store(false,std::memory_order_release);source_.reset();stage_.reset();proof_.reset();validity_.reset();binding_={};sourceBefore_={};event_=0;count_=0;active_=false;failure_=Failure::None;}
 Failure failure()const noexcept{return failure_;}unsigned uploaded()const noexcept{return count_;}
 // Called only after the original clear0/rawcopy and BEFORE any tile upload.
 // Both source and fully initialized staging planes are independently captured
 // original memory. All allocations/calls here must occur outside guest leases.
 bool begin(std::shared_ptr<const Source>source,const Binding&binding,const Plane&actualSourceBefore,const Plane&actualStageBefore,uint64_t event,const std::shared_ptr<const Catalog>&catalog)noexcept{
  reset();if(!source||!bindingMatches(*source,binding,catalog)||!sourceMatches(*source,actualSourceBefore))return reject(Failure::Source);if(!padded(*source,actualStageBefore))return reject(Failure::Stage);if(!event||event==UINT64_MAX||uploadEvents().load(std::memory_order_acquire)!=event)return reject(Failure::Overwrite);
  try{stage_=std::make_shared<const Plane>(actualStageBefore);validity_=std::make_shared<Proof::Validity>();}catch(...){return reject(Failure::Allocation);}source_=std::move(source);sourceBefore_=actualSourceBefore.target;binding_=binding;event_=event;active_=true;return checked(binding,catalog);
 }
 bool tile(const TileObservation&t,const Binding&binding,const std::shared_ptr<const Catalog>&catalog)noexcept{
  if(!active_||count_>=300)return reject(Failure::Order);if(!checked(binding,catalog))return false;
  if(!t.originalUploadReturned||t.descriptorGuest!=t.expectedSlotDescriptor||!span(t.descriptorGuest,28)||!span(t.payloadGuest,2048)||t.tableGuest)return reject(Failure::Descriptor);
  if(t.before[0]!=3||t.before[1]!=3||t.before[2]!=3||t.before[3]!=11||t.before[6]!=3||t.after[4]!=t.payloadGuest)return reject(Failure::Descriptor);
  // Original49755C binds field4 to its real payload; every other descriptor
  // word, including logical address/key5, must remain unchanged.
  for(unsigned k=0;k<7;++k)if(k!=4&&t.before[k]!=t.after[k])return reject(Failure::Descriptor);
  const uint32_t key=t.after[5];if(uint64_t(key)+2048>UINT64_C(0x100000000)||overlap(t.descriptorGuest,28,t.payloadGuest,2048)||aliases(t.descriptorGuest,28)||aliases(t.payloadGuest,2048))return reject(Failure::Alias);
  for(unsigned k=0;k<count_;++k)if(overlap(t.descriptorGuest,28,descriptors_[k],28)||overlap(key,2048,keys_[k],2048)||overlap(t.payloadGuest,2048,descriptors_[k],28)||overlap(t.descriptorGuest,28,payloads_[k],2048))return reject(Failure::Alias);
  const unsigned x=(count_/15)*32,y=(count_%15)*32;for(unsigned yy=0;yy<32;++yy)for(unsigned xx=0;xx<32;++xx){const unsigned at=2*(yy*32+xx);const uint16_t word=uint16_t(t.pixels[at])|(uint16_t(t.pixels[at+1])<<8);if(word!=to1555(stage_->pixels[(y+yy)*640+x+xx]))return reject(Failure::Payload);}
  if(!checked(binding,catalog))return false;descriptors_[count_]=t.descriptorGuest;payloads_[count_]=t.payloadGuest;keys_[count_]=key;++count_;return true;
 }
 // Finish BEFORE482DA0 frees temporary640. Full original source and stage
 // after planes are checked here. Proof use later needs only live source343.
 bool finish(const Plane&actualSourceAfter,const Plane&actualStageAfter,bool originalUploadReturned,const Binding&binding,const std::shared_ptr<const Catalog>&catalog,bool unwind=false)noexcept{
  if(unwind)return reject(Failure::Unwind);if(!active_||count_!=300||!originalUploadReturned)return reject(Failure::Incomplete);if(!checked(binding,catalog))return false;
  if(!sourceMatches(*source_,actualSourceAfter)||!nfs_logo_capture::same(sourceBefore_,actualSourceAfter.target))return reject(Failure::Source);if(!sameStage(*stage_,actualStageAfter))return reject(Failure::Stage);
  try{std::shared_ptr<Proof>proof(new Proof);proof->source_=source_;proof->stage_=stage_;proof->validity_=validity_;proof->binding_=binding_;proof->keys_=keys_;proof->event_=event_;proof_=std::move(proof);}catch(...){return reject(Failure::Allocation);}if(!checked(binding,catalog))return false;active_=false;return true;
 }
 std::shared_ptr<const Proof>proofForUse(const Plane&actualSourceNow,const Binding&binding,const std::shared_ptr<const Catalog>&catalog)noexcept{if(active_||!proof_)return {};if(!checked(binding,catalog))return {};if(!sourceMatches(*source_,actualSourceNow)){reject(Failure::Source);return {};}auto proof=proof_;return proof->current(binding,catalog)?proof:std::shared_ptr<const Proof>{};}
 // The independent global counter must be advanced for EVERY slot0 overwrite;
 // this local notification also revokes already escaped proof objects.
 void slot0Overwrite()noexcept{reject(Failure::Overwrite);}void ownerExit()noexcept{reject(Failure::Owner);}
 void retire(uint32_t at)noexcept{if(!source_||!at)return;const auto&t=source_->post().target;if(at==binding_.sidebar.formGuest||at==binding_.sidebar.archive||at==t.owner||at==t.w[8]||at==t.w[11]||at==t.w[12]){reject(Failure::Retire);return;}if(active_&&stage_){const auto&s=stage_->target;if(at==s.owner||at==s.w[8]||at==s.w[11]||at==s.w[12])reject(Failure::Retire);}}
 // Completed staging-only cleanup is expected and does NOT revoke the source
 // proof. retire() is for actual retired allocations; shared LUT descriptors
 // require the caller's original refcount-aware classification before calling.
};
} // namespace nfs_player_initial_native

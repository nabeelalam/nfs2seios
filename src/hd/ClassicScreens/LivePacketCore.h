// PRIVATE experiment only. No maintained registration or public font bytes.
#pragma once
#include "hd/LoadSaveSourcePolicy.h"
#include "hd/SoftwareFontProvider.h"
#include "hd/SoftwareTextLayerAbi.h"
#include "ProfileIdentity.h"
#include "LiveLayerAbi.h"
#include <atomic>
#include <utility>
namespace nfs_ls_live {
namespace source=nfs_loadsave_source;
class LiveIssuer;
class OwnedFrame;
// Only the actual Game adapter may construct a current observation or issue a
// packet. Neither a caller-set ready flag nor historical capture is authority.
class ObservedNow final {
 friend class LiveIssuer;friend class OwnedFrame;
 source::Authority authority_;source::Targets targets_;source::Snapshot font_;
 std::vector<uint16_t>post_;uint64_t ticket_=0;
 ObservedNow()=default;
public:
 ObservedNow(const ObservedNow&)=default;
};
class OwnedFrame final {
 friend class LiveIssuer;
 std::shared_ptr<const nfs_soft::FramePacket>packet_;
 source::Profile profile_;source::Evidence evidence_;uint64_t ticket_=0;
 OwnedFrame(std::shared_ptr<const nfs_soft::FramePacket>p,source::Profile f,source::Evidence e,uint64_t t):packet_(std::move(p)),profile_(std::move(f)),evidence_(std::move(e)),ticket_(t){}
public:
 const std::shared_ptr<const nfs_soft::FramePacket>&packet()const noexcept{return packet_;}
 const source::Evidence&evidence()const noexcept{return evidence_;}
 const source::Profile&profile()const noexcept{return profile_;}
 bool matches(const ObservedNow&n)const noexcept{
  return ticket_&&ticket_==n.ticket_&&source::sameAuthority(evidence_.exit,n.authority_)&&n.targets_.valid(n.authority_.owner)&&evidence_.targetsAfter.resourcesEqual(n.targets_)&&n.targets_.selected==evidence_.targetsAfter.selected&&n.targets_.selectedOwner==evidence_.targetsAfter.selectedOwner&&n.font_.valid(profile_,n.authority_.owner)&&n.font_.font==evidence_.post.font&&n.font_.palette==evidence_.post.palette&&n.font_.surface==evidence_.post.surface&&packet_&&packet_->matches(n.font_.surface,n.authority_.owner.serial,n.post_.data(),n.post_.size());
 }
};
// Shared pointers own every queued CPU allocation. The C renderer has a paired
// retain/release reference, so worker cancellation cannot dangle its borrows.
using FrameView=NfsLoadSaveLiveView;
// One bounded CPU cache retains the actual immutable catalog owner. Identity
// and shared-control-block equality are required; raw pointer equality alone
// cannot keep a replaced provider alive or authorize reused storage.
struct FontDigestCache {
 std::shared_ptr<const nfs_soft::FontCatalog>owner;uint64_t identity=0,calls=0;
 std::array<unsigned char,32>sha{};
 bool get(const std::shared_ptr<const nfs_soft::FontCatalog>&font)noexcept{
  if(!font)return false;
  if(owner.get()==font.get()&&!owner.owner_before(font)&&!font.owner_before(owner)&&identity==font->identity())return true;
  owner.reset();identity=0;sha={};if(calls==UINT64_MAX)return false;++calls;
  if(!nfs_texture_sha256(font->alpha().data(),font->alpha().size(),sha.data()))return false;
  owner=font;identity=font->identity();return true;
 }
};
inline FontDigestCache&fontDigestCache(){static thread_local FontDigestCache cache;return cache;}
class Lease final {
 std::atomic<unsigned>refs_{1};std::shared_ptr<const OwnedFrame>frame_;
 explicit Lease(std::shared_ptr<const OwnedFrame>f):frame_(std::move(f)){}
 static void retain(void*p)noexcept{static_cast<Lease*>(p)->refs_.fetch_add(1,std::memory_order_relaxed);}
 static void release(void*p)noexcept{auto*s=static_cast<Lease*>(p);if(s->refs_.fetch_sub(1,std::memory_order_acq_rel)==1)delete s;}
public:
 static std::shared_ptr<const Lease>create(std::shared_ptr<const OwnedFrame>f)noexcept{if(!f||!f->packet())return {};try{return std::shared_ptr<const Lease>(new Lease(std::move(f)),[](const Lease*p){release(const_cast<Lease*>(p));});}catch(...){return {};}}
 FrameView view(uint64_t request)const noexcept{
  FrameView v{};if(!request||!frame_||!frame_->packet()||!frame_->packet()->font())return v;
  const auto&p=*frame_->packet();const auto&font=*p.font();auto&digest=fontDigestCache();if(!digest.get(p.font()))return v;
  v.schema=1;v.width=640;v.height=480;v.request=request;v.content=p.serial();v.original=p.original().data();v.clean=p.clean().data();v.pixels=p.pixels();v.text.logicalWidth=640;v.text.logicalHeight=480;v.text.font.schema=1;v.text.font.width=font.width;v.text.font.height=font.height;v.text.font.identity=font.identity();v.text.font.alpha=font.alpha().data();v.text.font.bytes=font.alpha().size();std::copy(digest.sha.begin(),digest.sha.end(),v.text.font.sha256);v.text.vertices=p.vertices().data();v.text.vertexCount=p.vertices().size();v.owner=const_cast<Lease*>(this);v.retain=retain;v.release=release;return v;
 }
};
} // namespace nfs_ls_live

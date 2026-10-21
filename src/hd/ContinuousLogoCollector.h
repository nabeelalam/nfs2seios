// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundSceneCatalog.h"
#include "OriginalUiErrors.h"
#include <type_traits>
#include <limits>
namespace nfs_logo_publisher {
using namespace nfs_logo_capture;
using nfs_small_logo_live::Prepared;
using nfs_small_logo_live::Workspace;
// This is CPU content identity only, never GL/resource-generation authority.
struct OwnedPacket {
 uint64_t contentRevision=0;
 Prepared prepared{};
 std::array<uint16_t,MaxPixels> originalCopyPost565{};
};
using Publish=bool(*)(void*,const OwnedPacket&);
inline uint16_t to1555(uint16_t p)noexcept {const uint16_t rgb=uint16_t(((p&0xf800u)>>1)|((p&0x7c0u)>>1)|(p&31u));return rgb?uint16_t(rgb|0x8000):0;}
struct TileObservation {
 uint32_t descriptorGuest=0,expectedSlotDescriptor=0,payloadGuest=0,tableGuest=0;
 std::array<uint32_t,7> before{},after{};
 std::array<uint8_t,2048> pixels{};
 bool originalUploadReturned=false;
};
struct Collector {
 const nfs_small_logo_live::SourceCatalog*catalog=nullptr;
 Group group{};Copy copy{};Workspace workspace{};OwnedPacket packet{};
 Plane sourceScratch{};Target uploadOwner{};
 uintptr_t gameOwner=0;uint64_t thread=0,nextRevision=0,iterationRevision=0,nativeUploadEvent=0;
 uint32_t depth=0,ownerKind=0,ownerArg=0,invocationDepth=0;int32_t pending=-1;
 bool presentationArmed=false;
 bool inMenu=false,groupActive=false,groupFinished=false,copyActive=false,prepared=false,uploadActive=false,bound=false,invalid=false;
 unsigned uploaded=0;
 std::array<uint32_t,300> descriptors{},rendererAddresses{};
 enum class Stop {None,Owner,Nested,Order,Unwind,Verifier,TransparentUnderlay,Overwrite,UploadSource,UploadPayload,Incomplete,Retire,Overflow};
 Stop stop=Stop::None;
 void invalidate(Stop why)noexcept {prepared=uploadActive=bound=presentationArmed=false;packet.prepared.valid=false;stop=why;}
 bool beginMenu(uintptr_t owner,uint64_t ownerThread)noexcept {
  if(inMenu||!owner||!ownerThread){invalidate(Stop::Owner);invalid=true;return false;}
  invalidate(Stop::Owner);gameOwner=owner;thread=ownerThread;inMenu=true;invalid=false;depth=0;return true;
 }
 void endMenu(uintptr_t owner,bool unwind=false)noexcept {
  if(owner!=gameOwner){invalidate(Stop::Owner);invalid=true;return;}
  invalidate(unwind?Stop::Unwind:Stop::Owner);inMenu=false;groupActive=groupFinished=copyActive=false;depth=0;pending=-1;gameOwner=0;thread=0;
 }
 bool enterIteration(uint32_t kind,uint32_t arg,uint64_t ownerThread)noexcept {
  if(!inMenu||invalid||thread!=ownerThread){invalidate(Stop::Owner);return false;}
  if(depth++){invalidate(Stop::Nested);invalid=true;group.error|=Nested;group.complete=0;copy.error|=Nested;copy.complete=0;return false;}
  invalidate(Stop::Order);groupActive=groupFinished=copyActive=false;pending=-1;ownerKind=kind;ownerArg=arg;
  if(nextRevision==UINT64_MAX){invalidate(Stop::Overflow);invalid=true;return false;}
  iterationRevision=++nextRevision;
  static_assert(std::is_trivially_copyable<Group>::value&&std::is_trivially_copyable<Copy>::value,"native fixed records");
  std::fill_n(reinterpret_cast<unsigned char*>(&group),sizeof(group),0);std::fill_n(reinterpret_cast<unsigned char*>(&copy),sizeof(copy),0);for(auto&d:group.draw)d.role=UINT32_MAX;
  group.serial=iterationRevision;group.thread=thread;group.kind=kind;group.ownerArg=arg;
  return true;
 }
 void leaveIteration(bool unwind=false)noexcept {
  if(!depth){invalidate(Stop::Order);return;}if(--depth)return;
  if(unwind||groupActive||copyActive||invalid){invalidate(unwind?Stop::Unwind:Stop::Incomplete);group.complete=copy.complete=0;}
  groupActive=copyActive=false;pending=-1;
 }
 Group* current()noexcept{return groupActive?&group:nullptr;}
 Group* beginGroup()noexcept {
  if(!inMenu||invalid||depth!=1||ownerKind!=1||groupActive||copyActive||groupFinished){invalidate(Stop::Order);return nullptr;}
  groupActive=true;group.stage=1;return &group;
 }
 void finishGroup()noexcept {
  if(!groupActive||pending!=-1||group.draws!=3||!group.pre.valid||!group.post.valid||!same(group.pre.target,group.post.target))group.error|=Order;
  for(unsigned i=0;i<std::min(group.draws,3u);++i){group.error|=group.draw[i].error;if(!group.draw[i].complete)group.error|=Incomplete;}
  group.stage=2;group.complete=group.error==0;groupFinished=group.complete;groupActive=false;pending=-1;
 }
 bool armCopy()noexcept {
  if(!inMenu||invalid||depth!=1||!groupFinished||groupActive||copyActive||copy.serial){invalidate(Stop::Order);return false;}
  copy.serial=iterationRevision;copy.thread=thread;copy.linkedGroup=group.serial;copyActive=true;return true;
 }
 bool finishCopy()noexcept {
  copyActive=false;
  if(!inMenu||invalid||depth!=1||!groupFinished||!copy.complete||copy.error||!catalog||nfs_small_logo_live::verifyAndPrepare(group,copy,workspace,packet.prepared,*catalog)!=nfs_small_logo_live::Failure::None){invalidate(Stop::Verifier);return false;}
  // Native1555 must be opaque across the original cleanup support.
  for(unsigned i=0;i<MaxPixels;++i)if(packet.prepared.groupCleanupAffected[i]&&to1555(packet.prepared.cleanCopyPost565[i])==0){invalidate(Stop::TransparentUnderlay);return false;}
  packet.originalCopyPost565=copy.post.pixels;packet.contentRevision=iterationRevision;prepared=true;stop=Stop::None;return true;
 }
 // Called for EVERY slot1 overwrite, before trying to match or reading source.
 void slot1Overwrite()noexcept {bound=false;uploadActive=false;uploaded=0;stop=Stop::Overwrite;}
 bool sourceMatches(const Plane&p)const noexcept {
  return prepared&&p.valid==1&&p.width==640&&p.height==480&&same(p.target,copy.post.target)&&p.pixels==packet.originalCopyPost565;
 }
 bool beginUpload(const Plane&before,uint32_t slot)noexcept {
  if(slot!=1)return false;slot1Overwrite();
  if(!inMenu||invalid||depth!=1||!prepared||!sourceMatches(before)){stop=Stop::UploadSource;return false;}
  uploadOwner=before.target;uploadActive=true;return true;
 }
 // Actual call-bound before/after descriptor and owned payload, never an authorization boolean.
 bool uploadTile(const TileObservation&t)noexcept {
  bool ok=uploadActive&&uploaded<300&&t.originalUploadReturned&&t.descriptorGuest&&t.descriptorGuest==t.expectedSlotDescriptor&&t.payloadGuest&&t.tableGuest==0;
  // 32x32, one mip, square ARGB1555, both mip parity: original497464(32,32,3,0,0).
  ok=ok&&t.before[0]==3&&t.before[1]==3&&t.before[2]==3&&t.before[3]==11&&t.before[6]==3&&t.after[4]==t.payloadGuest;
  for(unsigned j=0;j<7;++j)if(j!=4&&t.before[j]!=t.after[j])ok=false;
  for(unsigned j=0;j<uploaded;++j)if(descriptors[j]==t.descriptorGuest||(uint64_t(rendererAddresses[j])<uint64_t(t.after[5])+2048&&uint64_t(t.after[5])<uint64_t(rendererAddresses[j])+2048))ok=false;
  if(ok){const unsigned x=(uploaded/15)*32,y=(uploaded%15)*32;for(unsigned yy=0;yy<32;++yy)for(unsigned xx=0;xx<32;++xx){const auto p=nfs_small_logo_live::le16(t.pixels.data()+2*(yy*32+xx));if(p!=to1555(packet.originalCopyPost565[(y+yy)*640+x+xx]))ok=false;}}
  if(!ok){uploadActive=false;bound=false;stop=Stop::UploadPayload;return false;}
  descriptors[uploaded]=t.descriptorGuest;rendererAddresses[uploaded]=t.after[5];++uploaded;return true;
 }
 bool finishUpload(const Plane&after,bool originalUploadReturned,bool unwind=false)noexcept {
  if(!uploadActive||!originalUploadReturned||unwind||uploaded!=300||!sourceMatches(after)||!same(uploadOwner,after.target)){uploadActive=bound=false;stop=unwind?Stop::Unwind:Stop::UploadSource;return false;}
  uploadActive=false;bound=true;stop=Stop::None;return true;
 }
 bool publishCurrent(uintptr_t owner,uint32_t slot,const Plane&current,Publish publish,void*context)noexcept {
  if(!inMenu||invalid||owner!=gameOwner||slot!=1||!bound||!sourceMatches(current)||!publish){invalidate(Stop::Owner);return false;}
  // Callback receives only owned native content, outside all guest leases. It must copy into its owned closure.
  bool ok=false;try{ok=publish(context,packet);}catch(...){ok=false;}if(!ok)invalidate(Stop::Order);return ok;
 }
 void retire(uint32_t address)noexcept {
  if(address&&(address==copy.post.target.owner||address==copy.post.target.w[8]||address==copy.sourceOwner.owner||address==copy.sourceOwner.w[8]))invalidate(Stop::Retire);
 }
 void reset()noexcept {invalidate(Stop::Retire);}
};
}

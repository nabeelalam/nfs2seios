// SPDX-License-Identifier: MIT
#pragma once
#include "ControlsLazyParentCopyRecords.h"
#include "SmallLogoPrepare.h"
#include "ControlsLazyParentKeyedTransform.h"
#include <memory>
namespace nfs_controls_lazy_parent_copy {
enum class Failure {None,Binding,Target,Source,Geometry,Sequence,Row,Alias,Pixels};
class VerifiedUpdate final {
 Binding binding_; Geometry geometry_; std::vector<uint8_t> before_,after_; uint32_t rows_;
 VerifiedUpdate(const Record&r):binding_(r.bindingBefore),geometry_(r.resolved),before_(r.mainPlaneBefore),after_(r.mainPlaneAfter),rows_(uint32_t(r.rowKind==RowKind::Direct?r.rows.size():r.keyedRows.size())){}
 friend struct ReplayVerifier;
public:
 const Binding& binding()const noexcept{return binding_;}
 const Geometry& geometry()const noexcept{return geometry_;}
 const std::vector<uint8_t>& beforePlane()const noexcept{return before_;}
 const std::vector<uint8_t>& afterPlane()const noexcept{return after_;}
 uint32_t rowCount()const noexcept{return rows_;}
};
struct Result {Failure failure=Failure::Binding;std::unique_ptr<const VerifiedUpdate> update;};
struct ReplayVerifier {
 static Result verify(const Record&r,const Current&c){
  auto fail=[](Failure f){return Result{f,nullptr};};
  if(!c.binding.valid()||!(r.bindingBefore==c.binding)||!(r.bindingAfter==c.binding)||r.complete!=1||r.error||r.site!=0x40bbe0||r.mode!=0x2800||r.originalArgs!=std::array<uint32_t,4>{c.binding.sourceBitmap,0x2800,0,0})return fail(Failure::Binding);
  const bool keyed=r.rowKind==RowKind::Keyed;
  if((r.rowKind!=RowKind::Direct&&!keyed)||(keyed?!r.rows.empty():!r.keyedRows.empty()))return fail(Failure::Row);
  const size_t rowCount=keyed?r.keyedRows.size():r.rows.size();
  if(keyed&&r.keyBefore!=r.keyAfter)return fail(Failure::Source);
  const auto&s=r.sourceBefore;const auto&m=r.mainBefore;
  if(!nfs_small_logo_live::target(s,343)||!nfs_small_logo_live::target(m,640)||!nfs_logo_capture::same(s,r.sourceAfter)||!nfs_logo_capture::same(m,r.mainAfter)||!nfs_logo_capture::same(s,c.sidebar)||!nfs_logo_capture::same(m,c.main)||s.owner!=c.binding.sourceOwner||s.w[8]!=c.binding.sourceBitmap||m.owner!=c.binding.mainOwner||m.ownerState[3]!=m.owner)return fail(Failure::Target);
  const uint32_t selected=s.ownerState[0]>>16;
  if(selected>1||s.ownerState[1+selected]!=s.owner||s.ownerState[3]!=m.owner)return fail(Failure::Target);
  if(c.sidebarBytes.size()!=SourceBytes||r.sidebarBefore!=c.sidebarBytes||r.sidebarAfter!=c.sidebarBytes||r.mainPlaneBefore.size()!=PlaneBytes||r.mainPlaneAfter.size()!=PlaneBytes)return fail(Failure::Source);
  for(unsigned i=0;i<4;++i)if(nfs_small_logo_live::le32(c.sidebarBytes.data()+i*4)!=s.header[i])return fail(Failure::Source);
  // Original 483384 uses signed bits 0..11 and 16..27 of Header[3].
  Geometry g;g.x=nfs_small_logo_live::signed12(s.header[3]);g.y=nfs_small_logo_live::signed12(s.header[3]>>16);
  g.left=std::max(g.x,int(m.w[3]));g.top=std::max(g.y,int(m.w[4]));g.right=std::min(g.x+343,int(m.w[5]));g.bottom=std::min(g.y+480,int(m.w[6]));
  if(!(g==r.resolved)||g.left>=g.right||g.top>=g.bottom||rowCount!=size_t(g.bottom-g.top)||rowCount>480)return fail(Failure::Geometry);
  struct Span{uint32_t at,n;};
  const Span buffers[]={{s.w[8],SourceBytes},{m.w[8],16+PlaneBytes}};
  const Span metadata[]={{s.owner,56},{m.owner,56},{s.w[11],480*4},{s.w[12],343*4},{m.w[11],480*4},{m.w[12],640*4}};
  auto overlaps=[](Span a,Span b){return uint64_t(a.at)<uint64_t(b.at)+b.n&&uint64_t(b.at)<uint64_t(a.at)+a.n;};
  for(auto a:buffers){if(uint64_t(a.at)+a.n>0x100000000ULL)return fail(Failure::Alias);for(auto b:metadata)if(!b.at||uint64_t(b.at)+b.n>0x100000000ULL||overlaps(a,b))return fail(Failure::Alias);}
  if(overlaps(buffers[0],buffers[1])||overlaps(metadata[0],metadata[1]))return fail(Failure::Alias);
  for(unsigned i=0;i<2;++i)for(unsigned j=2;j<6;++j)if(overlaps(metadata[i],metadata[j]))return fail(Failure::Alias);
  if(!(r.packetAccepted&&r.packetAccepted<r.copyBegin&&r.copyBegin<r.resolveBegin&&r.resolveBegin<r.resolveEnd&&r.resolveEnd<r.copyEnd))return fail(Failure::Sequence);
  auto replay=r.mainPlaneBefore;uint64_t previous=r.resolveBegin;
  const uint32_t count=uint32_t(g.right-g.left),bytes=count*2;
  for(size_t i=0;i<rowCount;++i){
   if(keyed){const auto&v=r.keyedRows[i];
    if(!(previous<v.rowBegin&&v.rowBegin<v.transformBegin&&v.transformBegin<v.transformEnd&&v.transformEnd<v.forwardBegin&&v.forwardBegin<v.forwardEnd&&v.forwardEnd<v.rowEnd&&v.rowEnd<r.resolveEnd))return fail(Failure::Sequence);previous=v.rowEnd;
    const uint32_t dy=uint32_t(g.top)+uint32_t(i),sy=uint32_t(int(dy)-g.y),sx=uint32_t(g.left-g.x);
    const uint32_t so=16+(sy*343+sx)*2,po=(dy*640+uint32_t(g.left))*2;
    const uint64_t sa=uint64_t(s.w[8])+so,da=uint64_t(m.w[8])+16+po;
    if(sa+bytes>0xffffffffULL||da+bytes>0xffffffffULL)return fail(Failure::Alias);
    if(v.wrapperEsp<0x1408u||v.temporaryAddress!=v.wrapperEsp-0x1408u||v.allocationBase!=v.temporaryAddress||v.allocationBytes!=0x1400u||uint64_t(v.allocationBase)+v.allocationBytes>0x100000000ULL)return fail(Failure::Alias);
    const Span temporary{v.allocationBase,v.allocationBytes};
    for(auto a:buffers)if(overlaps(temporary,a))return fail(Failure::Alias);
    for(auto a:metadata)if(overlaps(temporary,a))return fail(Failure::Alias);
    if(v.keyBefore!=r.keyBefore||v.keyAfter!=r.keyAfter||v.entry!=std::array<uint32_t,4>{uint32_t(sa),uint32_t(da),count,v.entry[3]}||v.transformArgs!=std::array<uint32_t,4>{uint32_t(sa),v.temporaryAddress,count,count}||v.forwardArgs!=std::array<uint32_t,4>{v.temporaryAddress,uint32_t(da),count,count})return fail(Failure::Row);
    if(v.sourceBefore.size()!=bytes||v.sourceAfter!=v.sourceBefore||v.temporaryAfterForward!=v.temporaryBeforeForward||v.destinationBefore.size()!=bytes||v.destinationAfter.size()!=bytes)return fail(Failure::Row);
    namespace transform=nfs_controls_lazy_parent_keyed_transform;
    transform::RowView view;view.wrapper=v.wrapper;view.transform=v.transform;view.forward=v.forward;view.key=v.keyBefore;view.pixels=count;
    view.source565={v.sourceBefore.data(),v.sourceBefore.size()};view.temporaryArgb={v.temporaryBeforeForward.data(),v.temporaryBeforeForward.size()};view.destinationBefore565={v.destinationBefore.data(),v.destinationBefore.size()};view.destinationAfter565={v.destinationAfter.data(),v.destinationAfter.size()};
    if(transform::verify(view)!=transform::Failure::None)return fail(Failure::Pixels);
    for(uint32_t k=0;k<bytes;++k){if(v.sourceBefore[k]!=c.sidebarBytes[so+k]||v.destinationBefore[k]!=replay[po+k])return fail(Failure::Pixels);replay[po+k]=v.destinationAfter[k];}
    continue;
   }
   const auto&v=r.rows[i];
   if(!(previous<v.dispatchBegin&&v.dispatchBegin<v.copyBegin&&v.copyBegin<v.copyEnd&&v.copyEnd<v.dispatchEnd&&v.dispatchEnd<r.resolveEnd))return fail(Failure::Sequence);previous=v.dispatchEnd;
   const uint32_t dy=uint32_t(g.top)+uint32_t(i),sy=uint32_t(int(dy)-g.y),sx=uint32_t(g.left-g.x);
   const uint32_t so=16+(sy*343+sx)*2,po=(dy*640+uint32_t(g.left))*2;
   const uint64_t sa=uint64_t(s.w[8])+so,da=uint64_t(m.w[8])+16+po;
   if(sa+bytes>0xffffffffULL||da+bytes>0xffffffffULL)return fail(Failure::Alias);
   const uint32_t ecx=v.copyArgs[3];
   if(v.dispatch!=DispatchId||v.callback!=CopyId||v.dispatchArgs!=std::array<uint32_t,4>{uint32_t(sa),uint32_t(da),count,ecx}||v.copyArgs!=std::array<uint32_t,4>{uint32_t(sa),uint32_t(da),bytes,ecx}||v.copyReturn!=std::array<uint32_t,4>{uint32_t(da+bytes),uint32_t(sa+bytes),0,ecx}||v.dispatchReturn!=v.copyReturn)return fail(Failure::Row);
   if(v.sourceBefore.size()!=bytes||v.sourceAfter!=v.sourceBefore||v.destinationBefore.size()!=bytes||v.destinationAfter.size()!=bytes)return fail(Failure::Row);
   for(uint32_t k=0;k<bytes;++k){const auto expected=c.sidebarBytes[so+k];if(v.sourceBefore[k]!=expected||v.destinationBefore[k]!=replay[po+k]||v.destinationAfter[k]!=expected)return fail(Failure::Pixels);replay[po+k]=expected;}
  }
  // Compares every byte, including unchanged copied bytes and everything outside
  // the footprint. This does not certify invisible idempotent guest writes.
  if(replay!=r.mainPlaneAfter)return fail(Failure::Pixels);
  return Result{Failure::None,std::unique_ptr<const VerifiedUpdate>(new VerifiedUpdate(r))};
 }
};
}

// SPDX-License-Identifier: MIT
#pragma once
#include "FeedbackModalBackdropRecords.h"
namespace nfs_feedback_backdrop {
namespace detail {
inline uint32_t le32(const uint8_t*p)noexcept{return nfs_small_logo_live::le32(p);}
inline uint16_t le16(const uint8_t*p)noexcept{return nfs_small_logo_live::le16(p);}
inline bool span(uint32_t p,size_t n)noexcept{return p&&n&&uint64_t(p)+n<=0x100000000ULL;}
inline bool overlap(uint32_t a,size_t an,uint32_t b,size_t bn)noexcept{return uint64_t(a)<uint64_t(b)+bn&&uint64_t(b)<uint64_t(a)+an;}
inline uint32_t converted(uint32_t c)noexcept{return(c&0xff000000u)|((c&0xf80000u)>>8)|((c&0xfc00u)>>5)|((c>>3)&31);}
inline uint32_t expanded(uint16_t c)noexcept{return 0xff000000u|((uint32_t(c)>>11)<<19)|(((uint32_t(c)>>5)&63)<<10)|((uint32_t(c)&31)<<3);}
inline uint32_t modulate(uint32_t c,unsigned opacity)noexcept{if(opacity>=255)return c;uint32_t v=0;for(unsigned k=0;k<32;k+=8)v|=((((c>>k)&255)*opacity)>>8)<<k;return v;}
inline uint16_t rgbaBlend(uint16_t old,uint32_t c)noexcept{unsigned a=c>>24;if(a<16)return old;if(a<252){unsigned inv=255-a;c+=((((old>>11)*inv)>>5)<<16)+(((((old>>5)&63)*inv)>>6)<<8)+(((old&31)*inv)>>5);}return uint16_t((((c>>19)&31)<<11)|(((c>>10)&63)<<5)|((c>>3)&31));}
inline uint16_t indexedBlend(uint16_t old,uint32_t c)noexcept{return nfs_small_logo_live::pixel(old,c,true);}
inline bool clip(const Target&t,int l,int top,int r,int b)noexcept{return t.w[3]==uint32_t(l)&&t.w[4]==uint32_t(top)&&t.w[5]==uint32_t(r)&&t.w[6]==uint32_t(b);}
}
// Copies and authenticates bytes once. The expected catalogue must itself come
// from an independent source-bound provider; SHA self-reporting is not authority.
class SourceCatalog final {
 std::array<SourceIdentity,2>identities_{};std::array<std::vector<uint8_t>,2>bytes_{};SourceCatalog()=default;
public:
 static std::shared_ptr<const SourceCatalog>create(const std::array<SourceIdentity,2>&expected,const std::array<std::vector<uint8_t>,2>&copied)noexcept{
  try{auto out=std::shared_ptr<SourceCatalog>(new SourceCatalog);
   const uint32_t widths[2]={344,117},heights[2]={47,61},spans[2]={17224,8196},tags[2]={0x72616264u,0x676f6c64u};
   for(unsigned k=0;k<2;++k){const auto&i=expected[k];const auto&b=copied[k];const uint64_t end=16+uint64_t(i.width)*i.height;const uint64_t palette=(end+3)&~uint64_t(3);
    if(!i.providerKey||!nonzero(i.sha)||!nonzero(i.archiveSha)||i.tag!=tags[k]||i.width!=widths[k]||i.height!=heights[k]||i.span!=spans[k]||b.size()!=i.span||i.paletteCount!=256||i.paletteOffset!=palette||palette+1040!=i.span||!i.archiveOffset||uint64_t(i.archiveOffset)+i.span>0x100000000ULL||i.hotX!=(k?-112:0)||i.hotY!=(k?-103:0))return {};
    if(k&&(i.providerKey==expected[0].providerKey||i.sha==expected[0].sha||i.archiveSha!=expected[0].archiveSha||detail::overlap(i.archiveOffset,i.span,expected[0].archiveOffset,expected[0].span)))return {};
    if(i.header[0]!=(0x7bu|(i.paletteOffset<<8))||i.header[1]!=(i.width|(i.height<<16))||int16_t(uint16_t(i.header[2]))!=i.hotX||int16_t(uint16_t(i.header[2]>>16))!=i.hotY||i.paletteHeader[0]!=0x2a||i.paletteHeader[1]!=0x10100||i.paletteHeader[2]!=0x100)return {};
    for(unsigned n=0;n<4;++n)if(detail::le32(b.data()+4*n)!=i.header[n]||detail::le32(b.data()+i.paletteOffset+4*n)!=i.paletteHeader[n])return {};
    // Every possible index is covered by the full256 palette. Padding bytes are
    // hashed, not silently discarded; no palette or original pixels are embedded.
    out->bytes_[k]=b;if(!nfs_small_logo_live::hashMatches(out->bytes_[k].data(),i.span,i.sha))return {};out->identities_[k]=i;
   }return out;
  }catch(...){return {};}
 }
 const SourceIdentity*identity(unsigned role)const noexcept{return role<2?&identities_[role]:nullptr;}
 const std::vector<uint8_t>*bytes(unsigned role)const noexcept{return role<2?&bytes_[role]:nullptr;}
};
class ReplayVerifier;
class VerifiedBackdrop final {
 Binding binding_{};Plane before_,pretext_;std::shared_ptr<const SourceCatalog>catalog_;size_t rows_=0,stores_=0;VerifiedBackdrop()=default;friend class ReplayVerifier;
public:
 const Binding&binding()const noexcept{return binding_;}const Plane&before()const noexcept{return before_;}const Plane&pretext()const noexcept{return pretext_;}
 const std::shared_ptr<const SourceCatalog>&catalog()const noexcept{return catalog_;}size_t rows()const noexcept{return rows_;}size_t stores()const noexcept{return stores_;}
 bool matches(const Binding&b,const uint16_t*p,size_t count)const noexcept{return binding_==b&&p&&count==Pixels&&pretext_.size()==Pixels&&!std::memcmp(p,pretext_.data(),Pixels*sizeof(uint16_t));}
};
class ReplayVerifier {
 static bool protectedSpan(uint32_t p,size_t n,const Binding&b,const SourceCatalog&c,uint32_t scratch=0,size_t scratchBytes=0)noexcept{
  if(!detail::span(p,n)||detail::overlap(p,n,b.surface.geometry[8],16+Pixels*2)||(scratch&&detail::overlap(p,n,scratch,scratchBytes)))return false;
  for(unsigned k=0;k<2;++k)if(detail::overlap(p,n,b.sources[k],c.identity(k)->span))return false;return true;
 }
 static bool rowBase(const Row&r,uint32_t seq,uint32_t src,uint32_t dst,unsigned n,uint32_t callback,uint32_t format)noexcept{
  return r.sequence==seq&&r.callback==callback&&r.dispatchBefore==callback&&r.dispatchAfter==callback&&r.callerStackBefore&&!(r.callerStackBefore&3)&&r.callerStackBefore==r.callerStackAfter&&uint64_t(r.callerStackBefore)+0x38==r.dispatchSlot&&uint64_t(r.callerStackBefore)+0x3c<=0x100000000ULL&&r.dispatchSlot&&!(r.dispatchSlot&3)&&r.format==format&&r.sourceAddress==src&&r.destinationAddress==dst&&r.count==n&&n&&n<=MaxRowPixels&&r.before.size()==n&&r.after.size()==n&&r.stores.size()<=n;
 }
 static bool dispatchValid(const Row&r,const Binding&b,const SourceCatalog&c,const Scratch&s,const SelectorContract&selectors)noexcept{
  if(!protectedSpan(r.dispatchSlot,4,b,c,s.source,s.postFill.size())||detail::overlap(r.dispatchSlot,4,s.descriptor,56))return false;
  const uint32_t addresses[9]={selectors.convertSlot,selectors.blendSlot,selectors.opacitySlot,selectors.lutAddress,b.targetOwner,b.surface.geometry[11],b.surface.geometry[12],s.words[11],s.words[12]};
  // Original descriptor slot11 owns Y rows; slot12 owns X columns.
  const unsigned sizes[9]={4,4,4,256,56,480*4,640*4,60*4,186*4};
  for(unsigned k=0;k<9;++k)if(detail::overlap(r.dispatchSlot,4,addresses[k],sizes[k]))return false;
  return !r.paletteAddress||!detail::overlap(r.dispatchSlot,4,r.paletteAddress,1024);
 }
 static bool stores(const Row&r,Plane&plane,unsigned x,unsigned y,const std::vector<uint16_t>&expected,const std::vector<uint8_t>&written,size_t&total)noexcept{
  size_t used=0;for(unsigned xx=0;xx<r.count;++xx){const size_t at=size_t(y)*640+x+xx;if(at>=plane.size()||r.before[xx]!=plane[at]||r.after[xx]!=expected[xx])return false;
   if(written[xx]){if(used>=r.stores.size()||total>=MaxStores)return false;const auto&s=r.stores[used++];if(s.address!=r.destinationAddress+xx*2||s.before!=plane[at]||s.computed!=expected[xx]||s.after!=expected[xx])return false;++total;}plane[at]=expected[xx];
  }return used==r.stores.size();
 }
public:
 // No output parameter is mutated. A rejected record gives no retained result;
 // the caller must preserve the original complete frame as its only fallback.
 static std::shared_ptr<const VerifiedBackdrop>verify(const Record&r,const Binding&current,const std::shared_ptr<const SourceCatalog>&catalog,const SelectorContract&requiredSelectors)noexcept{
  if(!current.valid()||!(r.binding==current)||!catalog||!requiredSelectors.valid()||r.complete!=1||r.error||r.before.size()!=Pixels||r.initialMainRestoreComplete!=1||r.initialRestored.size()!=Pixels||r.restored.size()!=Pixels||r.pretext.size()!=Pixels||r.steps.size()!=15||r.titleWidth!=96||r.messageWidth!=108)return {};
  static const Step order[15]={Step::initialMain,Step::allocateScratch,Step::selectScratch,Step::fillColor,Step::fillScratch,Step::restoreMainAfterScratch,Step::bodyBlit,Step::retireScratch,Step::bannerClip,Step::bannerResolve,Step::bannerDraw,Step::restoreClip,Step::emblemResolve,Step::emblemHotspot,Step::emblemDraw};
  for(unsigned k=0;k<15;++k)if(r.steps[k]!=order[k])return {};
  if(!targetValid(r.initial,current)||!targetValid(r.restoredMainAfterScratch,current)||!targetValid(r.pretextTarget,current)||!nfs_logo_capture::same(r.initial,r.restoredMainAfterScratch)||!nfs_logo_capture::same(r.initial,r.pretextTarget)||r.initialRestored!=r.before||r.restored!=r.before)return {};
  for(unsigned k=0;k<2;++k){const auto&i=*catalog->identity(k);if(current.providerKeys[k]!=i.providerKey||uint64_t(current.archive)+i.archiveOffset>0xffffffffULL||current.sources[k]!=current.archive+i.archiveOffset||!detail::span(current.sources[k],i.span)||detail::overlap(current.sources[k],i.span,current.surface.geometry[8],16+Pixels*2))return {};}
  const auto&s=r.body;const unsigned width=186,height=60;const size_t scratchBytes=16+size_t(width)*height*2;
  if(s.allocated!=1||s.selected!=1||s.fillComplete!=1||s.retired!=1||!s.descriptor||s.words!=s.retireWords||s.fillColor!=0xff080808||s.fillWord!=0x0841||s.fillCallback!=0xf0001aa8||s.fillCount!=width*height||s.opacity!=180||s.rows.size()!=height||s.postFill.size()!=scratchBytes||s.afterBlit!=s.postFill||!protectedSpan(s.source,scratchBytes,current,*catalog)||!protectedSpan(s.descriptor,56,current,*catalog,s.source,scratchBytes)||s.fillDestination!=s.source+16||s.x.size()!=width||s.y.size()!=height)return {};
  const auto&w=s.words;if(w[0]!=current.surface.geometry[0]||w[1]!=width||w[2]!=height||w[3]||w[4]||w[5]!=width||w[6]!=height||w[7]!=0x7810||w[8]!=s.source||w[9]||w[10]!=width*2||!w[11]||!w[12]||w[13]||s.header!=std::array<uint32_t,4>{{0x78,width|(height<<16),0,0}})return {};
  for(unsigned k=0;k<4;++k)if(detail::le32(s.postFill.data()+k*4)!=s.header[k])return {};
  for(unsigned k=0;k<width;++k)if(s.x[k]!=2*k)return {};for(unsigned k=0;k<height;++k)if(s.y[k]!=16+k*width*2)return {};
  for(size_t k=16;k<scratchBytes;k+=2)if(detail::le16(s.postFill.data()+k)!=s.fillWord)return {};
  const uint32_t selectorAddresses[4]={requiredSelectors.convertSlot,requiredSelectors.blendSlot,requiredSelectors.opacitySlot,requiredSelectors.lutAddress};
  const unsigned selectorSizes[4]={4,4,4,256};
  for(unsigned k=0;k<4;++k)if(!protectedSpan(selectorAddresses[k],selectorSizes[k],current,*catalog,s.source,scratchBytes)||detail::overlap(selectorAddresses[k],selectorSizes[k],s.descriptor,56))return {};
  if(!protectedSpan(w[11],height*4,current,*catalog,s.source,scratchBytes)||!protectedSpan(w[12],width*4,current,*catalog,s.source,scratchBytes)||detail::overlap(w[11],height*4,w[12],width*4)||detail::overlap(w[11],height*4,s.descriptor,56)||detail::overlap(w[12],width*4,s.descriptor,56))return {};
  try{Plane replay=r.before;size_t total=0;uint32_t sequence=0;
   nfs_feedback_modal_geometry::Geometry g;if(!nfs_feedback_modal_geometry::make(640,480,r.titleWidth,r.messageWidth,g)||g.left!=227||g.top!=210||g.width!=int(width)||g.height!=int(height))return {};
   for(unsigned yy=0;yy<height;++yy){const auto&row=s.rows[yy];const uint32_t source=s.source+16+yy*width*2,destination=current.surface.geometry[8]+16+((g.top+yy)*640+g.left)*2;
    if(!rowBase(row,sequence++,source,destination,width,0xf000211c,0x78)||!dispatchValid(row,current,*catalog,s,requiredSelectors)||row.dispatchSlot!=s.rows.front().dispatchSlot||row.source.size()!=width*2||row.sourceAfter!=row.source||std::memcmp(row.source.data(),s.postFill.data()+16+yy*width*2,width*2)||row.paletteAddress||row.selectors.complete!=1||!(row.selectors.before==requiredSelectors)||!(row.selectors.after==requiredSelectors))return {};
    std::vector<uint16_t>expected(width);std::vector<uint8_t>written(width,1);for(unsigned xx=0;xx<width;++xx)expected[xx]=detail::rgbaBlend(replay[(g.top+yy)*640+g.left+xx],detail::modulate(detail::expanded(detail::le16(row.source.data()+xx*2)),s.opacity));
    if(!stores(row,replay,g.left,g.top+yy,expected,written,total))return {};
   }
   for(unsigned k=0;k<2;++k){const auto&d=r.draws[k];const auto&i=*catalog->identity(k);const auto&source=*catalog->bytes(k);const int x=k?175:98,y=k?171:179,cl=k?0:247;
    if(unsigned(d.role)!=k||d.sequence!=k||d.complete!=1||d.error||d.archive!=current.archive||d.sourceAddress!=current.sources[k]||d.resolvedAddress!=d.sourceAddress||d.namedResolves!=1||d.mode||d.sourceBefore!=source||d.sourceAfter!=source||d.hotspot!=(k==1)||d.inputX!=(k?63:98)||d.inputY!=(k?68:179)||d.x!=x||d.y!=y||!targetValid(d.before,current,false)||!nfs_logo_capture::same(d.before,d.after)||!detail::clip(d.before,cl,0,640,480))return {};
    if(std::memcmp(d.name.data(),k?"dlog":"dbar",4)||(k&&(int64_t(d.inputX)-i.hotX!=x||int64_t(d.inputY)-i.hotY!=y)))return {};
    const int left=std::max(cl,x),top=std::max(0,y),right=std::min(640,x+int(i.width)),bottom=std::min(480,y+int(i.height));if(left>=right||top>=bottom||d.rows.size()!=size_t(bottom-top)||d.uv!=std::array<uint32_t,4>{{uint32_t(left-x),uint32_t(top-y),uint32_t(right-x),uint32_t(bottom-y)}})return {};
    const unsigned count=unsigned(right-left);uint32_t paletteAddress=0;
    for(int yy=top;yy<bottom;++yy){const auto&row=d.rows[unsigned(yy-top)];const size_t offset=16+size_t(yy-y)*i.width+unsigned(left-x);const uint32_t src=d.sourceAddress+uint32_t(offset),dst=current.surface.geometry[8]+16+(unsigned(yy)*640+unsigned(left))*2;
     if(!rowBase(row,sequence++,src,dst,count,0xf0001850,0x7b)||!dispatchValid(row,current,*catalog,s,requiredSelectors)||row.dispatchSlot!=d.rows.front().dispatchSlot||row.source.size()!=count||row.sourceAfter!=row.source||std::memcmp(row.source.data(),source.data()+offset,count)||row.paletteBefore!=row.paletteAfter||!protectedSpan(row.paletteAddress,1024,current,*catalog,s.source,scratchBytes)||detail::overlap(row.paletteAddress,1024,s.descriptor,56))return {};
     if(paletteAddress&&row.paletteAddress!=paletteAddress)return {};paletteAddress=row.paletteAddress;
     for(unsigned j=0;j<256;++j)if(row.paletteBefore[j]!=detail::converted(detail::le32(source.data()+i.paletteOffset+16+4*j)))return {};
     std::vector<uint16_t>expected(count);std::vector<uint8_t>written(count);for(unsigned xx=0;xx<count;++xx){const uint32_t c=row.paletteBefore[row.source[xx]];expected[xx]=detail::indexedBlend(replay[unsigned(yy)*640+unsigned(left)+xx],c);written[xx]=uint8_t(c>=0x10000000);}
     if(!stores(row,replay,unsigned(left),unsigned(yy),expected,written,total))return {};
    }
   }
   if(replay!=r.pretext)return {};auto out=std::shared_ptr<VerifiedBackdrop>(new VerifiedBackdrop);out->binding_=current;out->before_=r.before;out->pretext_=std::move(replay);out->catalog_=catalog;out->rows_=sequence;out->stores_=total;return out;
  }catch(...){return {};}
 }
};
}

// SPDX-License-Identifier: MIT
#pragma once
#include "AudioDialogRecords.h"
#include "AudioNormalizedLayout.h"
namespace nfs_audio_frame {
namespace detail {
inline uint32_t modulate(uint32_t c,uint32_t opacity)noexcept{if(opacity>=255)return c;uint32_t v=0;for(unsigned k=0;k<32;k+=8)v|=((((c>>k)&255)*opacity)>>8)<<k;return v;}
// Exact original482BD4 source-reviewed integer arithmetic, not GLES blending.
inline uint16_t blend(uint16_t old,uint32_t c)noexcept{uint32_t alpha=c>>24;if(alpha<16)return old;uint32_t v=c;if(alpha<252){uint32_t inverse=255-alpha;v+=((((old>>11)*inverse)>>5)<<16)+((((old>>5)&63)*inverse>>6)<<8)+(((old&31)*inverse)>>5);}return uint16_t((((v>>19)&31)<<11)|(((v>>10)&63)<<5)|((v>>3)&31));}
inline int32_t signed16(uint32_t v)noexcept{return int16_t(uint16_t(v));}
}
class SourceCatalog final {
 std::array<SourceIdentity,6>identities_;std::array<std::vector<uint8_t>,6>bytes_;SourceCatalog()=default;
public:
 // Expected identities must come from an independently authenticated provider;
 // self-reported hashes/numeric roles alone do not establish source provenance.
 static std::shared_ptr<const SourceCatalog>create(const std::array<SourceIdentity,6>&expected,const std::array<std::vector<uint8_t>,6>&copied)noexcept{
  try{auto p=std::shared_ptr<SourceCatalog>(new SourceCatalog);for(unsigned k=0;k<6;++k){const auto&i=expected[k];const auto&b=copied[k];if(!nonzero(i.sha)||i.span<32||i.span>MaxSource||b.size()!=i.span||i.width!=Width[k]||i.height!=Height[k]||i.hotX!=HotX[k]||i.hotY!=HotY[k]||!i.paletteCount||i.paletteCount>256||uint64_t(16)+i.width*i.height>i.paletteOffset||uint64_t(i.paletteOffset)+16+4*i.paletteCount>i.span)return {};p->bytes_[k]=b;const auto&owned=p->bytes_[k];if(!nfs_small_logo_live::hashMatches(owned.data(),i.span,i.sha))return {};for(unsigned j=0;j<4;++j)if(nfs_small_logo_live::le32(owned.data()+j*4)!=i.header[j])return {};if((i.header[0]&255)!=0x7b||(i.header[0]>>8)!=i.paletteOffset||i.header[1]!=(i.width|(i.height<<16))||detail::signed16(i.header[2])!=i.hotX||detail::signed16(i.header[2]>>16)!=i.hotY||(nfs_small_logo_live::le32(owned.data()+i.paletteOffset)&255)!=0x2a||(nfs_small_logo_live::le32(owned.data()+i.paletteOffset+4)&65535)!=i.paletteCount)return {};for(size_t at=16;at<16+i.width*i.height;++at)if(owned[at]>=i.paletteCount)return {};p->identities_[k]=i;}return p;}catch(...){return {};}
 }
 const SourceIdentity&identity(unsigned role)const noexcept{return identities_[role];}const std::vector<uint8_t>&bytes(unsigned role)const noexcept{return bytes_[role];}
};
class BarReplayVerifier;
class VerifiedBars final {
 Binding binding_;std::vector<uint16_t>pre_,post_;OwnedBarLayout layout_;std::shared_ptr<const ArtBinding>art_;std::shared_ptr<const SourceCatalog>catalog_;VerifiedBars()=default;friend class BarReplayVerifier;
public:
 const Binding&binding()const noexcept{return binding_;}const std::vector<uint16_t>&pre()const noexcept{return pre_;}const std::vector<uint16_t>&post()const noexcept{return post_;}const OwnedBarLayout&layout()const noexcept{return layout_;}const std::shared_ptr<const ArtBinding>&art()const noexcept{return art_;}
};
class BarReplayVerifier {
public:
 static std::shared_ptr<const VerifiedBars>verify(const BarRecord&r,const Binding&current,const std::shared_ptr<const SourceCatalog>&catalog,const std::shared_ptr<const ArtBinding>&art)noexcept{
  if(!current.valid()||!(r.binding==current)||r.complete!=1||r.error||!catalog||!art||r.pre.size()!=Pixels||r.post.size()!=Pixels||r.draws.size()<12||r.draws.size()>13)return {};
  try{BarState state{};state.values=r.draws[0].values;state.selected=r.draws[0].selected;state.held=r.draws[0].held;state.count=r.draws.size();auto replay=r.pre;
   static constexpr char names[6][5]={"audi","abar","laud","raud","laul","raul"};
   for(unsigned k=0;k<r.draws.size();++k){const auto&d=r.draws[k];if(d.asset>=6||d.sequence!=k||d.complete!=1||d.error||!targetValid(d.before,current,false)||!nfs_logo_capture::same(d.before,d.after)||d.values!=state.values||d.selected!=state.selected||d.held!=state.held||std::memcmp(d.name.data(),names[d.asset],4)||!d.namedArgs[0]||!d.args[0]||d.namedArgs[1]!=d.args[1]||d.namedArgs[2]!=d.args[2]||d.namedArgs[3]!=d.args[3])return {};
    const auto&meta=catalog->identity(d.asset);const auto&source=catalog->bytes(d.asset);if(d.sourceBefore!=source||d.sourceAfter!=source||uint64_t(d.args[0])+source.size()>0x100000000ULL||nfs_small_logo_live::overlap(d.args[0],source.size(),current.surface.geometry[8],614416))return {};uint32_t opacity=d.args[1]&511;if((d.args[1]&~511u)!=0x2000||opacity>256)return {};
    auto&layout=state.draws[k];layout.asset=d.asset;layout.sequence=k;layout.row=k<3?k:k<6?k-3:k<12?(k-6)/2:uint32_t(state.selected);layout.mode=d.args[1];layout.x=int32_t(d.args[2]);layout.y=int32_t(d.args[3]);layout.clip={int32_t(d.before.w[3]),int32_t(d.before.w[4]),int32_t(d.before.w[5]),int32_t(d.before.w[6])};
    const int64_t x=int64_t(layout.x)-meta.hotX,y=int64_t(layout.y)-meta.hotY;const int64_t l=std::max<int64_t>(layout.clip.left,x),t=std::max<int64_t>(layout.clip.top,y),right=std::min<int64_t>(layout.clip.right,x+meta.width),bottom=std::min<int64_t>(layout.clip.bottom,y+meta.height);if(l<0||t<0||right>640||bottom>480||l>=right||t>=bottom||d.rows.size()!=size_t(bottom-t))return {};const unsigned n=unsigned(right-l);
    for(unsigned yy=unsigned(t);yy<unsigned(bottom);++yy){const auto&row=d.rows[yy-unsigned(t)];unsigned sx=unsigned(l-x),sy=unsigned(int64_t(yy)-y);uint32_t src=d.args[0]+16+sy*meta.width+sx,dst=current.surface.geometry[8]+16+(yy*640+unsigned(l))*2;if(row.callback!=0xf0001818||row.callbackGlobal!=0xf0001818||row.callbackFormat!=0x7b||row.paletteGuest!=d.rows.front().paletteGuest||!row.paletteGuest||uint64_t(row.paletteGuest)+1024>0x100000000ULL||nfs_small_logo_live::overlap(row.paletteGuest,1024,current.surface.geometry[8],614416)||row.paletteBefore!=row.paletteAfter||row.args[0]!=src||row.args[1]!=dst||row.args[2]!=n||row.source.size()!=n||row.before.size()!=n||row.after.size()!=n||std::memcmp(row.source.data(),source.data()+16+sy*meta.width+sx,n))return {};
     for(unsigned index=0;index<meta.paletteCount;++index)if(row.paletteBefore[index]!=detail::modulate(nfs_small_logo_live::le32(source.data()+meta.paletteOffset+16+4*index),opacity))return {};
     for(unsigned xx=0;xx<n;++xx){size_t at=yy*640+unsigned(l)+xx;unsigned index=row.source[xx];if(index>=meta.paletteCount)return {};uint16_t expected=detail::blend(replay[at],row.paletteBefore[index]);if(row.before[xx]!=replay[at]||row.after[xx]!=expected)return {};replay[at]=expected;}
    }
   }
   OwnedBarLayout layout;if(!normalize(state,layout)||replay!=r.post)return {};auto p=std::shared_ptr<VerifiedBars>(new VerifiedBars);p->binding_=current;p->pre_=r.pre;p->post_=std::move(replay);p->layout_=layout;p->catalog_=catalog;p->art_=art;return p;
  }catch(...){return {};}
 }
};
}

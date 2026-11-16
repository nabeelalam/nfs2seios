// SPDX-License-Identifier: MIT
#pragma once
#include "LocationPhotoArtProvider.h"
#include "LocationPhotoSourceCatalog.h"
#include "LocationCompletedFlag.h"
#include "TextureDigest.h"
#include <memory>
#include <vector>
#include <cstring>
namespace nfs_location_preview_art {
// Only an immutable display derivative. Caller must already have accepted the
// whole current Location native fill/source/row replay; this grants no ownership.
class Prepared final {
 NfsLocationPhotoArtView art_{};std::shared_ptr<const nfs_location_photo_source::Catalog>catalog_;
 std::vector<unsigned char>rgba_;std::array<unsigned char,32>sha_{};
 Prepared()=default;
public:
 ~Prepared(){if(art_.owner&&art_.release)art_.release(art_.owner);}
 bool current()const noexcept{return art_.generation&&art_.generation==nfs_location_photo_art_generation()&&catalog_&&catalog_==nfs_location_photo_source::acquire();}
 uint64_t generation()const noexcept{return current()?art_.generation:0;}
 const std::vector<unsigned char>&pixels()const noexcept{return rgba_;}
 const std::array<unsigned char,32>&sha()const noexcept{return sha_;}
 template<class I,class P>static std::shared_ptr<const Prepared>create(const I&i,const P&p)noexcept{try{
  if(i.form!=0x404abdu||!i.complete||i.error||i.selection>=8||!i.decodedAuthenticated||!i.sourceCatalog||i.sourceCatalog!=nfs_location_photo_source::acquire()||(i.draws.size()!=1&&i.draws.size()!=3))return {};
  const auto&d=i.draws[0];const auto*source=i.sourceCatalog->entry(i.selection);if(!source||d.args[1]!=0x3000||d.args[2]!=211||d.args[3]!=316||uint64_t(i.archive)+source->archiveOffset!=d.args[0]||!i.sourceCatalog->matches(i.selection,d.name,source->width,source->height,d.sourceBefore.data(),d.sourceBefore.size())||d.sourceAfter!=d.sourceBefore)return {};
  const auto&r=p.bounds();if(!r.width||!r.height||uint64_t(r.left)+r.width>640||uint64_t(r.top)+r.height>480||p.coverage().size()!=size_t(r.width)*r.height)return {};
  std::shared_ptr<Prepared>out(new Prepared);out->catalog_=i.sourceCatalog;if(!nfs_location_photo_art_acquire(i.selection,&out->art_))return {};const auto&im=out->art_.images[0];
  if(!out->current()||out->art_.schema!=1||out->art_.count!=1||im.sourceIndex!=i.selection||im.width!=source->width*4||im.height!=source->height*4||im.bytes!=size_t(im.width)*im.height*4||!im.pixels)return {};
  const unsigned W=r.width*4,H=r.height*4;out->rgba_.assign(size_t(W)*H*4,0);
  // Real native fill is the independent280x128 black prefix. No final-screen
  // pixels or prior authored surface is copied into this derivative.
  for(unsigned y=316;y<444;++y)for(unsigned x=211;x<491;++x){if(x<r.left||y<r.top||x>=r.left+r.width||y>=r.top+r.height)continue;for(unsigned yy=0;yy<4;++yy)for(unsigned xx=0;xx<4;++xx)out->rgba_[4*((size_t(y-r.top)*4+yy)*W+(x-r.left)*4+xx)+3]=255;}
  // Source coordinates remain307x146/147 even when native visible bounds are
  //305x144. Clip by destination bounds; NEVER stretch the master to that crop.
  const auto*indices=d.sourceBefore.data()+16;
  for(unsigned sy=0;sy<source->height;++sy)for(unsigned sx=0;sx<source->width;++sx){const unsigned index=indices[size_t(sy)*source->width+sx];if(index==255||index>=256||d.paletteBefore[index]<0x10000000u)continue;const unsigned x=211+sx,y=316+sy;if(x<r.left||y<r.top||x>=r.left+r.width||y>=r.top+r.height)continue;
   for(unsigned yy=0;yy<4;++yy)for(unsigned xx=0;xx<4;++xx){const auto*at=im.pixels+4*((size_t(sy)*4+yy)*im.width+sx*4+xx);if(at[3]!=255)return {};std::memcpy(out->rgba_.data()+4*((size_t(y-r.top)*4+yy)*W+(x-r.left)*4+xx),at,4);}}
  if(i.draws.size()==3){auto over=nfs_location_completed_flag::prepare(i,p,&out->rgba_);if(over.empty())return {};out->rgba_=std::move(over);}
  // Preserve exactly the independently replayed native union footprint.
  for(unsigned y=0;y<H;++y)for(unsigned x=0;x<W;++x)if(!p.coverage()[size_t(y/4)*r.width+x/4])std::memset(out->rgba_.data()+4*(size_t(y)*W+x),0,4);
  if(!out->current()||!nfs_texture_sha256(out->rgba_.data(),out->rgba_.size(),out->sha_.data()))return {};return out;
 }catch(...){return {};}}
};
}

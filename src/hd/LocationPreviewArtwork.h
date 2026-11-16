// SPDX-License-Identifier: MIT
#pragma once
#include "LocationPhotoArtProvider.h"
#include "LocationPhotoSourceCatalog.h"
#include "LocationCompletedFlag.h"
#include "TextureDigest.h"
#include <algorithm>
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
  // The original union is still required to authenticate the draw. Its
  // silhouette does not mask the independent card and soft shadow.
  const unsigned left=std::max(r.left,211u),right=std::min(r.left+r.width,211u+source->width);
  const unsigned top=std::max(r.top,316u),bottom=std::min(r.top+r.height,316u+source->height);
  if(left>=right||top>=bottom)return {};
  for(unsigned y=top*4;y<bottom*4;++y){
   const auto* row=im.pixels+4*(size_t(y-316*4)*im.width+(left-211)*4);
   auto* destination=out->rgba_.data()+4*(size_t(y-r.top*4)*W+(left-r.left)*4);
   std::memcpy(destination,row,size_t(right-left)*16);
  }
  if(i.draws.size()==3){auto over=nfs_location_completed_flag::prepare(i,p,&out->rgba_);if(over.empty())return {};out->rgba_=std::move(over);}
  if(!out->current()||!nfs_texture_sha256(out->rgba_.data(),out->rgba_.size(),out->sha_.data()))return {};return out;
 }catch(...){return {};}}
};
}

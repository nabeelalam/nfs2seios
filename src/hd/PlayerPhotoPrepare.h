// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerPhotoArtProvider.h"
#include "PlayerPhotoSourceCatalog.h"
#include "TextureDigest.h"
#include <memory>
#include <cstring>
namespace nfs_player_photo {
struct Art {NfsPlayerPhotoArtView view{};~Art(){if(view.owner&&view.release)view.release(view.owner);}static std::shared_ptr<const Art>acquire(unsigned car){auto p=std::make_shared<Art>();if(!nfs_player_photo_art_acquire(car,&p->view)||p->view.schema!=1||p->view.count!=1||!p->view.generation)return {};return p;}bool current()const{return view.generation&&view.generation==nfs_player_photo_art_generation();}};
template<class P>inline bool source(const P&p,unsigned&car){const auto&s=p.selection();if(s.graphEnabled||s.selectedCar>=12||s.cachedCarIndex!=s.selectedCar||p.sources().size()!=1)return false;car=s.selectedCar;const auto&a=p.sources()[0];const auto&b=nfs_player_photo_sources[car];return !std::memcmp(a.name.data(),b.name,4)&&!std::memcmp(a.sha.data(),b.sha,32)&&a.width==b.width&&a.height==b.height;}
// Actual replayed source proof remains authoritative. Authored display is a
// separate, retained derivative; no game pixels or original operations change.
template<class P,class S,class O>inline void decorate(const P&p,const S*swatch,O&o){unsigned car=0;if(!source(p,car))return;auto art=Art::acquire(car);if(!art||!art->current())return;const auto&im=art->view.images[0];if(im.width!=1228||im.height!=584||im.bytes!=1228u*584u*4u)return;const unsigned W=o.width*4,H=o.height*4;if(!W||!H||W>2560||H>1920)return;auto&a=o.authored;a.resize(size_t(W)*H*4);for(unsigned y=0;y<H;++y)for(unsigned x=0;x<W;++x){size_t q=(y/4)*o.width+x/4;unsigned w=o.pixels[q],A=o.coverage[q];auto*d=a.data()+4*(y*W+x);d[0]=A?((w>>11)*255/31):0;d[1]=A?(((w>>5)&63)*255/63):0;d[2]=A?((w&31)*255/31):0;d[3]=A;}
 auto put=[&](int x,int y,const unsigned char*v){if(x<0||y<0||x>=int(W)||y>=int(H))return;std::memcpy(a.data()+4*(y*W+x),v,4);};for(unsigned y=0;y<584;++y)for(unsigned x=0;x<1228;++x)put((211-o.left)*4+int(x),(316-o.top)*4+int(y),im.pixels+4*(y*1228+x));
 // Original live color swatch remains the final independent preview operation.
 if(swatch){const auto&b=swatch->bounds();for(unsigned y=0;y<b.height;++y)for(unsigned x=0;x<b.width;++x){size_t q=y*b.width+x;if(!swatch->coverage()[q])continue;unsigned w=swatch->pixels()[q];unsigned char v[4]={uint8_t((w>>11)*255/31),uint8_t(((w>>5)&63)*255/63),uint8_t((w&31)*255/31),255};for(unsigned j=0;j<4;++j)for(unsigned i=0;i<4;++i)put((int(b.left+x)-o.left)*4+i,(int(b.top+y)-o.top)*4+j,v);}}
 if(!art->current()||!nfs_texture_sha256(a.data(),a.size(),o.authoredSha.data()))a.clear();
}
}

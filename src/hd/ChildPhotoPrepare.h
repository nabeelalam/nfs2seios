// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerPhotoArtProvider.h"
#include "ClassPictureArtProvider.h"
#include "LocationPhotoArtProvider.h"
#include "TextureDigest.h"
#include <memory>
#include <cstring>
namespace nfs_child_photo {
struct Image{const unsigned char*pixels=nullptr;uint32_t width=0,height=0;};
struct Art{NfsPlayerPhotoArtView photo{};NfsClassPictureArtView klass{};bool isClass=false;uint64_t generation=0;Image image{};~Art(){if(photo.owner&&photo.release)photo.release(photo.owner);if(klass.owner&&klass.release)klass.release(klass.owner);}
 bool current()const noexcept{return generation&&generation==(isClass?nfs_class_picture_art_generation():nfs_player_photo_art_generation());}
 static std::shared_ptr<const Art>acquire(unsigned index){if(index>=15)return {};auto a=std::make_shared<Art>();a->isClass=index>=12;size_t bytes=0;const unsigned char*sha=nullptr;uint32_t schema=0,count=0,id=0;if(a->isClass){if(!nfs_class_picture_art_acquire(index-12,&a->klass))return {};const auto&i=a->klass.images[0];a->image={i.pixels,i.width,i.height};bytes=i.bytes;sha=i.sha;a->generation=a->klass.generation;schema=a->klass.schema;count=a->klass.count;id=i.sourceIndex+12;}else{if(!nfs_player_photo_art_acquire(index,&a->photo))return {};const auto&i=a->photo.images[0];a->image={i.pixels,i.width,i.height};bytes=i.bytes;sha=i.sha;a->generation=a->photo.generation;schema=a->photo.schema;count=a->photo.count;id=i.sourceIndex;}unsigned char actual[32];if(schema!=1||count!=1||id!=index||!a->current()||!a->image.pixels||a->image.width!=(index<12?1228u:1224u)||a->image.height!=584||bytes!=size_t(a->image.width)*584*4||!nfs_texture_sha256(a->image.pixels,bytes,actual)||std::memcmp(actual,sha,32))return {};for(size_t k=0;k<bytes;k+=4)if(a->image.pixels[k]>a->image.pixels[k+3]||a->image.pixels[k+1]>a->image.pixels[k+3]||a->image.pixels[k+2]>a->image.pixels[k+3])return {};return a;}
};
struct LocationArt{NfsLocationPhotoArtView view{};Image image{};uint64_t generation=0;~LocationArt(){if(view.owner&&view.release)view.release(view.owner);}bool current()const noexcept{return generation&&generation==nfs_location_photo_art_generation();}static std::shared_ptr<const LocationArt>acquire(unsigned index){if(index>=8)return {};auto a=std::make_shared<LocationArt>();if(!nfs_location_photo_art_acquire(index,&a->view))return {};const auto&i=a->view.images[0];a->generation=a->view.generation;a->image={i.pixels,i.width,i.height};unsigned char actual[32];const unsigned h=index==2?588:584;if(a->view.schema!=1||a->view.count!=1||i.sourceIndex!=index||i.format!=1||!a->view.owner||!a->view.retain||!a->view.release||!a->current()||!i.pixels||i.width!=1228||i.height!=h||i.bytes!=size_t(1228)*h*4||!nfs_texture_sha256(i.pixels,i.bytes,actual)||std::memcmp(actual,i.sha,32))return {};for(size_t k=0;k<i.bytes;k+=4)if(i.pixels[k]>i.pixels[k+3]||i.pixels[k+1]>i.pixels[k+3]||i.pixels[k+2]>i.pixels[k+3])return {};return a;}};

}

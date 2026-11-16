// SPDX-License-Identifier: MIT
#include "hd/ClassPictureArtProvider.h"
#include "hd/ClassPictureArtworkLoader.h"
#include "TextureDigest.h"
#include <vector>
#include <cassert>
#include <cstring>
int main(){std::vector<unsigned char> pixels(1224u*584u*4u,0);pixels[0]=pixels[3]=128;NfsClassPictureArtImage images[3]{};for(unsigned k=0;k<3;++k){auto&i=images[k];i.sourceIndex=k;i.format=1;i.width=1224;i.height=584;i.logicalBounds=i.focalBounds={0,0,306,146};i.bytes=pixels.size();i.pixels=pixels.data();assert(nfs_texture_sha256(i.pixels,i.bytes,i.sha));}NfsClassPictureArtInput in{1,2,images};assert(!nfs_class_picture_art_install(&in));in.count=3;images[2].sourceIndex=1;assert(!nfs_class_picture_art_install(&in));images[2].sourceIndex=2;images[2].sha[0]^=1;assert(!nfs_class_picture_art_install(&in));images[2].sha[0]^=1;assert(nfs_class_picture_art_install(&in));assert(!nfs_class_picture_art_install(&in));NfsClassPictureArtView v{};assert(nfs_class_picture_art_acquire(2,&v));assert(!nfs_class_picture_art_acquire(12,&v));auto old=v.generation;pixels[0]=0;assert(v.images[0].pixels[0]==128);v.retain(v.owner);nfs_class_picture_art_clear();assert(!nfs_class_picture_art_generation());assert(v.images[0].pixels[0]==128);v.release(v.owner);v.release(v.owner);assert(!nfs_class_picture_art_acquire(2,&v));assert(old);}

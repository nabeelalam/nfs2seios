// SPDX-License-Identifier: MIT
#include "hd/ClassicScreens/CarInfoArtOwned.h"
#include "TextureDigest.h"
#include <cassert>
#include <cstring>
#include <limits>
#include <utility>
using namespace nfs_car_info;
struct Fixture {std::vector<unsigned char>p;std::vector<OwnedImage>i;Fixture():p(12*48,255){for(unsigned k=0;k<12;++k){OwnedImage e;e.offset=k*48;e.image.sourceIndex=k*4;e.image.format=1;e.image.width=4;e.image.height=3;e.image.bytes=48;e.image.logicalBounds=e.image.focalBounds={0,0,640,480};nfs_texture_sha256(p.data()+e.offset,48,e.image.sha);i.push_back(e);}}};
int main(){
 {Fixture f;const auto*base=f.p.data();assert(installOwned(std::move(f.p),f.i));uint32_t ids[]={0,NFS_CAR_INFO_NO_SOURCE};NfsCarInfoArtView v{};assert(nfs_car_info_art_acquire(ids,&v));assert(v.images[0].pixels==base);const auto gen=v.generation;nfs_car_info_art_clear();assert(!nfs_car_info_art_generation()&&v.images[0].pixels[0]==255);v.retain(v.owner);v.release(v.owner);assert(v.images[0].pixels[47]==255);v.release(v.owner);Fixture next;assert(installOwned(std::move(next.p),next.i));assert(nfs_car_info_art_generation()>gen);nfs_car_info_art_clear();}
 for(int bad=0;bad<8;++bad){Fixture f;if(bad==0)f.i[0].offset=std::numeric_limits<size_t>::max();if(bad==1)f.i[0].image.bytes=49;if(bad==2)f.i[1].image.sourceIndex=0;if(bad==3)f.i[0].image.sha[0]^=1;if(bad==4){f.p[3]=0;nfs_texture_sha256(f.p.data(),48,f.i[0].image.sha);}if(bad==5)f.i.pop_back();if(bad==6)f.i[0].image.focalBounds.right=641;if(bad==7)f.i[0].offset=f.p.size()-1;assert(!installOwned(std::move(f.p),f.i));assert(!nfs_car_info_art_generation());}
 {Fixture f;std::vector<NfsCarInfoArtImage>images;for(auto&e:f.i){e.image.pixels=f.p.data()+e.offset;images.push_back(e.image);}NfsCarInfoArtInput in{1,images.size(),images.data()};assert(nfs_car_info_art_install(&in));f.p.assign(f.p.size(),0);uint32_t ids[]={0,NFS_CAR_INFO_NO_SOURCE};NfsCarInfoArtView v{};assert(nfs_car_info_art_acquire(ids,&v));assert(v.images[0].pixels[0]==255);nfs_car_info_art_clear();assert(v.images[0].pixels[47]==255);v.release(v.owner);}
}

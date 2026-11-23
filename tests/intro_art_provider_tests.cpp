// SPDX-License-Identifier: MIT
#include "hd/ClassicScreens/IntroArtProvider.h"
#include "hd/ClassicScreens/IntroSourceCatalog.h"
#include "hd/SmallLogoArtAbi.h"
#include "TextureDigest.h"
#include <cassert>
#include <cstring>
#include <vector>
#include <cstdio>
extern "C" int nfs_private_small_logo_art(NfsSmallLogoArt*){return 0;}
extern "C" int nfs_private_small_logo_region(uint32_t,NfsSmallLogoRegion*){return 0;}
struct Fixture {
 NfsIntroArtInput input{};std::vector<unsigned char>photo,logo,wide,words[6];
 static void image(NfsIntroImage&v,std::vector<unsigned char>&bytes,unsigned width,unsigned height,unsigned format){bytes.assign(size_t(width)*height*(format==NFS_INTRO_ALPHA8?1:4),255);v={};v.format=format;v.width=width;v.height=height;v.bytes=bytes.size();v.pixels=bytes.data();assert(nfs_texture_sha256(v.pixels,v.bytes,v.sha));}
 Fixture(){input.schema=2;input.logoRect={20,30,600,260};image(input.photograph,photo,4,3,NFS_INTRO_RGBA_PREMULTIPLIED);image(input.logo,logo,2,1,NFS_INTRO_RGBA_PREMULTIPLIED);image(input.widePhotograph,wide,16,9,NFS_INTRO_RGBA_PREMULTIPLIED);for(unsigned k=0;k<6;++k)image(input.copyright[k],words[k],4*nfs_intro_sources[k+1].width,4*nfs_intro_sources[k+1].height,NFS_INTRO_ALPHA8);}
};
int main(){
 nfs_intro_art_clear();Fixture f;
 auto reject=[&](const NfsIntroArtInput&in){assert(!nfs_intro_art_install(&in));assert(!nfs_intro_art_generation());};
 auto in=f.input;in.schema=3;reject(in);in=f.input;in.widePhotograph={};reject(in);
 in=f.input;in.widePhotograph.width=4;in.widePhotograph.height=3;reject(in);
 in=f.input;in.widePhotograph.width=UINT32_MAX;reject(in);
 in=f.input;in.widePhotograph.bytes--;reject(in);
 in=f.input;in.widePhotograph.sha[0]^=1;reject(in);
 f.wide[3]=254;nfs_texture_sha256(f.wide.data(),f.wide.size(),f.input.widePhotograph.sha);reject(f.input);
 f.wide[3]=255;nfs_texture_sha256(f.wide.data(),f.wide.size(),f.input.widePhotograph.sha);
 in=f.input;in.schema=1;in.widePhotograph.pixels=reinterpret_cast<const unsigned char*>(1);
 assert(nfs_intro_art_install(&in));NfsIntroArtView legacy{};assert(nfs_intro_art_acquire(&legacy));assert(legacy.schema==1&&!legacy.widePhotograph.pixels);assert(legacy.photograph.width==4&&legacy.photoRect.left==0&&legacy.photoRect.right==640);nfs_intro_art_clear();legacy.release(legacy.owner);
 assert(nfs_intro_art_install(&f.input));assert(!nfs_intro_art_install(&f.input));NfsIntroArtView view{};assert(nfs_intro_art_acquire(&view));assert(view.schema==2&&view.widePhotograph.width==16&&view.widePhotograph.height==9);assert(view.widePhotograph.pixels!=f.wide.data()&&view.photograph.pixels!=f.photo.data());assert(view.photoRect.right==640&&view.photoRect.bottom==480);const auto generation=view.generation;
 f.wide[0]=0;f.photo[0]=0;assert(view.widePhotograph.pixels[0]==255&&view.photograph.pixels[0]==255);view.retain(view.owner);nfs_intro_art_clear();assert(!nfs_intro_art_generation());assert(view.widePhotograph.pixels[0]==255);view.release(view.owner);assert(view.widePhotograph.pixels[0]==255);view.release(view.owner);
 Fixture rounded;Fixture::image(rounded.input.widePhotograph,rounded.wide,1672,941,NFS_INTRO_RGBA_PREMULTIPLIED);assert(nfs_intro_art_install(&rounded.input));NfsIntroArtView next{};assert(nfs_intro_art_acquire(&next)&&next.generation>generation);nfs_intro_art_clear();next.release(next.owner);
 NfsIntroArtView absent{};assert(!nfs_intro_art_acquire(&absent));assert(!nfs_intro_art_install(nullptr));
 std::puts("PASS title provider: legacy compatibility, optional authenticated opaque 16:9 photo, malformed input refusal, independent copies, lease survival and generation retirement");
}

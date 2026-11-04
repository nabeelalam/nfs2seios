#include "../AspectLayout.h"
// SPDX-License-Identifier: MIT
#include "IntroArtProvider.h"
#include "IntroSourceCatalog.h"
#include "TextureDigest.h"
#include "hd/SmallLogoArtAbi.h"
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>
namespace {
constexpr size_t Limit=96u*1024u*1024u;
struct Image {NfsIntroImage view{};std::vector<unsigned char>bytes;};
struct Art {uint64_t generation=0;Image photograph,logo;std::array<Image,6>copyright;NfsIntroRect photoRect{},logoRect{},logoUv{0,0,1,1};};
std::mutex providerMutex;std::shared_ptr<const Art>active;std::atomic<uint64_t>published{0};uint64_t serial=0;
bool copy(Image&to,const NfsIntroImage&in,unsigned format,size_t&total,bool opaque=false){
 const size_t components=format==NFS_INTRO_RGBA_PREMULTIPLIED?4u:1u;
 if(in.format!=format||!in.pixels||!in.width||!in.height||in.width>8192||in.height>8192||size_t(in.width)>Limit/components/in.height)return false;
 const size_t count=size_t(in.width)*in.height*components;if(in.bytes!=count||count>Limit-total)return false;
 to.bytes.assign(in.pixels,in.pixels+count);unsigned char sha[32];if(!nfs_texture_sha256(to.bytes.data(),count,sha)||std::memcmp(sha,in.sha,32))return false;
 if(components==4)for(size_t k=0;k<count;k+=4){unsigned a=to.bytes[k+3];if(to.bytes[k]>a||to.bytes[k+1]>a||to.bytes[k+2]>a||(opaque&&a!=255))return false;}
 to.view=in;to.view.pixels=to.bytes.data();total+=count;return true;
}
struct Lease {std::atomic<unsigned>refs{1};std::shared_ptr<const Art>art;static void retain(void*p){static_cast<Lease*>(p)->refs.fetch_add(1,std::memory_order_relaxed);}static void release(void*p){auto*l=static_cast<Lease*>(p);if(l->refs.fetch_sub(1,std::memory_order_acq_rel)==1)delete l;}};
}
extern "C" int nfs_intro_art_install(const NfsIntroArtInput*in){
 if(!in||in->schema!=1)return 0;
 try{const auto&r=in->logoRect;for(float v:{r.left,r.top,r.right,r.bottom})if(!std::isfinite(v))return 0;if(r.left<0||r.top<0||r.right>640||r.bottom>480||r.left>=r.right||r.top>=r.bottom)return 0;
  auto art=std::make_shared<Art>();size_t total=0;if(!copy(art->photograph,in->photograph,NFS_INTRO_RGBA_PREMULTIPLIED,total,true))return 0;
  NfsAspectRect photo{};if(!nfs_aspect_contain((int32_t)in->photograph.width,(int32_t)in->photograph.height,640,480,&photo))return 0;art->photoRect={(float)photo.left,(float)photo.top,(float)(photo.left+photo.width),(float)(photo.top+photo.height)};
  if(in->logo.pixels){if(!copy(art->logo,in->logo,NFS_INTRO_RGBA_PREMULTIPLIED,total))return 0;}else{NfsSmallLogoArt atlas{};NfsSmallLogoRegion region{};if(!nfs_private_small_logo_art(&atlas)||!nfs_private_small_logo_region(NFS_SMALL_LOGO_ROLE_LOGO,&region)||atlas.schema!=1||atlas.premultiplied!=1||atlas.opacityBaked||!atlas.identity||!atlas.pixels||region.role!=0||region.x!=4||region.y!=4||region.width!=868||region.height!=368||region.scale!=4||uint64_t(region.x)+region.width>atlas.width||uint64_t(region.y)+region.height>atlas.height)return 0;NfsIntroImage image{};image.format=NFS_INTRO_RGBA_PREMULTIPLIED;image.width=atlas.width;image.height=atlas.height;image.bytes=atlas.bytes;image.pixels=atlas.pixels;std::memcpy(image.sha,atlas.sha256,32);if(image.bytes!=NFS_SMALL_LOGO_ART_BYTES||image.width!=NFS_SMALL_LOGO_ART_WIDTH||image.height!=NFS_SMALL_LOGO_ART_HEIGHT||image.bytes>Limit-total)return 0;unsigned char digest[32];if(!nfs_texture_sha256(image.pixels,image.bytes,digest)||std::memcmp(digest,image.sha,32))return 0;for(size_t k=0;k<image.bytes;k+=4)if(image.pixels[k]>image.pixels[k+3]||image.pixels[k+1]>image.pixels[k+3]||image.pixels[k+2]>image.pixels[k+3])return 0;art->logo.view=image;total+=image.bytes;/* Existing HdArtProvider guarantees immutable bytes alive for the entire program. */art->logoUv={float(region.x)/atlas.width,float(region.y)/atlas.height,float(region.x+region.width)/atlas.width,float(region.y+region.height)/atlas.height};}
  for(unsigned k=0;k<6;++k){const auto&s=nfs_intro_sources[k+1];if(in->copyright[k].width!=4*s.width||in->copyright[k].height!=4*s.height||!copy(art->copyright[k],in->copyright[k],NFS_INTRO_ALPHA8,total))return 0;}
  art->logoRect=r;std::lock_guard<std::mutex>lock(providerMutex);if(active||serial==UINT64_MAX)return 0;art->generation=++serial;active=std::move(art);published.store(active->generation,std::memory_order_release);return 1;
 }catch(...){return 0;}
}
extern "C" void nfs_intro_art_clear(){std::lock_guard<std::mutex>lock(providerMutex);published.store(0,std::memory_order_release);active.reset();}
extern "C" uint64_t nfs_intro_art_generation(){return published.load(std::memory_order_acquire);}
extern "C" int nfs_intro_art_acquire(NfsIntroArtView*out){if(!out)return 0;try{std::shared_ptr<const Art>art;{std::lock_guard<std::mutex>lock(providerMutex);art=active;}if(!art)return 0;auto lease=std::unique_ptr<Lease>(new Lease);lease->art=art;NfsIntroArtView v{};v.schema=1;v.generation=art->generation;v.photograph=art->photograph.view;v.logo=art->logo.view;v.photoRect=art->photoRect;v.logoRect=art->logoRect;v.logoUv=art->logoUv;for(unsigned k=0;k<6;++k)v.copyright[k]=art->copyright[k].view;v.owner=lease.get();v.retain=Lease::retain;v.release=Lease::release;*out=v;lease.release();return 1;}catch(...){return 0;}}

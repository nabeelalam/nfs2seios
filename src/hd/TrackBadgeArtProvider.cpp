// SPDX-License-Identifier: MIT
#include "TrackBadgeArtProvider.h"
enum {NFS_TRACK_BADGE_SOURCE_COUNT=18};
#include "TextureDigest.h"
#include <array>
#include <atomic>
#include <cmath>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>
namespace {
constexpr size_t Limit=384u*1024u*1024u;
struct Image {NfsTrackBadgeArtImage view{};std::vector<unsigned char>bytes;bool present=false;};
struct Art {uint64_t generation=0;std::array<Image,NFS_TRACK_BADGE_SOURCE_COUNT>images;};
std::mutex providerMutex;std::shared_ptr<const Art>active;std::atomic<uint64_t>published{0};uint64_t serial=0;
bool rect(const NfsTrackBadgeArtRect&r){for(float v:{r.left,r.top,r.right,r.bottom})if(!std::isfinite(v))return false;return r.left>=-1280&&r.top>=-960&&r.right<=1920&&r.bottom<=1440&&r.left<r.right&&r.top<r.bottom;}
bool copy(Image&to,const NfsTrackBadgeArtImage&in,size_t&total){
 if(in.sourceIndex>=NFS_TRACK_BADGE_SOURCE_COUNT||in.format!=1||!in.pixels||in.width!=1024||in.height!=1024||size_t(in.width)>Limit/4/in.height||!rect(in.logicalBounds)||!rect(in.focalBounds))return false;
 const auto&r=in.logicalBounds;const auto&f=in.focalBounds;if(f.left<r.left||f.top<r.top||f.right>r.right||f.bottom>r.bottom)return false;
 const double x=double(r.right)-r.left,y=double(r.bottom)-r.top;if(std::fabs(double(in.width)*y-double(in.height)*x)>1e-5*double(in.width)*y)return false;
 const bool background=false;if((r.left!=0||r.top!=0||r.right!=256||r.bottom!=256))return false;
 const size_t count=size_t(in.width)*in.height*4;if(in.bytes!=count||count>Limit-total)return false;to.bytes.assign(in.pixels,in.pixels+count);unsigned char sha[32];if(!nfs_texture_sha256(to.bytes.data(),count,sha)||std::memcmp(sha,in.sha,32))return false;
 for(size_t k=0;k<count;k+=4){const unsigned a=to.bytes[k+3];if(to.bytes[k]>a||to.bytes[k+1]>a||to.bytes[k+2]>a||(background&&a!=255))return false;}
 to.view=in;to.view.pixels=to.bytes.data();to.present=true;total+=count;return true;
}
struct Lease {std::atomic<unsigned>refs{1};std::shared_ptr<const Art>art;static void retain(void*p){static_cast<Lease*>(p)->refs.fetch_add(1,std::memory_order_relaxed);}static void release(void*p){auto*l=static_cast<Lease*>(p);if(l->refs.fetch_sub(1,std::memory_order_acq_rel)==1)delete l;}};
}
extern "C" int nfs_track_badge_art_install(const NfsTrackBadgeArtInput*in){if(!in||in->schema!=1||!in->images||!in->count||in->count>NFS_TRACK_BADGE_SOURCE_COUNT)return 0;try{auto art=std::make_shared<Art>();size_t total=0;for(size_t k=0;k<in->count;++k){const auto&i=in->images[k];if(i.sourceIndex>=NFS_TRACK_BADGE_SOURCE_COUNT||art->images[i.sourceIndex].present||!copy(art->images[i.sourceIndex],i,total))return 0;}for(unsigned car=0;car<18;++car)if(!art->images[car].present)return 0;std::lock_guard<std::mutex>lock(providerMutex);if(active||serial==UINT64_MAX)return 0;art->generation=++serial;active=std::move(art);published.store(active->generation,std::memory_order_release);return 1;}catch(...){return 0;}}
extern "C" void nfs_track_badge_art_clear(){std::lock_guard<std::mutex>lock(providerMutex);published.store(0,std::memory_order_release);active.reset();}
extern "C" uint64_t nfs_track_badge_art_generation(){return published.load(std::memory_order_acquire);}
extern "C" int nfs_track_badge_art_acquire(uint32_t id,NfsTrackBadgeArtView*out){if(!out||id>=18)return 0;try{std::shared_ptr<const Art>a;{std::lock_guard<std::mutex>lock(providerMutex);a=active;}if(!a||!a->images[id].present)return 0;auto lease=std::unique_ptr<Lease>(new Lease);lease->art=a;NfsTrackBadgeArtView v{};v.schema=1;v.count=1;v.generation=a->generation;v.images[0]=a->images[id].view;v.owner=lease.get();v.retain=Lease::retain;v.release=Lease::release;*out=v;lease.release();return 1;}catch(...){return 0;}}

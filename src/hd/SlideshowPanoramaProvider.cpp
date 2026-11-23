// SPDX-License-Identifier: MIT
#include "SlideshowPanoramaProvider.h"
#include "TextureDigest.h"
#include <algorithm>
#include <cstring>
#include <limits>
#include <utility>
#include <zlib.h>

namespace nfs_slideshow_panorama {
bool Key::operator==(const Key&o)const noexcept {
    return archiveSha==o.archiveSha&&entrySha==o.entrySha&&
        centerRgbaSha==o.centerRgbaSha&&footerRgbaSha==o.footerRgbaSha;
}
struct Registry::Catalog {
    std::vector<Entry>entries;
    std::shared_ptr<Lifetime>life;
    uint64_t generation=0;
};
namespace {
bool nonzero(const Digest&d){for(auto b:d)if(b)return true;return false;}
bool digest(const std::vector<unsigned char>&v,const Digest&expected){
    Digest actual{};return nfs_texture_sha256(v.data(),v.size(),actual.data())&&actual==expected;
}
bool opaque(const std::vector<unsigned char>&v){
    for(size_t k=3;k<v.size();k+=4)if(v[k]!=255)return false;
    return true;
}
bool metadata(const Entry&e){
    return e.reviewed&&nonzero(e.key.archiveSha)&&nonzero(e.key.entrySha)&&
        nonzero(e.key.centerRgbaSha)&&nonzero(e.key.footerRgbaSha)&&
        nonzero(e.sideSha)&&nonzero(e.composedSha)&&e.width&&e.height&&e.centerWidth&&
        e.width<=Registry::MaximumDimension&&e.height<=Registry::MaximumDimension&&
        e.centerWidth<e.width&&!((e.width-e.centerWidth)&1u)&&
        uint64_t(e.centerWidth)*3==uint64_t(e.height)*4&&
        uint64_t(e.width)*e.height*4<=Registry::MaximumImageBytes&&
        !e.compressedSides.empty()&&e.compressedSides.size()<=Registry::MaximumCompressedEntryBytes;
}
bool inflateSides(const Entry&e,std::vector<unsigned char>&out){
    const size_t expected=size_t(e.width-e.centerWidth)*e.height*4;
    // One spare byte detects streams expanding beyond the declared geometry.
    out.resize(expected+1);
    z_stream z{};
    z.next_in=const_cast<Bytef*>(e.compressedSides.data());
    z.avail_in=static_cast<uInt>(e.compressedSides.size());
    z.next_out=out.data();z.avail_out=static_cast<uInt>(out.size());
    if(inflateInit(&z)!=Z_OK)return false;
    const int status=inflate(&z,Z_FINISH);
    const bool okay=status==Z_STREAM_END&&z.total_out==expected&&z.avail_in==0;
    inflateEnd(&z);
    if(!okay)return false;
    out.resize(expected);
    return opaque(out)&&digest(out,e.sideSha);
}
}
Registry::~Registry(){clear();}
bool Registry::install(std::vector<Entry>entries)noexcept {
    try {
        if(entries.empty()||entries.size()>MaximumEntries)return false;
        size_t bytes=0;
        for(size_t i=0;i<entries.size();++i){
            const auto&e=entries[i];
            if(!metadata(e)||e.compressedSides.size()>MaximumCompressedBytes-bytes)return false;
            bytes+=e.compressedSides.size();
            for(size_t j=0;j<i;++j)if(e.key==entries[j].key)return false;
        }
        auto next=std::make_shared<Catalog>();
        next->entries=std::move(entries);next->life=std::make_shared<Lifetime>();
        std::lock_guard<std::mutex>lock(mutex_);
        if(!nextGeneration_)return false;
        next->generation=nextGeneration_;
        nextGeneration_=nextGeneration_==std::numeric_limits<uint64_t>::max()?0:nextGeneration_+1;
        if(catalog_)catalog_->life->alive.store(false,std::memory_order_release);
        catalog_=std::move(next);
        return true;
    }catch(...){return false;}
}
void Registry::clear()noexcept {
    std::lock_guard<std::mutex>lock(mutex_);
    if(catalog_)catalog_->life->alive.store(false,std::memory_order_release);
    catalog_.reset();
}
std::shared_ptr<const Panorama>Registry::acquire(const Key&key,uint32_t width,
    uint32_t height,const std::vector<unsigned char>&center)const noexcept {
    try {
        std::shared_ptr<const Catalog>catalog;
        {std::lock_guard<std::mutex>lock(mutex_);catalog=catalog_;}
        if(!catalog||!catalog->life->alive.load(std::memory_order_acquire))return {};
        const Entry*entry=nullptr;
        for(const auto&e:catalog->entries)if(e.key==key){entry=&e;break;}
        if(!entry||width!=entry->centerWidth||height!=entry->height||
            uint64_t(width)*height*4!=center.size()||!opaque(center)||!digest(center,key.centerRgbaSha))return {};
        std::vector<unsigned char>sides;
        if(!inflateSides(*entry,sides))return {};
        auto p=std::shared_ptr<Panorama>(new Panorama);
        p->key_=key;p->width_=entry->width;p->height_=entry->height;
        p->centerWidth_=width;p->generation_=catalog->generation;
        p->life_=catalog->life;p->sha_=entry->composedSha;
        p->rgba_.resize(size_t(p->width_)*height*4);
        const size_t sideRow=size_t(p->centerLeft())*4,centerRow=size_t(width)*4;
        const size_t sidePlane=sideRow*height,fullRow=size_t(p->width_)*4;
        for(size_t y=0;y<height;++y){
            auto*dst=p->rgba_.data()+y*fullRow;
            std::memcpy(dst,sides.data()+y*sideRow,sideRow);
            std::memcpy(dst+sideRow,center.data()+y*centerRow,centerRow);
            std::memcpy(dst+sideRow+centerRow,sides.data()+sidePlane+y*sideRow,sideRow);
        }
        if(!digest(p->rgba_,entry->composedSha)||!p->current())return {};
        return p;
    }catch(...){return {};}
}
}

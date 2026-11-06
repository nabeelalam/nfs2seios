// SPDX-License-Identifier: MIT
#pragma once
#include "SlideshowFooterCatalog.h"
#include "SlideshowPixels.h"
#include <fstream>

namespace nfs_slideshow_footer {
// Generated private pins are trusted; the resource supplies pixels only, never routing.
struct ResourcePin {
    uint64_t offset;
    uint32_t bytes,width,height;
    Digest compressedSha,rgbaSha;
};
inline std::shared_ptr<const Catalog> loadResource(
    const char*path,uint64_t fileBytes,const ResourcePin*pins,size_t count,
    const std::vector<Binding>&bindings,size_t byteBudget=Catalog::MaximumBytes) noexcept {
    try {
        if(!path||!*path||!pins||!count||count>Catalog::MaximumImages||
           fileBytes<16||fileBytes>256ull*1024*1024||!byteBudget||
           byteBudget>Catalog::MaximumBytes)return {};
        // Reject malformed pins before opening or allocating any resource pixels.
        uint64_t previousEnd=16,decodedBytes=0;
        for(size_t k=0;k<count;++k){
            const auto&p=pins[k];
            if(p.offset!=previousEnd||!p.bytes||p.bytes>32u*1024*1024||
               p.offset>fileBytes||p.bytes>fileBytes-p.offset||
               !p.width||!p.height||p.width>4096||p.height>4096||
               uint64_t(p.width)*3!=uint64_t(p.height)*4)return {};
            const uint64_t decoded=uint64_t(p.width)*p.height*4;
            if(decoded>FooterImage::MaximumBytes||decoded>byteBudget-decodedBytes)return {};
            decodedBytes+=decoded;previousEnd=p.offset+p.bytes;
        }
        if(previousEnd!=fileBytes)return {};
        std::ifstream stream(path,std::ios::binary);if(!stream)return {};
        stream.seekg(0,std::ios::end);const auto size=stream.tellg();
        if(size<0||uint64_t(size)!=fileBytes)return {};
        stream.seekg(0,std::ios::beg);
        unsigned char header[16];
        if(!stream.read(reinterpret_cast<char*>(header),16))return {};
        const unsigned char expected[16]={'N','F','S','F','T','R','0','1',1,0,0,0,
            static_cast<unsigned char>(count),0,0,0};
        if(std::memcmp(header,expected,16))return {};
        std::vector<ImagePtr> images;images.reserve(count);
        for(size_t k=0;k<count;++k){
            const auto&p=pins[k];std::vector<unsigned char> compressed(p.bytes),rgba;
            if(!stream.read(reinterpret_cast<char*>(compressed.data()),p.bytes))return {};
            Digest actual{};
            if(!nfs_texture_sha256(compressed.data(),compressed.size(),actual.data())||
               actual!=p.compressedSha)return {};
            const nfs_slideshow::PixelPin pixelPin{p.width,p.height,p.rgbaSha};
            if(!nfs_slideshow::inflatePixels(compressed.data(),compressed.size(),pixelPin,rgba))return {};
            auto image=FooterImage::create(p.width,p.height,std::move(rgba),p.rgbaSha);
            if(!image)return {};images.push_back(std::move(image));
        }
        return Catalog::create(images,bindings,byteBudget);
    }catch(...){return {};}
}
} // namespace nfs_slideshow_footer

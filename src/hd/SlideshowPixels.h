// SPDX-License-Identifier: MIT
#pragma once
#include "TextureDigest.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <vector>
// Original engine globals include a macro named buf. Keep it out of zlib declarations.
#pragma push_macro("buf")
#undef buf
#include <zlib.h>
#pragma pop_macro("buf")

namespace nfs_slideshow {
struct PixelPin {
    uint32_t width,height;
    std::array<unsigned char,32> rgbaSha;
    uint32_t predictor=0; // 0 raw RGBA, 1 PNG row filters (RGBA8), 2 PNG row filters (opaque RGB8).
};
// Decompress one pinned RGBA8 canvas. No scaling, color remapping or image edits.
// Output is changed only after the complete image and digest have been validated.
inline bool inflatePixels(const unsigned char* compressed, size_t bytes,
                          const PixelPin& pin, std::vector<unsigned char>& output) noexcept {
    if(!compressed || !bytes || bytes>32u*1024u*1024u ||
       !pin.width || !pin.height || pin.width>4096 || pin.height>4096 ||
       pin.width*3!=pin.height*4 || pin.predictor>2) return false;
    struct InflateOwner {
        z_stream z{}; bool initialized=false;
        ~InflateOwner(){if(initialized)inflateEnd(&z);}
    } owner;
    try {
        const size_t decoded=size_t(pin.width)*pin.height*4u;
        if(decoded>64u*1024u*1024u) return false;
        const bool opaque=pin.predictor==2;
        const size_t filteredStride=size_t(pin.width)*(opaque?3u:4u)+1;
        const size_t encoded=pin.predictor?filteredStride*pin.height:decoded;
        if(encoded>64u*1024u*1024u)return false;
        std::vector<unsigned char> candidate(encoded);
        owner.z.next_in=const_cast<Bytef*>(compressed);
        owner.z.avail_in=static_cast<uInt>(bytes);
        owner.z.next_out=candidate.data();
        owner.z.avail_out=static_cast<uInt>(candidate.size());
        if(inflateInit(&owner.z)!=Z_OK) return false;
        owner.initialized=true;
        int result=Z_OK;
        while(result!=Z_STREAM_END) {
            const auto beforeIn=owner.z.total_in,beforeOut=owner.z.total_out;
            result=inflate(&owner.z,Z_NO_FLUSH);
            if(result!=Z_OK && result!=Z_STREAM_END) return false;
            if(owner.z.total_in==beforeIn && owner.z.total_out==beforeOut) return false;
            if(!owner.z.avail_out && result!=Z_STREAM_END) {
                unsigned char extra;
                owner.z.next_out=&extra;owner.z.avail_out=1;
                result=inflate(&owner.z,Z_FINISH);
                if(result!=Z_STREAM_END || owner.z.total_out!=encoded) return false;
            }
        }
        if(owner.z.total_out!=encoded || owner.z.avail_in) return false;
        if(pin.predictor){
            // PNG filter method 0 for non-interlaced RGBA8 or opaque RGB8. The trusted
            // pixel pin supplies dimensions and raw digest, not an image header.
            // https://www.w3.org/TR/PNG-Filters.html
            const size_t stride=size_t(pin.width)*4;
            std::vector<unsigned char> reconstructed(decoded);
            for(uint32_t rowIndex=0;rowIndex<pin.height;++rowIndex){
                const auto*filtered=candidate.data()+size_t(rowIndex)*filteredStride;
                const unsigned filter=filtered[0];if(filter>4)return false;
                auto*rowPixels=reconstructed.data()+size_t(rowIndex)*stride;
                const auto*upperRow=rowIndex?rowPixels-stride:nullptr;
                const auto*encodedByte=filtered+1;
                for(size_t column=0;column<stride;++column){
                    if(opaque && (column&3u)==3u){
                        rowPixels[column]=255;
                        continue;
                    }
                    const int leftByte=column>=4?rowPixels[column-4]:0;
                    const int upperByte=upperRow?upperRow[column]:0;
                    const int cornerByte=upperRow&&column>=4?upperRow[column-4]:0;
                    int prediction=0;
                    if(filter==1)prediction=leftByte;
                    else if(filter==2)prediction=upperByte;
                    else if(filter==3)prediction=(leftByte+upperByte)/2;
                    else if(filter==4){
                        const int estimate=leftByte+upperByte-cornerByte;
                        int dl=estimate-leftByte,du=estimate-upperByte,dc=estimate-cornerByte;
                        if(dl<0)dl=-dl;if(du<0)du=-du;if(dc<0)dc=-dc;
                        prediction=dl<=du&&dl<=dc?leftByte:(du<=dc?upperByte:cornerByte);
                    }
                    rowPixels[column]=static_cast<unsigned char>(*encodedByte+prediction);
                    ++encodedByte;
                }
            }
            candidate.swap(reconstructed);
        }
        unsigned char sha[32];
        if(!nfs_texture_sha256(candidate.data(),candidate.size(),sha) ||
           std::memcmp(sha,pin.rgbaSha.data(),32)) return false;
        output.swap(candidate);
        return true;
    } catch (...) {return false;}
}
} // namespace nfs_slideshow

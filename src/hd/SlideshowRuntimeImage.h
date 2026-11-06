// SPDX-License-Identifier: MIT
#pragma once
#include "SlideshowChunkReader.h"
#include "SlideshowPixels.h"
namespace nfs_slideshow {
struct RuntimeImagePin {
    uint64_t fileBytes, offset;
    uint32_t bytes;
    std::array<unsigned char,32> sourceSha,entrySha,chunkSha;
    PixelPin pixels;
    bool accepted;
    uint32_t version=2;
};
// Trusted generated pins bind original archive identity to one lossless RGBA chunk.
inline bool loadImage(const char* path,const RuntimeImagePin& pin,
                      const std::array<unsigned char,32>& source,
                      const std::array<unsigned char,32>& entry,
                      std::vector<unsigned char>& rgba,bool preview=false) noexcept {
    try {
        if(!path || !*path || (!pin.accepted&&!preview) ||
           (pin.version!=2&&pin.version!=3) ||
           pin.pixels.predictor!=(pin.version==3?1u:0u) ||
           source!=pin.sourceSha || entry!=pin.entrySha ||
           pin.fileBytes<16 || pin.fileBytes>512ull*1024ull*1024ull ||
           pin.offset<16 || pin.offset>pin.fileBytes || !pin.bytes ||
           pin.bytes>32u*1024u*1024u || pin.bytes>pin.fileBytes-pin.offset) return false;
        std::ifstream stream(path,std::ios::binary);
        if(!stream)return false;
        stream.seekg(0,std::ios::end);const auto size=stream.tellg();
        if(size<0 || uint64_t(size)!=pin.fileBytes)return false;
        stream.seekg(0,std::ios::beg);unsigned char header[16];
        const unsigned char expected[16]={'N','F','S','S','L','D','R',
            static_cast<unsigned char>('0'+pin.version),static_cast<unsigned char>(pin.version),0,0,0,98,0,0,0};
        if(!stream.read(reinterpret_cast<char*>(header),16) ||
           std::memcmp(header,expected,16))return false;
        stream.seekg(std::streamoff(pin.offset),std::ios::beg);
        std::vector<unsigned char> chunk(pin.bytes);
        if(!stream.read(reinterpret_cast<char*>(chunk.data()),pin.bytes))return false;
        unsigned char digest[32];
        if(!nfs_texture_sha256(chunk.data(),chunk.size(),digest) ||
           std::memcmp(digest,pin.chunkSha.data(),32))return false;
        return inflatePixels(chunk.data(),chunk.size(),pin.pixels,rgba);
    } catch (...) {return false;}
}
} // namespace nfs_slideshow

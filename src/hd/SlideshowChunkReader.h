// SPDX-License-Identifier: MIT
#pragma once
#include "TextureDigest.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <vector>

namespace nfs_slideshow {
// Generated trusted pins remain separate from the public source and private artwork.
struct ChunkPin {
    uint64_t fileBytes, offset;
    uint32_t bytes, width, height;
    std::array<unsigned char,32> sourceSha, entrySha, pngSha;
    bool accepted;
};
inline uint32_t big32(const unsigned char* p) {
    return (uint32_t(p[0])<<24)|(uint32_t(p[1])<<16)|(uint32_t(p[2])<<8)|p[3];
}
// Only encoded bytes are returned. Decode/upload ownership belongs to the screen.
// A failed read preserves the caller's current image.
inline bool read(const char* path, const ChunkPin& pin,
                 const std::array<unsigned char,32>& source,
                 const std::array<unsigned char,32>& entry,
                 std::vector<unsigned char>& output, bool preview=false) noexcept {
    try {
        constexpr uint32_t maxChunk=32u*1024u*1024u;
        constexpr uint64_t maxFile=512ull*1024ull*1024ull;
        if (!path || !*path || (!pin.accepted && !preview) ||
            source!=pin.sourceSha || entry!=pin.entrySha ||
            pin.fileBytes>maxFile || pin.fileBytes<16 || pin.offset<16 ||
            pin.bytes<33 || pin.bytes>maxChunk || pin.offset>pin.fileBytes ||
            pin.bytes>pin.fileBytes-pin.offset || !pin.width || !pin.height ||
            pin.width>4096 || pin.height>4096 || pin.width*3!=pin.height*4) return false;
        std::ifstream stream(path,std::ios::binary);
        if (!stream) return false;
        stream.seekg(0,std::ios::end);
        const auto size=stream.tellg();
        if(size<0 || uint64_t(size)!=pin.fileBytes) return false;
        stream.seekg(0,std::ios::beg);
        unsigned char header[16];
        if(!stream.read(reinterpret_cast<char*>(header),16) ||
           std::memcmp(header,"NFSSLIDE",8) || header[8]!=1 ||
           header[9] || header[10] || header[11]) return false;
        const uint32_t metadata=uint32_t(header[12])|(uint32_t(header[13])<<8)|
            (uint32_t(header[14])<<16)|(uint32_t(header[15])<<24);
        if(!metadata || metadata>256u*1024u || pin.offset<16ull+metadata) return false;
        stream.seekg(std::streamoff(pin.offset),std::ios::beg);
        std::vector<unsigned char> candidate(pin.bytes);
        if(!stream.read(reinterpret_cast<char*>(candidate.data()),pin.bytes)) return false;
        unsigned char sha[32];
        const unsigned char signature[8]={137,80,78,71,13,10,26,10};
        if(!nfs_texture_sha256(candidate.data(),candidate.size(),sha) ||
           std::memcmp(sha,pin.pngSha.data(),32) ||
           std::memcmp(candidate.data(),signature,8) || big32(candidate.data()+8)!=13 ||
           std::memcmp(candidate.data()+12,"IHDR",4) ||
           big32(candidate.data()+16)!=pin.width || big32(candidate.data()+20)!=pin.height)
            return false;
        output.swap(candidate);
        return true;
    } catch (...) { return false; }
}
} // namespace nfs_slideshow

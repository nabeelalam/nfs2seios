// SPDX-License-Identifier: MIT
#pragma once
#include "TextureDigest.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace nfs_slideshow {
struct SourcePin {
    uint32_t archiveBytes, entryOffset, entryBytes, format, width, height;
    std::array<unsigned char,32> archiveSha, entrySha;
};
// The caller supplies an already validated readable original archive span.
// Names and equal-sized archives alone must never select replacement artwork.
inline const SourcePin* identify(const void* archive, size_t bytes,
                                const SourcePin* pins, size_t count) noexcept {
    if(!archive || !pins || !count || count>98 || bytes<16 || bytes>1024u*1024u)
        return nullptr;
    unsigned char archiveSha[32];
    if(!nfs_texture_sha256(archive,bytes,archiveSha)) return nullptr;
    const SourcePin* match=nullptr;
    const auto* data=static_cast<const unsigned char*>(archive);
    for(size_t i=0;i<count;++i) {
        const auto& p=pins[i];
        if(p.archiveBytes!=bytes || std::memcmp(archiveSha,p.archiveSha.data(),32)) continue;
        if(match || p.entryOffset>bytes || p.entryBytes>bytes-p.entryOffset ||
           p.entryBytes<16 || p.width!=640 || p.height!=480 ||
           (p.format!=120 && p.format!=126) ||
           p.entryBytes!=16u+p.width*p.height*2u) return nullptr;
        const auto* sourceEntry=data+p.entryOffset;
        const uint32_t sourceHeader=uint32_t(sourceEntry[0])|(uint32_t(sourceEntry[1])<<8)|
                              (uint32_t(sourceEntry[2])<<16)|(uint32_t(sourceEntry[3])<<24);
        const uint32_t width=uint32_t(sourceEntry[4])|(uint32_t(sourceEntry[5])<<8);
        const uint32_t height=uint32_t(sourceEntry[6])|(uint32_t(sourceEntry[7])<<8);
        unsigned char entrySha[32];
        if((sourceHeader&255u)!=p.format || width!=p.width || height!=p.height ||
           !nfs_texture_sha256(sourceEntry,p.entryBytes,entrySha) ||
           std::memcmp(entrySha,p.entrySha.data(),32)) return nullptr;
        match=&p;
    }
    return match;
}
} // namespace nfs_slideshow

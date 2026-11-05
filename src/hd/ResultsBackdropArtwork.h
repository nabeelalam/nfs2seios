// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundSceneArt.h"
#include "TextureDigest.h"
#include <array>
#include <cstring>
#include <fstream>
#include <memory>
#include <vector>
#include <zlib.h>

namespace nfs_hd {
inline bool loadResultsBackdrops(const std::string& path) {
    constexpr size_t fileSize = 5711653;
    constexpr size_t payloadSize = 14515288;
    constexpr unsigned char expected[] = {0xca,0xf2,0x9a,0x73,0xaf,0x89,0x66,0x38,0x5c,0x1a,0x48,0xd4,0x6b,0xe5,0x99,0x50,0xbe,0x6f,0x88,0xb9,0x24,0x59,0x56,0x6d,0x67,0xbf,0x67,0x14,0x6b,0xfa,0x81,0xae};
    const auto le32 = [](const unsigned char* p) {
        return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
    };
    const auto matches = [](const unsigned char* p, size_t n, const unsigned char* pin) {
        unsigned char digest[32]{};
        return nfs_texture_sha256(p, n, digest) && std::memcmp(digest, pin, sizeof digest) == 0;
    };
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in || in.tellg() != static_cast<std::streamoff>(fileSize)) return false;
    std::vector<unsigned char> file(fileSize);
    in.seekg(0);
    if (!in.read(reinterpret_cast<char*>(file.data()), file.size()) ||
        in.peek() != std::char_traits<char>::eof() || !matches(file.data(), file.size(), expected) ||
        std::memcmp(file.data(), "NFSRESB1", 8) || le32(file.data()+8) != 2 ||
        le32(file.data()+12) != payloadSize) return false;
    auto pixels = std::make_unique<unsigned char[]>(payloadSize);
    z_stream stream{};
    stream.next_in = file.data()+16;
    stream.avail_in = file.size()-16;
    stream.next_out = pixels.get();
    stream.avail_out = payloadSize;
    if (inflateInit(&stream) != Z_OK) return false;
    const bool exact = inflate(&stream, Z_FINISH) == Z_STREAM_END &&
                       stream.avail_in == 0 && stream.total_out == payloadSize;
    inflateEnd(&stream);
    if (!exact) return false;

    NfsResultsBackdropImage images[2]{};
    size_t offset = 0;
    for (unsigned i = 0; i < 2; ++i) {
        const unsigned width = i ? 1920 : 1440;
        constexpr unsigned height = 1080;
        const size_t bytes = size_t(width)*height*4;
        if (payloadSize-offset < 44+bytes) return false;
        const auto* record = pixels.get()+offset;
        if (le32(record) != width || le32(record+4) != height ||
            le32(record+8) != bytes || !matches(record+44, bytes, record+12)) return false;
        images[i] = {width, height, record+44};
        offset += 44+bytes;
    }
    if (offset != payloadSize || !nfs_results_install_backdrops(images)) return false;
    (void)pixels.release(); // Published pixels have process lifetime.
    return true;
}
}

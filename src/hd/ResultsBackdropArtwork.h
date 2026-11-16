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
    constexpr size_t fileSize = 8286561;
    constexpr size_t payloadSize = 20808068;
    constexpr unsigned char expected[] = {0x1c,0x41,0xd5,0xd6,0x0c,0xf9,0x51,0xcc,0x5b,0xc1,0x34,0xfc,0xfc,0x2f,0x8e,0xe9,0xf4,0x83,0xff,0x5e,0x5c,0xdf,0xe0,0xcf,0xaa,0xc0,0x32,0x51,0x03,0x09,0x69,0x69};
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
        std::memcmp(file.data(), "NFSRESB1", 8) || le32(file.data()+8) != 3 ||
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

    NfsResultsBackdropImage images[3]{};
    size_t offset = 0;
    for (unsigned i = 0; i < 3; ++i) {
        const unsigned widths[] = {1440,1920,1504};
        const unsigned heights[] = {1080,1080,1046};
        const unsigned width = widths[i], height = heights[i];
        const size_t bytes = size_t(width)*height*4;
        if (payloadSize-offset < 44+bytes) return false;
        const auto* record = pixels.get()+offset;
        if (le32(record) != width || le32(record+4) != height ||
            le32(record+8) != bytes || !matches(record+44, bytes, record+12)) return false;
        images[i] = {width, height, record+44};
        offset += 44+bytes;
    }
    if (offset != payloadSize || !nfs_results_install_backdrops_v2(images)) return false;
    (void)pixels.release(); // Published pixels have process lifetime.
    return true;
}
}

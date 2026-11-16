// SPDX-License-Identifier: MIT
#pragma once

#include "TextureDigest.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
#include <zlib.h>

namespace nfs_hd {
class WideMenuArtwork {
public:
    struct Layers {
        unsigned width = 0, height = 0;
        const uint8_t* map = nullptr;
        const uint8_t* panel = nullptr;
    };

    bool load(const std::string& path) {
        constexpr size_t fileBytes = 4822825;
        constexpr size_t payloadBytes = 15113792;
        constexpr unsigned headerBytes = 20;
        constexpr std::array<uint8_t, 32> pin = {0xc8,0xbb,0xa4,0x96,0x86,0xd8,0x75,0x37,0x7d,0x92,0x9b,0x0b,0xd5,0xf3,0xd2,0x83,0x6c,0x85,0x44,0xd2,0x38,0x2e,0xe8,0x2f,0x72,0xf0,0x31,0xf8,0xff,0xb1,0xd5,0xe4};
        std::ifstream input(path, std::ios::binary | std::ios::ate);
        if (!input || input.tellg() != static_cast<std::streamoff>(fileBytes)) return false;
        std::vector<uint8_t> file(fileBytes);
        input.seekg(0);
        if (!input.read(reinterpret_cast<char*>(file.data()), file.size()) ||
            input.peek() != std::char_traits<char>::eof() || !matches(file.data(), file.size(), pin.data()))
            return false;
        if (std::memcmp(file.data(), "NFSWIDE1", 8) || le32(file.data() + 8) != 1 ||
            le32(file.data() + 12) != 4 || le32(file.data() + 16) != payloadBytes) return false;

        std::vector<uint8_t> decoded(payloadBytes);
        z_stream stream{};
        stream.next_in = file.data() + headerBytes;
        stream.avail_in = file.size() - headerBytes;
        stream.next_out = decoded.data();
        stream.avail_out = decoded.size();
        if (inflateInit(&stream) != Z_OK) return false;
        const bool exact = inflate(&stream, Z_FINISH) == Z_STREAM_END &&
                           stream.avail_in == 0 && stream.total_out == decoded.size();
        inflateEnd(&stream);
        if (!exact) return false;

        std::array<std::array<size_t, 2>, 2> offsets{};
        size_t cursor = 0;
        for (size_t ratio = 0; ratio < dimensions().size(); ++ratio) {
            const auto& size = dimensions()[ratio];
            const size_t bytes = size[0] * size[1] * 4;
            for (unsigned role = 0; role < 2; ++role) {
                constexpr size_t recordBytes = 48;
                if (decoded.size() - cursor < recordBytes + bytes) return false;
                const auto* record = decoded.data() + cursor;
                if (le32(record) != role || le32(record + 4) != size[0] ||
                    le32(record + 8) != size[1] || le32(record + 12) != bytes) return false;
                cursor += recordBytes;
                const auto* pixels = decoded.data() + cursor;
                if (!matches(pixels, bytes, record + 16)) return false;
                for (size_t i = 0; i < bytes; i += 4) {
                    const auto alpha = pixels[i + 3];
                    if ((!role && alpha != 255) || pixels[i] > alpha ||
                        pixels[i + 1] > alpha || pixels[i + 2] > alpha) return false;
                }
                offsets[ratio][role] = cursor;
                cursor += bytes;
            }
        }
        if (cursor != decoded.size()) return false;
        data_.swap(decoded);
        offsets_ = offsets;
        return true;
    }

    Layers layers(size_t ratio) const {
        if (data_.empty() || ratio >= dimensions().size()) return {};
        return {dimensions()[ratio][0], dimensions()[ratio][1],
                data_.data() + offsets_[ratio][0], data_.data() + offsets_[ratio][1]};
    }

private:
    static uint32_t le32(const uint8_t* p) {
        return uint32_t(p[0]) | uint32_t(p[1]) << 8 | uint32_t(p[2]) << 16 | uint32_t(p[3]) << 24;
    }
    static bool matches(const uint8_t* data, size_t bytes, const uint8_t* expected) {
        std::array<uint8_t, 32> actual{};
        return nfs_texture_sha256(data, bytes, actual.data()) &&
               std::memcmp(actual.data(), expected, actual.size()) == 0;
    }
    // C++14: odr-used constexpr data members still need separate storage.
    // An inline accessor gives the immutable table one defined lifetime in
    // unoptimized/multi-unit builds as well as optimized app builds.
    static const std::array<std::array<unsigned, 2>, 2>& dimensions() {
        static const std::array<std::array<unsigned, 2>, 2> value{{{1180, 820}, {1280, 720}}};
        return value;
    }
    std::vector<uint8_t> data_;
    std::array<std::array<size_t, 2>, 2> offsets_{};
};
}

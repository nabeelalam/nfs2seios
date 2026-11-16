// SPDX-License-Identifier: MIT
#pragma once

#include <cstddef>

namespace nfs {
// Version 2 information packs store each RGBA row as a byte delta
// from the preceding row. Call only after checking the image span.
inline void restoreInformationArtworkRows(unsigned char *pixels, std::size_t stride,
                                          std::size_t count) {
    for (std::size_t i = stride; i < count; ++i)
        pixels[i] = static_cast<unsigned char>(pixels[i] + pixels[i - stride]);
}
}

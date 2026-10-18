// SPDX-License-Identifier: MIT
#ifndef NFS_BUILTIN_SMALL_FONT_H
#define NFS_BUILTIN_SMALL_FONT_H
#include <stddef.h>
#include <string.h>
// Validate the complete stream before writing any reserved auxiliary pixels.
// The original game textures are never destinations for this decoder.
static int nfs_small_font_expand(const unsigned char *rle, size_t bytes,
                                unsigned char *rgba, size_t capacity) {
    const unsigned pixels = 26u * 64u * 64u;
    if (!rle || !rgba || bytes % 6 || capacity < 1024u * 1024u * 4u)
        return 0;
    unsigned count = 0;
    for (size_t i = 0; i < bytes; i += 6) {
        unsigned run = rle[i] | ((unsigned)rle[i + 1] << 8);
        if (!run || run > pixels - count) return 0;
        count += run;
    }
    if (count != pixels) return 0;
    unsigned position = 0;
    for (size_t i = 0; i < bytes; i += 6) {
        unsigned run = rle[i] | ((unsigned)rle[i + 1] << 8);
        for (unsigned j = 0; j < run; ++j, ++position) {
            unsigned glyph = position / 4096, cell = position % 4096;
            unsigned x = (glyph % 16) * 64 + cell % 64;
            unsigned y = 512 + (glyph / 16) * 64 + cell / 64;
            memcpy(rgba + ((size_t)y * 1024 + x) * 4, rle + i + 2, 4);
        }
    }
    return 1;
}
#endif

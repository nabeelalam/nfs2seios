// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>

struct NfsFrontHitRect {
    int16_t item, left, top, right, bottom;
};

// Compatibility coordinates, not original game artwork. Require the complete
// English main-menu signature; other forms and languages retain their bounds.
static const NfsFrontHitRect nfs_front_hit_original[8] = {
    {0, 93,126,180,156}, {1,116,162,180,192},
    {2,100,198,180,228}, {3, 98,234,180,264},
    {4,100,271,180,301}, {5, 99,302,180,350},
    {6,120,364,180,394}, {7,144,400,180,430}
};
static const int16_t nfs_front_hit_left[8] = {73,102,83,82,81,93,106,136};

static inline int nfs_front_hit_expand(int enabled, uint32_t type,
                                     unsigned count, NfsFrontHitRect *rows) {
    if (!enabled || type != 0x401910 || count != 8 || !rows) return 0;
    for (unsigned i = 0; i < 8; ++i) {
        const NfsFrontHitRect &a = rows[i], &b = nfs_front_hit_original[i];
        if (a.item != b.item || a.left != b.left || a.top != b.top ||
            a.right != b.right || a.bottom != b.bottom) return 0;
    }
    for (unsigned i = 0; i < 8; ++i)
        if (nfs_front_hit_left[i] < rows[i].left)
            rows[i].left = nfs_front_hit_left[i];
    return 1;
}

// Writer uses checked guest stores, never a host pointer to translated memory.
// Restore even when the original function returns early or unwinds.
template<class Writer> class NfsFrontHitScope {
    Writer write;
    int16_t original[8];
    unsigned changed;
public:
    NfsFrontHitScope(int active, uint32_t type, unsigned count,
                     NfsFrontHitRect *rows, Writer writer)
        : write(writer), changed(0) {
        if (count == 8 && rows)
            for (unsigned i = 0; i < 8; ++i) original[i] = rows[i].left;
        if (!nfs_front_hit_expand(active, type, count, rows)) return;
        for (unsigned i = 0; i < 8; ++i) {
            write(i, rows[i].left);
            ++changed;
        }
    }
    ~NfsFrontHitScope() {
        for (unsigned i = 0; i < changed; ++i) write(i, original[i]);
    }
    NfsFrontHitScope(const NfsFrontHitScope &) = delete;
    NfsFrontHitScope &operator=(const NfsFrontHitScope &) = delete;
};

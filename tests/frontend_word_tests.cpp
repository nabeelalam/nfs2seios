// SPDX-License-Identifier: MIT
#include "FrontendHitBounds.h"
#include "FrontendWordMode.h"
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <thread>

#define CHECK(x) do { if (!(x)) std::abort(); } while (0)

int main() {
    // No renderer or SDL dependency: a headless engine starts with old bounds.
    CHECK(nfs_front_word_hit_active() == 0);
    for (int value : {0, 1, -1, 37, 0}) {
        nfs_front_word_set_available(value);
        CHECK(nfs_front_word_hit_active() == (value != 0));
    }
    std::atomic<bool> finished{false};
    std::thread publisher([&] {
        for (unsigned i = 0; i < 100000; ++i)
            nfs_front_word_set_available(i & 1);
        nfs_front_word_set_available(0);
        finished.store(true);
    });
    do {
        const int mode = nfs_front_word_hit_active();
        CHECK(mode == 0 || mode == 1);
    } while (!finished.load());
    publisher.join();
    CHECK(nfs_front_word_hit_active() == 0);

    NfsFrontHitRect rows[8], before[8];
    unsigned negatives = 0;
    for (unsigned invalid = 0; invalid < 44; ++invalid) {
        std::memcpy(rows, nfs_front_hit_original, sizeof rows);
        int active = 1;
        uint32_t type = 0x401910;
        unsigned count = 8;
        if (invalid == 0) active = 0;
        else if (invalid == 1) ++type;
        else if (invalid == 2) count = 7;
        else if (invalid == 3) count = 9;
        else {
            auto &row = rows[(invalid-4)/5];
            int16_t *fields[] = {&row.item, &row.left, &row.top,
                                &row.right, &row.bottom};
            ++*fields[(invalid-4)%5];
        }
        std::memcpy(before, rows, sizeof rows);
        unsigned writes = 0;
        auto writer = [&](unsigned, int16_t) { ++writes; };
        {
            NfsFrontHitScope<decltype(writer)> scope(active, type, count, rows, writer);
        }
        CHECK(writes == 0 && std::memcmp(before, rows, sizeof rows) == 0);
        ++negatives;
    }
    CHECK(!nfs_front_hit_expand(1, 0x401910, 8, nullptr));
    for (bool unwind : {false, true}) {
        int16_t guestLeft[8];
        for (unsigned i = 0; i < 8; ++i)
            guestLeft[i] = nfs_front_hit_original[i].left;
        std::memcpy(rows, nfs_front_hit_original, sizeof rows);
        auto writer = [&](unsigned i, int16_t value) { guestLeft[i] = value; };
        try {
            NfsFrontHitScope<decltype(writer)> scope(1, 0x401910, 8, rows, writer);
            for (unsigned i = 0; i < 8; ++i)
                CHECK(guestLeft[i] == nfs_front_hit_left[i]);
            if (unwind) throw std::runtime_error("Original function unwinds");
        } catch (const std::runtime_error &) {
            CHECK(unwind);
        }
        for (unsigned i = 0; i < 8; ++i)
            CHECK(guestLeft[i] == nfs_front_hit_original[i].left);
    }
    std::printf("PASS default-off renderer-independent mode, concurrent publication, "
                "%u signature negatives, null guard, normal/unwind restoration\n", negatives);
}

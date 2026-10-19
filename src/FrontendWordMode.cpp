// SPDX-License-Identifier: MIT
#include "FrontendWordMode.h"
#include <atomic>

namespace {
std::atomic<int> available{0};
}

extern "C" void nfs_front_word_set_available(int value) {
    available.store(value != 0, std::memory_order_release);
}

extern "C" int nfs_front_word_hit_active(void) {
    return available.load(std::memory_order_acquire);
}

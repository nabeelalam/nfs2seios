// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <stdint.h>
#include <stddef.h>
struct NfsFrameSpanToken {
    void *host=nullptr;
    uint32_t base=0;
    const std::atomic<uint64_t>*revision=nullptr;
    uint64_t revisionValue=0;
    const std::atomic<uint64_t>*binding=nullptr;
    uint64_t bindingValue=0;
};
extern "C" bool nfs_probe_frame_span(uint32_t,size_t,NfsFrameSpanToken*) noexcept;

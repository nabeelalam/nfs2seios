// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <mutex>

namespace nfs {
std::recursive_mutex *codec_operation_mutex(uint32_t header);

// Initialization and complete decoding jobs share one lock per original voice.
// Acquire before operand-copy locks. Buffered decoding can recurse on the same
// state; engine ownership keeps the mutex alive until all workers have joined.
class CodecOperationGuard {
    std::unique_lock<std::recursive_mutex> lock;
public:
    explicit CodecOperationGuard(uint32_t header){
        if(auto mutex=codec_operation_mutex(header))
            lock=std::unique_lock<std::recursive_mutex>(*mutex);
    }
};
}

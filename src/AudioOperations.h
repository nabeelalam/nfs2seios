// SPDX-License-Identifier: MIT
#pragma once
#include <mutex>

namespace nfs {
// Original sound jobs share mixer scratch storage across voices. Acquire this
// operation lock before the per-voice codec and checked-copy locks. Original
// stop/fade helpers can nest on the same thread, so ownership is recursive.
inline std::recursive_mutex &audio_operation_mutex(){
    static std::recursive_mutex value;
    return value;
}

class AudioOperationGuard {
    std::unique_lock<std::recursive_mutex> lock;
public:
    AudioOperationGuard():lock(audio_operation_mutex()){}
};
}

// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
namespace nfs {
inline uint32_t &platform_error() {
    static thread_local uint32_t value=0;
    return value;
}
}

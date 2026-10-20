// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
uint64_t nfs_front_text_context(void);
void nfs_front_text_context_set(uint64_t identity);
#ifdef __cplusplus
}

namespace nfs {
// Content identity for optional artwork selection, not a security primitive.
// Unsupported, unterminated or invalid guest text keeps the original font.
template<class Reader> uint64_t frontend_text_identity(uint32_t address, Reader read) {
    if (!address) return 0;
    uint64_t hash = UINT64_C(14695981039346656037);
    try {
        for (unsigned i = 0; i < 96; ++i) {
            if (address > UINT32_MAX-i) return 0;
            const uint8_t c = read(address+i);
            if (!c) return i ? hash : 0;
            if (c < 32 || c > 126) return 0;
            hash = (hash ^ c) * UINT64_C(1099511628211);
        }
    } catch (...) {
        return 0;
    }
    return 0;
}

class FrontendTextScope {
    const uint64_t previous;
public:
    explicit FrontendTextScope(uint64_t identity) : previous(nfs_front_text_context()) {
        nfs_front_text_context_set(identity);
    }
    ~FrontendTextScope() { nfs_front_text_context_set(previous); }
    FrontendTextScope(const FrontendTextScope &) = delete;
    FrontendTextScope &operator=(const FrontendTextScope &) = delete;
};
}
#endif

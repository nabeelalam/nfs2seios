// SPDX-License-Identifier: MIT
#include "MemoryWrappers.h"
#include "GuestAccess.h"
#include <stddef.h>
#include <cstring>
extern "C" void *malloc32(size_t);
extern "C" void free32(void *);
extern "C" int32_t malloc_wrap(int32_t size) {
    void *p = malloc32(static_cast<uint32_t>(size));
    return p ? static_cast<int32_t>(nfs_guest_address(p)) : 0;
}
extern "C" int32_t calloc_wrap(int32_t count, int32_t size) {
    const uint64_t bytes = uint64_t(static_cast<uint32_t>(count)) * static_cast<uint32_t>(size);
    if (bytes > UINT32_MAX) return 0;
    void *p = malloc32(static_cast<size_t>(bytes));
    if (!p) return 0;
    std::memset(p, 0, static_cast<size_t>(bytes));
    return static_cast<int32_t>(nfs_guest_address(p));
}
extern "C" int32_t free_wrap(int32_t address) {
    if (address) free32(nfs_host_address(static_cast<uint32_t>(address), 1));
    return 0;
}

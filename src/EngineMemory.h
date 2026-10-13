// SPDX-License-Identifier: MIT
#pragma once
#include "GuestAccess.h"
#include <memory>
extern "C" void *malloc32(size_t);
extern "C" void free32(void *);
namespace nfs {
// One original game image/session. DATA, BSS and the heap are mapped before
// workers run. All contexts/resources must be retired before destruction.
class EngineMemory {
    struct Impl;std::unique_ptr<Impl> impl;
public:
    explicit EngineMemory(size_t heap_bytes);
    ~EngineMemory();
    void *resolve(uint32_t,size_t);
    uint32_t encode(const void *);
    void *allocate(size_t);
    void release(void *);
    bool add(uint32_t,NativeGameFunction);
    NativeGameFunction function(uint32_t);
    size_t capacity()const;
    size_t available()const;
};
}

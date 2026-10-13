// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>

namespace nfs {
// A proposed boundary for translated game data, not a CPU interpreter.
// Registration happens before game threads start. All views remain caller-owned.
struct Region { uint32_t base; size_t size; uint8_t *host; };
class GameMemory {
    Region regions[32]{};
    size_t count = 0;
public:
    bool map(uint32_t base, uint8_t *host, size_t size) {
        if (!base || !host || !size || size > UINT32_MAX ||
            uint64_t(base) + size > uint64_t(UINT32_MAX) + 1 || count == 32) return false;
        if (size - 1 > UINTPTR_MAX - reinterpret_cast<uintptr_t>(host)) return false;
        for (size_t i = 0; i < count; ++i) {
            const Region &r = regions[i];
            if (uint64_t(base) < uint64_t(r.base) + r.size &&
                uint64_t(r.base) < uint64_t(base) + size) return false;
            const uintptr_t a = reinterpret_cast<uintptr_t>(host);
            const uintptr_t b = reinterpret_cast<uintptr_t>(r.host);
            if ((a >= b && a - b < r.size) || (b >= a && b - a < size)) return false;
        }
        regions[count++] = {base, size, host};
        return true;
    }
    bool unmap(uint32_t base) {
        for (size_t i = 0; i < count; ++i) {
            if (regions[i].base == base) {
                for (size_t j = i + 1; j < count; ++j) regions[j-1] = regions[j];
                --count;
                return true;
            }
        }
        return false;
    }
    uint8_t *resolve(uint32_t address, size_t length = 1) const {
        if (!address || !length) return nullptr;
        for (size_t i = 0; i < count; ++i) {
            const Region &r = regions[i];
            if (address >= r.base) {
                const size_t offset = address - r.base;
                if (offset < r.size && length <= r.size - offset) return r.host + offset;
            }
        }
        return nullptr;
    }
    bool encode(const void *pointer, uint32_t &address) const {
        if (!pointer) { address = 0; return true; }
        const uintptr_t p = reinterpret_cast<uintptr_t>(pointer);
        for (size_t i = 0; i < count; ++i) {
            const Region &r = regions[i];
            const uintptr_t begin = reinterpret_cast<uintptr_t>(r.host);
            if (p >= begin && p - begin < r.size) {
                address = r.base + uint32_t(p - begin);
                return true;
            }
        }
        return false; // Never silently truncate an unregistered host pointer.
    }
    bool load32(uint32_t address, uint32_t &value) const {
        const uint8_t *p = resolve(address, 4);
        if (!p) return false;
        value = uint32_t(p[0]) | (uint32_t(p[1]) << 8) |
            (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
        return true;
    }
    bool store32(uint32_t address, uint32_t value) const {
        uint8_t *p = resolve(address, 4);
        if (!p) return false;
        for (unsigned i = 0; i < 4; ++i) p[i] = uint8_t(value >> (8 * i));
        return true;
    }
};
// Separate function IDs from data addresses. Callable code retains full pointers.
using GameFunction = void (*)(void *);
class Functions {
    struct Entry { uint32_t id; GameFunction fn; } entries[4096]{};
    size_t count = 0;
    size_t position(uint32_t id) const {
        size_t low=0,high=count;
        while(low<high){
            const size_t middle=low+(high-low)/2;
            if(entries[middle].id<id)low=middle+1;
            else high=middle;
        }
        return low;
    }
public:
    bool add(uint32_t id, GameFunction fn) {
        if (!id || !fn || count == 4096) return false;
        const size_t at=position(id);
        if(at<count && entries[at].id==id)return false;
        // Registration precedes all dispatch threads. Maintain order once so
        // audio and other callbacks do not scan thousands of entries per call.
        for(size_t i=count;i>at;--i)entries[i]=entries[i-1];
        entries[at]={id,fn};++count;
        return true;
    }
    GameFunction find(uint32_t id) const {
        const size_t at=position(id);
        return at<count && entries[at].id==id?entries[at].fn:nullptr;
    }
};
}

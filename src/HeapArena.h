// SPDX-License-Identifier: MIT
#pragma once
#include "GameMemory.h"
#include <mutex>
#include <cstring>
#include <atomic>

namespace nfs {
// One stable guest mapping, many native allocations. Metadata contains native
// sizes and never enters game memory. Caller owns the arena buffer and must
// stop all users before teardown. GameMemory mappings stay fixed during play.
class HeapArena {
    struct Block { size_t offset, size; bool used; } blocks[2048]{};
    size_t count = 0;
    uint8_t *storage = nullptr;
    size_t capacity = 0;
    uint32_t base = 0;
    GameMemory *memory = nullptr;
    mutable std::mutex mutex;
    static uint64_t next_identity(){static std::atomic<uint64_t> next{1};return next.fetch_add(1,std::memory_order_relaxed);}
    const uint64_t identity=next_identity();
    std::atomic<uint64_t> revision{1};

    void *allocate_locked(size_t size) {
        if (!storage || size > SIZE_MAX - 15) return nullptr;
        size = (size ? size : 1) + 15;
        size &= ~size_t(15);
        for (size_t i = 0; i < count; ++i) {
            Block &b = blocks[i];
            if (b.used || b.size < size) continue;
            if (b.size != size) {
                if (count == 2048) continue;
                for (size_t j = count; j > i+1; --j) blocks[j] = blocks[j-1];
                blocks[i+1] = {b.offset + size, b.size - size, false};
                ++count;
            }
            b.size = size;
            b.used = true;
            revision.fetch_add(1,std::memory_order_release);
            return storage + b.offset;
        }
        return nullptr;
    }
    size_t find(const void *p) const {
        const uintptr_t value = reinterpret_cast<uintptr_t>(p);
        const uintptr_t begin = reinterpret_cast<uintptr_t>(storage);
        if (!storage || value < begin || value - begin >= capacity) return count;
        for (size_t i = 0; i < count; ++i)
            if (blocks[i].used && blocks[i].offset == value - begin) return i;
        return count;
    }
    bool release_locked(void *p) {
        if (!p) return true;
        size_t i = find(p);
        if (i == count) return false;
        blocks[i].used = false;
        for (size_t j = 0; j+1 < count;) {
            if (!blocks[j].used && !blocks[j+1].used) {
                blocks[j].size += blocks[j+1].size;
                for (size_t k = j+2; k < count; ++k) blocks[k-1] = blocks[k];
                --count;
            } else ++j;
        }
        revision.fetch_add(1,std::memory_order_release);
        return true;
    }
public:
    HeapArena() = default;
    HeapArena(const HeapArena &) = delete;
    HeapArena &operator=(const HeapArena &) = delete;
    ~HeapArena() { if (memory) memory->unmap(base); }
    bool initialize(GameMemory &space, uint32_t guest_base, uint8_t *buffer, size_t size) {
        std::lock_guard<std::mutex> lock(mutex);
        if (storage || !size || size % 16 || guest_base % 16 ||
            reinterpret_cast<uintptr_t>(buffer) % 16 || !space.map(guest_base, buffer, size)) return false;
        memory = &space; storage = buffer; capacity = size; base = guest_base;
        blocks[0] = {0, size, false}; count = 1;
        return true;
    }
    void *allocate(size_t size) {
        std::lock_guard<std::mutex> lock(mutex);
        return allocate_locked(size);
    }
    void *allocate_zeroed(size_t elements, size_t size) {
        if (elements && size > SIZE_MAX / elements) return nullptr;
        std::lock_guard<std::mutex> lock(mutex);
        const size_t total = elements * size;
        void *p = allocate_locked(total);
        if (p) std::memset(p, 0, total);
        return p;
    }
    bool release(void *p) {
        std::lock_guard<std::mutex> lock(mutex);
        return release_locked(p);
    }
    void *resize(void *p, size_t size) {
        std::lock_guard<std::mutex> lock(mutex);
        if (!p) return allocate_locked(size);
        size_t i = find(p);
        if (i == count) return nullptr;
        if (!size) { release_locked(p); return nullptr; }
        if (size <= blocks[i].size) return p;
        const size_t old_size = blocks[i].size;
        void *next = allocate_locked(size);
        if (!next) return nullptr; // Original allocation/data remain intact.
        std::memcpy(next, p, old_size);
        release_locked(p);
        return next;
    }
    bool contains(uint32_t address, size_t length) const {
        if (!storage || address < base || !length) return false;
        const size_t offset = address - base;
        // Cache only a copied allocation span, never mutable table pointers.
        // Every allocation/free invalidates all cached spans. Distinct lifetime
        // identities also prevent reuse of a destroyed arena's native address.
        struct Span {size_t offset=0,size=0;uint64_t revision=0;};
        struct Cache {const HeapArena *owner=nullptr;uint64_t identity=0;unsigned next=0;Span spans[4];};
        static thread_local Cache cache;
        if(cache.owner!=this || cache.identity!=identity){cache=Cache{};cache.owner=this;cache.identity=identity;}
        const auto observed=revision.load(std::memory_order_acquire);
        for(const auto &span:cache.spans)
            if(span.revision==observed && offset>=span.offset && offset-span.offset<span.size && length<=span.size-(offset-span.offset))return true;
        std::lock_guard<std::mutex> lock(mutex);
        // Splits and coalescing preserve ascending offsets. Every guest load
        // checks this table; locate its block without scanning all allocations.
        size_t low = 0, high = count;
        while (low < high) {
            const size_t middle = low + (high - low) / 2;
            if (blocks[middle].offset <= offset) low = middle + 1;
            else high = middle;
        }
        if (!low) return false;
        const Block &b = blocks[low - 1];
        const bool valid=b.used && offset-b.offset<b.size && length<=b.size-(offset-b.offset);
        if(valid){cache.spans[cache.next]={b.offset,b.size,revision.load(std::memory_order_relaxed)};cache.next=(cache.next+1)%4;}
        return valid;
    }
    size_t total_bytes() const { std::lock_guard<std::mutex> lock(mutex); return capacity; }
    size_t available_bytes() const {
        std::lock_guard<std::mutex> lock(mutex);
        size_t available=0;
        for (size_t i=0;i<count;++i) if (!blocks[i].used) available+=blocks[i].size;
        return available;
    }
};
}

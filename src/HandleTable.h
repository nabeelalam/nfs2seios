// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <memory>
#include <mutex>
#include <utility>
namespace nfs {
// Native resources never become game pointers. Generation IDs reject stale
// handles; a slot is retired rather than wrapping its generation. Resource
// access and close are serialized so close cannot race a native operation.
template<class T,uint32_t Tag=0xe0000000> class HandleTable {
    struct Slot { std::shared_ptr<T> resource; uint32_t generation = 0; } slots[256];
    std::mutex mutex;
    static constexpr uint32_t tag = Tag;
    bool valid(uint32_t id) const {
        return (id & 0xff000000) == tag && slots[id & 255].resource &&
            slots[id & 255].generation == ((id >> 8) & 65535);
    }
public:
    uint32_t insert(std::unique_ptr<T> resource) {
        std::lock_guard<std::mutex> lock(mutex);
        if (!resource) return 0;
        for (uint32_t i=0; i<256; ++i) {
            Slot &slot = slots[i];
            if (slot.resource || slot.generation == 65535) continue;
            ++slot.generation; slot.resource = std::move(resource);
            return tag | (slot.generation << 8) | i;
        }
        return 0;
    }
    template<class F> auto with(uint32_t id, F operation) -> decltype(operation(static_cast<T *>(nullptr))) {
        std::lock_guard<std::mutex> lock(mutex);
        return operation(valid(id) ? slots[id & 255].resource.get() : nullptr);
    }
    bool close(uint32_t id) {
        std::lock_guard<std::mutex> lock(mutex);
        if (!valid(id)) return false;
        slots[id & 255].resource.reset();
        return true;
    }
    std::shared_ptr<T> share(uint32_t id){std::lock_guard<std::mutex> lock(mutex);return valid(id)?slots[id&255].resource:nullptr;}
};
}

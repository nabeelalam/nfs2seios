// SPDX-License-Identifier: MIT
#pragma once

#include <atomic>
#include <cstdint>

namespace nfs {
// Draw calls capture this scope before crossing the renderer queue. The
// renderer also links independently for pixel tests, where the scope is zero.
class AdaptiveRaceDrawScope {
    static unsigned &slot() {
        static thread_local unsigned scope = 0;
        return scope;
    }

    unsigned previous_;

public:
    AdaptiveRaceDrawScope(int mode, int player) : previous_(slot()) {
        const bool valid = mode >= 0 && mode < 4 && player >= 0 && player < 2;
        slot() = valid ? 1u + unsigned(mode) * 2u + unsigned(player) : 0u;
    }

    ~AdaptiveRaceDrawScope() { slot() = previous_; }

    AdaptiveRaceDrawScope(const AdaptiveRaceDrawScope &) = delete;
    AdaptiveRaceDrawScope &operator=(const AdaptiveRaceDrawScope &) = delete;

    static unsigned current() { return slot(); }
};

struct AdaptiveRaceExtent {
    int32_t width=0,height=0;
    bool operator==(const AdaptiveRaceExtent &other)const {
        return width==other.width && height==other.height;
    }
    explicit operator bool()const{return width>0 && height>0;}
};

// The renderer offers dimensions. The engine keeps one committed extent for
// all camera calls in a frame. A resize is committed after the renderer has
// adopted the corresponding attachment, so a later offer cannot alter the
// projection of work already queued for this frame.
class AdaptiveRaceTarget {
    std::atomic<uint64_t> offered_{0},active_{0};
    static uint64_t pack(AdaptiveRaceExtent size) {
        return size?(uint64_t(size.width)<<32)|uint32_t(size.height):0;
    }
    static AdaptiveRaceExtent unpack(uint64_t packed) {
        return {int32_t(packed>>32),int32_t(uint32_t(packed))};
    }
public:
    void offer(AdaptiveRaceExtent size){offered_.store(pack(size),std::memory_order_release);}
    AdaptiveRaceExtent offered()const{return unpack(offered_.load(std::memory_order_acquire));}
    AdaptiveRaceExtent active()const{return unpack(active_.load(std::memory_order_acquire));}
    void commit(AdaptiveRaceExtent size){active_.store(pack(size),std::memory_order_release);}
};
}


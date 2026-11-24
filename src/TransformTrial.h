// SPDX-License-Identifier: MIT
#pragma once
#include <atomic>
#include <cstdint>
#include <limits>
namespace nfs {
// Only an instrumented Game backend registers this trial. Normal builds stay
// unavailable. This labels requested dispatch configuration, not executed work.
struct TransformTrialSnapshot {
    uint64_t token=0;
    int mode()const noexcept{return int(token&3)-1;} // -1 unavailable, 0 baseline, 1 cache
    uint64_t generation()const noexcept{return token>>2;}
};
class TransformTrialControl {
    std::atomic<uint64_t> state{0};
public:
    void register_backend()noexcept {
        uint64_t absent=0;state.compare_exchange_strong(absent,5,std::memory_order_relaxed);
    }
    TransformTrialSnapshot snapshot()const noexcept{return {state.load(std::memory_order_relaxed)};}
    bool set_cached(bool enabled)noexcept {
        auto old=state.load(std::memory_order_relaxed);
        for(;;){
            if(!(old&3))return false;
            const uint64_t mode=enabled?2:1;
            if((old&3)==mode)return true;
            if(old>std::numeric_limits<uint64_t>::max()-4)return false;
            const uint64_t next=((old&~uint64_t(3))+4)|mode;
            if(state.compare_exchange_weak(old,next,std::memory_order_relaxed))return true;
        }
    }
};
inline TransformTrialControl &transform_trial_control()noexcept {
    static TransformTrialControl instance;return instance;
}
class TransformTrialInterval {
    uint64_t previous=0;bool known=false;
public:
    bool observe(TransformTrialSnapshot current)noexcept {
        const bool stable=known && current.mode()>=0 && previous==current.token;
        known=true;previous=current.token;return stable;
    }
};
}

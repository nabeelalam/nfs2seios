// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <memory>
#include <mutex>
namespace nfs {
class TimerBridge {
public:
    struct State;
    using Tick=uint32_t(*)(void *);
private:
    std::mutex operation;
    std::shared_ptr<State> state;
public:
    void start(Tick tick,void *context);
    // Remove the dispatch token, cancel backend scheduling, then drain callbacks
    // before the runtime destroys its event/memory/context bindings.
    void stop();
    ~TimerBridge(){try{stop();}catch(...){}}
};
class GameTimerSchedule {
    uint32_t scheduled=0,fraction=0;
public:
    uint32_t next(uint32_t step,uint32_t now);
};
}
extern "C" uint32_t nfs_native_add_timer(uintptr_t token);
extern "C" void nfs_native_remove_timer(uint32_t id);
extern "C" uint32_t nfs_timer_dispatch(uintptr_t token);
extern "C" int32_t startTimer();
extern "C" int32_t stopTimer();

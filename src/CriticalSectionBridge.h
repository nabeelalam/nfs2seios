// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unordered_map>
namespace nfs {
// Native synchronization state stays outside the game's opaque 24-byte object.
class CriticalSectionBridge {
    struct Section {
        std::mutex mutex;
        std::condition_variable changed;
        std::thread::id owner;
        uint32_t depth=0,waiters=0;
        bool deleted=false;
    };
    std::mutex registry_mutex;
    std::unordered_map<uint32_t,std::shared_ptr<Section>> sections;
    std::shared_ptr<Section> find(uint32_t address);
public:
    void initialize(uint32_t address);
    void enter(uint32_t address);
    void leave(uint32_t address);
    void remove(uint32_t address);
    void wake_workers();
    void retire_current_thread(); // Only after that owned game thread stops execution.
};
// Bind before workers start; destroy/unbind after all workers have exited.
void bind_critical_sections(CriticalSectionBridge *bridge);
}
extern "C" int32_t InitializeCriticalSection_wrap(int32_t);
extern "C" int32_t EnterCriticalSection_wrap(int32_t);
extern "C" int32_t LeaveCriticalSection_wrap(int32_t);
extern "C" int32_t DeleteCriticalSection_wrap(int32_t);

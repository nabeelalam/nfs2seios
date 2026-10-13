// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mutex>
#include <condition_variable>
namespace nfs {
// Event IDs occupy a distinct domain from files and function callbacks.
// A single lock makes wait-all consumption atomic and avoids lost wakeups.
class EventBridge {
    struct Slot { uint32_t generation=0; bool alive=false,manual=false,signaled=false; } slots[256];
    std::mutex mutex;
    std::condition_variable changed;
    bool stopping=false;
    bool valid(uint32_t id) const;
public:
    uint32_t create(bool manual,bool signaled);
    bool signal(uint32_t id);
    bool close(uint32_t id);
    uint32_t wait(const uint32_t *ids,size_t count,bool all,uint32_t milliseconds);
    // Wake game workers before joining them; destroy only after they exit.
    void shutdown();
    void wake_workers();
};
// Bind before starting workers and unbind only after shutdown and joins.
void bind_event_bridge(EventBridge *bridge);
bool close_event_handle(uint32_t id);
}
extern "C" int32_t CreateEventA_wrap(int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t SetEvent_wrap(int32_t);
extern "C" int32_t WaitForMultipleObjects_wrap(int32_t,int32_t,int32_t,int32_t);

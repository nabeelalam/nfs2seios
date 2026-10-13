// SPDX-License-Identifier: MIT
#include "EventBridge.h"
#include "GuestAccess.h"
#include "PlatformError.h"
#include "WorkerControl.h"
#include <chrono>
namespace nfs {
constexpr uint32_t event_tag=0xd0000000;
bool EventBridge::valid(uint32_t id) const {
    return (id&0xff000000)==event_tag && slots[id&255].alive &&
        slots[id&255].generation==((id>>8)&65535);
}
uint32_t EventBridge::create(bool manual,bool signaled) {
    std::lock_guard<std::mutex> lock(mutex);
    if (stopping) return 0;
    for (uint32_t i=0;i<256;++i) {
        auto &slot=slots[i];
        if (slot.alive || slot.generation==65535) continue;
        ++slot.generation; slot.alive=true; slot.manual=manual; slot.signaled=signaled;
        return event_tag|(slot.generation<<8)|i;
    }
    return 0;
}
bool EventBridge::signal(uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex);
    if (stopping || !valid(id)) return false;
    slots[id&255].signaled=true;
    changed.notify_all();
    return true;
}
bool EventBridge::close(uint32_t id) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!valid(id)) return false;
    slots[id&255].alive=false;
    changed.notify_all();
    return true;
}
uint32_t EventBridge::wait(const uint32_t *ids,size_t count,bool all,uint32_t milliseconds) {
    if (!ids || !count || count>64) { platform_error()=87; return UINT32_MAX; }
    for (size_t i=0;i<count;++i) for (size_t j=0;j<i;++j)
        if (ids[i]==ids[j]) { platform_error()=87; return UINT32_MAX; }
    std::unique_lock<std::mutex> lock(mutex);
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(milliseconds);
    for (;;) {
        check_worker_stop();
        if (stopping) { platform_error()=6; return UINT32_MAX; }
        for (size_t i=0;i<count;++i)
            if (!valid(ids[i])) { platform_error()=6; return UINT32_MAX; }
        bool ready=all;
        size_t first=count;
        for (size_t i=0;i<count;++i) {
            if (slots[ids[i]&255].signaled) { if(first==count) first=i; }
            else if (all) ready=false;
        }
        if ((!all && first!=count) || ready) {
            if (all) {
                for (size_t i=0;i<count;++i) if(!slots[ids[i]&255].manual) slots[ids[i]&255].signaled=false;
            } else if(!slots[ids[first]&255].manual) slots[ids[first]&255].signaled=false;
            platform_error()=0;
            return all?0:uint32_t(first);
        }
        if (!milliseconds || (milliseconds!=UINT32_MAX && std::chrono::steady_clock::now()>=deadline)) {
            platform_error()=0; return 258;
        }
        if (milliseconds==UINT32_MAX) changed.wait(lock);
        else changed.wait_until(lock,deadline);
    }
}
void EventBridge::shutdown() {
    std::lock_guard<std::mutex> lock(mutex);
    stopping=true; changed.notify_all();
}
static EventBridge *active=nullptr;
void EventBridge::wake_workers(){std::lock_guard<std::mutex> lock(mutex);changed.notify_all();}
static void wake(){if(active)active->wake_workers();}
void bind_event_bridge(EventBridge *bridge) { active=bridge;event_worker_wake()=bridge?wake:nullptr; }
bool close_event_handle(uint32_t id) { return active && active->close(id); }
}
extern "C" int32_t CreateEventA_wrap(int32_t security,int32_t manual,int32_t signaled,int32_t name) {
    if (security || name || !nfs::active) { nfs::platform_error()=50; return 0; }
    const uint32_t id=nfs::active->create(manual!=0,signaled!=0);
    nfs::platform_error()=id?0:8;
    return int32_t(id);
}
extern "C" int32_t SetEvent_wrap(int32_t id) {
    const bool ok=nfs::active && nfs::active->signal(uint32_t(id));
    nfs::platform_error()=ok?0:6; return ok;
}
extern "C" int32_t WaitForMultipleObjects_wrap(int32_t count,int32_t handles,int32_t all,int32_t milliseconds) {
    if (!nfs::active || count<1 || count>64) { nfs::platform_error()=87; return -1; }
    uint32_t ids[64];
    nfs_host_address(uint32_t(handles),size_t(count)*4);
    for (int32_t i=0;i<count;++i) ids[i]=guest_load<uint32_t>(uint32_t(handles)+uint32_t(i)*4);
    return int32_t(nfs::active->wait(ids,size_t(count),all!=0,uint32_t(milliseconds)));
}

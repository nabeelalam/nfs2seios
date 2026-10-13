// SPDX-License-Identifier: MIT
#include "TimerBridge.h"
#include "ClockWrappers.h"
#include "GuestAccess.h"
#include "EventBridge.h"
#include "DATA.h"
#include "BSS.h"
namespace {
nfs::TimerBridge &timer(){static nfs::TimerBridge value;return value;}
nfs::GameTimerSchedule schedule;
uint32_t tick(void *){
    const uint32_t interval=schedule.next(guest_load<uint32_t>(dword_5637A0),uint32_t(nfs_native_ticks()));
    const uint32_t first=guest_load<uint32_t>(dword_5637CC);
    const uint32_t second=guest_load<uint32_t>(dword_5637D8);
    const uint32_t blocked=guest_load<uint32_t>(dword_4DB1B0);
    const uint32_t third=guest_load<uint32_t>(dword_4DDA70);
    if(first)SetEvent_wrap(int32_t(first));
    if(second && !blocked)SetEvent_wrap(int32_t(second));
    if(third)SetEvent_wrap(int32_t(third));
    return interval;
}
}
extern "C" int32_t startTimer(){timer().start(tick,nullptr);return 0;}
extern "C" int32_t stopTimer(){timer().stop();return 0;}

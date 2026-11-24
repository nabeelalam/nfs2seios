// SPDX-License-Identifier: MIT
#pragma once
#include "GuestAccess.h"
namespace nfs {
// Same-thread scope for bounded arithmetic approved by the static generator.
// Do not call other guest-copy helpers, allocate/free game memory, block or
// dispatch callbacks while it exists. Each accessor still resolves its full
// span; callers must retain the allocation's lifetime as before.
class GuestCopyTransaction {
    std::unique_lock<std::mutex> lock;
    // Cache this thread's slot address, never a WorkerControl value.
    WorkerControl **const control_slot;
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((noinline))
#endif
    static void handle_requested_stop(){
        if(!owned_critical_depth() && !worker_stop_deferrals())throw WorkerStopped();
    }
    void check_stop(){
        WorkerControl *control=*control_slot;
        if(control && control->stop.load())handle_requested_stop();
    }

public:
    GuestCopyTransaction():lock(guest_copy_mutex(),std::defer_lock),control_slot(&current_worker_control()){check_stop();lock.lock();}
    GuestCopyTransaction(const GuestCopyTransaction &)=delete;
    GuestCopyTransaction &operator=(const GuestCopyTransaction &)=delete;
    template<class T,class A> T load(A address){
        check_stop();
        static_assert(!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value,"Checked packed values only");
        T value;__builtin_memcpy(&value,guest_host_span(address,sizeof(T)),sizeof(T));return value;
    }
    template<class T,class A> void store(A address,T value){
        check_stop();
        static_assert(!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value,"Checked packed values only");
        __builtin_memcpy(guest_host_span(address,sizeof(T)),&value,sizeof(T));
    }
};
}

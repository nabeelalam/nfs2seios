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
    // TLS object addresses are stable on this thread; values stay live.
    WorkerControl **control_slot;
    unsigned *critical_depth_slot,*stop_deferrals_slot;
    void check_stop(){
        WorkerControl *control=*control_slot;
        if(control && control->stop.load() && !*critical_depth_slot && !*stop_deferrals_slot)
            throw WorkerStopped();
    }
public:
    GuestCopyTransaction():lock(guest_copy_mutex(),std::defer_lock),
        control_slot(&current_worker_control()),
        critical_depth_slot(&owned_critical_depth()),
        stop_deferrals_slot(&worker_stop_deferrals()){
        check_stop();lock.lock();
    }
    GuestCopyTransaction(const GuestCopyTransaction &)=delete;
    GuestCopyTransaction &operator=(const GuestCopyTransaction &)=delete;
    template<class T,class A> T load(A address){
        check_stop();
        static_assert(!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value,"Checked packed values only");
        T value;__builtin_memcpy(&value,nfs_host_address(guest_address(address),sizeof(T)),sizeof(T));return value;
    }
    template<class T,class A> void store(A address,T value){
        check_stop();
        static_assert(!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value,"Checked packed values only");
        __builtin_memcpy(nfs_host_address(guest_address(address),sizeof(T)),&value,sizeof(T));
    }
};
}

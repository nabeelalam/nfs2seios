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
public:
    GuestCopyTransaction():lock(guest_copy_mutex(),std::defer_lock){check_worker_stop();lock.lock();}
    GuestCopyTransaction(const GuestCopyTransaction &)=delete;
    GuestCopyTransaction &operator=(const GuestCopyTransaction &)=delete;
    template<class T,class A> T load(A address){
        check_worker_stop();
        static_assert(!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value,"Checked packed values only");
        T value;__builtin_memcpy(&value,nfs_host_address(guest_address(address),sizeof(T)),sizeof(T));return value;
    }
    template<class T,class A> void store(A address,T value){
        check_worker_stop();
        static_assert(!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value,"Checked packed values only");
        __builtin_memcpy(nfs_host_address(guest_address(address),sizeof(T)),&value,sizeof(T));
    }
};
}

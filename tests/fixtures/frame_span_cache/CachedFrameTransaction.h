// SPDX-License-Identifier: MIT
#pragma once
#include "GuestCopyTransaction.h"
#include "FrameSpanCacheAbi.h"
#ifdef NFS_REGION_CACHE_TEST
extern "C" void nfs_cache_test_span(uint32_t,size_t);
extern "C" void nfs_cache_test_backend();
extern "C" void *nfs_raw_host_address(uint32_t,size_t);
#endif
namespace nfs {
// Same bounded, same-thread and allocation-lifetime contract as the owner.
// No raw pointer/token escapes. Metadata probes occur only after an original
// successful access and never replace its error or cancellation checkpoint.
class CachedFrameTransaction {
    GuestCopyTransaction owner;
    NfsFrameSpanToken frame{};
    uint32_t base;
    bool attempted=false;
    void *resolve(uint32_t address,size_t length){
#ifdef NFS_REGION_CACHE_TEST
        nfs_cache_test_span(address,length);
#endif
        const uint32_t offset=address-base;
        if(frame.host && offset<64 && length<=64-offset &&
           frame.binding->load(std::memory_order_acquire)==frame.bindingValue &&
           frame.revision->load(std::memory_order_acquire)==frame.revisionValue)
            return static_cast<unsigned char*>(frame.host)+offset;
#ifdef NFS_REGION_CACHE_TEST
        nfs_cache_test_backend();
        void *pointer=nfs_raw_host_address(address,length);
#else
        void *pointer=guest_host_span(address,length);
#endif
        if(!attempted && offset<64 && length<=64-offset){attempted=true;nfs_probe_frame_span(base,64,&frame);}
        return pointer;
    }
public:
    explicit CachedFrameTransaction(uint32_t address):base(address){}
    CachedFrameTransaction(const CachedFrameTransaction&)=delete;
    CachedFrameTransaction&operator=(const CachedFrameTransaction&)=delete;
    template<class T,class A>typename std::enable_if<std::is_integral<A>::value,T>::type load(A address){
        check_worker_stop();
        static_assert(!std::is_pointer<T>::value&&std::is_trivially_copyable<T>::value,"Checked packed values only");
        T value;__builtin_memcpy(&value,resolve(static_cast<uint32_t>(address),sizeof(T)),sizeof(T));return value;
    }
    template<class T,class A>typename std::enable_if<!std::is_integral<A>::value,T>::type load(A address){return owner.load<T>(address);}
    template<class T,class A>typename std::enable_if<std::is_integral<A>::value,void>::type store(A address,T value){
        check_worker_stop();
        static_assert(!std::is_pointer<T>::value&&std::is_trivially_copyable<T>::value,"Checked packed values only");
        __builtin_memcpy(resolve(static_cast<uint32_t>(address),sizeof(T)),&value,sizeof(T));
    }
    template<class T,class A>typename std::enable_if<!std::is_integral<A>::value,void>::type store(A address,T value){owner.store<T>(address,value);}
};
}

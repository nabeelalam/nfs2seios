// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <type_traits>
#include <mutex>
#include "GuestRuntimeC.h"
#include "WorkerControl.h"
#if !defined(__BYTE_ORDER__) || __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error "Mapped game operands currently require a little-endian host"
#endif
// Experimental engine boundary. A runtime must register the full requested span.
using NativeGameFunction = void (*)(void *);
extern "C" NativeGameFunction nfs_lookup_guest_function(uint32_t function_id);
template<class T> typename std::enable_if<std::is_integral<T>::value, uint32_t>::type
guest_address(T value) { return static_cast<uint32_t>(value); }
template<class T> uint32_t guest_address(T *pointer) { return nfs_guest_address(pointer); }
// Timer and game workers share packed fields. Serialize value copies, including
// unaligned values; a native atomic_ref would require alignment these fields lack.
inline std::mutex &guest_copy_mutex() {static std::mutex mutex;return mutex;}
template<class T, class A> T guest_load(A address) {
    nfs::check_worker_stop();
    static_assert(!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value,
        "Guest values must be trivially copyable; native pointers require explicit encoding");
    T value;
    std::lock_guard<std::mutex> lock(guest_copy_mutex());
    __builtin_memcpy(&value, nfs_host_address(guest_address(address), sizeof(T)), sizeof(T));
    return value;
}
template<class T, class A> void guest_store(A address, T value) {
    nfs::check_worker_stop();
    static_assert(!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value,
        "Guest values must be trivially copyable; native pointers require explicit encoding");
    std::lock_guard<std::mutex> lock(guest_copy_mutex());
    __builtin_memcpy(nfs_host_address(guest_address(address), sizeof(T)), &value, sizeof(T));
}

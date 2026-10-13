// SPDX-License-Identifier: MIT
#include "SystemWrappers.h"
#include "GuestAccess.h"
#include <thread>
struct GuestSystemInfo {
    uint16_t architecture, reserved;
    uint32_t page_size, minimum_address, maximum_address, processor_mask;
    uint32_t processor_count, processor_type, allocation_granularity;
    uint16_t processor_level, processor_revision;
};
static_assert(sizeof(GuestSystemInfo)==36,"SYSTEM_INFO must retain its 32-bit guest layout");
extern "C" int32_t GetSystemInfo_wrap(int32_t address) {
    GuestSystemInfo info{};
    info.architecture=5; // ARM; the structure remains the original 32-bit ABI.
    info.page_size=nfs_native_page_size();
    info.processor_count=std::thread::hardware_concurrency();
    if (!info.processor_count) info.processor_count=1;
    info.processor_mask=info.processor_count>=32?UINT32_MAX:(uint32_t(1)<<info.processor_count)-1;
    info.allocation_granularity=info.page_size;
    guest_store<GuestSystemInfo>(uint32_t(address),info);
    return 0;
}
extern "C" int32_t GlobalMemoryStatus_wrap(int32_t address) {
    const uint32_t total=nfs_game_heap_capacity(), free=nfs_game_heap_available();
    const uint32_t available=free>total?total:free;
    struct GuestMemoryStatus { uint32_t fields[8]; } info{};
    static_assert(sizeof(info)==32,"MEMORYSTATUS guest layout");
    info.fields[0]=32;
    info.fields[1]=total?uint32_t(100-(uint64_t(available)*100/total)):100;
    info.fields[2]=info.fields[4]=info.fields[6]=total;
    info.fields[3]=info.fields[5]=info.fields[7]=available;
    guest_store<GuestMemoryStatus>(uint32_t(address),info);
    return 0;
}

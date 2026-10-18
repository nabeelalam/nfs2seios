// SPDX-License-Identifier: MIT
#include "SystemCpuLoad.h"
#include <chrono>
#include <thread>
#include <cstdio>
int main(){
    nfs::CpuLoadSnapshot a{};
#if !defined(__APPLE__)
    if(nfs::read_system_cpu_ticks(a)||a.count)return 1;
    std::puts("SKIP actual Mach CPU provider:non-Apple host");return 77;
#else
    nfs::CpuLoadSnapshot b{};
    if(!nfs::read_system_cpu_ticks(a)||!a.count)return 1;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if(!nfs::read_system_cpu_ticks(b)||a.count!=b.count)return 1;
    nfs::SystemCpuLoadWindow w;w.observe(a);const auto r=w.observe(b);unsigned valid=0;
    for(std::size_t i=0;i<r.count;++i){if(r.valid[i]){++valid;if(r.percent[i]<0||r.percent[i]>100)return 1;}}
    if(!valid)return 1;
    // Repeated ordinary API reads exercise port/OOL buffer ownership,not load.
    for(unsigned i=0;i<128;++i)if(!nfs::read_system_cpu_ticks(a))return 1;
    std::printf("PASS actual Apple whole-system per-core provider:cores=%zu;host proof only,not iPad sandbox acceptance\n",r.count);return 0;
#endif
}

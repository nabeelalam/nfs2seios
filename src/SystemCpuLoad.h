// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
namespace nfs {
constexpr std::size_t max_cpu_load_cores=64;
struct CpuLoadTicks {uint32_t user=0,system=0,idle=0,nice=0;};
struct CpuLoadSnapshot {
    std::size_t count=0;
    std::array<CpuLoadTicks,max_cpu_load_cores> cores{};
};
struct CpuLoadReading {
    std::size_t count=0;
    std::array<double,max_cpu_load_cores> percent{};
    std::array<bool,max_cpu_load_cores> valid{};
};
// Whole-system activity, not attribution of this app to individual cores.
bool read_system_cpu_ticks(CpuLoadSnapshot &out);
class SystemCpuLoadWindow {
    CpuLoadSnapshot previous{};
    bool have_previous=false;
    static bool delta(uint32_t before,uint32_t after,uint64_t &out){
        // Accept a bounded 32-bit rollover; other backwards counters rebaseline.
        if(after<before && !(before>=0xf0000000u && after<=0x0fffffffu))return false;
        out=uint32_t(after-before);return true;
    }
public:
    void reset(){have_previous=false;previous.count=0;}
    CpuLoadReading observe(const CpuLoadSnapshot &next){
        CpuLoadReading result{};
        if(!next.count || next.count>max_cpu_load_cores){reset();return result;}
        result.count=next.count;
        if(have_previous && previous.count==next.count){
            for(std::size_t i=0;i<next.count;++i){
                const auto &a=previous.cores[i],&b=next.cores[i];uint64_t u=0,s=0,idle=0,n=0;
                if(!delta(a.user,b.user,u)||!delta(a.system,b.system,s)||!delta(a.idle,b.idle,idle)||!delta(a.nice,b.nice,n))continue;
                const uint64_t busy=u+s+n,total=busy+idle;
                if(total){result.percent[i]=100.0*double(busy)/double(total);result.valid[i]=true;}
            }
        }
        previous=next;have_previous=true;return result;
    }
};
}

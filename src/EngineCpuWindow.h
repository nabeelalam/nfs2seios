// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>
namespace nfs {
// CPU time between sixty owner swap submissions, not GPU time or frame latency.
class EngineCpuWindow {
    uint64_t epoch=0;
    unsigned frames=0;
    int64_t previous=-1;
    bool initialized=false;
public:
    static constexpr unsigned interval=60;
    bool frame_due(uint64_t generation){
        if(!initialized || epoch!=generation){initialized=true;epoch=generation;frames=0;previous=-1;return true;}
        if(++frames<interval)return false;
        frames=0;return true;
    }
    int64_t observe(int64_t cpu_ns){
        const auto before=previous;previous=cpu_ns;
        if(cpu_ns<0 || before<0 || cpu_ns<before)return -1;
        return (cpu_ns-before)/interval;
    }
    void reset(){epoch=0;frames=0;previous=-1;initialized=false;}
};
}

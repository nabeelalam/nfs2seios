// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>
namespace nfs {
// Owner-thread only. Run-loop callbacks may take/reset a sample reentrantly.
// A wait spanning a sample boundary is excluded rather than misattributed.
struct MainWaitTiming {
    uint64_t calls=0,elapsed_ns=0,max_ns=0;
    uint64_t finished=0,stopped=0,timed_out=0,handled_source=0,other=0;
};
class MainWaitCapture {
    bool enabled=false;
    uint64_t generation=0;
    MainWaitTiming totals{};
public:
    void enable(bool value){enabled=value;++generation;totals={};}
    uint64_t begin()const{return enabled?generation:0;}
    void observe(uint64_t token,uint64_t elapsed,int reason){
        if(!token || !enabled || token!=generation)return;
        ++totals.calls;totals.elapsed_ns+=elapsed;
        if(elapsed>totals.max_ns)totals.max_ns=elapsed;
        switch(reason){
        case 1:++totals.finished;break;
        case 2:++totals.stopped;break;
        case 3:++totals.timed_out;break;
        case 4:++totals.handled_source;break;
        default:++totals.other;
        }
    }
    MainWaitTiming take(){auto result=totals;totals={};++generation;return result;}
};
// iOS owner-thread diagnostic recording only; not kernel wakeups or idle CPU.
void set_main_wait_capture(bool enabled);
MainWaitTiming take_main_wait_timing();
}

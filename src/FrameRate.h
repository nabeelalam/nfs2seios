// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
namespace nfs {
// Count completed presentations, never draw commands or event-loop iterations.
class FrameRate {
    bool started=false;
    uint64_t start=0,count=0;
public:
    void reset(){started=false;start=count=0;}
    bool presented(uint64_t now,double &fps){
        if(!started || now<start){started=true;start=now;count=0;return false;}
        ++count;
        const auto elapsed=now-start;
        if(elapsed<1000000000ull)return false;
        fps=double(count)*1000000000.0/double(elapsed);
        start=now;count=0;return true;
    }
};
}

// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
namespace nfs {
// One render-submitting worker owns deadlines; UI changes only the limit.
class FramePacer {
    uint64_t next=0;bool started=false;
public:
    void reset(){next=0;started=false;}
    uint64_t remaining(uint64_t now,bool limited){
        if(!limited){reset();return 0;}
        constexpr uint64_t period=1000000000ull/30;
        if(!started || now+period<next){started=true;next=now;}
        return now<next?next-now:0;
    }
    void admitted(uint64_t now){next=now+1000000000ull/30;started=true;}
};
}

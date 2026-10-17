// SPDX-License-Identifier: MIT
#include "FramePacer.h"
#include <stdexcept>
#include <cstdio>
static void check(bool condition){if(!condition)throw std::runtime_error("frame pacing deadline");}
int main(){
    nfs::FramePacer p;
    check(p.remaining(100,true)==0);p.admitted(100);
    check(p.remaining(101,true)==33333332);
    check(p.remaining(33333433,true)==0);p.admitted(33333433);
    check(p.remaining(40000000,true)==26666766);
    // A slow frame is admitted immediately; no catch-up burst is scheduled.
    check(p.remaining(90000000,true)==0);p.admitted(90000000);
    check(p.remaining(90000001,true)==33333332);
    check(p.remaining(90000002,false)==0);check(p.remaining(90000003,true)==0);
    p.admitted(90000003);check(p.remaining(1,true)==0);
    p.reset();check(p.remaining(42,true)==0);
    std::puts("PASS: 30 Hz deadlines, slow frames, disable, reset and clock regression");
}

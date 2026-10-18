// SPDX-License-Identifier: MIT
#include "SystemCpuLoad.h"
#include <cstdio>
#include <cmath>
#include <stdexcept>
static void check(bool ok){if(!ok)throw std::runtime_error("System CPU tick-window check failed");}
int main(){try{
    nfs::SystemCpuLoadWindow window;nfs::CpuLoadSnapshot a{};a.count=2;
    check(!window.observe(a).valid[0]);
    a.cores[0]={10,10,80,0};a.cores[1]={25,25,0,50};
    auto r=window.observe(a);check(r.count==2 && r.valid[0] && r.valid[1]);
    check(std::abs(r.percent[0]-20)<1e-9 && r.percent[1]==100);
    r=window.observe(a);check(!r.valid[0] && !r.valid[1]);
    a.cores[0].idle+=100;r=window.observe(a);check(r.valid[0] && r.percent[0]==0 && !r.valid[1]);
    a.count=3;check(!window.observe(a).valid[0]);
    a.count=0;check(window.observe(a).count==0);a.count=nfs::max_cpu_load_cores+1;check(window.observe(a).count==0);
    window.reset();a={};a.count=1;a.cores[0].user=0xfffffff0u;window.observe(a);
    a.cores[0].user=0x10;a.cores[0].idle=32;r=window.observe(a);check(r.valid[0] && r.percent[0]==50);
    a.cores[0].user=8;r=window.observe(a);check(!r.valid[0]);
    a.cores[0].user=18;a.cores[0].idle+=10;r=window.observe(a);check(r.valid[0] && r.percent[0]==50);
    window.reset();check(!window.observe(a).valid[0]);
    a={};a.count=nfs::max_cpu_load_cores;window.reset();window.observe(a);for(auto &c:a.cores)c={0xffffffffu,0xffffffffu,0xffffffffu,0xffffffffu};
    r=window.observe(a);for(std::size_t i=0;i<r.count;++i)check(r.valid[i] && std::abs(r.percent[i]-75)<1e-9);
    std::puts("PASS system per-core deltas,scope,baseline/reset,zero ticks,32-bit rollover,regression,maximum count and64-bit totals");return 0;
}catch(const std::exception &e){std::fprintf(stderr,"%s\n",e.what());return 1;}}

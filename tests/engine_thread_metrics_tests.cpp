// SPDX-License-Identifier: MIT
#include "EngineThreadMetrics.h"
#include "EngineCpuWindow.h"
#include <atomic>
#include <future>
#include <thread>
#include <limits>
#include <cstdio>
#include <stdexcept>
static std::atomic<unsigned> queries{0};
static std::atomic<int64_t> cpu{0};
static std::atomic<bool> block{false};
static std::promise<void> *entered=nullptr;
static std::shared_future<void> release;
extern "C" int64_t nfs_test_engine_cpu_ns(){
    ++queries;
    if(block.exchange(false)){
        entered->set_value();
        if(release.wait_for(std::chrono::seconds(2))!=std::future_status::ready)throw std::runtime_error("Clock fixture release timed out");
    }
    return cpu.load();
}
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL engine metrics line%d: %s\n",__LINE__,#x);return 1;}}while(0)
static void frames(unsigned n){while(n--)nfs_engine_frame_boundary();}
int main(){
    nfs::EngineCpuWindow interval;
    CHECK(interval.frame_due(0) && interval.observe(0)==-1);
    for(unsigned i=0;i<59;++i)CHECK(!interval.frame_due(0));
    CHECK(interval.frame_due(0) && interval.observe(60000000)==1000000);
    CHECK(interval.observe(60000000)==0);
    CHECK(interval.observe(1)==-1 && interval.observe(61)==1);
    CHECK(interval.observe(-1)==-1 && interval.observe(61)==-1 && interval.observe(121)==1);
    CHECK(interval.frame_due(1) && interval.observe(100)==-1);
    CHECK(interval.frame_due(std::numeric_limits<uint64_t>::max()) && interval.observe(0)==-1);
    CHECK(interval.observe(std::numeric_limits<int64_t>::max())==std::numeric_limits<int64_t>::max()/60);
    interval.reset();CHECK(interval.frame_due(0) && interval.observe(100)==-1);

    nfs_set_engine_metrics_capture(0);nfs_engine_metrics_owner_begin();frames(100000);
    CHECK(!queries && nfs_engine_cpu_ms_per_frame()<0);
    nfs_set_engine_metrics_capture(1);frames(1);CHECK(queries==1 && nfs_engine_cpu_ms_per_frame()<0);
    cpu=120000000;frames(59);CHECK(queries==1);frames(1);
    CHECK(queries==2 && nfs_engine_cpu_ms_per_frame()==2);
    std::thread foreign([]{frames(100000);nfs_engine_metrics_owner_end();});foreign.join();
    CHECK(queries==2 && nfs_engine_cpu_ms_per_frame()==2);
    cpu=120000000;frames(60);CHECK(queries==3 && nfs_engine_cpu_ms_per_frame()==0);
    cpu=-1;frames(60);CHECK(queries==4 && nfs_engine_cpu_ms_per_frame()<0);
    frames(59);CHECK(queries==4);frames(1);CHECK(queries==5 && nfs_engine_cpu_ms_per_frame()<0);
    cpu=500000000;frames(60);CHECK(queries==6 && nfs_engine_cpu_ms_per_frame()<0);
    cpu=560000000;frames(60);CHECK(queries==7 && nfs_engine_cpu_ms_per_frame()==1);
    cpu=1;frames(60);CHECK(queries==8 && nfs_engine_cpu_ms_per_frame()<0);
    cpu=60000001;frames(60);CHECK(queries==9 && nfs_engine_cpu_ms_per_frame()==1);
    nfs_reset_engine_metrics();CHECK(nfs_engine_cpu_ms_per_frame()<0);frames(1);CHECK(queries==10);
    nfs_set_engine_metrics_capture(0);frames(100000);CHECK(queries==10 && nfs_engine_cpu_ms_per_frame()<0);
    nfs_set_engine_metrics_capture(1);frames(1);CHECK(queries==11 && nfs_engine_cpu_ms_per_frame()<0);
    cpu+=180000000;frames(60);CHECK(nfs_engine_cpu_ms_per_frame()==3);
    nfs_engine_metrics_owner_end();CHECK(nfs_engine_cpu_ms_per_frame()<0);const auto before=queries.load();frames(1000);CHECK(queries==before);

    // A platform query may overlap a UI toggle/reset. It must not publish a
    // value for the old epoch, and reset must not wait for that query.
    std::promise<void> clock_entered,unblock,completed,finish,baseline_ready,advance;
    entered=&clock_entered;release=unblock.get_future().share();block=false;
    auto finish_signal=finish.get_future().share();
    auto advance_signal=advance.get_future().share();
    std::thread delayed([&]{
        nfs::EngineMetricsOwner owner;nfs_engine_frame_boundary();baseline_ready.set_value();
        if(advance_signal.wait_for(std::chrono::seconds(2))!=std::future_status::ready)std::terminate();
        frames(60);completed.set_value();
        if(finish_signal.wait_for(std::chrono::seconds(2))!=std::future_status::ready)std::terminate();
    });
    const bool baseline=baseline_ready.get_future().wait_for(std::chrono::seconds(2))==std::future_status::ready;
    cpu+=60000000;block=true;advance.set_value();
    const bool started=clock_entered.get_future().wait_for(std::chrono::seconds(2))==std::future_status::ready;
    nfs_set_engine_metrics_capture(0);nfs_set_engine_metrics_capture(1);unblock.set_value();
    const bool published=completed.get_future().wait_for(std::chrono::seconds(2))==std::future_status::ready;
    const bool invalid=nfs_engine_cpu_ms_per_frame()<0;finish.set_value();delayed.join();
    CHECK(baseline && started && published && invalid);
    nfs_set_engine_metrics_capture(0);
    std::puts("PASS engine CPU:60-frame cadence,hidden/non-owner zero queries,CPU deltas,unavailable retry,regression,epoch resets and in-flight publication rejection");
}

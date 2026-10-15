// SPDX-License-Identifier: MIT
// Standalone lock comparison. No original game code, data or app build needed.
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>
#ifdef __APPLE__
#include <os/lock.h>
class UnfairMutex {
    os_unfair_lock value = OS_UNFAIR_LOCK_INIT;
public:
    UnfairMutex() = default;
    UnfairMutex(const UnfairMutex &) = delete;
    UnfairMutex &operator=(const UnfairMutex &) = delete;
    void lock() noexcept { os_unfair_lock_lock(&value); }
    void unlock() noexcept { os_unfair_lock_unlock(&value); }
};
#endif

struct alignas(64) Payload { unsigned char bytes[16]{}; };
template<class Mutex> void run(const char *backend,const char *workload,
                              unsigned round,std::vector<uint64_t> counts){
    Mutex mutex;
    // Check unwinding releases this BasicLockable implementation.
    try { std::lock_guard<Mutex> lock(mutex);throw 17; } catch(int value) {
        if(value!=17)throw std::runtime_error("Unexpected unwind result");
    }
    {std::lock_guard<Mutex> lock(mutex);}
    std::vector<Payload> payloads(counts.size());
    std::vector<uint64_t> sampled_wait_ns(counts.size(),0);
    std::vector<std::thread> threads;
    std::atomic<unsigned> ready{0};std::atomic<bool> start{false};
    for(size_t slot=0;slot<counts.size();++slot)threads.emplace_back([&,slot]{
        ready.fetch_add(1,std::memory_order_release);
        while(!start.load(std::memory_order_acquire))std::this_thread::yield();
        for(uint64_t i=0;i<counts[slot];++i){
            bool sample=(i&4095)==0;
            auto wait_start=sample?std::chrono::steady_clock::now():std::chrono::steady_clock::time_point{};
            std::lock_guard<Mutex> lock(mutex);
            if(sample){
                auto wait=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-wait_start).count();
                if(uint64_t(wait)>sampled_wait_ns[slot])sampled_wait_ns[slot]=uint64_t(wait);
            }
            uint64_t value=0;
            // Preserve unaligned packed copies; payloads stay on separate cache
            // lines so the shared lock, rather than a shared counter, contends.
            std::memcpy(&value,payloads[slot].bytes+1,sizeof(value));
            ++value;std::memcpy(payloads[slot].bytes+1,&value,sizeof(value));
        }
    });
    while(ready.load(std::memory_order_acquire)!=counts.size())std::this_thread::yield();
    auto before=std::chrono::steady_clock::now();start.store(true,std::memory_order_release);
    for(auto &thread:threads)thread.join();
    auto after=std::chrono::steady_clock::now();uint64_t total=0;
    for(size_t slot=0;slot<counts.size();++slot){
        uint64_t value=0;std::memcpy(&value,payloads[slot].bytes+1,sizeof(value));
        if(value!=counts[slot])throw std::runtime_error("Packed payload verification failed");
        total+=value;
    }
    double seconds=std::chrono::duration<double>(after-before).count();
    std::printf("BENCH backend=%s workload=%s round=%u seconds=%.9f operations=%llu ns_per_operation=%.3f\n",
        backend,workload,round,seconds,static_cast<unsigned long long>(total),seconds*1e9/double(total));
    for(size_t slot=0;slot<counts.size();++slot)std::printf("WAIT backend=%s workload=%s round=%u worker=%zu max_sampled_wait_ns=%llu\n",
        backend,workload,round,slot,static_cast<unsigned long long>(sampled_wait_ns[slot]));
}
template<class Mutex> void suite(const char *name,unsigned round,uint64_t scale){
    run<Mutex>(name,"uncontended",round,{scale});
    run<Mutex>(name,"two_equal_workers",round,{scale/4,scale/4});
    run<Mutex>(name,"eight_to_one_workers",round,{scale,scale/8});
}
int main(int argc,char **argv){
    if(argc>2 || (argc==2 && std::strcmp(argv[1],"--quick-check"))){
        std::fprintf(stderr,"Expected no arguments or --quick-check\n");return 2;
    }
    const bool quick=argc==2;const uint64_t scale=quick?20000:2000000;
#ifdef __aarch64__
    const char *architecture="arm64";
#else
    const char *architecture="other";
#endif
    std::printf("MODE %s architecture=%s; standalone proxy, not game FPS\n",quick?"safety":"benchmark",architecture);
    for(unsigned round=0;round<(quick?1u:6u);++round){
#ifdef __APPLE__
        if(round&1){suite<UnfairMutex>("os_unfair_lock",round,scale);suite<std::mutex>("std_mutex",round,scale);}
        else {suite<std::mutex>("std_mutex",round,scale);suite<UnfairMutex>("os_unfair_lock",round,scale);}
#else
        suite<std::mutex>("std_mutex",round,scale);
#endif
    }
#ifndef __APPLE__
    std::puts("Apple backend unavailable on this host; no Apple comparison made");
#endif
    std::puts("PASS: packed payload counts and exception cleanup; no game assets used");
}

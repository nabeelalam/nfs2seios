// SPDX-License-Identifier: MIT
// Synthetic component comparison only. No game code or energy/FPS inference.
#include "EngineMemory.h"
#include "GuestCopyTransaction.h"
#include "CachedFrameTransaction.h"
#include "DATA.h"
#include "BSS.h"
#include <array>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <stdexcept>
#include <string>
#include <cmath>
#include <time.h>

alignas(4) DataLayout _data{};
alignas(4) BssLayout _bss{};
using Kernel=uint32_t(*)(uint32_t,unsigned char*,unsigned,unsigned);
template<class Transaction>
static inline uint32_t work(Transaction&tx,uint32_t base,unsigned char*global,unsigned mode,unsigned words){
    uint32_t result=0;
    for(unsigned i=0;i<words;++i){
        uint32_t value;
        if(mode==1&&(i&1)){
            value=tx.template load<uint32_t>(global+4*i);
            tx.template store<uint32_t>(global+4*i,value+1u);
        }else{
            const uint32_t address=base+(mode==2?128u:0u)+4*i;
            value=tx.template load<uint32_t>(address);
            tx.template store<uint32_t>(address,value+1u);
        }
        result^=value+i*0x9e3779b9u;
    }
    return result;
}
extern "C" __attribute__((noinline,aligned(64)))
uint32_t nfs_frame_control(uint32_t base,unsigned char*global,unsigned mode,unsigned words){
    nfs::GuestCopyTransaction tx;return work(tx,base,global,mode,words);
}
extern "C" __attribute__((noinline,aligned(64)))
uint32_t nfs_frame_candidate(uint32_t base,unsigned char*global,unsigned mode,unsigned words){
    nfs::CachedFrameTransaction tx{base};return work(tx,base,global,mode,words);
}
static uint64_t clock_ns(clockid_t id){
    timespec value{};if(clock_gettime(id,&value))throw std::runtime_error("Thread clock unavailable");
    return uint64_t(value.tv_sec)*1000000000ull+uint64_t(value.tv_nsec);
}
struct Peer{
    std::mutex mutex;std::condition_variable changed;bool ready=false,done=false;std::thread worker;
    Peer():worker([this]{std::unique_lock<std::mutex>lock(mutex);ready=true;changed.notify_one();changed.wait(lock,[this]{return done;});}){
        std::unique_lock<std::mutex>lock(mutex);changed.wait(lock,[this]{return ready;});
    }
    ~Peer(){{std::lock_guard<std::mutex>lock(mutex);done=true;}changed.notify_one();worker.join();}
};
struct Image{std::array<unsigned char,256>heap;std::array<unsigned char,80>global;};
static bool operator==(const Image&a,const Image&b){return a.heap==b.heap&&a.global==b.global;}
struct Fixture{
    nfs::EngineMemory memory{65536};unsigned char*heap=nullptr;unsigned char*global=nullptr;uint32_t base=0;
    nfs::WorkerControl control;Image pristine{};Peer peer;
    Fixture(){
        heap=static_cast<unsigned char*>(memory.allocate(256));assert(heap);base=memory.encode(heap);
        global=reinterpret_cast<unsigned char*>(&_data)+1024;
        for(unsigned i=0;i<256;++i)heap[i]=static_cast<unsigned char>(17*i+9);
        for(unsigned i=0;i<80;++i)global[i]=static_cast<unsigned char>(31*i+7);
        pristine=image();nfs::current_worker_control()=&control;
    }
    Image image()const{Image x;std::memcpy(x.heap.data(),heap,256);std::memcpy(x.global.data(),global,80);return x;}
    void restore(){std::memcpy(heap,pristine.heap.data(),256);std::memcpy(global,pristine.global.data(),80);}
    void stop(unsigned mode){control.stop=mode!=0;nfs::owned_critical_depth()=mode==1?1:0;nfs::worker_stop_deferrals()=mode==2?1:0;}
    ~Fixture(){stop(0);nfs::current_worker_control()=nullptr;memory.release(heap);}
};
static uint32_t reference(Image&image,unsigned mode,unsigned words,unsigned alignment){
    uint32_t result=0;
    for(unsigned i=0;i<words;++i){
        auto*pointer=mode==1&&(i&1)?image.global.data()+alignment+4*i:image.heap.data()+alignment+(mode==2?128:0)+4*i;
        uint32_t value;std::memcpy(&value,pointer,4);const uint32_t next=value+1u;std::memcpy(pointer,&next,4);result^=value+i*0x9e3779b9u;
    }
    return result;
}
static void verify(Fixture&fixture){
    unsigned cases=0;uint64_t digest=1469598103934665603ull;
    for(unsigned mode=0;mode<3;++mode)for(unsigned words:{1u,4u,8u,16u})for(unsigned alignment:{0u,1u,3u})for(unsigned stop=0;stop<3;++stop){
        Image expected=fixture.pristine;const auto value=reference(expected,mode,words,alignment);fixture.stop(stop);
        for(Kernel kernel:{nfs_frame_control,nfs_frame_candidate}){
            fixture.restore();assert(kernel(fixture.base+alignment,fixture.global+alignment,mode,words)==value);assert(fixture.image()==expected);
        }
        fixture.stop(0);++cases;digest^=value;digest*=1099511628211ull;
    }
    for(unsigned mode=0;mode<3;++mode)for(unsigned stop=0;stop<2;++stop){
        fixture.restore();const auto before=fixture.image();fixture.stop(0);fixture.control.stop=stop!=0;
        for(Kernel kernel:{nfs_frame_control,nfs_frame_candidate}){
            bool correct=false;try{kernel(0,nullptr,mode,1);}catch(const nfs::WorkerStopped&){correct=stop!=0;}catch(const std::out_of_range&){correct=stop==0;}
            assert(correct&&fixture.image()==before);
        }
        fixture.stop(0);++cases;
    }
    assert(cases==114&&guest_copy_mutex().try_lock());guest_copy_mutex().unlock();
    std::cout<<"{\"type\":\"verification\",\"cases\":"<<cases<<",\"digest\":"<<digest<<",\"referenceStateMatched\":true,\"stopAndFaultPrecedence\":true,\"sameAllocation\":true}"<<std::endl;
}
struct Timing{uint64_t cpu,raw,wall,firstRaw,firstWall,iterations,checksum;double drift;bool quality;};
static void study(Fixture&fixture){
#ifndef __APPLE__
    throw std::runtime_error("Performance study requires native Apple host; local timing disabled");
#endif
    const auto started=std::chrono::steady_clock::now();Kernel functions[]={nfs_frame_control,nfs_frame_control,nfs_frame_candidate};assert(functions[0]==functions[1]);
    std::cout<<"{\"type\":\"study_start\",\"sameProcessAndAllocation\":true,\"negativeFunctionPointersEqual\":true,\"threadedProcess\":true}"<<std::endl;
    uint64_t counts[6]{};bool qualified=true;unsigned measurements=0;
    for(unsigned phase=0;phase<3;++phase){
        if(phase&&!qualified)break;
        const unsigned cells=phase==2?4:6;
        for(unsigned cell=0;cell<cells;++cell){
            const unsigned mode=phase==2?0:cell/2,words=(cell&1)?16:8,stop=phase==2?1+cell/2:0;
            fixture.stop(stop);Image expected=fixture.pristine;const uint32_t value=reference(expected,mode,words,0);
            auto measure=[&](unsigned variant,uint64_t count,const char*kind,unsigned block,unsigned order){
                if(std::chrono::steady_clock::now()-started>std::chrono::seconds(90))throw std::runtime_error("Native measurement budget exhausted");
                uint64_t checksum=0;const auto raw=clock_ns(CLOCK_MONOTONIC_RAW),wall=clock_ns(CLOCK_MONOTONIC),cpu=clock_ns(CLOCK_THREAD_CPUTIME_ID);
                for(uint64_t i=0;i<count/2;++i){fixture.restore();checksum+=functions[variant](fixture.base,fixture.global,mode,words);}
                const auto rawMid=clock_ns(CLOCK_MONOTONIC_RAW),wallMid=clock_ns(CLOCK_MONOTONIC);
                for(uint64_t i=count/2;i<count;++i){fixture.restore();checksum+=functions[variant](fixture.base,fixture.global,mode,words);}
                const auto cpuNs=clock_ns(CLOCK_THREAD_CPUTIME_ID)-cpu,rawNs=clock_ns(CLOCK_MONOTONIC_RAW)-raw,wallNs=clock_ns(CLOCK_MONOTONIC)-wall;
                assert(checksum==uint64_t(value)*count&&fixture.image()==expected);
                const auto firstRaw=rawMid-raw,firstWall=wallMid-wall;assert(firstRaw&&firstWall&&rawNs>firstRaw&&wallNs>firstWall);
                const double first=double(firstRaw)/firstWall,second=double(rawNs-firstRaw)/(wallNs-firstWall),drift=100*std::abs(first-second)/std::min(first,second);
                const bool quality=cpuNs<=1.005*rawNs&&drift<=.5;
                std::cout<<std::setprecision(17)<<"{\"type\":\""<<kind<<"\",\"phase\":"<<phase<<",\"cell\":"<<cell<<",\"mode\":"<<mode<<",\"words\":"<<words<<",\"stopMode\":"<<stop<<",\"block\":"<<block<<",\"order\":"<<order<<",\"variant\":"<<variant<<",\"iterations\":"<<count<<",\"cpuNs\":"<<cpuNs<<",\"rawNs\":"<<rawNs<<",\"wallNs\":"<<wallNs<<",\"firstRawNs\":"<<firstRaw<<",\"firstWallNs\":"<<firstWall<<",\"clockRateDriftPercent\":"<<drift<<",\"qualityPassed\":"<<(quality?"true":"false")<<",\"checksum\":"<<checksum<<",\"stateVerified\":true}"<<std::endl;
                return Timing{cpuNs,rawNs,wallNs,firstRaw,firstWall,count,checksum,drift,quality};
            };
            uint64_t count=phase==2?counts[cell&1]:counts[cell];
            if(phase==0||phase==2){if(!phase)count=1024;bool calibrated=false;for(unsigned refinement=0;refinement<5;++refinement){const auto row=measure(0,count,"calibration",0,0);assert(row.cpu);if(row.cpu>=200000000&&row.cpu<=220000000){calibrated=true;break;}count=std::max<uint64_t>(1024,(uint64_t(double(count)*205000000.0/row.cpu)+1)&~uint64_t(1));if(count>8388608)throw std::runtime_error("Calibration count bound");}if(!calibrated)throw std::runtime_error("Calibration band unstable");if(!phase)counts[cell]=count;}
            for(unsigned variant=0;variant<(phase?2u:1u);++variant){uint64_t warm=0;while(warm<(phase==2?1000000000ull:2000000000ull))warm+=measure(variant?2:0,count,"warmup",0,0).cpu;}
            std::vector<uint64_t> negativeOuter;
            const unsigned blocks=phase==1?3:2;
            for(unsigned block=0;block<blocks;++block){const unsigned order[]={0,phase?2u:1u,phase?2u:1u,0};Timing rows[4];for(unsigned i=0;i<4;++i){rows[i]=measure(order[i],count,"measurement",block,i);++measurements;}
                if(!phase){negativeOuter.push_back(rows[0].cpu);negativeOuter.push_back(rows[3].cpu);const double outer=(double(rows[0].cpu)+rows[3].cpu)/2,inner=(double(rows[1].cpu)+rows[2].cpu)/2;qualified=qualified&&100*std::abs(inner-outer)/outer<=2&&100*std::abs(double(rows[0].cpu)-rows[3].cpu)/std::min(rows[0].cpu,rows[3].cpu)<=5;for(const auto&row:rows)qualified=qualified&&row.quality;}
            }
            if(!phase){const auto limits=std::minmax_element(negativeOuter.begin(),negativeOuter.end());qualified=qualified&&100*double(*limits.second-*limits.first)/ *limits.first<=5;}
            fixture.stop(0);
        }
    }
    std::cout<<"{\"type\":\"study_complete\",\"timingControlQualified\":"<<(qualified?"true":"false")<<",\"candidateTimed\":"<<(qualified?"true":"false")<<",\"measurements\":"<<measurements<<",\"stopStateRestored\":true}"<<std::endl;
}
int main(int argc,char**argv){try{
    Fixture fixture;
    if(argc==2&&std::string(argv[1])=="--verify")verify(fixture);
    else if(argc==2&&std::string(argv[1])=="--study")study(fixture);
    else throw std::invalid_argument("Explicit verify or native study mode required");
    return 0;
}catch(const std::exception&error){std::cerr<<error.what()<<std::endl;return 2;}}

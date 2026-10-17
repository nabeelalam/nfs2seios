// SPDX-License-Identifier: MIT
#include "GameTrace.h"
#include <sstream>
#include <atomic>
#include <array>
namespace {
struct Call {uint32_t id=0,stack=0;};thread_local Call calls[32];thread_local unsigned next=0,count=0;
// Each writer owns a separate cache line. No shared counter or allocation on
// the call path; the monitor reads these words only after a presentation stall.
struct alignas(64) Sample {std::atomic<bool> owned{false};std::atomic<uint64_t> value{0};};
static_assert(ATOMIC_LLONG_LOCK_FREE==2,"Diagnostic samples require lock-free 64-bit stores");
std::array<Sample,16> samples;
struct Lease {
    Sample *slot=nullptr;
    Lease(){for(auto &sample:samples){bool free=false;if(sample.owned.compare_exchange_strong(free,true)){slot=&sample;break;}}}
    ~Lease(){if(slot){slot->value=0;slot->owned=false;}}
};
thread_local Lease lease;
}
extern "C" void nfs_trace_guest_call(uint32_t id,uint32_t stack) noexcept{
    calls[next]={id,stack};next=(next+1)%32;if(count<32)++count;
    if(lease.slot)lease.slot->value.store((uint64_t(id)<<32)|stack,std::memory_order_relaxed);
}
namespace nfs {std::string recent_game_calls(){std::ostringstream text;text<<"recent native game calls (oldest first):";for(unsigned i=0;i<count;++i){const auto &call=calls[(next+32-count+i)%32];text<<" 0x"<<std::hex<<call.id<<"@0x"<<call.stack;}return text.str();}}
namespace nfs {std::string sampled_game_calls(){
    std::ostringstream text;text<<"Latest sampled original call per active thread slot (not a stack trace):";
    for(size_t i=0;i<samples.size();++i){const auto value=samples[i].value.load(std::memory_order_relaxed);if(value)text<<"\nslot "<<i<<" 0x"<<std::hex<<uint32_t(value>>32)<<"@0x"<<uint32_t(value)<<std::dec;}
    return text.str();
}}

#include "GuestAccess.h"
#include "GameMemory.h"
#include <thread>
#include <atomic>
#include <cstdio>
#include <stdexcept>
static nfs::GameMemory memory;
extern "C" void *nfs_host_address(uint32_t address,size_t size){void *p=memory.resolve(address,size);if(!p)throw std::out_of_range("Bad shared span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t id=0;if(!memory.encode(p,id))throw std::out_of_range("Bad shared pointer");return id;}
int main(){
    uint8_t bytes[32]{};if(!memory.map(0x80000000,bytes,sizeof(bytes)))return 1;
    guest_store<uint64_t>(0x80000003u,UINT32_MAX);
    std::atomic<bool> done{false},valid{true};
    std::thread reader([&]{do{const uint64_t value=guest_load<uint64_t>(0x80000003u);if(uint32_t(value)!=~uint32_t(value>>32))valid=false;}while(!done);});
    for(uint32_t i=0;i<50000;++i)guest_store<uint64_t>(0x80000003u,(uint64_t(i)<<32)|~i);
    done=true;reader.join();if(!valid)return 1;
    std::puts("PASS: 50000 concurrent unaligned guest updates retain whole-value consistency");
}

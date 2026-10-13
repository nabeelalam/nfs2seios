#include "ClockWrappers.h"
#include "GuestAccess.h"
#include "GameMemory.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static nfs::GameMemory memory;static uint64_t ticks=0;static uint32_t delay=0;static int64_t seconds=0;
extern "C" void *nfs_host_address(uint32_t address,size_t size){void *p=memory.resolve(address,size);if(!p)throw std::out_of_range("Bad clock span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t id=0;if(!memory.encode(p,id))throw std::out_of_range("Bad clock pointer");return id;}
extern "C" uint64_t nfs_native_ticks(){return ticks;}
extern "C" void nfs_native_delay(uint32_t milliseconds){delay=milliseconds;}
extern "C" int64_t nfs_native_unix_seconds(){return seconds;}
int main(){
    uint8_t bytes[64];std::memset(bytes,0xcc,sizeof(bytes));CHECK(reinterpret_cast<uintptr_t>(bytes)>UINT32_MAX && memory.map(0x80000000,bytes,sizeof(bytes)));
    ticks=0x100000123ull;CHECK(SDL_GetTicks_wrap()==0x123);
    ticks=0xffffffffull;CHECK(SDL_GetTicks_wrap()==-1);
    CHECK(SDL_Delay_wrap(-1)==0 && delay==UINT32_MAX);
    seconds=0x100000123ll;CHECK(time_wrap(int32_t(0x80000003u))==0x123 && guest_load<uint32_t>(0x80000003u)==0x123);
    CHECK(bytes[2]==0xcc && bytes[7]==0xcc);
    seconds=0x80000000ll;CHECK(time_wrap(0)==INT32_MIN);
    CHECK(timeGetDevCaps_wrap(int32_t(0x80000003u),8)==0);
    CHECK(guest_load<uint32_t>(0x80000003u)==1 && guest_load<uint32_t>(0x80000007u)==1000 && bytes[11]==0xcc);
    CHECK(timeGetDevCaps_wrap(0,7)==97 && timeGetDevCaps_wrap(0,8)==97 && timeBeginPeriod_wrap(0)==97 && timeBeginPeriod_wrap(1001)==97);
    CHECK(timeBeginPeriod_wrap(1)==0 && timeBeginPeriod_wrap(1)==0 && timeEndPeriod_wrap(1)==0 && timeEndPeriod_wrap(1)==0 && timeEndPeriod_wrap(1)==97);
    bool bounds=false;try{time_wrap(int32_t(0x8000003fu));}catch(const std::out_of_range &){bounds=true;}CHECK(bounds);
    std::puts("PASS: 32-bit game ticks/calendar outputs, unaligned canaries, native delay marshalling and paired period leases");
}

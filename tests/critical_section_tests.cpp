#include "CriticalSectionBridge.h"
#include "GuestAccess.h"
#include "GameMemory.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <atomic>
#include <vector>
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static nfs::GameMemory memory;
extern "C" void *nfs_host_address(uint32_t address,size_t size) {void *p=memory.resolve(address,size);if(!p)throw std::out_of_range("Bad critical section span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p) {uint32_t id=0;if(!memory.encode(p,id))throw std::out_of_range("Bad critical pointer");return id;}
int main() {
    uint8_t bytes[64];std::memset(bytes,0xcc,sizeof(bytes));
    CHECK(reinterpret_cast<uintptr_t>(bytes)>UINT32_MAX && memory.map(0x80000000,bytes,sizeof(bytes)));
    nfs::CriticalSectionBridge bridge;nfs::bind_critical_sections(&bridge);
    const int32_t address=int32_t(0x80000003u);
    CHECK(InitializeCriticalSection_wrap(address)==0);
    CHECK(bytes[2]==0xcc && bytes[27]==0xcc);
    for(unsigned i=3;i<27;++i)CHECK(bytes[i]==0);
    EnterCriticalSection_wrap(address);EnterCriticalSection_wrap(address);
    std::atomic<bool> rejected{false};
    std::thread wrong([&]{try{LeaveCriticalSection_wrap(address);}catch(const std::logic_error &){rejected=true;}});wrong.join();CHECK(rejected);
    bool deletion_rejected=false;try{DeleteCriticalSection_wrap(address);}catch(const std::logic_error &){deletion_rejected=true;}CHECK(deletion_rejected);
    LeaveCriticalSection_wrap(address);LeaveCriticalSection_wrap(address);
    // Plain shared state requires real exclusion, rather than an atomic increment.
    uint32_t count=0;std::vector<std::thread> workers;
    for(int t=0;t<4;++t)workers.emplace_back([&]{for(int i=0;i<2000;++i){EnterCriticalSection_wrap(address);EnterCriticalSection_wrap(address);++count;LeaveCriticalSection_wrap(address);LeaveCriticalSection_wrap(address);}});
    for(auto &worker:workers)worker.join();CHECK(count==8000);
    CHECK(DeleteCriticalSection_wrap(address)==0);
    bool stale=false;try{EnterCriticalSection_wrap(address);}catch(const std::out_of_range &){stale=true;}CHECK(stale);
    CHECK(InitializeCriticalSection_wrap(address)==0);EnterCriticalSection_wrap(address);LeaveCriticalSection_wrap(address);DeleteCriticalSection_wrap(address);
    CHECK(InitializeCriticalSection_wrap(address)==0);EnterCriticalSection_wrap(address);EnterCriticalSection_wrap(address);
    bridge.retire_current_thread();CHECK(nfs::owned_critical_depth()==0);
    std::thread next_owner([&]{EnterCriticalSection_wrap(address);LeaveCriticalSection_wrap(address);});next_owner.join();DeleteCriticalSection_wrap(address);
    bool bounds=false;try{InitializeCriticalSection_wrap(int32_t(0x80000030u));}catch(const std::out_of_range &){bounds=true;}CHECK(bounds && bytes[48]==0xcc);
    nfs::bind_critical_sections(nullptr);
    std::puts("PASS: guest critical-section layout/canaries, recursion, 8000 concurrent protected updates, ownership, active deletion rejection, reuse and bounds");
}

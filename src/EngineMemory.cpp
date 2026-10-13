// SPDX-License-Identifier: MIT
#include "EngineMemory.h"
#include "GameMemory.h"
#include "HeapArena.h"
#include "GameTrace.h"
#include "DATA.h"
#include "BSS.h"
#include <vector>
#include <stdexcept>
#include <sstream>
#if defined(__linux__) && defined(__GLIBC__)
#include <execinfo.h>
#include <cstdlib>
#endif
#ifndef _WIN32
#include <unistd.h>
#endif
namespace nfs {
struct EngineMemory::Impl {GameMemory addresses;Functions functions;HeapArena heap;std::vector<uint8_t> storage;explicit Impl(size_t size):storage(size){}};
static EngineMemory *active=nullptr;
static EngineMemory &current(){if(!active)throw std::logic_error("Original engine memory unbound");return *active;}
EngineMemory::EngineMemory(size_t size){
    if(active)throw std::logic_error("Original engine already owns memory");
    if(!size || size>0x20000000u)throw std::invalid_argument("Engine heap outside bounded 512 MiB guest region");
    impl.reset(new Impl(size));
    if(!impl->addresses.map(0x00400000,reinterpret_cast<uint8_t *>(&_data),sizeof(_data)) ||
       !impl->addresses.map(0x02000000,reinterpret_cast<uint8_t *>(&_bss),sizeof(_bss)) ||
       !impl->heap.initialize(impl->addresses,0x10000000,impl->storage.data(),impl->storage.size()))throw std::runtime_error("Original engine regions could not be mapped");
    active=this;
}
EngineMemory::~EngineMemory(){if(active==this)active=nullptr;}
void *EngineMemory::resolve(uint32_t address,size_t length){
    if(address>=0x10000000 && uint64_t(address)<uint64_t(0x10000000)+capacity() && !impl->heap.contains(address,length)){std::ostringstream text;text<<"Unallocated original engine heap span address=0x"<<std::hex<<address<<" bytes=0x"<<length;throw std::out_of_range(text.str());}
    auto pointer=impl->addresses.resolve(address,length);if(!pointer){std::ostringstream text;text<<"Unmapped original engine span address=0x"<<std::hex<<address<<" bytes=0x"<<length;throw std::out_of_range(text.str());}return pointer;
}
uint32_t EngineMemory::encode(const void *pointer){uint32_t address=0;if(!impl->addresses.encode(pointer,address))throw std::out_of_range("Unmapped original engine native pointer");return address;}
void *EngineMemory::allocate(size_t size){return impl->heap.allocate(size);}
void EngineMemory::release(void *pointer){if(!impl->heap.release(pointer))throw std::out_of_range("Invalid original engine allocation release");}
bool EngineMemory::add(uint32_t id,NativeGameFunction function){return impl->functions.add(id,function);}
NativeGameFunction EngineMemory::function(uint32_t id){
    auto value=impl->functions.find(id);
    if(!value){
        std::ostringstream text;text<<"Unknown original engine function ID=0x"<<std::hex<<id<<"; dispatch-thread "<<recent_game_calls();
#if defined(__linux__) && defined(__GLIBC__)
        // Resolve these module offsets locally with addr2line. Capture only at
        // the invalid dispatch, avoiding debugger interference with game timing.
        void *frames[24];const int count=backtrace(frames,24);
        if(auto symbols=backtrace_symbols(frames,count)){
            text<<"; native dispatch stack:";
            for(int i=0;i<count;++i)text<<"\n"<<symbols[i];
            std::free(symbols);
        }
#endif
        throw std::out_of_range(text.str());
    }
    return value;
}
size_t EngineMemory::capacity()const{return impl->heap.total_bytes();}
size_t EngineMemory::available()const{return impl->heap.available_bytes();}
}
extern "C" void *nfs_host_address(uint32_t address,size_t length){return nfs::current().resolve(address,length);}
extern "C" uint32_t nfs_guest_address(const void *pointer){return nfs::current().encode(pointer);}
extern "C" void *malloc32(size_t size){return nfs::current().allocate(size);}
extern "C" void free32(void *pointer){if(pointer)nfs::current().release(pointer);}
extern "C" bool nfs_register_guest_function(uint32_t id,NativeGameFunction function){return nfs::current().add(id,function);}
extern "C" NativeGameFunction nfs_lookup_guest_function(uint32_t id){return nfs::current().function(id);}
extern "C" uint32_t nfs_game_heap_capacity(){return uint32_t(nfs::current().capacity());}
extern "C" uint32_t nfs_game_heap_available(){return uint32_t(nfs::current().available());}
extern "C" uint32_t nfs_native_page_size(){
#ifdef _WIN32
    return 4096;
#else
    const auto size=sysconf(_SC_PAGESIZE);if(size<=0 || uint64_t(size)>UINT32_MAX)throw std::runtime_error("Native page size unavailable");return uint32_t(size);
#endif
}

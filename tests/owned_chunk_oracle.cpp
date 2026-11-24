// SPDX-License-Identifier: MIT
#include "EngineMemory.h"
#include "HeapArena.h"
#include "DATA.h"
#include "BSS.h"
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <thread>
#include <atomic>
#include <cassert>
alignas(4) DataLayout _data{};
alignas(4) BssLayout _bss{};
struct Span {uint32_t base;size_t size;uint8_t *host;};
static uint64_t digest=1469598103934665603ull;static unsigned checks=0;
static void add(uint64_t value){for(unsigned i=0;i<8;++i){digest^=(value>>(i*8))&255;digest*=1099511628211ull;}}
int main(){
 auto unbound=[](){bool caught=false;try{(void)nfs_host_address(0x00400000,4);}catch(const std::logic_error &error){caught=std::string(error.what())=="Original engine memory unbound";}assert(caught);};
 unbound();{
 nfs::EngineMemory memory(65536);std::vector<Span> spans{{0x00400000,sizeof(_data),reinterpret_cast<uint8_t*>(&_data)},{0x02000000,sizeof(_bss),reinterpret_cast<uint8_t*>(&_bss)}};
 std::vector<void*> allocations;
 for(size_t bytes:{size_t(1),size_t(17),size_t(4096),size_t(5000),size_t(32768)}){void *p=memory.allocate(bytes);assert(p);allocations.push_back(p);spans.push_back({memory.encode(p),(bytes+15)&~size_t(15),static_cast<uint8_t*>(p)});}
 auto check=[&](uint32_t address,size_t length){
  uint8_t *expected=nullptr;if(length)for(const auto &s:spans)if(address>=s.base&&uint64_t(address)-s.base<s.size&&length<=s.size-(address-s.base)){expected=s.host+(address-s.base);break;}
  ++checks;add(address);add(length);
  try{void *actual=memory.resolve(address,length);assert(expected&&actual==expected);add(1);}
  catch(const std::out_of_range &error){assert(!expected);const bool heap=address>=0x10000000&&address<0x10010000;std::ostringstream exact;exact<<(heap?"Unallocated original engine heap span":"Unmapped original engine span")<<" address=0x"<<std::hex<<address<<" bytes=0x"<<length;assert(exact.str()==error.what());add(0);for(unsigned char c:std::string(error.what()))add(c);}
 };
 for(const auto &s:spans){check(s.base,0);check(s.base,1);check(s.base,s.size);check(s.base,s.size+1);check(s.base+uint32_t(s.size)-1,1);check(s.base+uint32_t(s.size)-1,2);check(s.base-1,1);check(s.base+uint32_t(s.size),1);check(s.base,SIZE_MAX);}
 for(uint32_t a:{0u,1u,0x003fffffu,0x10000000u,0x1000ffffu,0x10010000u,0xffffffffu})for(size_t n:{size_t(0),size_t(1),size_t(4),size_t(65536),SIZE_MAX})check(a,n);
 uint32_t random=0x13579bdf;for(unsigned i=0;i<12000;++i){random=random*1664525+1013904223;uint32_t a=(i%4==0)?0x10000000+(random&65535):(i%4==1)?0x00400000+(random%uint32_t(sizeof(_data)+64)):(i%4==2)?0x02000000+(random%uint32_t(sizeof(_bss)+64)):random;size_t n=(i%17==0)?SIZE_MAX:(random>>24);check(a,n);}
 const auto freed=memory.encode(allocations[0]);memory.release(allocations[0]);spans.erase(spans.begin()+2);check(freed,1);check(freed,16);void *reused=memory.allocate(16);assert(reused&&memory.encode(reused)==freed);spans.push_back({freed,16,static_cast<uint8_t*>(reused)});check(freed,16);check(freed,17);
 const uint32_t live=memory.encode(allocations[2]);const uint32_t large=memory.encode(allocations[4]);const uint32_t chunk=(large+4095)&~uint32_t(4095);std::atomic<unsigned> passed{0};std::vector<std::thread> threads;
 for(unsigned i=0;i<4;++i)threads.emplace_back([&]{for(unsigned j=0;j<20000;++j){assert(memory.resolve(live,4096)==allocations[2]);assert(memory.resolve(chunk+((j*17)&2047),16)==static_cast<uint8_t*>(allocations[4])+(chunk-large)+((j*17)&2047));assert(memory.resolve(0x00400000,4)==reinterpret_cast<uint8_t*>(&_data));assert(memory.resolve(0x02000001,7)==reinterpret_cast<uint8_t*>(&_bss)+1);}++passed;});for(auto &t:threads)t.join();assert(passed==4);
 memory.release(allocations[4]);spans.erase(std::find_if(spans.begin(),spans.end(),[&](const Span &span){return span.base==large;}));check(chunk,16);check(chunk+4080,16);check(chunk+4095,1);check(large,32768);
 void *small=memory.allocate(16);assert(small&&memory.encode(small)==large);spans.push_back({large,16,static_cast<uint8_t*>(small)});check(large,16);check(large,17);check(chunk,1);

 }
 unbound();{nfs::EngineMemory second(32768);assert(nfs_host_address(0x00400000,4)==reinterpret_cast<uint8_t*>(&_data));assert(nfs_host_address(0x02000001,7)==reinterpret_cast<uint8_t*>(&_bss)+1);}
 unbound();
 std::cout<<"{\"checks\":"<<checks<<",\"oracleDigest\":"<<digest<<",\"concurrentLiveSpanReads\":80000,\"concurrentStaticSpanReads\":160000,\"concurrentOwnedChunkReads\":80000,\"bindingTransitionsChecked\":true,\"engineObjectBytes\":"<<sizeof(nfs::EngineMemory)<<",\"heapObjectBytes\":"<<sizeof(nfs::HeapArena)<<",\"dataBytes\":"<<sizeof(_data)<<",\"bssBytes\":"<<sizeof(_bss)<<"}\n";
}

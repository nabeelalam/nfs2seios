// SPDX-License-Identifier: MIT
#include "EngineMemory.h"
#include "GuestCopyTransaction.h"
#include "DATA.h"
#include "BSS.h"
#include <cassert>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <atomic>
#include <limits>
alignas(4) DataLayout _data{};
alignas(4) BssLayout _bss{};
static void *checked_native_span(const void *p,size_t n){
#ifdef NFS_NATIVE_SPAN_CANDIDATE
 return nfs_host_native_span(p,n);
#else
 return nfs_host_address(nfs_guest_address(p),n);
#endif
}
struct Outcome{void *pointer=nullptr;int kind=0;std::string message;};
static Outcome original(const void *p,size_t n){Outcome o;try{o.pointer=nfs_host_address(nfs_guest_address(p),n);}catch(const std::out_of_range &e){o.kind=1;o.message=e.what();}catch(const std::logic_error &e){o.kind=2;o.message=e.what();}return o;}
static Outcome proposed(const void *p,size_t n){Outcome o;try{o.pointer=checked_native_span(p,n);}catch(const std::out_of_range &e){o.kind=1;o.message=e.what();}catch(const std::logic_error &e){o.kind=2;o.message=e.what();}return o;}
static unsigned checks=0;
static void check(const void *p,size_t n){const auto a=original(p,n),b=proposed(p,n);assert(a.pointer==b.pointer&&a.kind==b.kind&&a.message==b.message);++checks;}
static void unbound(){auto a=proposed(&_data,4);assert(a.kind==2&&a.message=="Original engine memory unbound");check(&_data,4);check(nullptr,0);}
static const void *offset(const void *p,uintptr_t n){return reinterpret_cast<const void *>(reinterpret_cast<uintptr_t>(p)+n);}
int main(){
 unbound();unsigned concurrent=0;
 {
 nfs::EngineMemory memory(65536);std::vector<void*> allocations;
 for(size_t n:{size_t(1),size_t(17),size_t(4096),size_t(5000),size_t(32768)}){auto*p=memory.allocate(n);assert(p);allocations.push_back(p);}
 struct Span{const void *p;size_t size;};std::vector<Span> spans{{&_data,sizeof(_data)},{&_bss,sizeof(_bss)}};
 for(unsigned i=0;i<allocations.size();++i)spans.push_back({allocations[i],std::vector<size_t>{16,32,4096,5008,32768}[i]});
 for(auto v:spans){for(size_t n:{size_t(0),size_t(1),size_t(2),size_t(4),v.size,v.size+1,SIZE_MAX}){check(v.p,n);check(offset(v.p,1),n);check(offset(v.p,v.size-1),n);check(offset(v.p,v.size),n);check(offset(v.p,UINTPTR_MAX),n);}}
 uint32_t random=0x2468ace1;
 for(unsigned i=0;i<15000;++i){random=random*1664525+1013904223;auto v=spans[i%spans.size()];size_t pos=random%(v.size+32);size_t n=i%19==0?SIZE_MAX:(random>>24);check(offset(v.p,pos),n);}
 int stack=7;for(auto p:{static_cast<const void*>(nullptr),static_cast<const void*>(&stack),reinterpret_cast<const void*>(uintptr_t(1)),reinterpret_cast<const void*>(UINTPTR_MAX)})for(size_t n:{size_t(0),size_t(1),size_t(16),SIZE_MAX})check(p,n);
 const void *freed=allocations[0];memory.release(allocations[0]);check(freed,1);check(freed,16);auto*reused=memory.allocate(16);assert(reused==freed);check(freed,16);check(freed,17);
 void *large=allocations[4];memory.release(large);check(large,1);check(offset(large,4096),16);assert(memory.allocate(16)==large);check(large,16);check(offset(large,4096),16);
 auto *data=reinterpret_cast<uint8_t*>(&_data);auto*bss=reinterpret_cast<uint8_t*>(&_bss);
 std::atomic<unsigned> count{0};std::vector<std::thread> threads;
 for(unsigned t=0;t<4;++t)threads.emplace_back([&,t]{for(unsigned i=0;i<10000;++i){const uint32_t value=(t<<24)|i;auto *d=data+3+t*8,*b=bss+3+t*8;guest_store<uint32_t>(d,value);assert(guest_load<uint32_t>(d)==value);{nfs::GuestCopyTransaction txn;txn.store<uint32_t>(b,value);assert(txn.load<uint32_t>(b)==value);txn.store<uint32_t>(memory.encode(b+4),value^0x13579bdf);assert(txn.load<uint32_t>(memory.encode(b+4))==(value^0x13579bdf));}assert(checked_native_span(allocations[2],16)==allocations[2]);++count;}});
 for(auto&t:threads)t.join();assert(count==40000);concurrent=count;
 }
 unbound();{nfs::EngineMemory second(32768);check(&_data,4);check(&_bss,7);}unbound();
 std::cout<<"{\"passed\":true,\"nativeSpanDifferentialCases\":"<<checks<<",\"concurrentNativeCopyCycles\":"<<concurrent<<",\"bindingTransitionsChecked\":true,\"exactErrorsPreserved\":true,\"engineObjectBytes\":"<<sizeof(nfs::EngineMemory)<<"}\n";
}

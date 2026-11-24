#include "EngineMemory.h"
#include "FrameSpanCacheAbi.h"
#include "DATA.h"
#include "BSS.h"
#include <cassert>
#include <cstring>
#include <memory>
#include <iostream>
#include <string>
#include <cerrno>
#include <cfenv>
alignas(4) DataLayout _data{};alignas(4) BssLayout _bss{};
static unsigned probes=0,backends=0;
extern "C" void*nfs_raw_host_address(uint32_t,size_t);
extern "C" void*nfs_raw_host_native_span(const void*,size_t);
extern "C" void*nfs_host_address(uint32_t a,size_t n){++backends;return nfs_raw_host_address(a,n);}
extern "C" void*nfs_host_native_span(const void*p,size_t n){++backends;return nfs_raw_host_native_span(p,n);}
static bool counted_probe(uint32_t a,size_t n,NfsFrameSpanToken*out)noexcept{++probes;return nfs_probe_frame_span(a,n,out);}
#define nfs_probe_frame_span counted_probe
#include "CachedFrameTransaction.h"
#undef nfs_probe_frame_span

template<class F>static std::string error(F f){try{f();return "";}catch(const std::exception&e){return e.what();}}
static void equal_error(nfs::CachedFrameTransaction &cache,uint32_t address,size_t length){
 auto expected=error([&]{(void)nfs_raw_host_address(address,length);});assert(!expected.empty());
 auto actual=error([&]{(void)cache.load<uint64_t>(address);});assert(length==8 && actual==expected);
}
int main(){
 {
  nfs::EngineMemory memory(65536);auto*frame=static_cast<unsigned char*>(memory.allocate(128));assert(frame);
  unsigned char*data[4];uint32_t ids[4];for(unsigned i=0;i<4;++i){data[i]=static_cast<unsigned char*>(memory.allocate(128));assert(data[i]);std::memset(data[i],0x11*(i+1),128);ids[i]=memory.encode(data[i]);}
  const auto base=memory.encode(frame);std::memset(frame,0x77,128);probes=backends=0;
  {
   nfs::CachedFrameTransaction cache(base);
   assert(cache.load<uint32_t>(base+4)==0x77777777 && probes==1 && backends==1);
   for(unsigned i=0;i<3;++i){auto before=backends;assert(cache.load<uint32_t>(ids[i])==0x11111111u*(i+1));assert(backends==before+1 && probes==i+2);assert(cache.load<uint32_t>(ids[i]+4)==0x11111111u*(i+1) && backends==before+1);}
   assert(probes==4);auto before=backends;
   assert(cache.load<uint32_t>(ids[3])==0x44444444 && backends==before+1 && probes==4);
   assert(cache.load<uint32_t>(ids[3]+4)==0x44444444 && backends==before+2 && probes==4);
   before=backends;assert(cache.load<unsigned char>(ids[0]+63)==0x11 && backends==before);
   assert(cache.load<uint64_t>(ids[0]+60)==0x1111111111111111ull && backends==before+1 && probes==4);
   assert(cache.load<unsigned char>(ids[0]+64)==0x11 && backends==before+2 && probes==4);
   cache.store<uint32_t>(ids[0]+8,0x12345678);uint32_t stored;std::memcpy(&stored,data[0]+8,4);assert(stored==0x12345678);
   unsigned char saved[128];std::memcpy(saved,data[0],128);auto expected=error([&]{(void)nfs_raw_host_address(ids[0]+124,8);});auto actual=error([&]{cache.store<uint64_t>(ids[0]+124,0);});assert(!expected.empty()&&expected==actual&&!std::memcmp(saved,data[0],128));
   void*unrelated=memory.allocate(16);assert(unrelated);before=backends;assert(cache.load<uint32_t>(ids[0]+8)==0x12345678 && backends==before+1 && probes==4);memory.release(unrelated);
   memory.release(data[0]);equal_error(cache,ids[0],8);
   auto*small=static_cast<unsigned char*>(memory.allocate(16));assert(small==data[0]);std::memset(small,0x5a,16);
   assert(cache.load<uint32_t>(ids[0]+4)==0x5a5a5a5a);equal_error(cache,ids[0]+12,8);memory.release(small);
  }
  for(unsigned i=1;i<4;++i)memory.release(data[i]);memory.release(frame);
 }
 {
  nfs::EngineMemory memory(65536);void*frame=memory.allocate(128);auto*tiny=static_cast<unsigned char*>(memory.allocate(16));assert(frame&&tiny);std::memset(tiny,0x66,16);auto id=memory.encode(tiny);probes=backends=0;
  {nfs::CachedFrameTransaction cache(memory.encode(frame));errno=ERANGE;std::feclearexcept(FE_ALL_EXCEPT);std::feraiseexcept(FE_OVERFLOW|FE_DIVBYZERO);const int flags=std::fetestexcept(FE_ALL_EXCEPT);
   assert(cache.load<uint32_t>(id)==0x66666666 && probes==1 && backends==1);
   assert(errno==ERANGE && std::fetestexcept(FE_ALL_EXCEPT)==flags);
   assert(cache.load<uint32_t>(id+4)==0x66666666 && probes==1 && backends==2);
   equal_error(cache,id+12,8);assert(probes==1);
  }
  memory.release(tiny);memory.release(frame);
 }
 {
  auto first=std::unique_ptr<nfs::EngineMemory>(new nfs::EngineMemory(65536));void*frame=first->allocate(128);void*data=first->allocate(128);assert(frame&&data);std::memset(data,0x33,128);auto frameId=first->encode(frame),dataId=first->encode(data);
  auto cache=std::unique_ptr<nfs::CachedFrameTransaction>(new nfs::CachedFrameTransaction(frameId));assert(cache->load<uint32_t>(dataId)==0x33333333);
  first.reset();equal_error(*cache,dataId,8);
  auto second=std::unique_ptr<nfs::EngineMemory>(new nfs::EngineMemory(65536));void*newFrame=second->allocate(128);void*newData=second->allocate(16);assert(second->encode(newFrame)==frameId && second->encode(newData)==dataId);std::memset(newData,0x99,16);
  assert(cache->load<uint32_t>(dataId)==0x99999999);equal_error(*cache,dataId+12,8);
  cache.reset();second->release(newData);second->release(newFrame);
 }
 std::cout<<"{\"passed\":true,\"framePlusThreeProbeBudgetChecked\":true,\"fourthWindowFallsBack\":true,\"widthCrossingAndNoPartialWriteChecked\":true,\"dataRevisionAndSmallerReuseChecked\":true,\"negativeProbeRemembered\":true,\"errnoAndFpFlagsPreserved\":true,\"retiredDataRevisionNotDereferenced\":true}\n";
}

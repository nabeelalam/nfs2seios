#include "EngineMemory.h"
#include "CachedFrameTransaction.h"
#include "DATA.h"
#include "BSS.h"
#include <cassert>
#include <cstring>
#include <memory>
#include <iostream>
#include <string>
#include <cerrno>
#include <cfenv>
#include <climits>
alignas(4) DataLayout _data{};alignas(4) BssLayout _bss{};
extern "C" void*nfs_raw_host_address(uint32_t,size_t);extern "C" void*nfs_raw_host_native_span(const void*,size_t);
static unsigned backends=0;
extern "C" void*nfs_host_address(uint32_t a,size_t n){++backends;return nfs_raw_host_address(a,n);}
extern "C" void*nfs_host_native_span(const void*p,size_t n){++backends;return nfs_raw_host_native_span(p,n);}
static unsigned checks=0;
template<class F>static std::string error(F f){try{f();return "";}catch(const std::logic_error&e){return std::string("logic:")+e.what();}catch(const std::exception&e){return std::string("other:")+e.what();}}
static void equal_error(uint32_t a,size_t n,const std::string&actual){auto expected=error([&]{(void)nfs_raw_host_address(a,n);});assert(!expected.empty()&&expected==actual);++checks;}
static void probe(uint32_t a,size_t n){void*expected=nullptr;bool valid=false;try{expected=nfs_raw_host_address(a,n);valid=true;}catch(const std::exception&){};NfsFrameSpanToken token;token.host=reinterpret_cast<void*>(uintptr_t(1));errno=EDOM;std::feclearexcept(FE_ALL_EXCEPT);std::feraiseexcept(FE_DIVBYZERO);int flags=std::fetestexcept(FE_ALL_EXCEPT);bool got=nfs_probe_frame_span(a,n,&token);assert(got==valid&&errno==EDOM&&std::fetestexcept(FE_ALL_EXCEPT)==flags);if(got){assert(token.host==expected&&token.base==a&&token.binding&&token.revision&&token.binding->load()==token.bindingValue&&token.revision->load()==token.revisionValue);}else assert(!token.host&&!token.binding&&!token.revision);++checks;}
int main(){
 probe(0,0);probe(0x10000000,64);assert(!nfs_probe_frame_span(0,4,nullptr));
 {nfs::CachedFrameTransaction cache(0x10000000);equal_error(0x10000000,4,error([&]{(void)cache.load<uint32_t>(0x10000000);}));}
 {
 nfs::EngineMemory memory(65536);auto*frame=static_cast<unsigned char*>(memory.allocate(128));void*adjacent=memory.allocate(32);assert(frame&&adjacent);auto base=memory.encode(frame);for(unsigned i=0;i<128;++i)frame[i]=static_cast<unsigned char>(i*13+5);
 for(uint32_t a:{0u,1u,0x00400000u,0x02000000u,base,base+1,base+63,base+124,base+128,0xffffffffu})for(size_t n:{size_t(0),size_t(1),size_t(4),size_t(8),size_t(64),size_t(128),SIZE_MAX})probe(a,n);
 {
  nfs::CachedFrameTransaction cache(base);unsigned before=backends;uint32_t expected;std::memcpy(&expected,frame+4,4);assert(cache.load<uint32_t>(base+4)==expected&&backends==before+1);assert(cache.load<uint32_t>(base+4)==expected&&backends==before+1);++checks;
  uint64_t wide;std::memcpy(&wide,frame+60,8);assert(cache.load<uint64_t>(base+60)==wide&&backends==before+2);++checks;
  equal_error(base+124,8,error([&]{(void)cache.load<uint64_t>(base+124);}));
  cache.store<uint64_t>(base+61,0x123456789abcdef0ull);std::memcpy(&wide,frame+61,8);assert(wide==0x123456789abcdef0ull);++checks;
  assert(cache.load<uint32_t>(frame+4)==expected);++checks;
  before=backends;void*unrelated=memory.allocate(16);assert(unrelated);assert(cache.load<uint32_t>(base+4)==expected&&backends==before+1);memory.release(unrelated);before=backends;assert(cache.load<uint32_t>(base+4)==expected&&backends==before+1);++checks;
 }
 {
  nfs::CachedFrameTransaction cache(base);(void)cache.load<uint32_t>(base+4);memory.release(frame);equal_error(base+4,4,error([&]{(void)cache.load<uint32_t>(base+4);}));auto*small=static_cast<unsigned char*>(memory.allocate(16));assert(small==frame);std::memset(small,0x6b,16);assert(cache.load<uint32_t>(base+4)==0x6b6b6b6bu);++checks;equal_error(base+20,4,error([&]{(void)cache.load<uint32_t>(base+20);}));unsigned char saved[16];std::memcpy(saved,small,16);equal_error(base+12,8,error([&]{cache.store<uint64_t>(base+12,0);}));assert(!std::memcmp(saved,small,16));++checks;memory.release(small);
 }
 memory.release(adjacent);
 auto*tiny=static_cast<unsigned char*>(memory.allocate(32));assert(tiny);auto tinyBase=memory.encode(tiny);std::memset(tiny,0x37,32);
 {nfs::CachedFrameTransaction cache(tinyBase);errno=ERANGE;std::feclearexcept(FE_ALL_EXCEPT);std::feraiseexcept(FE_OVERFLOW|FE_INEXACT);int flags=std::fetestexcept(FE_ALL_EXCEPT);auto before=backends;assert(cache.load<uint32_t>(tinyBase+4)==0x37373737u&&backends==before+1);assert(errno==ERANGE&&std::fetestexcept(FE_ALL_EXCEPT)==flags);assert(cache.load<uint32_t>(tinyBase+8)==0x37373737u&&backends==before+2);++checks;equal_error(tinyBase+30,4,error([&]{(void)cache.load<uint32_t>(tinyBase+30);}));}
 memory.release(tiny);
 }
 {
 auto first=std::unique_ptr<nfs::EngineMemory>(new nfs::EngineMemory(65536));auto*frame=first->allocate(128);assert(frame);auto base=first->encode(frame);std::memset(frame,0x42,128);
 {nfs::CachedFrameTransaction cache(base);assert(cache.load<uint32_t>(base)==0x42424242u);first.reset();equal_error(base,4,error([&]{(void)cache.load<uint32_t>(base);}));auto second=std::unique_ptr<nfs::EngineMemory>(new nfs::EngineMemory(65536));auto*next=second->allocate(128);assert(next&&second->encode(next)==base);std::memset(next,0x5c,128);assert(cache.load<uint32_t>(base)==0x5c5c5c5cu);++checks;second.reset();equal_error(base,4,error([&]{(void)cache.load<uint32_t>(base);}));}
 }
 probe(0x00400000,4);std::feclearexcept(FE_ALL_EXCEPT);
 std::cout<<"{\"passed\":true,\"checks\":"<<checks<<",\"probeMatchesCanonicalValidity\":true,\"probePreservesErrnoAndFpFlags\":true,\"fullWidthAndPartialWindowsChecked\":true,\"revisionInvalidationAndSmallerReuseChecked\":true,\"unboundAndRebindingChecked\":true,\"noRetiredRevisionPointerDereference\":true}"<<std::endl;
}

// SPDX-License-Identifier: MIT
// Tiny native component probe. No game code, original data, rendering or assets.
#include "GuestCopyTransaction.h"
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <stdexcept>
#include <thread>
#include <time.h>
extern "C" int32_t nfs_probe_accessor(nfs::GuestCopyTransaction*,uint32_t);
extern "C" void nfs_probe_transform(uint32_t,uint32_t);
extern "C" void nfs_probe_rep8(uint32_t&,uint32_t&,uint32_t&);
namespace {
constexpr uint32_t Base=0x1000;
constexpr size_t Bytes=8192;
std::array<unsigned char,Bytes> memory;
bool count_resolutions=true;
uint64_t resolutions=0,checks=0,proof_hash=1469598103934665603ull;
void require(bool okay){++checks;if(!okay)throw std::runtime_error("Probe oracle mismatch");}
void reset(){for(size_t i=0;i<Bytes;++i)memory[i]=unsigned((i*37+19)&255);}
uint64_t hash_bytes(const unsigned char*p,size_t n){uint64_t h=1469598103934665603ull;for(size_t i=0;i<n;++i){h^=p[i];h*=1099511628211ull;}return h;}
void seal(){proof_hash^=hash_bytes(memory.data(),Bytes);proof_hash*=1099511628211ull;}
int32_t read32(size_t offset){int32_t v=0;std::memcpy(&v,memory.data()+offset,sizeof v);return v;}
template<class E,class F> void expect(F fn){bool seen=false;try{fn();}catch(const E&){seen=true;}require(seen);}
void expect_bytes(const std::array<unsigned char,Bytes>& expected){require(memory==expected);seal();}
void reset_stop(nfs::WorkerControl& control){control.stop=false;nfs::current_worker_control()=&control;nfs::owned_critical_depth()=0;nfs::worker_stop_deferrals()=0;}
void verify(){
 nfs::WorkerControl control;reset_stop(control);resolutions=checks=0;proof_hash=1469598103934665603ull;
 // The caller TU and the actual probe-function TU must share the same TLS.
 for(unsigned alignment=0;alignment<4;++alignment){
  reset();const auto before=memory;const auto expected=read32(64+alignment);const auto prior=resolutions;
  {nfs::GuestCopyTransaction tx;require(nfs_probe_accessor(&tx,Base+64+alignment)==expected);}
  require(resolutions==prior+1);expect_bytes(before);
 }
 for(unsigned source_alignment=0;source_alignment<4;++source_alignment)
 for(unsigned target_alignment=0;target_alignment<4;++target_alignment){
  reset();auto expected=memory;const size_t src=64+source_alignment,dst=512+target_alignment;
  std::memcpy(expected.data()+dst,expected.data()+src,36);
  nfs_probe_transform(Base+uint32_t(src),Base+uint32_t(dst));expect_bytes(expected);
 }
 // Exact ends, forward overlap, backward overlap and alias; simulate the
 // original ordered 4-byte copies, never replace them with memmove semantics.
 for(const auto pair:std::array<std::array<size_t,2>,4>{{{{64,Bytes-36}},{{65,69}},{{69,65}},{{67,67}}}}){
  reset();auto expected=memory;for(unsigned k=0;k<9;++k){int32_t v;std::memcpy(&v,expected.data()+pair[0]+4*k,4);std::memcpy(expected.data()+pair[1]+4*k,&v,4);}
  nfs_probe_transform(Base+uint32_t(pair[0]),Base+uint32_t(pair[1]));expect_bytes(expected);
 }
 for(unsigned alignment=0;alignment<4;++alignment)for(unsigned initial: {0u,1u,7u,8u,9u}){
  reset();auto expected=memory;uint32_t src=Base+64+alignment,dst=Base+512+alignment,count=initial;
  const unsigned copied=initial<8?initial:8;for(unsigned k=0;k<copied;++k)std::memcpy(expected.data()+512+alignment+4*k,expected.data()+64+alignment+4*k,4);
  const auto prior=resolutions;nfs_probe_rep8(src,dst,count);
  require(src==Base+64+alignment+4*copied&&dst==Base+512+alignment+4*copied&&count==initial-copied);
  require(resolutions==prior+2*copied);expect_bytes(expected);
 }
 // Invalid full span faults before reading/writing out of bounds.
 for(uint32_t bad: {0u,Base-1,Base+uint32_t(Bytes)-3,Base+uint32_t(Bytes),UINT32_MAX}){
  reset();const auto expected=memory;const auto prior=resolutions;
  {nfs::GuestCopyTransaction tx;expect<std::out_of_range>([&]{(void)nfs_probe_accessor(&tx,bad);});}
  require(resolutions==prior+1);expect_bytes(expected);
 }
 // Partial transform failure retains the first eight writes, failing the ninth.
 reset();auto partial=memory;for(unsigned k=0;k<8;++k)std::memcpy(partial.data()+Bytes-32+4*k,partial.data()+64+4*k,4);
 expect<std::out_of_range>([]{nfs_probe_transform(Base+64,Base+uint32_t(Bytes)-32);});expect_bytes(partial);
 // A stop preempts even an invalid address before the resolver is entered.
 reset();auto unchanged=memory;const auto prior=resolutions;control.stop=true;
 expect<nfs::WorkerStopped>([]{nfs_probe_transform(0,0);});require(resolutions==prior);expect_bytes(unchanged);control.stop=false;
 {
  nfs::GuestCopyTransaction tx;control.stop=true;const auto before=resolutions;
  expect<nfs::WorkerStopped>([&]{(void)nfs_probe_accessor(&tx,0);});require(resolutions==before);
  // The existing transaction must observe live TLS slot values, not snapshots.
  nfs::owned_critical_depth()=1;require(nfs_probe_accessor(&tx,Base+64)==read32(64));nfs::owned_critical_depth()=0;
  nfs::worker_stop_deferrals()=1;require(nfs_probe_accessor(&tx,Base+64)==read32(64));nfs::worker_stop_deferrals()=0;
  nfs::WorkerControl alternate;alternate.stop=false;nfs::current_worker_control()=&alternate;require(nfs_probe_accessor(&tx,Base+64)==read32(64));
  nfs::current_worker_control()=&control;const auto again=resolutions;expect<nfs::WorkerStopped>([&]{(void)nfs_probe_accessor(&tx,Base+64);});require(resolutions==again);control.stop=false;
 }
 expect_bytes(unchanged);
 // A real second thread starts with independent slots; its caller and probe TUs
 // share that thread's new state, without contaminating the owner thread.
 bool thread_ok=false;std::exception_ptr thread_failure;
 std::thread other([&]{try{
  require(!nfs::current_worker_control()&&!nfs::owned_critical_depth()&&!nfs::worker_stop_deferrals());
  nfs::WorkerControl second;reset_stop(second);{nfs::GuestCopyTransaction tx;require(nfs_probe_accessor(&tx,Base+65)==read32(65));second.stop=true;const auto before=resolutions;expect<nfs::WorkerStopped>([&]{(void)nfs_probe_accessor(&tx,0);});require(resolutions==before);second.stop=false;}
  nfs::current_worker_control()=nullptr;thread_ok=true;
 }catch(...){thread_failure=std::current_exception();}});other.join();if(thread_failure)std::rethrow_exception(thread_failure);require(thread_ok);require(nfs::current_worker_control()==&control&&!nfs::owned_critical_depth()&&!nfs::worker_stop_deferrals());
 nfs::current_worker_control()=nullptr;
 std::printf("{\"type\":\"verification\",\"checks\":%llu,\"resolver_calls\":%llu,\"memory_checksum\":%llu,\"separate_probe_tu\":true,\"actual_thread_tls\":true}\n",(unsigned long long)checks,(unsigned long long)resolutions,(unsigned long long)proof_hash);
}
uint64_t clock_ns(clockid_t id){timespec t{};if(clock_gettime(id,&t)||t.tv_sec<0||t.tv_nsec<0)throw std::runtime_error("Probe clock unavailable");return uint64_t(t.tv_sec)*1000000000ull+uint64_t(t.tv_nsec);}
volatile uint64_t final_sink=0;
void measure(unsigned kernel){
 nfs::WorkerControl control;reset_stop(control);reset();count_resolutions=false;
 constexpr unsigned AccessorIterations=2000000,CopyIterations=100000;
 const unsigned iterations=kernel==0?AccessorIterations:CopyIterations;
 uint64_t checksum=0;const uint64_t wall_start=clock_ns(CLOCK_MONOTONIC),cpu_start=clock_ns(CLOCK_THREAD_CPUTIME_ID);
 if(kernel==0){nfs::GuestCopyTransaction tx;for(unsigned i=0;i<iterations;++i)checksum+=uint32_t(nfs_probe_accessor(&tx,Base+64+(i&3)));}
 else if(kernel==1){for(unsigned i=0;i<iterations;++i)nfs_probe_transform(Base+65,Base+513);}
 else {for(unsigned i=0;i<iterations;++i){uint32_t source=Base+65,target=Base+513,count=8;nfs_probe_rep8(source,target,count);checksum+=source+target+count;}}
 const uint64_t cpu_end=clock_ns(CLOCK_THREAD_CPUTIME_ID),wall_end=clock_ns(CLOCK_MONOTONIC);
 if(cpu_end<=cpu_start||wall_end<=wall_start)throw std::runtime_error("Probe clock did not advance");
 if(kernel==0){uint64_t expected=0;for(unsigned a=0;a<4;++a)expected+=uint32_t(read32(64+a));expected*=iterations/4;if(checksum!=expected)throw std::runtime_error("Accessor checksum mismatch");}
 else {for(unsigned k=0;k<(kernel==1?9u:8u);++k)if(read32(513+4*k)!=read32(65+4*k))throw std::runtime_error("Copy bytes mismatch");if(kernel==2&&checksum!=uint64_t(iterations)*(Base+65+32+Base+513+32))throw std::runtime_error("REP cursor mismatch");}
 checksum^=hash_bytes(memory.data(),Bytes);final_sink^=checksum;nfs::current_worker_control()=nullptr;
 const char*names[]={"accessor","transform9","rep8"};
 std::printf("{\"type\":\"measurement\",\"kernel\":\"%s\",\"iterations\":%u,\"cpu_ns\":%llu,\"wall_ns\":%llu,\"checksum\":%llu}\n",names[kernel],iterations,(unsigned long long)(cpu_end-cpu_start),(unsigned long long)(wall_end-wall_start),(unsigned long long)checksum);
}
}
// This resolver TU is separate from the existing noinline accessor/transform/REP
// TU, compiled without LTO. The probe compiler cannot fold its checks or memory.
extern "C" void*nfs_host_address(uint32_t address,size_t length){
 if(count_resolutions)++resolutions;
 if(address<Base||!length||uint64_t(address-Base)>=Bytes||length>Bytes-size_t(address-Base))throw std::out_of_range("Probe unmapped span");
 return memory.data()+size_t(address-Base);
}
extern "C" uint32_t nfs_guest_address(const void*pointer){
 const auto p=reinterpret_cast<uintptr_t>(pointer),begin=reinterpret_cast<uintptr_t>(memory.data());
 if(p<begin||p-begin>=Bytes)throw std::out_of_range("Probe unmapped pointer");return Base+uint32_t(p-begin);
}
int main(int argc,char**argv){try{
 if(argc==2&&!std::strcmp(argv[1],"--verify")){verify();return 0;}
 if(argc==3&&!std::strcmp(argv[1],"--measure")&&std::strlen(argv[2])==1&&argv[2][0]>='0'&&argv[2][0]<='2'){
  const unsigned order=unsigned(argv[2][0]-'0');for(unsigned k=0;k<3;++k)measure((order+k)%3);return 0;
 }
 std::fputs("Expected --verify or --measure0..2\n",stderr);return 2;
 }catch(...){std::fputs("Stop-slot runtime oracle failed\n",stderr);return 1;}}

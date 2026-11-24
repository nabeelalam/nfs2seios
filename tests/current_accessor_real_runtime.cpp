// SPDX-License-Identifier: MIT
// Synthetic finite accessor probe, not game code or a gameplay/energy claim.
#include "EngineMemory.h"
#include "GuestCopyTransaction.h"
#include "DATA.h"
#include "BSS.h"
#include <vector>
#include <cstring>
#include <cassert>
#include <iostream>
#include <chrono>
#include <time.h>
#include <cstdlib>
#include <string>
#include <stdexcept>
#include <thread>
#include <condition_variable>
alignas(4) DataLayout _data{};
alignas(4) BssLayout _bss{};
extern "C" uint32_t nfs_current_heap(uint32_t,uint32_t,unsigned);
extern "C" uint32_t nfs_current_native(void*,void*,unsigned);
extern "C" uint32_t nfs_current_mixed(uint32_t,uint32_t,void*,void*,unsigned);
static uint64_t clock_ns(clockid_t id){timespec t{};if(clock_gettime(id,&t))std::abort();return uint64_t(t.tv_sec)*1000000000ull+uint64_t(t.tv_nsec);}
// Keep a real peer alive so runtime mutex paths match a threaded process.
// The peer parks on its own condition variable and never takes the copy mutex.
struct ParkedPeer {
 std::mutex mutex;std::condition_variable changed;bool ready=false,done=false;
 std::thread worker;
 ParkedPeer():worker([this]{std::unique_lock<std::mutex> lock(mutex);ready=true;changed.notify_one();changed.wait(lock,[this]{return done;});}){
  std::unique_lock<std::mutex> lock(mutex);changed.wait(lock,[this]{return ready;});
 }
 ~ParkedPeer(){{std::lock_guard<std::mutex> lock(mutex);done=true;}changed.notify_one();worker.join();}
 ParkedPeer(const ParkedPeer&)=delete;ParkedPeer&operator=(const ParkedPeer&)=delete;
};
struct Inputs{uint32_t hs,hd;unsigned char*ns,*nd;};
static uint32_t invoke(unsigned mode,const Inputs&x,unsigned n){if(mode==0)return nfs_current_heap(x.hs,x.hd,n);if(mode==1)return nfs_current_native(x.ns,x.nd,n);return nfs_current_mixed(x.hs,x.hd,x.ns,x.nd,n);}
struct Image {std::vector<unsigned char> heap,data,bss;};
static Image snapshot(void*heap){Image out;out.heap.assign(static_cast<unsigned char*>(heap),static_cast<unsigned char*>(heap)+16384);auto d=reinterpret_cast<unsigned char*>(&_data),b=reinterpret_cast<unsigned char*>(&_bss);out.data.assign(d,d+sizeof(_data));out.bss.assign(b,b+sizeof(_bss));return out;}
static void restore(void*heap,const Image&x){std::memcpy(heap,x.heap.data(),x.heap.size());std::memcpy(&_data,x.data.data(),x.data.size());std::memcpy(&_bss,x.bss.data(),x.bss.size());}
static bool equal(const Image&a,const Image&b){return a.heap==b.heap&&a.data==b.data&&a.bss==b.bss;}
int main(int argc,char**argv){try{
 nfs::EngineMemory memory(65536);void*heap=memory.allocate(16384);assert(heap);auto base=memory.encode(heap);nfs::WorkerControl control;nfs::current_worker_control()=&control;
 auto h=static_cast<unsigned char*>(heap);auto ds=reinterpret_cast<unsigned char*>(&_data)+1024;auto bs=reinterpret_cast<unsigned char*>(&_bss)+1024;
 for(unsigned i=0;i<16384;++i)h[i]=static_cast<unsigned char>(i*17+9);
 for(unsigned i=0;i<512;++i){ds[i]=static_cast<unsigned char>(i*31+7);bs[i]=static_cast<unsigned char>(i*29+3);}
 auto pristine=snapshot(heap);Inputs ordinary{base+64,base+8192+1,ds+1,bs+3};
 if(argc==2&&std::string(argv[1])=="--verify"){
  unsigned cases=0;uint64_t digest=1469598103934665603ull;
  for(unsigned mode=0;mode<3;++mode)for(unsigned words:{0u,1u,4u,8u,16u,32u})for(unsigned alignment=0;alignment<4;++alignment)for(int delta:{-4,0,4,512}){
   restore(heap,pristine);Inputs x{base+256+alignment,base+uint32_t(256+int(alignment)+delta),bs+256+alignment,bs+256+int(alignment)+delta};Image before=snapshot(heap);uint32_t expected=0;
   // Reference simulation preserves forward read/store order, including overlap.
   for(unsigned i=0;i<words;++i){unsigned char*source=mode==0||(mode==2&&!(i&1))?static_cast<unsigned char*>(memory.resolve(x.hs+4*i,4)):x.ns+4*i;unsigned char*destination=mode==0||(mode==2&&(i&1))?static_cast<unsigned char*>(memory.resolve(x.hd+4*i,4)):x.nd+4*i;uint32_t value;std::memcpy(&value,source,4);auto output=value+1u;std::memcpy(destination,&output,4);expected^=value+i*0x9e3779b9u;}
   auto expected_image=snapshot(heap);restore(heap,before);auto value=invoke(mode,x,words);assert(value==expected&&equal(snapshot(heap),expected_image));++cases;digest^=value;digest*=1099511628211ull;
  }
  for(unsigned mode=0;mode<3;++mode){restore(heap,pristine);auto before=snapshot(heap);Inputs bad{0,0,nullptr,nullptr};bool fault=false;try{invoke(mode,bad,1);}catch(const std::out_of_range&){fault=true;}assert(fault&&equal(snapshot(heap),before));++cases;
   control.stop=true;bool stopped=false;try{invoke(mode,bad,1);}catch(const nfs::WorkerStopped&){stopped=true;}assert(stopped&&equal(snapshot(heap),before));control.stop=false;++cases;
   for(unsigned kind=0;kind<2;++kind){control.stop=true;if(kind==0)nfs::owned_critical_depth()=1;else nfs::worker_stop_deferrals()=1;(void)invoke(mode,ordinary,8);nfs::owned_critical_depth()=nfs::worker_stop_deferrals()=0;bool resumed=false;try{invoke(mode,bad,1);}catch(const nfs::WorkerStopped&){resumed=true;}assert(resumed);control.stop=false;++cases;}
  }
  assert(guest_copy_mutex().try_lock());guest_copy_mutex().unlock();nfs::current_worker_control()=nullptr;
  std::cout<<"{\"type\":\"verification\",\"transactionBytes\":"<<sizeof(nfs::GuestCopyTransaction)<<",\"cases\":"<<cases<<",\"checksum\":"<<digest<<",\"actualEngineMemory\":true,\"overlapAndStopPrecedenceVerified\":true}"<<std::endl;memory.release(heap);return 0;
 }
 if(argc==6&&std::string(argv[1])=="--measure"){
  for(unsigned i=2;i<6;++i)if(!argv[i][0]||std::strspn(argv[i],"0123456789")!=std::strlen(argv[i]))throw std::invalid_argument("Integer arguments required");
  auto mode=std::stoul(argv[2]),words=std::stoul(argv[3]),iterations=std::stoul(argv[4]),stop_mode=std::stoul(argv[5]);if(mode>2||!(words==1||words==4||words==8||words==16||words==32)||!iterations||iterations>2000000||stop_mode>2)throw std::invalid_argument("Finite workload required");
  ParkedPeer peer;
  if(stop_mode){control.stop=true;if(stop_mode==1)nfs::owned_critical_depth()=1;else nfs::worker_stop_deferrals()=1;}
  uint64_t checksum=0;auto wall_before=std::chrono::steady_clock::now();auto cpu_before=clock_ns(CLOCK_THREAD_CPUTIME_ID);
  for(unsigned long i=0;i<iterations;++i)checksum+=invoke(unsigned(mode),ordinary,unsigned(words));
  auto cpu=clock_ns(CLOCK_THREAD_CPUTIME_ID)-cpu_before;auto wall=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-wall_before).count();
  nfs::owned_critical_depth()=nfs::worker_stop_deferrals()=0;control.stop=false;nfs::current_worker_control()=nullptr;memory.release(heap);
  std::cout<<"{\"type\":\"measurement\",\"threadedProcess\":true,\"mode\":"<<mode<<",\"words\":"<<words<<",\"iterations\":"<<iterations<<",\"stopMode\":"<<stop_mode<<",\"cpuNs\":"<<cpu<<",\"wallNs\":"<<wall<<",\"checksum\":"<<checksum<<"}"<<std::endl;return 0;
 }
 throw std::invalid_argument("Expected verify or finite measure arguments");
 }catch(...){std::cerr<<"Current accessor probe failed"<<std::endl;return 1;}}

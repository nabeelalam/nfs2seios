#include "InputDevices.h"
#include "GuestAccess.h"
#include "HeapArena.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <climits>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);throw std::runtime_error("pointer contract");}}while(0)
static nfs::GameMemory memory;static nfs::HeapArena heap;static uint8_t storage[128*1024];
extern "C" void *malloc32(size_t size){return heap.allocate(size);}
extern "C" void free32(void *p){if(!heap.release(p))throw std::out_of_range("Pointer heap release");}
extern "C" void *nfs_host_address(uint32_t address,size_t size){auto p=memory.resolve(address,size);if(!p || (address>=0x10000000 && address<0x10020000 && !heap.contains(address,size)))throw std::out_of_range("Pointer span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t address=0;if(!memory.encode(p,address))throw std::out_of_range("Pointer address");return address;}
static void dummy(void *){}
extern "C" NativeGameFunction nfs_lookup_guest_function(uint32_t){return dummy;}
struct Event {uint32_t offset,data,time,sequence;};static_assert(sizeof(Event)==16,"Legacy mouse record");
int main(){try{
 CHECK(heap.initialize(memory,0x10000000,storage,sizeof(storage)));uint8_t bytes[16*1024];std::memset(bytes,0xcc,sizeof(bytes));CHECK(memory.map(0x80000000,bytes,sizeof(bytes)));const auto available=heap.available_bytes();
 {
  nfs::InputDevices devices(nullptr);nfs::bind_input_devices(&devices);CHECK(devices.create(0x500,0x80000003,0)==0);const auto root=guest_load<uint32_t>(0x80000003);
  guest_store(0x80000020,std::array<uint32_t,4>{0x6f1d2b60,0,0,0});uint32_t args[5]={root,0x80000020,0x80000043,0,0};CHECK(devices.invoke(nullptr,nfs::InputMethod::create_device,args)==0);const auto device=guest_load<uint32_t>(0x80000043);
  constexpr uint32_t buffer=0x80000103,count_address=0x80000080;
  auto read=[&](uint32_t wanted,uint32_t flags,uint32_t destination){args[0]=device;args[1]=16;args[2]=destination;args[3]=count_address;args[4]=flags;guest_store(count_address,wanted);return devices.invoke(nullptr,nfs::InputMethod::data,args);};
  auto count=[&](){return guest_load<uint32_t>(count_address);};auto event=[&](unsigned index){return guest_load<Event>(buffer+index*16);};
  CHECK(read(32,0,buffer)==0 && count()==0);
  devices.pointer_motion(5,-3,100);devices.pointer_motion(2,1,101);devices.pointer_click(102);
  CHECK(read(UINT32_MAX,1,0)==0 && count()==4);CHECK(read(UINT32_MAX,1,0)==0 && count()==4);
  CHECK(read(1,0,buffer)==0 && count()==1);CHECK(event(0).offset==0 && int32_t(event(0).data)==7 && event(0).time==101);const auto first_sequence=event(0).sequence;CHECK(bytes[0x102]==0xcc && bytes[0x113]==0xcc);
  CHECK(read(3,0,buffer)==0 && count()==3);CHECK(event(0).offset==4 && int32_t(event(0).data)==-2 && event(0).sequence>first_sequence);CHECK(event(1).offset==12 && event(1).data==0x80 && event(2).offset==12 && event(2).data==0 && event(1).sequence<event(2).sequence);CHECK(bytes[0x133]==0xcc);
  CHECK(read(32,0,buffer)==0 && count()==0);
  devices.pointer_motion(3,4,103);CHECK(read(0,0,buffer)==0 && count()==0);CHECK(read(UINT32_MAX,1,0)==0 && count()==2);CHECK(read(UINT32_MAX,0,0)==0 && count()==2);CHECK(read(UINT32_MAX,1,0)==0 && count()==0);
  devices.pointer_motion(INT32_MAX,INT32_MIN,104);devices.pointer_motion(1,-1,105);CHECK(read(32,0,buffer)==0 && count()==2);CHECK(int32_t(event(0).data)==INT32_MAX && int32_t(event(1).data)==INT32_MIN);
  devices.pointer_click(200);CHECK(read(1,0,buffer)==0 && count()==1 && event(0).data==0x80);devices.clear_pointer(201);CHECK(read(32,0,buffer)==0 && count()==1 && event(0).offset==12 && event(0).data==0);
  for(unsigned i=0;i<70;++i)devices.pointer_click(300+i);
  CHECK(read(UINT32_MAX,1,0)==1 && count()>0 && count()<=64);const auto pending=count();CHECK(read(64,1,buffer)==1 && count()==pending);CHECK(event(0).offset==12 && event(0).data==0 && event(pending-1).data==0);CHECK(read(64,0,buffer)==1 && count()==pending);CHECK(read(32,0,buffer)==0 && count()==0 && bytes[0x503]==0xcc);
  devices.pointer_motion(9,8,400);CHECK(read(32,2,buffer)<0 && count()==32);CHECK(read(UINT32_MAX,1,0)==0 && count()==2);
  args[0]=device;args[1]=20;args[2]=buffer;args[3]=count_address;args[4]=0;CHECK(devices.invoke(nullptr,nfs::InputMethod::data,args)<0);
  args[1]=16;args[3]=0;CHECK(devices.invoke(nullptr,nfs::InputMethod::data,args)<0);
  CHECK(read(UINT32_MAX,0,0)==0 && count()==2);devices.pointer_motion(0,0,401);CHECK(read(UINT32_MAX,1,0)==0 && count()==0);
  args[0]=device;CHECK(devices.invoke(nullptr,nfs::InputMethod::release,args)==0);args[0]=root;CHECK(devices.invoke(nullptr,nfs::InputMethod::release,args)==0);nfs::bind_input_devices(nullptr);
 }
 CHECK(heap.available_bytes()==available);std::puts("PASS: legacy16-byte mouse ABI, coalesced relative axes, ordered click/release, partial reads, peek, flush, overflow neutral recovery, range checks and lifetime");return 0;
 }catch(const std::exception &error){std::fprintf(stderr,"FAIL: %s\n",error.what());return 1;}}

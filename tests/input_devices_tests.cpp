#include "InputDevices.h"
#include "GuestAccess.h"
#include "HeapArena.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <thread>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static nfs::GameMemory memory;static nfs::HeapArena heap;static uint8_t storage[128*1024];
static unsigned enumerated=0;static bool stop_enumeration=false;
extern "C" void *malloc32(size_t size){return heap.allocate(size);}
extern "C" void free32(void *p){if(!heap.release(p))throw std::out_of_range("Input heap release");}
extern "C" void *nfs_host_address(uint32_t address,size_t size){auto p=memory.resolve(address,size);if(!p || (address>=0x10000000 && address<0x10020000 && !heap.contains(address,size)))throw std::out_of_range("Input span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t address=0;if(!memory.encode(p,address))throw std::out_of_range("Input pointer");return address;}
static void dummy(void *){}
extern "C" NativeGameFunction nfs_lookup_guest_function(uint32_t id){if(id!=0xdead0000)throw std::out_of_range("Input callback ID");return dummy;}
static int32_t enumerate(void *context,uint32_t function,uint32_t instance,uint32_t ref){
    if(context!=&memory || function!=0xdead0000 || ref!=0x1234 || guest_load<uint32_t>(instance)!=580 || guest_load<uint32_t>(instance+4)!=0x6f1d2b70 || guest_load<uint32_t>(instance+8)!=enumerated)throw std::runtime_error("Input enumeration frame");
    ++enumerated;return !stop_enumeration;
}
int main(){
    CHECK(reinterpret_cast<uintptr_t>(storage)>UINT32_MAX);
    CHECK(heap.initialize(memory,0x10000000,storage,sizeof(storage)));
    uint8_t bytes[1024];std::memset(bytes,0xcc,sizeof(bytes));CHECK(memory.map(0x80000000,bytes,sizeof(bytes)));
    const auto available=heap.available_bytes();
    {
        nfs::InputDevices devices(enumerate);nfs::bind_input_devices(&devices);
        CHECK(DirectInputCreateA_wrap(0,0,0x500,int32_t(0x80000003),0)==0);
        const auto root=guest_load<uint32_t>(0x80000003);CHECK(root && bytes[2]==0xcc && bytes[7]==0xcc);
        const auto table=guest_load<uint32_t>(root);
        CHECK(guest_load<uint32_t>(table+12)==nfs::input_function_id(nfs::InputMethod::create_device));
        CHECK(nfs::input_argument_count(nfs::InputMethod::create_device)==4 && SDL_NumJoysticks_wrap()==2);
        uint32_t args[5]={root,4,0xdead0000,0x1234,0};CHECK(devices.invoke(&memory,nfs::InputMethod::enumerate,args)==0 && enumerated==2);
        stop_enumeration=true;enumerated=0;CHECK(devices.invoke(&memory,nfs::InputMethod::enumerate,args)==0 && enumerated==1);
        guest_store(0x80000020,std::array<uint32_t,4>{0x6f1d2b70,1,0,0});
        args[1]=0x80000020;args[2]=0x80000043;args[3]=0;
        CHECK(devices.invoke(nullptr,nfs::InputMethod::create_device,args)==0);const auto device=guest_load<uint32_t>(0x80000043);
        args[0]=device;args[1]=80;args[2]=0x80000063;
        CHECK(devices.invoke(nullptr,nfs::InputMethod::state,args)==0);
        auto state=guest_load<nfs::GameJoystickState>(0x80000063);CHECK(state.axes[0]==32768 && state.pov[0]==UINT32_MAX && !state.buttons[0]);
        nfs::GameJoystickState sample;sample.axes[0]=65535;sample.axes[6]=0;sample.buttons[31]=0x80;devices.publish(1,&sample);
        CHECK(devices.invoke(nullptr,nfs::InputMethod::state,args)==0);state=guest_load<nfs::GameJoystickState>(0x80000063);CHECK(state.axes[0]==65535 && state.axes[6]==0 && state.buttons[31]==0x80);
        devices.publish(1,nullptr);CHECK(devices.invoke(nullptr,nfs::InputMethod::state,args)==0);state=guest_load<nfs::GameJoystickState>(0x80000063);CHECK(state.axes[0]==32768 && !state.buttons[31]);
        CHECK(bytes[0x62]==0xcc && bytes[0xb3]==0xcc);
        args[1]=79;CHECK(devices.invoke(nullptr,nfs::InputMethod::state,args)<0);
        args[1]=0x80000103;guest_store<uint32_t>(args[1],44);CHECK(devices.invoke(nullptr,nfs::InputMethod::capabilities,args)==0);
        CHECK(guest_load<uint32_t>(args[1]+4)==0 && guest_load<uint32_t>(args[1]+12)==6 && guest_load<uint32_t>(args[1]+16)==32 && bytes[0x102]==0xcc && bytes[0x12f]==0xcc);
        args[1]=0;args[2]=0x80000003;CHECK(devices.invoke(nullptr,nfs::InputMethod::query,args)==0 && guest_load<uint32_t>(args[2])==device);
        CHECK(devices.invoke(nullptr,nfs::InputMethod::release,args)==1);CHECK(devices.invoke(nullptr,nfs::InputMethod::release,args)==0);
        CHECK(devices.invoke(nullptr,nfs::InputMethod::acquire,args)<0);
        args[0]=root;CHECK(devices.invoke(nullptr,nfs::InputMethod::release,args)==0);
        CHECK(heap.available_bytes()==available);
        // Concurrent publication is copied as one complete 80-byte record.
        CHECK(devices.create(0x500,0x80000003,0)==0);args[0]=guest_load<uint32_t>(0x80000003);args[1]=0x80000020;args[2]=0x80000043;args[3]=0;
        CHECK(devices.invoke(nullptr,nfs::InputMethod::create_device,args)==0);args[0]=guest_load<uint32_t>(0x80000043);args[1]=80;args[2]=0x80000063;
        nfs::GameJoystickState initial;for(auto &axis:initial.axes)axis=0;devices.publish(1,&initial);
        std::thread publisher([&]{for(unsigned i=0;i<5000;++i){nfs::GameJoystickState s;for(auto &axis:s.axes)axis=int32_t(i);devices.publish(1,&s);}});
        for(unsigned i=0;i<5000;++i){CHECK(devices.invoke(nullptr,nfs::InputMethod::state,args)==0);const auto s=guest_load<nfs::GameJoystickState>(args[2]);for(auto axis:s.axes)CHECK(axis==s.axes[0]);}publisher.join();
        // The runtime bounds facade count independently of available heap.
        for(unsigned i=0;i<62;++i)CHECK(devices.create(0x500,0x80000003,0)==0);
        const auto before_failure=heap.available_bytes();
        CHECK(devices.create(0x500,0x80000003,0)<0 && guest_load<uint32_t>(0x80000003)==0 && heap.available_bytes()==before_failure);
        CHECK(devices.create(0x400,0x80000003,0)<0 && devices.create(0x500,0x80000003,1)<0);
        // Both live facades are deliberately retired by runtime destruction.
        nfs::bind_input_devices(nullptr);
    }
    CHECK(heap.available_bytes()==available);
    std::puts("PASS: mapped input facades/function IDs, enumeration/callback stop, 80-byte snapshots, reconnect neutral state, references, native lifetime and concurrent publication");
}

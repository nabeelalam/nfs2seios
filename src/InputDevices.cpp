// SPDX-License-Identifier: MIT
#include "InputDevices.h"
#include "GuestAccess.h"
#include <cstring>
#include <limits>
#include <stdexcept>
extern "C" void *malloc32(size_t);
extern "C" void free32(void *);
namespace nfs {
namespace {
constexpr int32_t invalid=int32_t(0x80070057u),unsupported=int32_t(0x80004001u);
constexpr uint32_t mouse=0x6f1d2b60,joystick=0x6f1d2b70;
struct Instance {uint32_t size,instance[4],product[4],type;char name[260],product_name[260];uint32_t force[4];uint16_t page,usage;};
static_assert(sizeof(Instance)==580,"Original device instance ABI");
Instance instance(uint32_t kind,uint32_t slot){Instance value{};value.size=sizeof(value);value.instance[0]=kind;value.instance[1]=slot;value.product[0]=kind;value.type=kind==joystick?4:2;std::strcpy(value.name,kind==joystick?"iPad Controller":"iPad Menu Pointer");std::strcpy(value.product_name,value.name);return value;}
struct GuestAllocation {void *pointer;explicit GuestAllocation(size_t size):pointer(malloc32(size)){}~GuestAllocation(){if(pointer)free32(pointer);}GuestAllocation(const GuestAllocation &)=delete;};
InputDevices *active=nullptr;
}
GameJoystickState::GameJoystickState(){for(auto &axis:axes)axis=32768;axes[6]=axes[7]=0;for(auto &hat:pov)hat=UINT32_MAX;std::memset(buttons,0,sizeof(buttons));}
struct InputDevices::Object {
    GuestAllocation storage{33*4};bool device;uint32_t kind,slot,refs=1;
    Object(bool is_device,uint32_t type,uint32_t index):device(is_device),kind(type),slot(index){}
};
InputDevices::~InputDevices()=default;
std::shared_ptr<InputDevices::Object> InputDevices::find(uint32_t handle){std::lock_guard<std::mutex> lock(mutex);auto it=objects.find(handle);return it==objects.end()?nullptr:it->second;}
uint32_t input_argument_count(InputMethod method){
    switch(method){
    case InputMethod::addref:case InputMethod::release:case InputMethod::acquire:case InputMethod::unacquire:case InputMethod::poll:return 1;
    case InputMethod::capabilities:case InputMethod::format:case InputMethod::event:case InputMethod::device_info:case InputMethod::feedback:case InputMethod::feedback_state:case InputMethod::escape:case InputMethod::status:return 2;
    case InputMethod::query:case InputMethod::state:case InputMethod::cooperative:case InputMethod::property:case InputMethod::get_property:case InputMethod::effect_info:case InputMethod::control:case InputMethod::root_initialize:return 3;
    case InputMethod::object_info:case InputMethod::enum_objects:case InputMethod::initialize:case InputMethod::enum_effects:case InputMethod::enum_created:case InputMethod::create_device:return 4;
    case InputMethod::enumerate:case InputMethod::data:case InputMethod::effect:case InputMethod::send_data:return 5;
    default:throw std::invalid_argument("Unknown input method");
    }
}
uint32_t InputDevices::make(bool device,uint32_t kind,uint32_t slot){
    ScopedWorkerStopDeferral defer;
    auto object=std::make_shared<Object>(device,kind,slot);if(!object->storage.pointer)return 0;
    const uint32_t handle=nfs_guest_address(object->storage.pointer);
    std::array<uint32_t,33> words{};words[0]=handle+4;
    const auto set=[&](size_t index,InputMethod method){words[index+1]=input_function_id(method);};
    set(0,InputMethod::query);set(1,InputMethod::addref);set(2,InputMethod::release);
    if(!device){set(3,InputMethod::create_device);set(4,InputMethod::enumerate);set(5,InputMethod::status);set(6,InputMethod::control);set(7,InputMethod::root_initialize);}
    else {
        const InputMethod methods[]={InputMethod::capabilities,InputMethod::enum_objects,InputMethod::get_property,InputMethod::property,InputMethod::acquire,InputMethod::unacquire,InputMethod::state,InputMethod::data,InputMethod::format,InputMethod::event,InputMethod::cooperative,InputMethod::object_info,InputMethod::device_info,InputMethod::control,InputMethod::initialize,InputMethod::effect,InputMethod::enum_effects,InputMethod::effect_info,InputMethod::feedback_state,InputMethod::feedback,InputMethod::enum_created,InputMethod::escape,InputMethod::poll,InputMethod::send_data};
        for(size_t i=0;i<sizeof(methods)/sizeof(methods[0]);++i)set(i+3,methods[i]);
    }
    guest_store(handle,words);
    std::lock_guard<std::mutex> lock(mutex);if(objects.size()>=64)return 0;
    if(!objects.emplace(handle,std::move(object)).second)throw std::logic_error("Duplicate input allocation");return handle;
}
int32_t InputDevices::create(uint32_t version,uint32_t output,uint32_t outer){
    if(!output || outer || version!=0x500)return invalid;
    nfs_host_address(output,4);ScopedWorkerStopDeferral defer;
    auto handle=make(false,0,0);guest_store(output,handle);return handle?0:int32_t(0x8007000eu);
}
void InputDevices::publish(uint32_t slot,const GameJoystickState *state){if(slot>=2)throw std::out_of_range("Controller slot");std::lock_guard<std::mutex> lock(mutex);snapshots[slot]=state?*state:GameJoystickState{};}
int32_t InputDevices::invoke(void *context,InputMethod method,const uint32_t *a){
    auto object=find(a[0]);if(!object)return invalid;
    if(method==InputMethod::addref || method==InputMethod::release){
        std::lock_guard<std::mutex> lock(mutex);auto it=objects.find(a[0]);if(it==objects.end())return invalid;
        if(method==InputMethod::addref){if(object->refs==UINT32_MAX)return invalid;return int32_t(++object->refs);}
        const auto refs=--object->refs;if(!refs)objects.erase(it);return int32_t(refs);
    }
    if(method==InputMethod::query){if(!a[2])return invalid;nfs_host_address(a[2],4);ScopedWorkerStopDeferral defer;uint32_t args[]={a[0]};auto refs=invoke(context,InputMethod::addref,args);if(uint32_t(refs)==uint32_t(invalid))return invalid;guest_store(a[2],a[0]);return 0;}
    if(!object->device){
        if(method==InputMethod::create_device){
            if(!a[1] || !a[2] || a[3])return invalid;
            const auto guid=guest_load<std::array<uint32_t,4>>(a[1]);nfs_host_address(a[2],4);
            if((guid[0]!=mouse && guid[0]!=joystick) || (guid[0]==joystick && guid[1]>=2)){guest_store<uint32_t>(a[2],0);return invalid;}
            ScopedWorkerStopDeferral defer;const auto device=make(true,guid[0],guid[1]);guest_store(a[2],device);return device?0:int32_t(0x8007000eu);
        }
        if(method==InputMethod::enumerate){
            if(a[1]!=4)return 0;if(!callback || !a[2])return invalid;
            nfs_lookup_guest_function(a[2]);GuestAllocation allocation(sizeof(Instance));if(!allocation.pointer)return int32_t(0x8007000eu);
            const auto address=nfs_guest_address(allocation.pointer);
            for(uint32_t slot=0;slot<2;++slot){guest_store(address,instance(joystick,slot));if(!callback(context,a[2],address,a[3]))break;}return 0;
        }
        return unsupported;
    }
    switch(method){
    case InputMethod::state:{
        if(object->kind!=joystick || a[1]!=80 || !a[2])return invalid;
        GameJoystickState state;{std::lock_guard<std::mutex> lock(mutex);state=snapshots[object->slot];}guest_store(a[2],state);return 0;
    }
    case InputMethod::capabilities:{
        if(!a[1])return invalid;const auto size=guest_load<uint32_t>(a[1]);if(size!=44 && size!=24)return invalid;
        std::array<uint32_t,11> caps{};caps[0]=size;caps[2]=object->kind==joystick?4:2;caps[3]=object->kind==joystick?6:2;caps[4]=object->kind==joystick?32:1;caps[5]=object->kind==joystick?1:0;
        check_worker_stop();std::lock_guard<std::mutex> lock(guest_copy_mutex());std::memcpy(nfs_host_address(a[1],size),caps.data(),size);return 0;
    }
    case InputMethod::device_info:if(!a[1] || guest_load<uint32_t>(a[1])!=580)return invalid;guest_store(a[1],instance(object->kind,object->slot));return 0;
    case InputMethod::object_info:{
        if(!a[1])return invalid;const auto size=guest_load<uint32_t>(a[1]);if(size!=316 && size!=292)return invalid;
        // The pinned candidate supplies a zero object descriptor. Preserve its
        // behavior with an exact caller-sized copy; no native SDL fields escape.
        std::array<uint8_t,316> info{};std::memcpy(info.data(),&size,4);
        check_worker_stop();std::lock_guard<std::mutex> lock(guest_copy_mutex());std::memcpy(nfs_host_address(a[1],size),info.data(),size);return 0;
    }
    case InputMethod::data: // Menu input is delivered through window messages.
        if(!a[3] || a[1]!=16)return invalid;guest_store<uint32_t>(a[3],0);return 0;
    case InputMethod::format:if(!a[1])return invalid;return guest_load<uint32_t>(a[1])==24?0:invalid;
    case InputMethod::effect:if(a[3])guest_store<uint32_t>(a[3],0);return unsupported;
    case InputMethod::acquire:case InputMethod::unacquire:case InputMethod::poll:case InputMethod::event:case InputMethod::cooperative:case InputMethod::property:return 0;
    default:return unsupported;
    }
}
void bind_input_devices(InputDevices *devices){active=devices;}
InputDevices &input_devices(){if(!active)throw std::logic_error("Input runtime unbound");return *active;}
}
extern "C" int32_t DirectInputCreateA_wrap(int32_t,int32_t,int32_t version,int32_t output,int32_t outer){return nfs::input_devices().create(uint32_t(version),uint32_t(output),uint32_t(outer));}
// Preserve the original wrapper's two stable virtual slots. Availability is
// represented by neutral snapshots; hardware reconnect never reorders slots.
extern "C" int32_t SDL_NumJoysticks_wrap(){return 2;}

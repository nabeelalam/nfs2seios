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
struct PointerEvent {uint32_t offset,data,time,sequence;};
static_assert(sizeof(PointerEvent)==16,"Pinned legacy buffered mouse ABI");
struct PointerQueue {
    std::array<PointerEvent,64> events{};unsigned count=0;uint32_t sequence=0;bool overflow=false;
    static uint32_t add_delta(uint32_t old,int32_t delta){
        const int64_t sum=int64_t(int32_t(old))+delta;
        return uint32_t(int32_t(sum>INT32_MAX?INT32_MAX:sum<INT32_MIN?INT32_MIN:sum));
    }
    void append(uint32_t offset,uint32_t data,uint32_t now){events[count++]={offset,data,now,++sequence};}
    void reserve_pair(uint32_t now){
        if(count+2>events.size()){
            count=0;overflow=true;append(12,0,now); // Recover neutral state before the newest input.
        }
    }
    void motion(int32_t dx,int32_t dy,uint32_t now){
        if(!dx && !dy)return;
        if(count>=2 && events[count-2].offset==0 && events[count-1].offset==4){
            auto &x=events[count-2];auto &y=events[count-1];x.data=add_delta(x.data,dx);y.data=add_delta(y.data,dy);
            x.time=y.time=now;x.sequence=++sequence;y.sequence=++sequence;return;
        }
        reserve_pair(now);append(0,uint32_t(dx),now);append(4,uint32_t(dy),now);
    }
    void click(uint32_t now){reserve_pair(now);append(12,0x80,now);append(12,0,now);}
    void reset(uint32_t now){count=0;overflow=false;append(12,0,now);}
};
InputDevices *active=nullptr;
}
GameJoystickState::GameJoystickState(){for(auto &axis:axes)axis=32768;axes[6]=axes[7]=0;for(auto &hat:pov)hat=UINT32_MAX;std::memset(buttons,0,sizeof(buttons));}
struct InputDevices::Object {
    GuestAllocation storage{33*4};bool device;uint32_t kind,slot,refs=1;
    std::unique_ptr<PointerQueue> pointer;
    Object(bool is_device,uint32_t type,uint32_t index):device(is_device),kind(type),slot(index){if(is_device && type==mouse)pointer.reset(new PointerQueue);}
};
InputDevices::~InputDevices()=default;
void InputDevices::pointer_motion(int32_t dx,int32_t dy,uint32_t now){
    std::lock_guard<std::mutex> lock(mutex);for(auto &entry:objects)if(entry.second->pointer)entry.second->pointer->motion(dx,dy,now);
}
void InputDevices::pointer_click(uint32_t now){
    std::lock_guard<std::mutex> lock(mutex);for(auto &entry:objects)if(entry.second->pointer)entry.second->pointer->click(now);
}
void InputDevices::clear_pointer(uint32_t now){
    std::lock_guard<std::mutex> lock(mutex);for(auto &entry:objects)if(entry.second->pointer)entry.second->pointer->reset(now);
}
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
static uint8_t held_frontend_direction(const std::array<GameJoystickState,2> &snapshots);
void InputDevices::publish(uint32_t slot,const GameJoystickState *state){
    if(slot>=2)throw std::out_of_range("Controller slot");
    std::lock_guard<std::mutex> lock(mutex);
    const auto previous_direction=held_frontend_direction(snapshots);
    const auto previous=snapshots[slot];
    if(!state)frontend_button_edges[slot]=0;
    snapshots[slot]=state?*state:GameJoystickState{};
    if(!state)frontend_cancel_pending&=uint8_t(~(1u<<slot));
    if(!state)frontend_confirm_pending&=uint8_t(~(1u<<slot));
    if(!state && slot==0){reset_held=false;reset_consumed=false;}
    if(!state){
        unsigned retained=0;
        for(unsigned i=0;i<frontend_press_count;++i)
            if(frontend_presses[i].slot!=slot)frontend_presses[retained++]=frontend_presses[i];
        frontend_press_count=retained;
    }else{
        const auto direction=held_frontend_direction(snapshots);
        if(direction && direction!=previous_direction){
            // Bounded retention: keep the latest eight transitions if a busy
            // original frontend has not consumed older presses yet.
            if(frontend_press_count==frontend_presses.size()){
                for(unsigned i=1;i<frontend_press_count;++i)frontend_presses[i-1]=frontend_presses[i];
                --frontend_press_count;
            }
            frontend_presses[frontend_press_count++]={direction,uint8_t(slot)};
        }
    }
    for(unsigned i=0;i<32;++i){
        const uint32_t bit=uint32_t(1)<<i;
        if(!snapshots[slot].buttons[i])consumed_buttons[slot]&=~bit;
        else if(consumed_buttons[slot]&bit)snapshots[slot].buttons[i]=0;
        if((frontend_capture || binding_capture) && snapshots[slot].buttons[i] && !previous.buttons[i])frontend_button_edges[slot]|=bit;
    }
}
void InputDevices::begin_frontend(){
    std::lock_guard<std::mutex> lock(mutex);if(!binding_capture)frontend_capture=true;
}
void InputDevices::begin_driving(){
    std::lock_guard<std::mutex> lock(mutex);frontend_capture=binding_capture=false;frontend_button_edges[0]=frontend_button_edges[1]=0;
}
void InputDevices::begin_binding(){
    std::lock_guard<std::mutex> lock(mutex);frontend_capture=false;binding_capture=true;frontend_button_edges[0]=frontend_button_edges[1]=0;
}
void InputDevices::end_binding(){
    std::lock_guard<std::mutex> lock(mutex);frontend_capture=binding_capture=false;consume_held_locked();
}
uint32_t InputDevices::binding_buttons(uint32_t slot){
    std::lock_guard<std::mutex> lock(mutex);
    if(!binding_capture || slot>=2)return 0;
    const auto pressed=frontend_button_edges[slot];frontend_button_edges[slot]=0;return pressed;
}
void InputDevices::consume_held_buttons(){
    std::lock_guard<std::mutex> lock(mutex);consume_held_locked();
}
void InputDevices::consume_held_locked(){
    frontend_cancel_pending=false;
    frontend_confirm_pending=0;
    frontend_button_edges[0]=frontend_button_edges[1]=0;
    reset_consumed=reset_held;
    frontend_press_count=0;
    for(unsigned slot=0;slot<2;++slot)for(unsigned i=0;i<32;++i){
        if(snapshots[slot].buttons[i])consumed_buttons[slot]|=uint32_t(1)<<i;
        snapshots[slot].buttons[i]=0;
    }
}
static uint8_t held_frontend_direction(const std::array<GameJoystickState,2> &snapshots){
    uint8_t directions=0;
    for(const auto &state:snapshots)switch(state.pov[0]){
    case 0:directions|=1;break;case 4500:directions|=1|8;break;
    case 9000:directions|=8;break;case 13500:directions|=2|8;break;
    case 18000:directions|=2;break;case 22500:directions|=2|4;break;
    case 27000:directions|=4;break;case 31500:directions|=1|4;break;
    default:break;
    }
    if((directions&3)==3)directions&=uint8_t(~3);
    if((directions&12)==12)directions&=uint8_t(~12);
    // A diagonal makes one selection change, with vertical navigation first.
    return (directions&1)?1:(directions&2)?2:(directions&4)?4:(directions&8)?8:0;
}
uint32_t InputDevices::frontend_direction_actions(){
    std::lock_guard<std::mutex> lock(mutex);
    // The original Pause frontend tests action bits1..4 and implements its
    // own repeat. It does not consume the character-menu command queue.
    return uint32_t(held_frontend_direction(snapshots))<<1;
}
uint32_t InputDevices::frontend_hat_command(uint64_t now){
    std::lock_guard<std::mutex> lock(mutex);
    const bool queued=frontend_press_count!=0;
    const uint8_t selected=queued?frontend_presses[0].direction:held_frontend_direction(snapshots);
    if(queued){
        for(unsigned i=1;i<frontend_press_count;++i)frontend_presses[i-1]=frontend_presses[i];
        --frontend_press_count;
    }
    if(!selected){frontend_direction=0;frontend_repeat=0;return 0;}
    if(!queued && selected==frontend_direction && now<frontend_repeat)return 0;
    frontend_repeat=now+(!queued && selected==frontend_direction?120:400);
    frontend_direction=selected;
    return selected==1?0x4800:selected==2?0x5000:selected==4?0x4b00:0x4d00;
}
bool InputDevices::consume_buttons_locked(uint32_t mask){
    bool pressed=false;
    for(unsigned slot=0;slot<2;++slot){
        if(frontend_capture && (frontend_button_edges[slot]&mask))pressed=true;
        frontend_button_edges[slot]&=~mask;
    }
    for(unsigned slot=0;slot<2;++slot)for(unsigned i=0;i<32;++i){
        const uint32_t bit=uint32_t(1)<<i;
        if((mask&bit) && snapshots[slot].buttons[i]){
            consumed_buttons[slot]|=bit;snapshots[slot].buttons[i]=0;pressed=true;
        }
    }
    return pressed;
}
bool InputDevices::consume_frontend_buttons(uint32_t mask){
    std::lock_guard<std::mutex> lock(mutex);return !binding_capture && consume_buttons_locked(mask);
}
void InputDevices::capture_frontend_cancel(uint32_t mask){
    std::lock_guard<std::mutex> lock(mutex);
    for(unsigned slot=0;slot<2;++slot)for(unsigned i=0;i<32;++i)
        if((mask&(uint32_t(1)<<i)) && snapshots[slot].buttons[i])frontend_cancel_pending|=uint8_t(1u<<slot);
    consume_buttons_locked(mask);
}
bool InputDevices::take_frontend_cancel(uint32_t mask){
    std::lock_guard<std::mutex> lock(mutex);
    if(binding_capture)return false;
    if(frontend_cancel_pending){frontend_cancel_pending=false;frontend_confirm_pending=0;return true;}
    const bool pressed=consume_buttons_locked(mask);
    if(pressed){frontend_confirm_pending=0;frontend_button_edges[0]=frontend_button_edges[1]=0;}
    return pressed;
}
void InputDevices::capture_pause_buttons(uint32_t cancel_mask){
    std::lock_guard<std::mutex> lock(mutex);
    // The original Pause UI accepts any action button. Capture that semantic
    // choice before its joystick refresh also exposes the button to driving.
    for(unsigned slot=0;slot<2;++slot)for(unsigned i=0;i<32;++i)
        if(snapshots[slot].buttons[i]){
            if(cancel_mask&(uint32_t(1)<<i))frontend_cancel_pending|=uint8_t(1u<<slot);
            else frontend_confirm_pending|=uint8_t(1u<<slot);
        }
    consume_buttons_locked(~uint32_t(0));
}
uint32_t InputDevices::pause_actions(){
    std::lock_guard<std::mutex> lock(mutex);
    const auto confirm=frontend_confirm_pending;frontend_confirm_pending=0;
    return (uint32_t(held_frontend_direction(snapshots))<<1)|(confirm?uint32_t(1)<<5:0);
}
void InputDevices::publish_reset(bool pressed){
    std::lock_guard<std::mutex> lock(mutex);
    reset_held=pressed;if(!pressed)reset_consumed=false;
}
bool InputDevices::take_reset(){
    std::lock_guard<std::mutex> lock(mutex);
    if(!reset_held || reset_consumed)return false;
    reset_consumed=true;return true;
}
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
    case InputMethod::data:{
        if(!a[3] || a[1]!=16)return invalid;
        if(!object->pointer){guest_store<uint32_t>(a[3],0);return 0;}
        if(a[4]&~uint32_t(1))return invalid;
        const uint32_t requested=guest_load<uint32_t>(a[3]);
        const uint32_t limit=requested<64?requested:64;
        if(a[2] && limit)nfs_host_address(a[2],size_t(limit)*16);
        ScopedWorkerStopDeferral defer;
        std::array<PointerEvent,64> copied;uint32_t take=0;int32_t result=0;
        {
            std::lock_guard<std::mutex> lock(mutex);auto &queue=*object->pointer;
            take=limit<queue.count?limit:queue.count;result=queue.overflow?1:0;
            for(uint32_t i=0;i<take;++i)copied[i]=queue.events[i];
            if(!(a[4]&1)){
                for(uint32_t i=take;i<queue.count;++i)queue.events[i-take]=queue.events[i];
                queue.count-=take;queue.overflow=false;
            }
        }
        if(a[2] && take){check_worker_stop();std::lock_guard<std::mutex> lock(guest_copy_mutex());std::memcpy(nfs_host_address(a[2],size_t(take)*16),copied.data(),size_t(take)*16);}
        guest_store<uint32_t>(a[3],take);return result;
    }
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

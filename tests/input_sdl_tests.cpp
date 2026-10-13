#include "InputDevices.h"
#include "MenuInput.h"
#include "GuestAccess.h"
#include "HeapArena.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <stdexcept>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL line %d: %s; SDL: %s\n",__LINE__,#x,SDL_GetError());return 1;}}while(0)
static nfs::GameMemory memory;static nfs::HeapArena heap;static uint8_t storage[65536],output[256];
extern "C" void *malloc32(size_t size){return heap.allocate(size);}
extern "C" void free32(void *p){if(!heap.release(p))throw std::out_of_range("SDL input release");}
extern "C" void *nfs_host_address(uint32_t address,size_t size){auto p=memory.resolve(address,size);if(!p)throw std::out_of_range("SDL input span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t address=0;if(!memory.encode(p,address))throw std::out_of_range("SDL input pointer");return address;}
extern "C" NativeGameFunction nfs_lookup_guest_function(uint32_t){throw std::out_of_range("Unused SDL enumeration callback");}
static SDL_JoystickID attach(){SDL_VirtualJoystickDesc desc;SDL_INIT_INTERFACE(&desc);desc.type=SDL_JOYSTICK_TYPE_GAMEPAD;desc.naxes=SDL_GAMEPAD_AXIS_COUNT;desc.nbuttons=SDL_GAMEPAD_BUTTON_COUNT;desc.axis_mask=(1u<<SDL_GAMEPAD_AXIS_COUNT)-1;desc.button_mask=(1u<<SDL_GAMEPAD_BUTTON_COUNT)-1;desc.name="NFS Native Input Test";return SDL_AttachVirtualJoystick(&desc);}
int main(){
    CHECK(SDL_Init(SDL_INIT_GAMEPAD));CHECK(heap.initialize(memory,0x10000000,storage,sizeof(storage)));CHECK(memory.map(0x80000000,output,sizeof(output)));
    {
        nfs::InputDevices devices(nullptr);nfs::SDLGamepadInput input(devices);
        CHECK(devices.create(0x500,0x80000000,0)==0);const auto root=guest_load<uint32_t>(0x80000000);uint32_t handles[2]{};
        for(uint32_t slot=0;slot<2;++slot){guest_store(0x80000010,std::array<uint32_t,4>{0x6f1d2b70,slot,0,0});uint32_t args[]={root,0x80000010,0x80000020,0};CHECK(devices.invoke(nullptr,nfs::InputMethod::create_device,args)==0);handles[slot]=guest_load<uint32_t>(0x80000020);}
        const auto read=[&](uint32_t slot){uint32_t args[]={handles[slot],80,0x80000040};if(devices.invoke(nullptr,nfs::InputMethod::state,args))throw std::runtime_error("SDL state read");return guest_load<nfs::GameJoystickState>(0x80000040);};
        input.update();CHECK(read(0).axes[0]==32768 && read(0).axes[6]==0);
        const auto first=attach(),second=attach();CHECK(first && second && SDL_IsGamepad(first) && SDL_IsGamepad(second));
        auto joystick=SDL_OpenJoystick(first),other=SDL_OpenJoystick(second);CHECK(joystick && other);
        CHECK(SDL_SetJoystickVirtualAxis(joystick,SDL_GAMEPAD_AXIS_LEFTX,32767));
        CHECK(SDL_SetJoystickVirtualAxis(joystick,SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,32767));
        CHECK(SDL_SetJoystickVirtualAxis(joystick,SDL_GAMEPAD_AXIS_LEFT_TRIGGER,-32768));
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_SOUTH,true));
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_DPAD_UP,true));
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_DPAD_RIGHT,true));
        CHECK(SDL_SetJoystickVirtualAxis(other,SDL_GAMEPAD_AXIS_LEFTX,-32768));
        input.update();auto state=read(0);
        std::printf("Initial SDK snapshot: steering=%d left pedal=%d right pedal=%d A=%u POV=%u\n",state.axes[0],state.axes[6],state.axes[7],state.buttons[SDL_GAMEPAD_BUTTON_SOUTH],state.pov[0]);
        CHECK(state.axes[0]==65535 && state.axes[7]==65535 && state.axes[6]==0 && state.buttons[SDL_GAMEPAD_BUTTON_SOUTH]==0x80 && state.pov[0]==4500);
        CHECK(input.menu_buttons()==(nfs::menu_confirm|nfs::menu_up|nfs::menu_right));
        input.update(false);CHECK(read(0).axes[0]==32768 && read(0).axes[7]==0 && read(0).buttons[0]==0 && read(0).pov[0]==UINT32_MAX);
        input.update();CHECK(read(0).axes[0]==65535 && read(0).axes[7]==65535 && read(0).buttons[0]==0x80);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        CHECK(read(1).axes[0]==0);
        CHECK(SDL_DetachVirtualJoystick(first));SDL_CloseJoystick(joystick);input.update();state=read(0);
        CHECK(state.axes[0]==32768 && state.axes[7]==0 && state.buttons[0]==0 && state.pov[0]==UINT32_MAX && read(1).axes[0]==0);
        CHECK(input.menu_buttons()==nfs::menu_confirm); // Other pad still owns confirm.
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_SOUTH,false));
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_START,true));input.update();CHECK(input.menu_buttons()==nfs::menu_cancel);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_START,false));input.update();CHECK(input.menu_buttons()==0);
        const auto replacement=attach();CHECK(replacement);auto reconnected=SDL_OpenJoystick(replacement);CHECK(reconnected);
        CHECK(SDL_SetJoystickVirtualAxis(reconnected,SDL_GAMEPAD_AXIS_LEFTX,1234));CHECK(SDL_SetJoystickVirtualButton(reconnected,SDL_GAMEPAD_BUTTON_SOUTH,true));
        input.update();CHECK(read(0).axes[0]==34002 && read(0).buttons[0]==0x80 && read(1).axes[0]==0);
        CHECK(SDL_DetachVirtualJoystick(replacement));SDL_CloseJoystick(reconnected);CHECK(SDL_DetachVirtualJoystick(second));SDL_CloseJoystick(other);input.update();
        CHECK(input.menu_buttons()==0);
    }
    CHECK(heap.available_bytes()==heap.total_bytes());SDL_Quit();
    std::puts("PASS: actual SDL gamepad axes/pedals/buttons/POV, neutral disconnect and stable two-slot reconnect into guest DIJOYSTATE");
}

#include "InputDevices.h"
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
        nfs::InputDevices devices(nullptr);nfs::SDLGamepadInput input(devices);nfs::bind_input_devices(&devices);
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
        CHECK(input.menu_buttons()==0); // Native POV/buttons never become keyboard aliases.
        CHECK(state.buttons[SDL_GAMEPAD_BUTTON_DPAD_UP]==0 && state.buttons[SDL_GAMEPAD_BUTTON_DPAD_RIGHT]==0);
        CHECK(devices.frontend_hat_command(0)==0x4800); // One command for a diagonal, no action button.
        CHECK(devices.frontend_direction_actions()==2); // Native Pause action1/up, independent of queue repeat.
        CHECK(devices.frontend_hat_command(399)==0);
        CHECK(devices.frontend_hat_command(400)==0x4800);
        CHECK(devices.frontend_hat_command(401)==0);
        CHECK(devices.frontend_hat_command(1000)==0x4800); // No accumulated repeat burst.
        CHECK(devices.frontend_hat_command(1000)==0);
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_DPAD_UP,false));input.update();
        CHECK(devices.frontend_hat_command(1001)==0x4d00);
        CHECK(devices.frontend_direction_actions()==16);
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_DPAD_RIGHT,false));input.update();
        CHECK(devices.frontend_hat_command(1002)==0);
        CHECK(devices.frontend_direction_actions()==0);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_DPAD_DOWN,true));input.update();
        CHECK(devices.frontend_hat_command(1003)==0x5000);
        CHECK(devices.frontend_direction_actions()==4);
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_DPAD_DOWN,true));input.update();
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_DPAD_DOWN,false));input.update();
        CHECK(devices.frontend_hat_command(1004)==0); // First pad still owns the same held direction.
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_DPAD_DOWN,false));input.update();
        // A short complete pulse can occur between original frontend polls.
        CHECK(devices.frontend_hat_command(1005)==0);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_DPAD_UP,true));input.update();
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_DPAD_UP,false));input.update();
        CHECK(devices.frontend_hat_command(1006)==0x4800);
        CHECK(devices.frontend_hat_command(1006)==0);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_DPAD_DOWN,true));input.update();
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_DPAD_DOWN,false));input.update();
        devices.publish(0,nullptr);CHECK(devices.frontend_hat_command(1007)==0x5000);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_DPAD_UP,true));input.update();
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_DPAD_UP,false));input.update();
        devices.publish(1,nullptr);CHECK(devices.frontend_hat_command(1008)==0);
        input.update();
        for(unsigned pulse=0;pulse<10;++pulse){
            const auto button=pulse%2?SDL_GAMEPAD_BUTTON_DPAD_DOWN:SDL_GAMEPAD_BUTTON_DPAD_UP;
            CHECK(SDL_SetJoystickVirtualButton(other,button,true));input.update();
            CHECK(SDL_SetJoystickVirtualButton(other,button,false));input.update();
        }
        for(unsigned pulse=0;pulse<8;++pulse)
            CHECK(devices.frontend_hat_command(1100+pulse)==(pulse%2?0x5000:0x4800));
        CHECK(devices.frontend_hat_command(1108)==0); // Overflow stays bounded; no stuck repeat.
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_DPAD_DOWN,true));input.update();
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_DPAD_DOWN,false));input.update();
        devices.consume_held_buttons();CHECK(devices.frontend_hat_command(1109)==0);
        // Restore the unrelated held native action consumed by that close.
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_SOUTH,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        const uint32_t cancelMask=(uint32_t(1)<<SDL_GAMEPAD_BUTTON_EAST)|(uint32_t(1)<<SDL_GAMEPAD_BUTTON_START);
        CHECK(!devices.consume_frontend_buttons(cancelMask));
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,true));input.update();
        CHECK(devices.consume_frontend_buttons(cancelMask));
        CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_EAST]==0 && read(0).buttons[SDL_GAMEPAD_BUTTON_SOUTH]==0x80);
        CHECK(read(0).axes[0]==65535 && read(0).axes[7]==65535);
        input.update();CHECK(!devices.consume_frontend_buttons(cancelMask));
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_START,true));input.update();
        CHECK(devices.consume_frontend_buttons(cancelMask));
        input.update();CHECK(!devices.consume_frontend_buttons(cancelMask));
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,false));
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_START,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,true));input.update();
        CHECK(devices.consume_frontend_buttons(cancelMask)); // Fresh release/repress works.
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,false));input.update();
        CHECK(!nfs_controller_pause_pressed());
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,true));input.update();
        nfs_controller_capture_frontend_cancel();
        CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_EAST]==0); // Mask before original joystick refresh/confirm.
        CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_SOUTH]==0x80 && read(0).axes[7]==65535);
        input.update();nfs_controller_capture_frontend_cancel();
        CHECK(nfs_controller_frontend_cancel()); // Pending semantic cancel survives the masked snapshot read.
        CHECK(!nfs_controller_frontend_cancel());
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,true));input.update();
        nfs_controller_capture_frontend_cancel();devices.publish(1,nullptr);
        CHECK(nfs_controller_frontend_cancel()); // Neutralizing the other pad does not discard this request.
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,true));input.update();
        nfs_controller_capture_frontend_cancel();devices.publish(0,nullptr);
        CHECK(!nfs_controller_frontend_cancel()); // The originating pad disconnect/background cancels its request.
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,true));input.update();
        nfs_controller_capture_frontend_cancel();devices.consume_held_buttons();
        CHECK(!nfs_controller_frontend_cancel()); // Closing the frontend discards an unconsumed cancel.
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_EAST,false));
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_SOUTH,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_START,true));input.update();
        CHECK(nfs_controller_pause_pressed());
        CHECK(!nfs_controller_frontend_cancel()); // Same held Start cannot immediately resume.
        input.update();CHECK(!nfs_controller_pause_pressed());
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_START,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_START,true));input.update();
        CHECK(nfs_controller_pause_pressed()); // The second controller can pause too.
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_START,false));input.update();
        CHECK(devices.frontend_hat_command(1005)==0);
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_DPAD_DOWN,true));input.update();
        CHECK(devices.frontend_hat_command(1006)==0x5000); // Fresh press is immediate.
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_DPAD_DOWN,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_BACK,true));input.update();
        CHECK(input.menu_buttons()==0 && read(0).buttons[SDL_GAMEPAD_BUTTON_BACK]==0);
        CHECK(nfs_controller_reset_pressed());input.update();CHECK(!nfs_controller_reset_pressed());
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_BACK,false));input.update();CHECK(input.menu_buttons()==0);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_BACK,true));input.update();
        CHECK(!nfs_controller_reset_pressed() && read(1).buttons[SDL_GAMEPAD_BUTTON_BACK]==0x80);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_BACK,false));
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_BACK,true));input.update();
        devices.consume_held_buttons();input.update();CHECK(!nfs_controller_reset_pressed());
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_BACK,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_BACK,true));input.update();CHECK(nfs_controller_reset_pressed());
        input.update(false);CHECK(!nfs_controller_reset_pressed());
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_BACK,false));input.update();
        // Both players can hold a menu button when the original frontend
        // closes. Consume digital actions, preserving both analog profiles.
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_WEST,true));input.update();
        nfs_controller_capture_pause_buttons();
        CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_SOUTH]==0 && read(1).buttons[SDL_GAMEPAD_BUTTON_WEST]==0);
        CHECK(read(0).axes[0]==65535 && read(0).axes[7]==65535 && read(1).axes[0]==0);
        CHECK(nfs_controller_frontend_actions()==32); // Original Pause confirm action, never a driving button.
        input.update();nfs_controller_capture_pause_buttons();CHECK(nfs_controller_frontend_actions()==0);
        devices.consume_held_buttons();
        CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_SOUTH]==0 && read(1).buttons[SDL_GAMEPAD_BUTTON_WEST]==0);
        devices.consume_held_buttons(); // Re-entry does not release a held mask.
        input.update();input.update();
        CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_SOUTH]==0 && read(1).buttons[SDL_GAMEPAD_BUTTON_WEST]==0);
        CHECK(read(0).axes[0]==65535 && read(0).axes[7]==65535 && read(1).axes[0]==0);
        CHECK(input.menu_buttons()==0); // Held native confirm cannot emit a keyboard alias.
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_WEST,true));input.update();
        CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_WEST]==0x80); // Fresh driving action is not masked.
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_WEST,false));
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_SOUTH,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(joystick,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_SOUTH]==0x80 && read(1).buttons[SDL_GAMEPAD_BUTTON_WEST]==0);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_WEST,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_WEST,true));input.update();
        CHECK(read(1).buttons[SDL_GAMEPAD_BUTTON_WEST]==0x80);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_WEST,false));input.update();
        input.update(false);CHECK(read(0).axes[0]==32768 && read(0).axes[7]==0 && read(0).buttons[0]==0 && read(0).pov[0]==UINT32_MAX);
        input.update();CHECK(read(0).axes[0]==65535 && read(0).axes[7]==65535 && read(0).buttons[0]==0x80);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        CHECK(read(1).axes[0]==0);
        CHECK(SDL_DetachVirtualJoystick(first));SDL_CloseJoystick(joystick);input.update();state=read(0);
        CHECK(state.axes[0]==32768 && state.axes[7]==0 && state.buttons[0]==0 && state.pov[0]==UINT32_MAX && read(1).axes[0]==0);
        CHECK(input.menu_buttons()==0); // The other pad also remains native.
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_SOUTH,false));
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_START,true));input.update();CHECK(input.menu_buttons()==0);
        CHECK(SDL_SetJoystickVirtualButton(other,SDL_GAMEPAD_BUTTON_START,false));input.update();CHECK(input.menu_buttons()==0);
        const auto replacement=attach();CHECK(replacement);auto reconnected=SDL_OpenJoystick(replacement);CHECK(reconnected);
        CHECK(SDL_SetJoystickVirtualAxis(reconnected,SDL_GAMEPAD_AXIS_LEFTX,1234));CHECK(SDL_SetJoystickVirtualButton(reconnected,SDL_GAMEPAD_BUTTON_SOUTH,true));
        input.update();CHECK(read(0).axes[0]==34002 && read(0).buttons[0]==0x80 && read(1).axes[0]==0);
        CHECK(SDL_DetachVirtualJoystick(replacement));SDL_CloseJoystick(reconnected);CHECK(SDL_DetachVirtualJoystick(second));SDL_CloseJoystick(other);input.update();
        CHECK(input.menu_buttons()==0);
        const auto menu_pad_id=attach();CHECK(menu_pad_id);auto menu_pad=SDL_OpenJoystick(menu_pad_id);CHECK(menu_pad);
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        CHECK(nfs_controller_frontend_confirm());CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_SOUTH]==0);
        input.update();CHECK(!nfs_controller_frontend_confirm());
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_EAST,true));input.update();
        CHECK(!nfs_controller_frontend_confirm());CHECK(nfs_controller_frontend_cancel());
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        CHECK(nfs_controller_frontend_confirm());
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,false));input.update();
        devices.begin_frontend();
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,false));input.update();
        CHECK(nfs_controller_frontend_confirm());CHECK(!nfs_controller_frontend_confirm());
        CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_SOUTH]==0);
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        CHECK(nfs_controller_frontend_confirm());input.update();CHECK(!nfs_controller_frontend_confirm());
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_EAST,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_EAST,true));input.update();
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_EAST,false));input.update();
        CHECK(nfs_controller_frontend_cancel());CHECK(!nfs_controller_frontend_cancel());
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,false));input.update();
        devices.begin_driving();devices.begin_frontend();CHECK(!nfs_controller_frontend_confirm());
        devices.begin_driving();
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,false));input.update();
        devices.begin_frontend();CHECK(!nfs_controller_frontend_confirm());
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,true));input.update();
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_SOUTH,false));input.update();
        devices.consume_held_buttons();CHECK(!nfs_controller_frontend_confirm());
        {
            nfs::NativeControlBindingScope recording;
            CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,true));input.update();
            CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,false));input.update();
            CHECK(!nfs_controller_frontend_confirm());
            CHECK(nfs_controller_binding_buttons(0)==(1u<<SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER));
            CHECK(nfs_controller_binding_buttons(0)==0 && nfs_controller_binding_buttons(1)==0 && nfs_controller_binding_buttons(2)==0);
            CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_WEST,true));input.update();
            CHECK(nfs_controller_binding_buttons(0)==(1u<<SDL_GAMEPAD_BUTTON_WEST));
            CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_WEST]==0x80);
        }
        devices.begin_frontend();input.update();CHECK(!nfs_controller_frontend_confirm());
        CHECK(read(0).buttons[SDL_GAMEPAD_BUTTON_WEST]==0);
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_WEST,false));input.update();
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_WEST,true));input.update();
        CHECK(nfs_controller_frontend_confirm());
        CHECK(SDL_SetJoystickVirtualButton(menu_pad,SDL_GAMEPAD_BUTTON_WEST,false));input.update();
        CHECK(SDL_DetachVirtualJoystick(menu_pad_id));SDL_CloseJoystick(menu_pad);input.update();
        nfs::bind_input_devices(nullptr);
    }
    {
        nfs::InputDevices pending(nullptr);nfs::GameJoystickState state;
        const uint32_t cancelMask=(uint32_t(1)<<SDL_GAMEPAD_BUTTON_EAST)|(uint32_t(1)<<SDL_GAMEPAD_BUTTON_START);
        state.buttons[SDL_GAMEPAD_BUTTON_WEST]=0x80;
        pending.publish(1,&state);pending.capture_pause_buttons(cancelMask);pending.publish(0,nullptr);
        CHECK(pending.pause_actions()==32 && pending.pause_actions()==0);
        pending.publish(1,nullptr);pending.publish(1,&state);pending.capture_pause_buttons(cancelMask);
        pending.publish(1,nullptr);CHECK(pending.pause_actions()==0);
        pending.publish(1,&state);pending.capture_pause_buttons(cancelMask);pending.consume_held_buttons();
        CHECK(pending.pause_actions()==0);
        pending.publish(1,nullptr);state.buttons[SDL_GAMEPAD_BUTTON_EAST]=0x80;
        pending.publish(1,&state);pending.capture_pause_buttons(cancelMask);
        CHECK(pending.take_frontend_cancel(cancelMask));CHECK(pending.pause_actions()==0);
    }
    CHECK(heap.available_bytes()==heap.total_bytes());SDL_Quit();
    std::puts("PASS: actual SDL gamepad axes/pedals/buttons/POV, neutral disconnect and stable two-slot reconnect into guest DIJOYSTATE");
}

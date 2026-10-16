// SPDX-License-Identifier: MIT
#include "InputDevices.h"
#include <SDL3/SDL.h>
#include <stdexcept>
namespace nfs {
struct SDLGamepadInput::Impl {
    InputDevices &devices;SDL_Gamepad *pads[2]{};SDL_JoystickID ids[2]{};
    explicit Impl(InputDevices &input):devices(input){}
    ~Impl(){for(unsigned slot=0;slot<2;++slot){if(pads[slot])SDL_CloseGamepad(pads[slot]);devices.publish(slot,nullptr);}}
};
SDLGamepadInput::SDLGamepadInput(InputDevices &devices):impl(new Impl(devices)){
    if(!SDL_IsMainThread() || !(SDL_WasInit(SDL_INIT_GAMEPAD)&SDL_INIT_GAMEPAD))throw std::logic_error("Gamepad input needs SDL initialization on the app main thread");
}
SDLGamepadInput::~SDLGamepadInput()=default;
uint8_t SDLGamepadInput::menu_buttons()const{
    if(!SDL_IsMainThread())throw std::logic_error("Menu input must use app main thread");
    // Controllers use original native frontend and driving actions. Keep the
    // legacy keyboard-message adapter neutral, including on held buttons.
    return 0;
}
void SDLGamepadInput::update(bool active){
    if(!SDL_IsMainThread())throw std::logic_error("Gamepad polling must use app main thread");
    auto &input=*impl;
    for(unsigned slot=0;slot<2;++slot)if(input.pads[slot] && !SDL_GamepadConnected(input.pads[slot])){
        SDL_CloseGamepad(input.pads[slot]);input.pads[slot]=nullptr;input.ids[slot]=0;input.devices.publish(slot,nullptr);
    }
    int count=0;auto ids=SDL_GetGamepads(&count);
    if(!ids)throw std::runtime_error(SDL_GetError());
    for(int i=0;i<count;++i){
        if(ids[i]==input.ids[0] || ids[i]==input.ids[1])continue;
        for(unsigned slot=0;slot<2;++slot)if(!input.pads[slot]){
            auto pad=SDL_OpenGamepad(ids[i]);if(pad){input.pads[slot]=pad;input.ids[slot]=ids[i];}break;
        }
    }
    SDL_free(ids);
    // SDL_OpenGamepad establishes trigger zero points. Open new devices before
    // consuming their first input update, or a pressed pedal can be reset to
    // zero until a later hardware change (also reproducible with SDL virtual IO).
    SDL_UpdateGamepads();
    for(unsigned slot=0;slot<2;++slot){
        auto pad=input.pads[slot];
        if(pad && !SDL_GamepadConnected(pad)){SDL_CloseGamepad(pad);input.pads[slot]=nullptr;input.ids[slot]=0;pad=nullptr;}
        if(!pad || !active){input.devices.publish(slot,nullptr);continue;}
        if(slot==0)input.devices.publish_reset(SDL_GetGamepadButton(pad,SDL_GAMEPAD_BUTTON_BACK));
        GameJoystickState state;
        state.axes[0]=int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_LEFTX))+32768;
        state.axes[1]=int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_LEFTY))+32768;
        state.axes[2]=int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_RIGHTX))+32768;
        state.axes[5]=int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_RIGHTY))+32768;
        // Full-range pedals: SDL's standardized trigger axes are 0..32767.
        state.axes[6]=(int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_LEFT_TRIGGER))*65535)/32767;
        state.axes[7]=(int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_RIGHT_TRIGGER))*65535)/32767;
        for(unsigned button=0;button<32 && button<unsigned(SDL_GAMEPAD_BUTTON_COUNT);++button){
            // A native joystick hat is not also a face/action button. The
            // original frontend activates a choice for any button press;
            // duplicating standardized D-pad buttons here would activate the
            // choice reached by native hat navigation. Preserve POV below.
            const bool direction=button>=unsigned(SDL_GAMEPAD_BUTTON_DPAD_UP) && button<=unsigned(SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
            const bool reset=slot==0 && button==unsigned(SDL_GAMEPAD_BUTTON_BACK);
            state.buttons[button]=!direction && !reset && SDL_GetGamepadButton(pad,SDL_GamepadButton(button))?0x80:0;
        }
        const bool up=SDL_GetGamepadButton(pad,SDL_GAMEPAD_BUTTON_DPAD_UP),down=SDL_GetGamepadButton(pad,SDL_GAMEPAD_BUTTON_DPAD_DOWN);
        const bool left=SDL_GetGamepadButton(pad,SDL_GAMEPAD_BUTTON_DPAD_LEFT),right=SDL_GetGamepadButton(pad,SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
        if(up && right)state.pov[0]=4500;else if(down && right)state.pov[0]=13500;else if(down && left)state.pov[0]=22500;else if(up && left)state.pov[0]=31500;
        else if(up)state.pov[0]=0;else if(right)state.pov[0]=9000;else if(down)state.pov[0]=18000;else if(left)state.pov[0]=27000;
        input.devices.publish(slot,&state);
    }
}
}
extern "C" uint32_t nfs_controller_frontend_hat(){return nfs::input_devices().frontend_hat_command(SDL_GetTicks());}
extern "C" bool nfs_controller_frontend_cancel(){
    return nfs::input_devices().take_frontend_cancel((uint32_t(1)<<SDL_GAMEPAD_BUTTON_EAST)|(uint32_t(1)<<SDL_GAMEPAD_BUTTON_START));
}
extern "C" bool nfs_controller_frontend_confirm(){
    const uint32_t cancel=(uint32_t(1)<<SDL_GAMEPAD_BUTTON_EAST)|(uint32_t(1)<<SDL_GAMEPAD_BUTTON_START);
    return nfs::input_devices().consume_frontend_buttons(~cancel);
}
extern "C" bool nfs_controller_pause_pressed(){
    return nfs::input_devices().consume_frontend_buttons(uint32_t(1)<<SDL_GAMEPAD_BUTTON_START);
}
extern "C" uint32_t nfs_controller_frontend_actions(){return nfs::input_devices().pause_actions();}
extern "C" void nfs_controller_capture_frontend_cancel(){
    nfs::input_devices().capture_frontend_cancel((uint32_t(1)<<SDL_GAMEPAD_BUTTON_EAST)|(uint32_t(1)<<SDL_GAMEPAD_BUTTON_START));
}
extern "C" bool nfs_controller_reset_pressed(){return nfs::input_devices().take_reset();}
extern "C" void nfs_controller_capture_pause_buttons(){
    nfs::input_devices().capture_pause_buttons((uint32_t(1)<<SDL_GAMEPAD_BUTTON_EAST)|(uint32_t(1)<<SDL_GAMEPAD_BUTTON_START));
}

extern "C" void nfs_controller_begin_frontend(){nfs::input_devices().begin_frontend();}
extern "C" void nfs_controller_begin_driving(){nfs::input_devices().begin_driving();}

extern "C" uint32_t nfs_controller_binding_buttons(uint32_t slot){return nfs::input_devices().binding_buttons(slot);}

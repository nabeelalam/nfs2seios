// SPDX-License-Identifier: MIT
#include "InputDevices.h"
#include "MenuInput.h"
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
    uint8_t buttons=0;
    for(auto pad:impl->pads)if(pad && SDL_GamepadConnected(pad)){
        const SDL_GamepadButton mappings[]={SDL_GAMEPAD_BUTTON_DPAD_LEFT,SDL_GAMEPAD_BUTTON_DPAD_UP,SDL_GAMEPAD_BUTTON_DPAD_RIGHT,SDL_GAMEPAD_BUTTON_DPAD_DOWN,SDL_GAMEPAD_BUTTON_SOUTH,SDL_GAMEPAD_BUTTON_EAST};
        for(unsigned i=0;i<6;++i)if(SDL_GetGamepadButton(pad,mappings[i]))buttons|=uint8_t(1u<<i);
        if(SDL_GetGamepadButton(pad,SDL_GAMEPAD_BUTTON_START))buttons|=menu_cancel;
    }
    return buttons;
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
        GameJoystickState state;
        state.axes[0]=int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_LEFTX))+32768;
        state.axes[1]=int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_LEFTY))+32768;
        state.axes[2]=int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_RIGHTX))+32768;
        state.axes[5]=int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_RIGHTY))+32768;
        // Full-range pedals: SDL's standardized trigger axes are 0..32767.
        state.axes[6]=(int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_LEFT_TRIGGER))*65535)/32767;
        state.axes[7]=(int32_t(SDL_GetGamepadAxis(pad,SDL_GAMEPAD_AXIS_RIGHT_TRIGGER))*65535)/32767;
        for(unsigned button=0;button<32 && button<unsigned(SDL_GAMEPAD_BUTTON_COUNT);++button)
            state.buttons[button]=SDL_GetGamepadButton(pad,SDL_GamepadButton(button))?0x80:0;
        const bool up=SDL_GetGamepadButton(pad,SDL_GAMEPAD_BUTTON_DPAD_UP),down=SDL_GetGamepadButton(pad,SDL_GAMEPAD_BUTTON_DPAD_DOWN);
        const bool left=SDL_GetGamepadButton(pad,SDL_GAMEPAD_BUTTON_DPAD_LEFT),right=SDL_GetGamepadButton(pad,SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
        if(up && right)state.pov[0]=4500;else if(down && right)state.pov[0]=13500;else if(down && left)state.pov[0]=22500;else if(up && left)state.pov[0]=31500;
        else if(up)state.pov[0]=0;else if(right)state.pov[0]=9000;else if(down)state.pov[0]=18000;else if(left)state.pov[0]=27000;
        input.devices.publish(slot,&state);
    }
}
}

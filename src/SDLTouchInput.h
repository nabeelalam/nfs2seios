// SPDX-License-Identifier: MIT
#pragma once
#include "TouchPointer.h"
#include "SDLKeyboardInput.h"
namespace nfs {
inline TouchResult original_touch_event(TouchPointer &pointer,const SDL_Event &event,
                                       SDL_WindowID window,uint32_t now){
    TouchPhase phase;
    switch(event.type){
    case SDL_EVENT_FINGER_DOWN:phase=TouchPhase::down;break;
    case SDL_EVENT_FINGER_MOTION:phase=TouchPhase::motion;break;
    case SDL_EVENT_FINGER_UP:phase=TouchPhase::up;break;
    case SDL_EVENT_FINGER_CANCELED:phase=TouchPhase::cancel;break;
    default:return {};
    }
    // Ignore SDL's mouse-to-touch synthesis and other windows. The application
    // consumes finger events only, so touch-to-mouse synthesis cannot double it.
    if(!window || event.tfinger.windowID!=window || event.tfinger.touchID==SDL_MOUSE_TOUCHID)return {};
    return pointer.update(phase,event.tfinger.touchID,event.tfinger.fingerID,
                          event.tfinger.x,event.tfinger.y,now);
}
inline void original_touch_key(TouchAction action,uint32_t now,MenuMessageSink sink,void *context){
    if(action!=TouchAction::escape && action!=TouchAction::enter)return;
    SDL_Event key{};
    key.type=SDL_EVENT_KEY_DOWN;
    key.key.scancode=action==TouchAction::escape?SDL_SCANCODE_ESCAPE:SDL_SCANCODE_RETURN;
    key.key.key=action==TouchAction::escape?SDLK_ESCAPE:SDLK_RETURN;
    original_keyboard_event(key,now,sink,context);
    key.type=SDL_EVENT_KEY_UP;
    original_keyboard_event(key,now,sink,context);
}
}

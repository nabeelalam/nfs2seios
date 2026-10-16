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
inline void original_touch_key_edge(TouchAction action,bool down,uint32_t now,MenuMessageSink sink,void *context){
    if(action!=TouchAction::escape && action!=TouchAction::enter)return;
    SDL_Event key{};
    key.type=down?SDL_EVENT_KEY_DOWN:SDL_EVENT_KEY_UP;
    key.key.scancode=action==TouchAction::escape?SDL_SCANCODE_ESCAPE:SDL_SCANCODE_RETURN;
    key.key.key=action==TouchAction::escape?SDLK_ESCAPE:SDLK_RETURN;
    original_keyboard_event(key,now,sink,context);
}
// Racing samples held keys separately from the frontend character queue.
// Keep a gesture key down briefly across those polls without blocking the app
// thread, and give consecutive pulses a neutral interval. No repeats are sent.
class TouchKeyPulse {
    std::array<TouchAction,8> pending{};unsigned head=0,count=0;
    TouchAction active=TouchAction::none;uint32_t pressed=0,released_at=0;bool released=false;
public:
    bool push(TouchAction action){
        if(action!=TouchAction::escape && action!=TouchAction::enter)return false;
        if(count==pending.size())return false;
        pending[(head+count)%pending.size()]=action;++count;return true;
    }
    void update(uint32_t now,MenuMessageSink sink,void *context){
        if(active!=TouchAction::none){
            if(uint32_t(now-pressed)>=250){original_touch_key_edge(active,false,now,sink,context);active=TouchAction::none;released_at=now;released=true;}
            return;
        }
        if(!count || (released && uint32_t(now-released_at)<40))return;
        active=pending[head];head=(head+1)%pending.size();--count;pressed=now;
        original_touch_key_edge(active,true,now,sink,context);
    }
    void cancel(uint32_t now,MenuMessageSink sink,void *context){
        if(active!=TouchAction::none)original_touch_key_edge(active,false,now,sink,context);
        active=TouchAction::none;head=count=0;released=false;
    }
};
}

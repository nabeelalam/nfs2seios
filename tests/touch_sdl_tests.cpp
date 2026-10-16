#include "SDLTouchInput.h"
#include <vector>
#include <stdexcept>
#include <cstdio>
static void check(bool value){if(!value)throw std::runtime_error("SDL touch contract");}
static bool send(void *context,const nfs::GameMessage &message){static_cast<std::vector<nfs::GameMessage> *>(context)->push_back(message);return true;}
int main(){try{
    nfs::TouchPointer pointer;std::vector<nfs::GameMessage> messages;
    auto event=[&](SDL_EventType type,SDL_FingerID finger,SDL_TouchID device=0,SDL_WindowID window=7){
        SDL_Event e{};e.type=type;e.tfinger.touchID=device;e.tfinger.fingerID=finger;
        e.tfinger.windowID=window;e.tfinger.x=.5f;e.tfinger.y=.5f;
        return nfs::original_touch_event(pointer,e,7,123);
    };
    event(SDL_EVENT_FINGER_DOWN,0);event(SDL_EVENT_FINGER_DOWN,1);
    check(event(SDL_EVENT_FINGER_UP,0).action==nfs::TouchAction::none);
    const auto escape=event(SDL_EVENT_FINGER_UP,1);check(escape.action==nfs::TouchAction::escape);
    nfs::TouchKeyPulse pulse;check(pulse.push(escape.action));pulse.update(123,send,&messages);
    check(pulse.push(nfs::TouchAction::enter)); // A second gesture waits for release and neutral spacing.
    check(messages.size()==2);pulse.update(372,send,&messages);check(messages.size()==2);
    pulse.update(373,send,&messages);
    check(messages.size()==3 && messages[0].message==0x100 && messages[1].message==0x102 && messages[2].message==0x101);
    check(messages[0].wparam==27 && messages[0].lparam==0x00010001 && messages[2].lparam==0xc0010001);
    messages.clear();pulse.update(412,send,&messages);check(messages.empty());pulse.update(413,send,&messages);pulse.update(663,send,&messages);
    check(messages.size()==3 && messages[0].wparam==13 && messages[0].lparam==0x001c0001 && messages[2].lparam==0xc01c0001);
    messages.clear();check(!pulse.push(nfs::TouchAction::click));pulse.update(1000,send,&messages);check(messages.empty());
    check(pulse.push(nfs::TouchAction::escape));pulse.update(1001,send,&messages);check(messages.size()==2);
    check(pulse.push(nfs::TouchAction::enter));
    pulse.cancel(1002,send,&messages);check(messages.size()==3 && messages.back().message==0x101);pulse.update(2000,send,&messages);check(messages.size()==3);
    for(unsigned i=0;i<8;++i)check(pulse.push(nfs::TouchAction::enter));check(!pulse.push(nfs::TouchAction::escape));pulse.cancel(2001,send,&messages);pulse.update(3000,send,&messages);check(messages.size()==3);
    check(pulse.push(nfs::TouchAction::escape));pulse.update(UINT32_MAX-10,send,&messages);check(messages.size()==5);pulse.update(239,send,&messages);check(messages.size()==6);
    event(SDL_EVENT_FINGER_DOWN,1,SDL_MOUSE_TOUCHID);check(event(SDL_EVENT_FINGER_UP,1,SDL_MOUSE_TOUCHID).action==nfs::TouchAction::none);
    event(SDL_EVENT_FINGER_DOWN,1,0,8);check(event(SDL_EVENT_FINGER_UP,1).action==nfs::TouchAction::none);
    event(SDL_EVENT_FINGER_DOWN,1);check(event(SDL_EVENT_FINGER_CANCELED,1).clear);check(event(SDL_EVENT_FINGER_UP,1).action==nfs::TouchAction::none);
    std::puts("PASS: actual SDL finger events and single Escape/Enter key sequences");return 0;
}catch(const std::exception &e){std::fprintf(stderr,"%s\n",e.what());return 1;}}

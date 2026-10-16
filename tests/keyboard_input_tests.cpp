#include "SDLKeyboardInput.h"
#include <vector>
#include <stdexcept>
#include <cstdio>
struct Sink {std::vector<nfs::GameMessage> messages;};
static bool send(void *p,const nfs::GameMessage &m){static_cast<Sink *>(p)->messages.push_back(m);return true;}
static void check(bool ok){if(!ok)throw std::runtime_error("original keyboard message mismatch");}
int main(){try{
    Sink sink;
    auto key=[&](SDL_Scancode scan,SDL_Keycode symbol,Uint32 type,bool repeat=false){
        SDL_Event e{};e.type=type;e.key.scancode=scan;e.key.key=symbol;e.key.repeat=repeat;
        nfs::original_keyboard_event(e,123,send,&sink);
    };
    const uint8_t expected[]={0x1e,0x30,0x2e,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32,0x31,0x18,0x19,0x10,0x13,0x1f,0x14,0x16,0x2f,0x11,0x2d,0x15,0x2c};
    for(unsigned i=0;i<26;++i){
        sink.messages.clear();key(SDL_Scancode(SDL_SCANCODE_A+i),'a'+i,SDL_EVENT_KEY_DOWN);
        key(SDL_Scancode(SDL_SCANCODE_A+i),'a'+i,SDL_EVENT_KEY_UP);
        check(sink.messages.size()==2 && sink.messages[0].message==0x100 && sink.messages[1].message==0x101);
        check(sink.messages[0].wparam=='A'+i && sink.messages[0].lparam==(1u|(uint32_t(expected[i])<<16)));
        check(sink.messages[1].lparam==(sink.messages[0].lparam|0xc0000000u));
    }
    sink.messages.clear();key(SDL_SCANCODE_1,SDLK_1,SDL_EVENT_KEY_DOWN);
    key(SDL_SCANCODE_0,SDLK_0,SDL_EVENT_KEY_DOWN);key(SDL_SCANCODE_SPACE,SDLK_SPACE,SDL_EVENT_KEY_DOWN);
    check(sink.messages.size()==3 && sink.messages[0].lparam==0x00020001 && sink.messages[1].lparam==0x000b0001 && sink.messages[2].lparam==0x00390001);
    SDL_Event text{};text.type=SDL_EVENT_TEXT_INPUT;text.text.text="Ab 10";
    const auto before=sink.messages.size();nfs::original_keyboard_event(text,124,send,&sink);
    check(sink.messages.size()==before+5);
    for(unsigned i=0;i<5;++i)check(sink.messages[before+i].message==0x102 && sink.messages[before+i].wparam==unsigned("Ab 10"[i]));
    sink.messages.clear();key(SDL_SCANCODE_RETURN,SDLK_RETURN,SDL_EVENT_KEY_DOWN);key(SDL_SCANCODE_RETURN,SDLK_RETURN,SDL_EVENT_KEY_UP);
    text.text.text="\n";nfs::original_keyboard_event(text,125,send,&sink);
    check(sink.messages.size()==3 && sink.messages[1].message==0x102 && sink.messages[1].wparam==13);
    sink.messages.clear();key(SDL_SCANCODE_BACKSPACE,SDLK_BACKSPACE,SDL_EVENT_KEY_DOWN,true);
    check(sink.messages.size()==2 && sink.messages[0].lparam==0x400e0001 && sink.messages[1].wparam==8);
    sink.messages.clear();key(SDL_SCANCODE_LEFT,SDLK_LEFT,SDL_EVENT_KEY_DOWN);key(SDL_SCANCODE_LEFT,SDLK_LEFT,SDL_EVENT_KEY_UP);
    check(sink.messages.size()==2 && sink.messages[0].lparam==0x014b0001 && sink.messages[1].lparam==0xc14b0001);
    sink.messages.clear();key(SDL_SCANCODE_F11,SDLK_F11,SDL_EVENT_KEY_DOWN);
    check(sink.messages.size()==1 && sink.messages[0].wparam==0x7a && sink.messages[0].lparam==0x00570001);
    sink.messages.clear();SDL_Event unrelated{};unrelated.type=SDL_EVENT_GAMEPAD_BUTTON_DOWN;nfs::original_keyboard_event(unrelated,125,send,&sink);check(sink.messages.empty());
    text.text.text="\xc3\xa9";nfs::original_keyboard_event(text,126,send,&sink);check(sink.messages.empty());
    std::puts("PASS: original PC scan codes, key releases/repeat, committed text once, editing/control keys and controller-event isolation");
}catch(const std::exception &e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}return 0;}

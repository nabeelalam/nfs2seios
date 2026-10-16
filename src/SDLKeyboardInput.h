// SPDX-License-Identifier: MIT
#pragma once
#include "MenuInput.h"
#include <SDL3/SDL.h>
namespace nfs {
// SDL uses USB scan codes; the original keyboard state uses PC set-1 codes.
inline uint32_t original_scan(SDL_Scancode code){
    static const uint8_t letters[]={0x1e,0x30,0x2e,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32,0x31,0x18,0x19,0x10,0x13,0x1f,0x14,0x16,0x2f,0x11,0x2d,0x15,0x2c};
    if(code>=SDL_SCANCODE_A && code<=SDL_SCANCODE_Z)return letters[code-SDL_SCANCODE_A];
    if(code>=SDL_SCANCODE_1 && code<=SDL_SCANCODE_9)return 2+code-SDL_SCANCODE_1;
    switch(code){
    case SDL_SCANCODE_0:return 0x0b;case SDL_SCANCODE_RETURN:return 0x1c;
    case SDL_SCANCODE_ESCAPE:return 1;case SDL_SCANCODE_BACKSPACE:return 0x0e;
    case SDL_SCANCODE_TAB:return 0x0f;case SDL_SCANCODE_SPACE:return 0x39;
    case SDL_SCANCODE_LSHIFT:return 0x2a;case SDL_SCANCODE_RSHIFT:return 0x36;
    case SDL_SCANCODE_LCTRL:return 0x1d;case SDL_SCANCODE_RCTRL:return 0x11d;
    case SDL_SCANCODE_LALT:return 0x38;case SDL_SCANCODE_RALT:return 0x138;
    case SDL_SCANCODE_MINUS:return 0x0c;case SDL_SCANCODE_EQUALS:return 0x0d;
    case SDL_SCANCODE_LEFTBRACKET:return 0x1a;case SDL_SCANCODE_RIGHTBRACKET:return 0x1b;
    case SDL_SCANCODE_BACKSLASH:return 0x2b;case SDL_SCANCODE_SEMICOLON:return 0x27;
    case SDL_SCANCODE_APOSTROPHE:return 0x28;case SDL_SCANCODE_GRAVE:return 0x29;
    case SDL_SCANCODE_COMMA:return 0x33;case SDL_SCANCODE_PERIOD:return 0x34;case SDL_SCANCODE_SLASH:return 0x35;
    case SDL_SCANCODE_LEFT:return 0x14b;case SDL_SCANCODE_UP:return 0x148;
    case SDL_SCANCODE_RIGHT:return 0x14d;case SDL_SCANCODE_DOWN:return 0x150;
    case SDL_SCANCODE_DELETE:return 0x153;case SDL_SCANCODE_HOME:return 0x147;
    case SDL_SCANCODE_END:return 0x14f;case SDL_SCANCODE_PAGEUP:return 0x149;case SDL_SCANCODE_PAGEDOWN:return 0x151;
    case SDL_SCANCODE_F11:return 0x57;case SDL_SCANCODE_F12:return 0x58;
    default:if(code>=SDL_SCANCODE_F1 && code<=SDL_SCANCODE_F10)return 0x3b+code-SDL_SCANCODE_F1;return 0;
    }
}
inline void original_keyboard_event(const SDL_Event &event,uint32_t now,MenuMessageSink sink,void *context){
    if(event.type==SDL_EVENT_TEXT_INPUT){
        // The supplied English game uses a byte character queue. Committed
        // ASCII is forwarded once; never reinterpret UTF-8 bytes as PC keys.
        if(event.text.text)for(const unsigned char *p=reinterpret_cast<const unsigned char *>(event.text.text);*p;++p)
            if(*p>=32 && *p<127)sink(context,{0,0x102,*p,1,now,0,0});
        return;
    }
    if(event.type!=SDL_EVENT_KEY_DOWN && event.type!=SDL_EVENT_KEY_UP)return;
    uint32_t scan=original_scan(event.key.scancode),key=0;
    const auto symbol=event.key.key;
    if(symbol>='a' && symbol<='z')key=symbol-32;
    else if(symbol>=32 && symbol<127)key=symbol;
    else switch(symbol){
    case SDLK_RETURN:key=13;break;case SDLK_ESCAPE:key=27;break;case SDLK_BACKSPACE:key=8;break;
    case SDLK_TAB:key=9;break;case SDLK_LEFT:key=0x25;break;case SDLK_UP:key=0x26;break;
    case SDLK_LSHIFT:case SDLK_RSHIFT:key=0x10;break;
    case SDLK_LCTRL:case SDLK_RCTRL:key=0x11;break;
    case SDLK_LALT:case SDLK_RALT:key=0x12;break;
    case SDLK_RIGHT:key=0x27;break;case SDLK_DOWN:key=0x28;break;
    case SDLK_DELETE:key=0x2e;break;case SDLK_HOME:key=0x24;break;case SDLK_END:key=0x23;break;
    case SDLK_PAGEUP:key=0x21;break;case SDLK_PAGEDOWN:key=0x22;break;
    default:if(symbol>=SDLK_F1 && symbol<=SDLK_F12)key=0x70+symbol-SDLK_F1;
    }
    if(!key || !scan)return;
    uint32_t flags=1|((scan&0x7f)<<16)|((scan&0x100)<<16);
    const bool down=event.type==SDL_EVENT_KEY_DOWN;
    if(!down)flags|=0xc0000000u;else if(event.key.repeat)flags|=0x40000000u;
    if(!sink(context,{0,down?0x100u:0x101u,key,flags,now,0,0}))return;
    // Printable text comes only from TEXT_INPUT, never both event streams.
    if(down && (key==13 || key==27 || key==8 || key==9))sink(context,{0,0x102,key,flags,now,0,0});
}
}

// SPDX-License-Identifier: MIT
#include "RuntimeServices.h"
#include "MainThreadBridge.h"
#include <SDL3/SDL.h>
#include <stdexcept>
#include <vector>
namespace nfs {
int32_t sdl_game_message(const std::string &text,const std::string &caption,uint32_t flags){
    return renderer_operation([&]{
        const uint32_t kind=flags&15;
        std::vector<SDL_MessageBoxButtonData> buttons;
        const auto add=[&](int id,const char *label){buttons.push_back({0,id,label});};
        switch(kind){
        case 0:add(1,"OK");break;case 1:add(1,"OK");add(2,"Cancel");break;
        case 2:add(3,"Abort");add(4,"Retry");add(5,"Ignore");break;
        case 3:add(6,"Yes");add(7,"No");add(2,"Cancel");break;
        case 4:add(6,"Yes");add(7,"No");break;case 5:add(4,"Retry");add(2,"Cancel");break;
        case 6:add(2,"Cancel");add(10,"Try Again");add(11,"Continue");break;
        default:throw std::invalid_argument("Unsupported original message box button set");
        }
        const auto default_button=(flags>>8)&3;buttons[default_button<buttons.size()?default_button:0].flags|=SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
        for(auto &button:buttons)if(button.buttonID==2 || button.buttonID==7 || (buttons.size()==1))button.flags|=SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
        SDL_MessageBoxData data{SDL_MESSAGEBOX_WARNING,nullptr,caption.c_str(),text.c_str(),int(buttons.size()),buttons.data(),nullptr};
        int choice=0;if(!SDL_ShowMessageBox(&data,&choice))throw std::runtime_error(SDL_GetError());return int32_t(choice);
    });
}
}

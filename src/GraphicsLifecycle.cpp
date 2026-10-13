// SPDX-License-Identifier: MIT
#include "GraphicsLifecycle.h"
#include "MainThreadBridge.h"
#include "FramePresentation.h"
#include <SDL3/SDL_opengles2.h>
#include <stdexcept>
namespace nfs {
void GraphicsLifecycle::set_suspended(bool value){
    if(!SDL_IsMainThread())throw std::logic_error("Graphics lifecycle needs app main thread");
    if(value==suspended)return;
    nfs_reset_frame_rate();
    if(value){
        context=SDL_GL_GetCurrentContext();window=SDL_GL_GetCurrentWindow();
        if(context)glFinish();
        suspend_renderer(true);suspended=true;
        if(context && !SDL_GL_MakeCurrent(window,nullptr))throw std::runtime_error(SDL_GetError());
    }else{
        if(context && !SDL_GL_MakeCurrent(window,context))throw std::runtime_error(SDL_GetError());
        suspend_renderer(false);suspended=false;context=nullptr;window=nullptr;
    }
}
}

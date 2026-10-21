// SPDX-License-Identifier: MIT
#include "WindowBridge.h"
#include "MainThreadBridge.h"
#include <SDL3/SDL.h>
#include <exception>
#include <stdexcept>
#include <cmath>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
extern "C" {extern SDL_Window *sdlWin;extern int32_t winWidth,winHeight,windowResized;extern float dpr;}
namespace {
// All configuration publication and renderer callbacks share the SDL main owner.
// Query physical pixels; logical points are incorrect on high-density displays.
bool refresh_pixels(SDL_Window *window,bool *changed=nullptr){
    if(!SDL_IsMainThread())throw std::logic_error("Window pixel refresh needs SDL main thread");
    int width=0,height=0;
    if(!window || !SDL_GetWindowSizeInPixels(window,&width,&height) || width<=0 || height<=0)return false;
    const float density=SDL_GetWindowPixelDensity(window);
    if(!std::isfinite(density) || density<=0)return false;
    const bool differs=winWidth!=width || winHeight!=height || dpr!=density;
    if(differs){winWidth=width;winHeight=height;dpr=density;windowResized=1;}
    if(changed)*changed=differs;
    return true;
}
struct Request {SDL_Window *window=nullptr;bool create;std::exception_ptr failure;};
void on_main(void *p){
    auto &request=*static_cast<Request *>(p);
    try{
        if(request.create){
            if(!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_ES) ||
               !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2) ||
               !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0))throw std::runtime_error(SDL_GetError());
            SDL_WindowFlags flags=SDL_WINDOW_OPENGL|SDL_WINDOW_HIGH_PIXEL_DENSITY;
#if defined(__APPLE__) && TARGET_OS_IOS
            flags|=SDL_WINDOW_FULLSCREEN;
            SDL_SetHint(SDL_HINT_IOS_HIDE_HOME_INDICATOR,"2");
#else
            flags|=SDL_WINDOW_RESIZABLE;
#endif
            request.window=SDL_CreateWindow("Need for Speed II SE",640,480,flags);
            if(!request.window)throw std::runtime_error(SDL_GetError());
            if(!refresh_pixels(request.window)){
                SDL_DestroyWindow(request.window);request.window=nullptr;
                throw std::runtime_error("Cannot query valid window pixel dimensions");
            }
#if !defined(__APPLE__) || !TARGET_OS_IOS
            // SDL3 does not enable committed text by default. Desktop physical
            // keyboards need this character stream alongside their key edges.
            // Platforms with a screen keyboard retain their explicit toggle.
            if(!SDL_HasScreenKeyboardSupport() && !SDL_StartTextInput(request.window))
                SDL_LogWarn(SDL_LOG_CATEGORY_INPUT,"Cannot enable desktop text input: %s",SDL_GetError());
#endif
            sdlWin=request.window;
        }else {
            if(SDL_TextInputActive(request.window))SDL_StopTextInput(request.window);
            SDL_DestroyWindow(request.window);if(sdlWin==request.window)sdlWin=nullptr;
        }
    }catch(...){request.failure=std::current_exception();}
}
void run(Request &request){
    nfs::run_renderer_on_main(on_main,&request);
    if(request.failure)std::rethrow_exception(request.failure);
}
void *create(){Request request;request.create=true;run(request);return request.window;}
void destroy(void *window){Request request;request.create=false;request.window=static_cast<SDL_Window *>(window);run(request);}
}
namespace nfs {
WindowOperations sdl_window_operations(){return {create,destroy};}
bool sdl_window_event(const SDL_Event &event){
    if(!SDL_IsMainThread())throw std::logic_error("Window events need SDL main thread");
    if((event.type!=SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED && event.type!=SDL_EVENT_WINDOW_RESIZED) ||
       !sdlWin || event.window.windowID!=SDL_GetWindowID(sdlWin))return false;
    bool changed=false;
    return refresh_pixels(sdlWin,&changed) && changed;
}
}

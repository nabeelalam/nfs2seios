// SPDX-License-Identifier: MIT
#include "WindowBridge.h"
#include "MainThreadBridge.h"
#include <SDL3/SDL.h>
#include <exception>
#include <stdexcept>
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
extern "C" {extern SDL_Window *sdlWin;extern int32_t winWidth,winHeight;extern float dpr;}
namespace {
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
#endif
            request.window=SDL_CreateWindow("Need for Speed II SE",640,480,flags);
            if(!request.window)throw std::runtime_error(SDL_GetError());
            sdlWin=request.window;SDL_GetWindowSize(request.window,&winWidth,&winHeight);dpr=SDL_GetWindowPixelDensity(request.window);
        }else {SDL_DestroyWindow(request.window);if(sdlWin==request.window)sdlWin=nullptr;}
    }catch(...){request.failure=std::current_exception();}
}
void run(Request &request){
    nfs::run_renderer_on_main(on_main,&request);
    if(request.failure)std::rethrow_exception(request.failure);
}
void *create(){Request request;request.create=true;run(request);return request.window;}
void destroy(void *window){Request request;request.create=false;request.window=static_cast<SDL_Window *>(window);run(request);}
}
namespace nfs {WindowOperations sdl_window_operations(){return {create,destroy};}}

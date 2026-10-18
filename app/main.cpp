// SPDX-License-Identifier: MIT
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "AssetSelection.h"
#include "GameSession.h"
#include "RunLog.h"
#include "MainThreadBridge.h"
#include "FramePresentation.h"
#include "HdAtlas.h"
#include <cstdio>
#include <memory>
#include <exception>
namespace {
struct HdAtlasLifetime {
    explicit HdAtlasLifetime(const std::string &root){
        if(nfs_glide_hd_atlas_load_file((root+"/gamedata/hud-hd.nfsatlas").c_str()))
            nfs::app_log("HD interface","Optional texture atlas loaded");
    }
    ~HdAtlasLifetime(){nfs_glide_hd_atlas_clear();}
};
int32_t present(const std::string &text,const std::string &caption,uint32_t flags){
    nfs::app_log("Game message",caption+": "+text);
#ifdef NFS_APP_HEADLESS_MESSAGES
    (void)flags;return 1;
#else
    return nfs::sdl_game_message(text,caption,flags);
#endif
}
void report(const std::string &error){
    nfs::app_log("Native game app failure",error);
#ifndef NFS_APP_HEADLESS_MESSAGES
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Need for Speed II SE",error.c_str(),nullptr);
#endif
}
}
int main(int argc,char **argv){
    nfs::open_app_log(nfs::app_log_path(argc,argv));
    nfs::app_log("Native app","Need for Speed II SE; platform="+std::string(SDL_GetPlatform()));
    if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMEPAD)){nfs::app_log("SDL initialization failed",SDL_GetError());SDL_Quit();nfs::close_app_log();return 1;}
    int status=1;
    try{
        const auto root=nfs::select_app_assets(argc,argv);
        if(!root.empty()){
            HdAtlasLifetime hd_atlas(root);
            // No host process exit inside the engine. Retain the root context
            // and continue pumping SDL until renderer/worker cleanup finishes.
            auto session=std::unique_ptr<nfs::GameSession>(new nfs::GameSession(root,present));
            session->start();std::string app_error;Uint64 timing_start=SDL_GetTicks();
            while(!session->result().finished){
                try{session->poll();}
                catch(const std::exception &error){if(app_error.empty())app_error=error.what();session->request_stop();SDL_PumpEvents();}
                const auto now=SDL_GetTicks();
                if(now-timing_start>=5000){
                    const auto timing=nfs::take_renderer_timing();
                    nfs::app_log("Renderer timing","interval_ms="+std::to_string(now-timing_start)+
                        " calls="+std::to_string(timing.calls)+" queue_ms="+std::to_string(timing.queue_ns/1000000)+
                        " execution_ms="+std::to_string(timing.execution_ns/1000000)+
                        " presentation_fps="+std::to_string(nfs_frame_rate_value()));
                    timing_start=now;
                }
                nfs::wait_for_main_work();
            }
            const auto result=session->result();
            nfs::app_log("Native game app stopped","exit="+std::to_string(result.exit_code)+" error="+result.error);
            session.reset();
            if(!app_error.empty())report(app_error);
            else if(!result.error.empty())report(result.error);
            else status=int(result.exit_code);
        }
    }catch(const std::exception &error){report(error.what());}
    SDL_Quit();nfs::app_log("Native app","SDL shutdown completed; status="+std::to_string(status));nfs::close_app_log();return status;
}

// SPDX-License-Identifier: MIT
#include <SDL3/SDL.h>
#if !defined(SDL_PLATFORM_IOS)
#include "DesktopDiagnosticRecorder.h"
#endif
#include <SDL3/SDL_main.h>
#include "AssetSelection.h"
#include "GameSession.h"
#include "RunLog.h"
#include "GameTrace.h"
#include "MainThreadBridge.h"
#include "FramePresentation.h"
#include "HdAtlas.h"
#include "hd/ArtworkActivation.h"
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <exception>
namespace {
struct HdAtlasLifetime {
    explicit HdAtlasLifetime(const std::string &root){
        if(nfs_glide_hd_atlas_load_file((root+"/gamedata/hud-hd.nfsatlas").c_str()))
            nfs::app_log("HD interface","Optional texture atlas loaded");
        else{
            nfs_glide_hd_builtin_enable(1);
            nfs::app_log("HD interface","Built-in timing artwork enabled");
        }
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
bool SDLCALL log_memory_warning(void *,SDL_Event *event) noexcept {
    if(event->type==SDL_EVENT_LOW_MEMORY){
        try {
            nfs::app_log("Memory warning","Operating system reported memory pressure");
        } catch (...) {
            // Diagnostics must never let exceptions escape the SDL callback.
        }
    }
    return true;
}
void report(const std::string &error){
    nfs::app_log("Native game app failure",error);
#ifndef NFS_APP_HEADLESS_MESSAGES
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Need for Speed II SE",error.c_str(),nullptr);
#endif
}
}
int main(int argc,char **argv){
#if !defined(SDL_PLATFORM_IOS)
    std::string recording_path;
    try{recording_path=nfs::desktop_recording_option(argc,argv);}
    catch(const std::exception &error){std::fprintf(stderr,"Diagnostic option: %s\n",error.what());return 1;}
#endif
    nfs::open_app_log(nfs::app_log_path(argc,argv));
#if !defined(SDL_PLATFORM_IOS)
    const char *opponents_trace = std::getenv("NFS_PRIVATE_OPPONENT_DIAG");
    if (opponents_trace && opponents_trace[0] == '1' && !opponents_trace[1])
#endif
    nfs::set_opponents_visit_sink([](nfs::OpponentsVisit stage){
        switch(stage){
        case nfs::OpponentsVisit::Entered:nfs::app_log("Opponents","entered menu");break;
        case nfs::OpponentsVisit::ScenePrepared:nfs::app_log("Opponents","prepared first scene");break;
        case nfs::OpponentsVisit::RendererBegin:nfs::app_log("Opponents","renderer began first scene");break;
        case nfs::OpponentsVisit::RendererReady:nfs::app_log("Opponents","renderer accepted first scene");break;
        case nfs::OpponentsVisit::RendererDeclined:nfs::app_log("Opponents","renderer declined first scene");break;
        case nfs::OpponentsVisit::PreviewBegin:nfs::app_log("Opponents","renderer began first four-car transition");break;
        case nfs::OpponentsVisit::PreviewReady:nfs::app_log("Opponents","renderer accepted first four-car transition");break;
        case nfs::OpponentsVisit::PreviewDeclined:nfs::app_log("Opponents","renderer declined first four-car transition");break;
        case nfs::OpponentsVisit::PreviewCommitBegin:nfs::app_log("Opponents","renderer began committing first four-car frame");break;
        case nfs::OpponentsVisit::PreviewCommitted:nfs::app_log("Opponents","renderer committed first four-car frame");break;
        case nfs::OpponentsVisit::PreviewCommitDeclined:nfs::app_log("Opponents","renderer declined first four-car frame commit");break;
        case nfs::OpponentsVisit::Left:nfs::app_log("Opponents","left menu");break;
        }
    });
    nfs::app_log("Native app","Need for Speed II SE; platform="+std::string(SDL_GetPlatform()));
#if defined(NFS_DIAGNOSTIC_SOURCE)
    nfs::app_log("Source revision",NFS_DIAGNOSTIC_SOURCE);
#endif
    if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMEPAD)){nfs::app_log("SDL initialization failed",SDL_GetError());SDL_Quit();nfs::close_app_log();return 1;}
    const bool watching_memory=SDL_AddEventWatch(log_memory_warning,nullptr);
    if(!watching_memory)nfs::app_log("Memory warning diagnostics unavailable",SDL_GetError());
    int status=1;
    try{
        const auto root=nfs::select_app_assets(argc,argv);
        if(!root.empty()){
            #ifdef NFS_CLASSIC_BUNDLED_ART
            if(nfs_hd::activateConfiguredArtwork(root,nfs::app_artwork_template_path()))
#else
            if(nfs_hd::activateConfiguredArtwork(root))
#endif
            nfs::app_log("HD interface","Separate authored menu artwork loaded");
            if(!nfs_hd::requiredArtworkReady())throw std::runtime_error("Required Location photo artwork is missing or invalid");
            HdAtlasLifetime hd_atlas(root);
            // No host process exit inside the engine. Retain the root context
            // and continue pumping SDL until renderer/worker cleanup finishes.
            auto session=std::unique_ptr<nfs::GameSession>(new nfs::GameSession(root,present));
#if !defined(SDL_PLATFORM_IOS)
            std::unique_ptr<nfs::DesktopDiagnosticRecorder> recorder;
            if(!recording_path.empty())recorder.reset(new nfs::DesktopDiagnosticRecorder(recording_path));
#endif
#if defined(SDL_PLATFORM_IOS)
            nfs::set_renderer_timing_capture(false);
#endif
            session->start();std::string app_error;
#if !defined(SDL_PLATFORM_IOS)
            Uint64 timing_start=SDL_GetTicks();
#endif
            while(!session->result().finished){
                try{session->poll();}
                catch(const std::exception &error){if(app_error.empty())app_error=error.what();session->request_stop();SDL_PumpEvents();}
#if !defined(SDL_PLATFORM_IOS)
                if(recorder)recorder->poll();
                const auto now=SDL_GetTicks();
                if(now-timing_start>=5000){
                    const auto timing=nfs::take_renderer_timing();
                    if(timing.calls)nfs::app_log("Renderer timing","interval_ms="+std::to_string(now-timing_start)+
                        " calls="+std::to_string(timing.calls)+" queue_ms="+std::to_string(timing.queue_ns/1000000)+
                        " execution_ms="+std::to_string(timing.execution_ns/1000000)+
                        " presentation_fps="+std::to_string(nfs_frame_rate_value()));
                    timing_start=now;
                }
#endif
                nfs::wait_for_main_work(nfs_frame_limit()==30);
            }
            const auto result=session->result();
            nfs::app_log("Native game app stopped","exit="+std::to_string(result.exit_code)+" error="+result.error);
#if !defined(SDL_PLATFORM_IOS)
            if(recorder){recorder->stop(app_error.empty()&&result.error.empty()?"session_end":"session_error");if(recorder->failed())nfs::app_log("Diagnostic recording","Recording ended with a writer failure");}
#endif
            session.reset();
            if(!app_error.empty())report(app_error);
            else if(!result.error.empty())report(result.error);
            else status=int(result.exit_code);
        }
    }catch(const std::exception &error){report(error.what());}
    if(watching_memory)SDL_RemoveEventWatch(log_memory_warning,nullptr);
    SDL_Quit();nfs::app_log("Native app","SDL shutdown completed; status="+std::to_string(status));nfs::close_app_log();return status;
}

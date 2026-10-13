#include "GameSession.h"
#include "MainThreadBridge.h"
#include <SDL3/SDL.h>
#include <cstdio>
#include <stdexcept>
#include <cstring>
static int messages=0;
static bool install_failed=false;
static bool language_failed=false;
static bool sound_bank_failed=false;
static int32_t message(const std::string &text,const std::string &caption,uint32_t){++messages;install_failed|=text.find("'install.win'")!=std::string::npos;language_failed|=text.find("text.eng'")!=std::string::npos;sound_bank_failed|=text.find("fesfx.bnk'")!=std::string::npos;std::fprintf(stderr,"Original game message: %s: %s\n",caption.c_str(),text.c_str());return 1;}
int main(int argc,char **argv){
    if(argc!=2 && argc!=3)return 2;
    const bool configuration_present=argc==3 && std::strcmp(argv[2],"config")==0;
    const bool suspended_start=argc==3 && std::strcmp(argv[2],"suspended-language")==0;
    const bool language_present=argc==3 && (std::strcmp(argv[2],"language")==0 || suspended_start);
    if(argc==3 && !configuration_present && !language_present)return 2;
    if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMEPAD)){std::fprintf(stderr,"SDL startup failed: %s\n",SDL_GetError());return 1;}
    int status=0;try{
        nfs::GameSession session(argv[1],message);
        if(suspended_start){
            SDL_Event event{};event.type=SDL_EVENT_WILL_ENTER_BACKGROUND;if(!SDL_PushEvent(&event))throw std::runtime_error("Could not queue background event");
            bool suspended_immediately=false;try{nfs::renderer_operation([]{return 0;});}catch(const std::logic_error &){suspended_immediately=true;}
            if(!suspended_immediately)throw std::runtime_error("Background watcher did not suspend graphics before event polling");
            session.poll();
        }
        session.start();const auto deadline=SDL_GetTicks()+3000;
        if(suspended_start){
            const auto foreground_at=SDL_GetTicks()+50;
            while(SDL_GetTicks()<foreground_at){session.poll();SDL_Delay(1);}
            if(session.result().finished){std::fprintf(stderr,"FAIL: original startup completed while graphics were suspended\n");status=1;}
            SDL_Event event{};event.type=SDL_EVENT_DID_ENTER_FOREGROUND;if(!SDL_PushEvent(&event))throw std::runtime_error("Could not queue foreground event");session.poll();
        }
        bool timed_out=false;
        while(!session.result().finished){session.poll();if(!timed_out && SDL_GetTicks()>deadline){timed_out=true;std::fprintf(stderr,"Startup deadline reached; pending error=%s\n",session.result().error.c_str());session.request_stop();}SDL_Delay(1);}
        const auto result=session.result();std::fprintf(stderr,"Original startup result: exit=%u messages=%d error=%s\n",result.exit_code,messages,result.error.c_str());
        if(timed_out || !result.error.empty() || !messages || result.exit_code!=1)status=1;
        if(configuration_present && install_failed){std::fprintf(stderr,"FAIL: install.win exists in the fixture but the original loader could not load it\n");status=1;}
        if(configuration_present && !language_failed){std::fprintf(stderr,"FAIL: configuration startup did not reach the missing English language resource\n");status=1;}
        if(language_present && (install_failed || language_failed || !sound_bank_failed)){std::fprintf(stderr,"FAIL: language startup did not reach the missing front-end sound bank\n");status=1;}
    }catch(const std::exception &error){std::fprintf(stderr,"Native startup failure: %s\n",error.what());status=1;}
    SDL_Quit();if(!status)std::puts("PASS: original entry executed through owned runtime, safely reported required-file load failure and completed shutdown; no successful real-asset startup/menus/races claim");return status;
}

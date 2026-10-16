// SPDX-License-Identifier: MIT
#include "GameSession.h"
#include "EngineMemory.h"
#include "GameContexts.h"
#include "GameTrace.h"
#include "FileBridge.h"
#include "FileMappings.h"
#include "DirectoryBridge.h"
#include "EventBridge.h"
#include "CriticalSectionBridge.h"
#include "ThreadBridge.h"
#include "MainThreadBridge.h"
#include "WindowBridge.h"
#include "MenuInput.h"
#include "SDLKeyboardInput.h"
#include "SDLTouchInput.h"
#include "GraphicsLifecycle.h"
#include "SDLThreadCleanup.h"
#include "InputDevices.h"
#include "AudioBridge.h"
#include "ExitCallbacks.h"
#include "TrackRecords.h"
#include "TimerBridge.h"
#include <SDL3/SDL.h>
#include <atomic>
#include <thread>
#include <cstring>
#include <vector>
#include <stdexcept>
#include "Game.h"
#include "BSS.h"
#include "DATA.h"
#include "FunctionIds.h"
#undef true
#undef false
extern "C" bool register_original_game_functions();
extern "C" int32_t stopTimer();
extern "C" SDL_Window *sdlWin;
namespace nfs {
struct GameSession::Impl {
    static Impl *active;
    EngineMemory memory{128*1024*1024};
    FileBridge files;FileMappings mappings{files};DirectoryBridge directories{files.sandbox()};
    EventBridge events;CriticalSectionBridge critical;ThreadBridge workers;
    MainThreadBridge main{sdl_main_thread_scheduler()};WindowBridge window{sdl_window_operations()};
    InputDevices input{nfs_dispatch_game_input};AudioBridge audio;ExitCallbacks exits;
    RuntimeServices services{{initialize,exit_requested,message}};
    std::unique_ptr<SDLGamepadInput> gamepads;
    MenuInput menu;
    TouchPointer touch;
    bool foreground=true,background_shutdown=false,pointer_focused=true;
    GraphicsLifecycle graphics;
    std::unique_ptr<void,void(*)(void *)> game{nullptr,nfs_destroy_game_context};
    std::vector<void *> bootstrap;std::thread thread;WorkerControl control;
    std::mutex result_mutex;GameSessionResult state;bool started=false;
    int32_t(*present)(const std::string &,const std::string &,uint32_t);
    static bool SDLCALL lifecycle_watch(void *context,SDL_Event *event){
        auto &self=*static_cast<Impl *>(context);
        if(SDL_IsMainThread())try{self.lifecycle(event->type);}catch(...){self.failure(std::current_exception());self.request_stop();}
        return true;
    }
    void lifecycle(uint32_t type){
        if(type==SDL_EVENT_TERMINATING){request_stop();return;}
        if(type==SDL_EVENT_WILL_ENTER_BACKGROUND && foreground){
            touch.reset();input.clear_pointer(uint32_t(SDL_GetTicks()));pointer_focused=false;
            foreground=false;graphics.set_suspended(true);audio.set_suspended(true);
            menu.update(0,SDL_GetTicks(),[](void *context,const GameMessage &message){return static_cast<Impl *>(context)->window.post(message);},this);
            window.post({0,8,0,0,uint32_t(SDL_GetTicks()),0,0});
        }
        if(type==SDL_EVENT_DID_ENTER_FOREGROUND && !foreground){
            pointer_focused=true;
            graphics.set_suspended(false);foreground=true;audio.set_suspended(false);window.post({0,7,0,0,uint32_t(SDL_GetTicks()),0,0});
        }
    }
    explicit Impl(const std::string &root,decltype(present) show):files(root),present(show){
        if(active || !SDL_IsMainThread())throw std::logic_error("Game session needs the sole SDL main-thread owner");
        if((SDL_WasInit(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMEPAD)&(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMEPAD))!=(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMEPAD))throw std::logic_error("SDL video/audio/gamepad initialization required before session");
        if(!register_original_game_functions() || !nfs_register_input_functions())throw std::runtime_error("Original native function registration failed");
        game.reset(nfs_create_game_context());if(!game)throw std::bad_alloc();
        gamepads.reset(new SDLGamepadInput(input));
        if(!SDL_AddEventWatch(lifecycle_watch,this))throw std::runtime_error(SDL_GetError());
        bind_file_bridge(&files);bind_file_mappings(&mappings);bind_directory_bridge(&directories);
        bind_event_bridge(&events);bind_critical_sections(&critical);bind_thread_bridge(&workers);
        bind_main_thread_bridge(&main);bind_window_bridge(&window);bind_input_devices(&input);
        bind_audio_bridge(&audio);bind_exit_callbacks(&exits);bind_track_record_files(&files);bind_runtime_services(&services);active=this;
    }
    ~Impl(){
        SDL_RemoveEventWatch(lifecycle_watch,this);
        // Destruction before completion would deadlock the app main queue or
        // free bindings still borrowed by workers. App iterate must finish first.
        if(thread.joinable()){if(!state.finished)std::terminate();thread.join();}
        gamepads.reset();game.reset();for(auto p:bootstrap)memory.release(p);
        bind_runtime_services(nullptr);bind_track_record_files(nullptr);bind_exit_callbacks(nullptr);bind_audio_bridge(nullptr);
        bind_input_devices(nullptr);bind_window_bridge(nullptr);bind_main_thread_bridge(nullptr);bind_thread_bridge(nullptr);
        bind_critical_sections(nullptr);bind_event_bridge(nullptr);bind_directory_bridge(nullptr);bind_file_mappings(nullptr);bind_file_bridge(nullptr);active=nullptr;
    }
    void *storage(size_t size){auto p=memory.allocate(size);if(!p)throw std::bad_alloc();try{bootstrap.push_back(p);}catch(...){memory.release(p);throw;}std::memset(p,0,size);return p;}
    uint32_t string(const char *text){auto p=storage(std::strlen(text)+1);std::strcpy(static_cast<char *>(p),text);return memory.encode(p);}
    static void initialize(){
        auto &self=*active;
        for(const auto path:{"fedata/pc/config","fedata/pc/save","fedata/pc/stats/prh","gamedata/tmptrk"})if(!self.files.sandbox().create_directories(path))throw std::runtime_error("Game sandbox directory creation failed");
        const auto data=self.string("fedata/pc/");guest_store<uint32_t>(dword_5134B4,data);guest_store<uint32_t>(dword_5134B8,data);
        guest_store<uint32_t>(dword_5134C4,self.string("fedata/pc/stats/"));guest_store<uint32_t>(dword_5134D8,self.memory.encode(self.storage(4)));
        // Original non-x86 entry forces initialization of this relative table.
        auto table=dword_4DB6A8;
#undef dword_4DB6A8
        {std::lock_guard<std::mutex> lock(guest_copy_mutex());std::memset(table,0,sizeof(_data.dword_4DB6A8));}
    }
    static void exit_requested(uint32_t){active->request_stop();}
    static int32_t message(const std::string &text,const std::string &caption,uint32_t flags){if(!active->present)throw std::runtime_error(caption+": "+text);return active->present(text,caption,flags);}
    void request_stop(){
        {std::lock_guard<std::mutex> lock(control.mutex);control.stop=true;control.changed.notify_all();}
        wake_worker_waits();
    }
    void failure(std::exception_ptr error){try{std::rethrow_exception(error);}catch(const std::exception &e){std::lock_guard<std::mutex> lock(result_mutex);if(state.error.empty())state.error=std::string(e.what())+"; "+recent_game_calls();}catch(...){std::lock_guard<std::mutex> lock(result_mutex);if(state.error.empty())state.error="Unknown native game failure";}}
    template<class F> void cleanup(F action){try{action();}catch(...){failure(std::current_exception());}}
    void run(uint32_t function){
        SDLThreadCleanup thread_cleanup;
        current_worker_control()=&control;
        try{nfs_dispatch_game_procedure(game.get(),function);}catch(const GameExited &exit){std::lock_guard<std::mutex> lock(result_mutex);state.exit_code=exit.code;}catch(const WorkerStopped &){}catch(...){failure(std::current_exception());}
        critical.retire_current_thread();request_stop();
        ScopedWorkerStopDeferral orderly_cleanup;
        // Keep app main dispatch alive while joining and draining renderer/audio.
        // Original cleanup signals worker quit flags and waits for their original
        // completion fields. Cancelling them first skips those epilogues.
        cleanup([&]{exits.drain(game.get());});workers.request_stop_all();window.stop();
        cleanup([&]{workers.join_all();});cleanup([&]{audio.stop();});cleanup([]{stopTimer();});
        cleanup([&]{if(!mappings.flush_and_unmap_all())throw std::runtime_error("Game mapped save writeback failed");});
        events.shutdown();cleanup([&]{window.shutdown();});current_worker_control()=nullptr;
        {std::lock_guard<std::mutex> lock(result_mutex);state.finished=true;}
    }
};
GameSession::Impl *GameSession::Impl::active=nullptr;
GameSession::GameSession(const std::string &root,decltype(Impl::present) show):impl(new Impl(root,show)){}
GameSession::~GameSession()=default;
void GameSession::start(uint32_t function){if(impl->started)throw std::logic_error("Original game session already started");impl->started=true;impl->thread=std::thread([self=impl.get(),function]{self->run(function?function:game_id_start);});}
void GameSession::request_stop(){impl->request_stop();}
GameSessionResult GameSession::result(){std::lock_guard<std::mutex> lock(impl->result_mutex);return impl->state;}
void GameSession::poll(){
    if(!SDL_IsMainThread())throw std::logic_error("Game event polling needs app main thread");
    SDL_Event event;while(SDL_PollEvent(&event)){
        if(event.type==SDL_EVENT_QUIT || event.type==SDL_EVENT_TERMINATING)impl->request_stop();
        impl->lifecycle(event.type); // Fallback for events pushed from other threads; owner notifications were handled immediately.
        const auto now=uint32_t(SDL_GetTicks());
        if(event.type==SDL_EVENT_WINDOW_FOCUS_LOST){impl->pointer_focused=false;impl->touch.reset();impl->input.clear_pointer(now);}
        if(event.type==SDL_EVENT_WINDOW_FOCUS_GAINED)impl->pointer_focused=true;
        if(impl->foreground && impl->pointer_focused){
            const auto touch=original_touch_event(impl->touch,event,sdlWin?SDL_GetWindowID(sdlWin):0,now);
            if(touch.clear)impl->input.clear_pointer(now);
            if(touch.dx || touch.dy)impl->input.pointer_motion(touch.dx,touch.dy,now);
            if(touch.action==TouchAction::click)impl->input.pointer_click(now);
            original_touch_key(touch.action,now,[](void *context,const GameMessage &message){return static_cast<Impl *>(context)->window.post(message);},impl.get());
        }
        original_keyboard_event(event,uint32_t(SDL_GetTicks()),[](void *context,const GameMessage &message){return static_cast<Impl *>(context)->window.post(message);},impl.get());
    }
    if(!impl->foreground && impl->control.stop.load() && !impl->background_shutdown){
        impl->main.retire_renderer();
        impl->graphics.abandon_suspended();
        impl->background_shutdown=true;
    }
    impl->gamepads->update(impl->foreground);
    impl->menu.update(impl->foreground?impl->gamepads->menu_buttons():0,SDL_GetTicks(),[](void *context,const GameMessage &message){return static_cast<Impl *>(context)->window.post(message);},impl.get());
}
}

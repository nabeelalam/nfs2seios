// SPDX-License-Identifier: MIT
#include "AudioBridge.h"
#include "GameContexts.h"
#include "GuestAccess.h"
#include <SDL3/SDL.h>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <future>
class Game {};
static Game root;
alignas(16) static uint8_t buffer[1024];
static std::atomic<unsigned> frames{0},created{0},destroyed{0};static bool occupied=false;
static std::atomic<bool> block_next{false};static std::promise<void> callback_entered,callback_release;
static std::shared_future<void> callback_gate;
extern "C" Game *nfs_create_game_context(){++created;return new Game;}
extern "C" void nfs_destroy_game_context(void *p){delete static_cast<Game *>(p);++destroyed;}
extern "C" void *malloc32(size_t bytes){if(bytes!=1024 || occupied)return nullptr;occupied=true;return buffer;}
extern "C" void free32(void *p){if(p!=buffer || !occupied)throw std::runtime_error("SDL audio buffer ownership");occupied=false;}
extern "C" uint32_t nfs_guest_address(const void *p){if(p!=buffer)throw std::out_of_range("SDL sample buffer");return 0x90000000;}
extern "C" void *nfs_host_address(uint32_t id,size_t bytes){if(id!=0x90000000 || bytes>1024)throw std::out_of_range("SDL sample span");return buffer;}
extern "C" void *nfs_resolve_game_context(uint32_t){return &root;}
static void noop(void *){}
extern "C" NativeGameFunction nfs_lookup_guest_function(uint32_t id){if(id!=1 && id!=2)throw std::out_of_range("SDL audio function");return noop;}
extern "C" void nfs_dispatch_game_procedure(void *,uint32_t){}
extern "C" void nfs_dispatch_game_regparm2(void *,uint32_t id,uint32_t samples,uint32_t count){
    if(id!=1 || samples!=0x90000000 || count!=256)throw std::runtime_error("SDL audio callback ABI");
    if(block_next.exchange(false)){callback_entered.set_value();callback_gate.wait();}
    const auto start=frames.fetch_add(count);
    for(unsigned i=0;i<512;++i){const int16_t sample=((start+i/2)%100)<50?10000:-10000;std::memcpy(buffer+i*2,&sample,2);}
}
int main(){
    if(!SDL_Init(SDL_INIT_AUDIO)){std::fprintf(stderr,"Audio init: %s\n",SDL_GetError());return 1;}
    nfs::AudioBridge audio;audio.functions(1,2);audio.set_suspended(true);if(!audio.start()){std::fprintf(stderr,"Open audio: %s\n",SDL_GetError());return 1;}audio.serve(&root);
    SDL_Delay(40);if(frames!=0){std::fprintf(stderr,"FAIL: suspended startup consumed audio\n");return 1;}
    audio.set_suspended(false);audio.serve(&root);
    const auto deadline=SDL_GetTicks()+2000;while(frames<512 && SDL_GetTicks()<deadline)SDL_Delay(5);
    callback_gate=callback_release.get_future().share();auto blocked=callback_entered.get_future();block_next=true;
    if(blocked.wait_for(std::chrono::seconds(2))!=std::future_status::ready){std::fprintf(stderr,"FAIL: contention callback did not start\n");callback_release.set_value();return 1;}
    // The real SDL callback is now holding its device lock. App suspension and
    // foreground requests must return while that callback remains blocked.
    const auto control_start=SDL_GetTicks();audio.set_suspended(true);audio.set_suspended(false);audio.serve(&root);
    const bool nonblocking=SDL_GetTicks()-control_start<500;
    callback_release.set_value();if(!nonblocking){std::fprintf(stderr,"FAIL: lifecycle audio controls blocked on device callback\n");return 1;}
    audio.set_suspended(true);SDL_Delay(40);const auto paused_frames=frames.load();audio.serve(&root);SDL_Delay(40);
    if(frames!=paused_frames || created!=1 || destroyed!=0){std::fprintf(stderr,"FAIL: paused stream consumed audio or retired its context\n");return 1;}
    audio.set_suspended(false);audio.serve(&root);
    const auto resume_deadline=SDL_GetTicks()+2000;while(frames==paused_frames && SDL_GetTicks()<resume_deadline)SDL_Delay(5);
    if(frames==paused_frames){std::fprintf(stderr,"FAIL: resumed stream did not consume audio\n");return 1;}
    audio.set_suspended(true); // Shutdown must also work while suspended.
    audio.stop();const auto stopped_frames=frames.load();SDL_Delay(30);
    const bool passed=stopped_frames>=512 && frames==stopped_frames && created==destroyed && !occupied;
    SDL_Quit();if(!passed){std::fprintf(stderr,"FAIL: SDL callback consumption or retirement\n");return 1;}
    std::printf("PASS: actual SDL dummy output consumed %u mapped stereo frames, suspended startup, nonblocking controls while callback holds device lock, retained-context pause/resume, paused shutdown and callback/buffer retirement; no physical audio claim\n",stopped_frames);
}

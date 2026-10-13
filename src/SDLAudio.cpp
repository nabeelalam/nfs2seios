// SPDX-License-Identifier: MIT
#include "AudioBridge.h"
#include "SDLThreadCleanup.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
namespace {
struct NativeAudio {
    SDL_AudioStream *stream=nullptr;
    uintptr_t token=0;
    std::mutex mutex;std::condition_variable changed;std::thread worker;
    bool requested_paused=true,paused=true,stopping=false,failed=false;
    ~NativeAudio(){
        {std::lock_guard<std::mutex> lock(mutex);stopping=true;changed.notify_all();}
        if(worker.joinable())worker.join();
        if(stream)SDL_DestroyAudioStream(stream);
    }
    bool request(bool value){
        std::lock_guard<std::mutex> lock(mutex);if(stopping || failed)return false;
        requested_paused=value;changed.notify_all();return true;
    }
    void run(){
        nfs::SDLThreadCleanup thread_cleanup;
        std::unique_lock<std::mutex> lock(mutex);
        for(;;){
            changed.wait(lock,[&]{return stopping || paused!=requested_paused;});
            if(stopping)return;
            const bool target=requested_paused;lock.unlock();
            // SDL takes the device lock here; callbacks can hold that lock while
            // waiting on original game work. Never do this on the app thread or
            // while holding our request lock. Latest requests remain writable.
            const bool ok=target?SDL_PauseAudioStreamDevice(stream):SDL_ResumeAudioStreamDevice(stream);
            if(!ok){nfs_audio_report_error(token,SDL_GetError());lock.lock();failed=true;return;}
            lock.lock();paused=target;
        }
    }
};
}
static void on_audio(void *userdata,SDL_AudioStream *stream,int additional,int){
    uint8_t buffer[4096];const uintptr_t token=reinterpret_cast<uintptr_t>(userdata);
    while(additional>0){
        const size_t count=nfs_audio_render(token,buffer,size_t(std::min(additional,int(sizeof(buffer)))));
        if(!count)break;
        if(!SDL_PutAudioStreamData(stream,buffer,int(count))){nfs_audio_report_error(token,SDL_GetError());break;}
        additional-=int(count);
    }
}
extern "C" void *nfs_native_audio_open(uintptr_t token){
    const SDL_AudioSpec input{SDL_AUDIO_S16LE,2,22050};
    std::unique_ptr<NativeAudio> backend(new NativeAudio);
    backend->token=token;
    backend->stream=SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,&input,on_audio,reinterpret_cast<void *>(token));
    if(!backend->stream)return nullptr;
    auto pointer=backend.get();backend->worker=std::thread([pointer]{pointer->run();});
    return backend.release();
}
extern "C" bool nfs_native_audio_resume(void *backend){return backend && static_cast<NativeAudio *>(backend)->request(false);}
extern "C" bool nfs_native_audio_pause(void *backend){return backend && static_cast<NativeAudio *>(backend)->request(true);}
extern "C" void nfs_native_audio_close(void *backend){delete static_cast<NativeAudio *>(backend);}

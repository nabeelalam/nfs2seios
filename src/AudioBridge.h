// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <memory>
#include <mutex>
namespace nfs {
class AudioBridge {
public:
    struct State;
private:
    std::mutex operation;
    std::shared_ptr<State> state;
    uint32_t samples=0,fade=0;
    bool suspended=false;
    void retire(); // operation mutex held; callbacks never call this.
public:
    void functions(uint32_t sample_function,uint32_t fade_function);
    bool start();
    void serve(void *game);
    void stop();
    void set_suspended(bool value);
    ~AudioBridge(){try{stop();}catch(...) {}}
};
void bind_audio_bridge(AudioBridge *bridge);
}
extern "C" void *nfs_native_audio_open(uintptr_t token);
// Native SDL controls acknowledge queued requests; backend errors are reported
// through nfs_audio_report_error. Close joins the owned control worker.
extern "C" bool nfs_native_audio_resume(void *stream);
extern "C" bool nfs_native_audio_pause(void *stream);
extern "C" void nfs_native_audio_close(void *stream);
extern "C" size_t nfs_audio_render(uintptr_t token,void *output,size_t bytes) noexcept;
extern "C" void nfs_audio_report_error(uintptr_t token,const char *message) noexcept;
extern "C" int32_t iSNDdirectsetfunctions_wrap(int32_t,int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t iSNDdirectstart_(int32_t,int32_t);
extern "C" int32_t iSNDdirectstop_();
extern "C" int32_t iSNDdirectserve_(int32_t);
extern "C" int32_t iSNDdirectcaps_(int32_t);
extern "C" int32_t iSNDdllversion_();

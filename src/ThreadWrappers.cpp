// SPDX-License-Identifier: MIT
#include "ThreadBridge.h"
#include "GuestAccess.h"
#include "GameContexts.h"
#include "PlatformError.h"
namespace nfs {ThreadBridge *current_thread_bridge();}
extern "C" int32_t CreateThread_wrap(int32_t security,int32_t,int32_t function,int32_t parameter,int32_t flags,int32_t output) {
    if(output)nfs_host_address(uint32_t(output),4);
    auto *bridge=nfs::current_thread_bridge();
    void *context=nfs_resolve_game_context(uint32_t(parameter));
    if(!bridge || security || (uint32_t(flags)&~4u) || !context) {
        nfs_destroy_game_context(context);nfs::platform_error()=50;return 0;
    }
    try {
        nfs_lookup_guest_function(uint32_t(function));
        nfs_prepare_game_worker(context,uint32_t(function));
    }
    catch (...) {nfs_destroy_game_context(context);throw;}
    uint32_t thread_id=0;
    const uint32_t handle=bridge->create(nfs_run_game_worker,context,nfs_destroy_game_context,(flags&4)!=0,thread_id);
    if(output)guest_store<uint32_t>(uint32_t(output),thread_id);
    nfs::platform_error()=handle?0:8;return int32_t(handle);
}
extern "C" int32_t ResumeThread_wrap(int32_t handle) {
    auto *bridge=nfs::current_thread_bridge();
    const uint32_t previous=bridge?bridge->resume(uint32_t(handle)):UINT32_MAX;
    nfs::platform_error()=previous==UINT32_MAX?6:0;return int32_t(previous);
}
extern "C" int32_t GetCurrentThreadId_wrap(){return int32_t(nfs::current_game_thread_id());}
extern "C" int32_t GetCurrentThread_wrap(){return -2;}
extern "C" int32_t GetCurrentProcess_wrap(){return -1;}
extern "C" int32_t TerminateThread_wrap(int32_t handle,int32_t){
    auto *bridge=nfs::current_thread_bridge();const bool ok=bridge && bridge->terminate(uint32_t(handle));
    nfs::platform_error()=ok?0:6;return ok;
}
extern "C" int32_t DuplicateHandle_wrap(int32_t source_process,int32_t source,int32_t target_process,int32_t output,int32_t,int32_t inherit,int32_t options) {
    if(!output){nfs::platform_error()=87;return 0;}
    nfs_host_address(uint32_t(output),4);
    auto *bridge=nfs::current_thread_bridge();
    // The translated game requests same-process, same-access thread handles.
    if(!bridge || source_process!=-1 || target_process!=-1 || inherit || options!=2){nfs::platform_error()=50;return 0;}
    const uint32_t id=bridge->duplicate(uint32_t(source));
    if(id)guest_store<uint32_t>(uint32_t(output),id);
    nfs::platform_error()=id?0:6;return id?1:0;
}

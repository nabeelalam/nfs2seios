// SPDX-License-Identifier: MIT
#include "ExitCallbacks.h"
#include "GameContexts.h"
#include "GuestAccess.h"
#include "PlatformError.h"
#include <exception>
#include <stdexcept>
namespace nfs {
bool ExitCallbacks::add(uint32_t function){
    nfs_lookup_guest_function(function);
    std::lock_guard<std::mutex> lock(mutex);
    if(state==State::closed || count==512)return false;
    callbacks[count++]=function;return true;
}
void ExitCallbacks::drain(void *game){
    if(!game)throw std::invalid_argument("Missing exit Game context");
    {std::lock_guard<std::mutex> lock(mutex);if(state==State::closed)return;if(state==State::draining)throw std::logic_error("Recursive exit drain");state=State::draining;}
    ScopedWorkerStopDeferral cleanup;
    std::exception_ptr failure;
    for(;;){
        uint32_t function;
        {std::lock_guard<std::mutex> lock(mutex);if(!count){state=State::closed;break;}function=callbacks[--count];}
        try{nfs_dispatch_game_exit(game,function);}catch(...){if(!failure)failure=std::current_exception();}
    }
    if(failure)std::rethrow_exception(failure);
}
static ExitCallbacks *active=nullptr;
void bind_exit_callbacks(ExitCallbacks *callbacks){active=callbacks;}
}
extern "C" int32_t WrapperAtExit(int32_t function){
    if(!nfs::active){nfs::platform_error()=50;return -1;}
    const bool ok=nfs::active->add(uint32_t(function));nfs::platform_error()=ok?0:8;return ok?0:-1;
}

// SPDX-License-Identifier: MIT
#include "RuntimeServices.h"
#include "GuestAccess.h"
#include "PlatformError.h"
#include <stdexcept>
namespace nfs {
static RuntimeServices *active=nullptr;
void bind_runtime_services(RuntimeServices *services){active=services;}
RuntimeServices &runtime_services(){if(!active)throw std::logic_error("App runtime services unbound");return *active;}
void RuntimeServices::initialize(){
    {std::lock_guard<std::mutex> lock(mutex);if(exiting)throw std::logic_error("Game initialization after exit");}
    std::call_once(startup,[&]{
        {std::lock_guard<std::mutex> lock(mutex);if(exiting)throw GameExited(status);}
        if(!operations.initialize)throw std::logic_error("Native game startup unbound");
        operations.initialize();
        std::lock_guard<std::mutex> lock(mutex);if(exiting)throw GameExited(status);initialized=true;
    });
    {std::lock_guard<std::mutex> lock(mutex);if(exiting)throw GameExited(status);}
}
[[noreturn]] void RuntimeServices::exit(uint32_t code){
    bool first;uint32_t result;
    {std::lock_guard<std::mutex> lock(mutex);first=!exiting;if(first){exiting=true;status=code;}result=status;}
    // Notification requests shutdown; it must not join the current worker or
    // synchronously drain a main-thread operation. App owns subsequent cleanup.
    if(first && operations.exit_requested)operations.exit_requested(result);
    throw GameExited(result);
}
bool RuntimeServices::exit_status(uint32_t &code){std::lock_guard<std::mutex> lock(mutex);code=status;return exiting;}
int32_t RuntimeServices::message(const std::string &text,const std::string &caption,uint32_t flags){if(!operations.message)throw std::logic_error("Game error presentation unbound");return operations.message(text,caption,flags);}
}
static std::string text(uint32_t address,size_t bound){std::string result;for(size_t i=0;i<bound && uint64_t(address)+i<=UINT32_MAX;++i){const auto ch=guest_load<char>(address+uint32_t(i));if(!ch)return result;result+=ch;}throw std::out_of_range("Unterminated game message");}
extern "C" int32_t WrapperInit(){nfs::runtime_services().initialize();return 0;}
extern "C" int32_t ExitProcess_wrap(int32_t code){nfs::runtime_services().exit(uint32_t(code));}
extern "C" int32_t MessageBoxA_wrap(int32_t,int32_t message,int32_t caption,int32_t flags){return nfs::runtime_services().message(text(uint32_t(message),4096),text(uint32_t(caption),260),uint32_t(flags));}
// The original non-Windows wrapper also has no Windows keyboard/module object.
extern "C" int32_t GetKeyboardType_wrap(int32_t){return 0;}
extern "C" int32_t GetModuleHandleA_wrap(int32_t){return 0;}
extern "C" int32_t SetThreadPriority_wrap(int32_t,int32_t){nfs::platform_error()=50;return 0;}

// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <exception>
#include <mutex>
#include <string>
namespace nfs {
struct GameExited:std::exception {
    uint32_t code;explicit GameExited(uint32_t status):code(status){}
    const char *what()const noexcept override{return "Original game requested exit";}
};
struct RuntimeOperations {
    void (*initialize)();
    void (*exit_requested)(uint32_t);
    int32_t (*message)(const std::string &,const std::string &,uint32_t);
};
// Bind before game workers. Initialization must configure native SDL, the owned
// sandbox and original game paths; failures propagate to the app, not exit().
class RuntimeServices {
    RuntimeOperations operations;std::mutex mutex;std::once_flag startup;
    bool initialized=false,exiting=false;uint32_t status=0;
public:
    explicit RuntimeServices(RuntimeOperations ops):operations(ops){}
    void initialize();
    [[noreturn]] void exit(uint32_t);
    int32_t message(const std::string &,const std::string &,uint32_t);
    bool exit_status(uint32_t &);
};
void bind_runtime_services(RuntimeServices *);
RuntimeServices &runtime_services();
int32_t sdl_game_message(const std::string &,const std::string &,uint32_t);
}
extern "C" int32_t WrapperInit();
extern "C" int32_t ExitProcess_wrap(int32_t);
extern "C" int32_t MessageBoxA_wrap(int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t GetKeyboardType_wrap(int32_t);
extern "C" int32_t GetModuleHandleA_wrap(int32_t);
extern "C" int32_t SetThreadPriority_wrap(int32_t,int32_t);

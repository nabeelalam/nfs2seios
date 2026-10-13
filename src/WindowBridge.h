// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <mutex>
#include <condition_variable>
#include <deque>
namespace nfs {
struct GameMessage {uint32_t window,message,wparam,lparam,time,x,y;};
static_assert(sizeof(GameMessage)==28,"Original 32-bit MSG layout");
struct WindowOperations {void *(*create)();void (*destroy)(void *);};
// One original game window; native pointers never enter the message record.
class WindowBridge {
    WindowOperations operations;
    std::mutex mutex,operation_mutex;
    std::condition_variable changed;
    std::deque<GameMessage> messages;
    void *native=nullptr;
    uint32_t handle=0,generation=0,callback=0;
    bool stopping=false;
public:
    explicit WindowBridge(WindowOperations ops):operations(ops){}
    ~WindowBridge();
    uint32_t create(uint32_t function);
    bool post(GameMessage message);
    int32_t get(GameMessage &message,uint32_t window);
    int32_t dispatch(void *context,const GameMessage &message);
    bool destroy(uint32_t window);
    void stop(); // Close message delivery; join game workers before shutdown.
    void shutdown();
    void wake_workers();
};
void bind_window_bridge(WindowBridge *bridge);
WindowOperations sdl_window_operations();
}
extern "C" int32_t WrapperCreateWindow(int32_t);
extern "C" int32_t GetMessageA_wrap(int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t PostMessageA_wrap(int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t DispatchMessageA_wrap(int32_t,int32_t);
extern "C" int32_t DestroyWindow_wrap(int32_t);
extern "C" int32_t DefWindowProcA_wrap(int32_t,int32_t,int32_t,int32_t);

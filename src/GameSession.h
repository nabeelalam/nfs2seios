// SPDX-License-Identifier: MIT
#pragma once
#include "RuntimeServices.h"
#include <memory>
namespace nfs {
struct GameSessionResult {bool finished=false;uint32_t exit_code=0;std::string error;};
// Construct/poll/retire on the initialized SDL app main thread. The event loop
// must continue through finished() so synchronous renderer cleanup can complete.
class GameSession {
    struct Impl;std::unique_ptr<Impl> impl;
public:
    explicit GameSession(const std::string &asset_root,
        int32_t(*message)(const std::string &,const std::string &,uint32_t)=sdl_game_message);
    ~GameSession(); // Owner must wait for finished before destruction/SDL_Quit.
    void start(uint32_t function=0); // Zero selects the original _start.
    void poll();
    void request_stop();
    GameSessionResult result();
};
}

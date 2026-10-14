// SPDX-License-Identifier: MIT
#pragma once
#include <SDL3/SDL.h>
namespace nfs {
class GraphicsLifecycle {
    SDL_GLContext context=nullptr;SDL_Window *window=nullptr;bool suspended=false;
public:
    void set_suspended(bool value);
    void abandon_suspended();
};
}

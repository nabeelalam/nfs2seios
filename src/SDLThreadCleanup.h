// SPDX-License-Identifier: MIT
#pragma once
#include <SDL3/SDL_thread.h>
namespace nfs {
// SDL requires explicit TLS retirement for threads created outside SDL.
// Declare before other thread-local scope guards so cleanup runs last.
struct SDLThreadCleanup {
    ~SDLThreadCleanup() { SDL_CleanupTLS(); }
};
}

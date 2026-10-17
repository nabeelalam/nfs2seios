// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <mutex>
namespace nfs {
// App-owned cleanup, not process atexit: the mapped Game must outlive draining.
class ExitCallbacks {
    uint32_t callbacks[512]{};
    size_t count=0;
    std::mutex mutex;
    enum class State {open,draining,closed} state=State::open;
public:
    bool add(uint32_t function);
    void drain(void *game);
    // Failure-cancellable original cleanup; first error closes remaining callbacks.
    void drain_cancellable(void *game);
private:
    void drain_impl(void *game,bool cancellable);
};
void bind_exit_callbacks(ExitCallbacks *callbacks);
}
extern "C" int32_t WrapperAtExit(int32_t);

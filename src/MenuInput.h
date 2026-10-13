// SPDX-License-Identifier: MIT
#pragma once
#include "WindowBridge.h"
namespace nfs {
enum MenuButton : uint8_t {menu_left=1,menu_up=2,menu_right=4,menu_down=8,menu_confirm=16,menu_cancel=32};
using MenuMessageSink=bool(*)(void *,const GameMessage &);
// Aggregate both pads before calling update; a second held pad owns the same
// key until all sources release it. Only directional navigation repeats.
class MenuInput {
    uint8_t held=0;uint64_t repeat_at[4]{};
public:
    void update(uint8_t desired,uint64_t now,MenuMessageSink sink,void *context);
};
}

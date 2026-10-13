// SPDX-License-Identifier: MIT
#include "InputAxes.h"
#include "GuestAccess.h"
#include "BSS.h"
#include <atomic>
namespace {std::atomic<bool> use_deadzone{false};}
namespace nfs {void set_game_axis_deadzone(bool enabled){use_deadzone.store(enabled);}}
extern "C" int32_t processAxes(int32_t calibration){
    return nfs::read_calibrated_axis(uint32_t(calibration),nfs_guest_address(dword_557540),use_deadzone.load());
}

// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <string>
extern "C" void nfs_trace_guest_call(uint32_t,uint32_t) noexcept;
namespace nfs {std::string recent_game_calls();}

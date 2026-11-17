// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <string>
extern "C" void nfs_trace_guest_call(uint32_t,uint32_t) noexcept;
namespace nfs {
std::string recent_game_calls();
std::string sampled_game_calls();
enum class OpponentsVisit { Entered, ScenePrepared, Left };
using OpponentsVisitSink = void(*)(OpponentsVisit stage);
void set_opponents_visit_sink(OpponentsVisitSink sink) noexcept;
void log_opponents_visit(OpponentsVisit stage) noexcept;
}

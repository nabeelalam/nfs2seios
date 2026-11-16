// SPDX-License-Identifier: MIT
#pragma once
#include "MenuResponsiveCached.h"
#include "MenuResponsivePointerHooks.h"
#include <array>
#include <cerrno>
#include <stdint.h>
// Pure host lifetime metadata shared by source observers and scene owners.
// One inline TLS state definition; no guest/Game/data-header dependency.
namespace nfs_menu_cached_widget_source {
constexpr unsigned Capacity=40,ItemCapacity=64;
struct Row {NfsMenuCachedWidget value{};uint32_t address=0;};
struct Snapshot {
 uint32_t tag=0,descriptor=0,header0=0,header1=0,cursorX=0,cursorY=0,count=0,itemCount=0,opponentAddress=0,opponentValid=0;
 uintptr_t game=0;std::array<Row,Capacity> rows{};
};
struct State {uint64_t ownerGeneration=0;unsigned depth=0;bool invalid=false,valid=false;Snapshot producer;};
inline State& state(){static thread_local State s;return s;}
inline void invalidate()noexcept {
 state().valid=false;
 try{nfs_menu_cached_widgets(0,0,0,0,nullptr,0,0,0);}catch(...){nfs_menu_pointer_metadata_failure();}
}
inline void ownerBoundary()noexcept{const int saved=errno;auto&s=state();++s.ownerGeneration;if(s.depth)s.invalid=true;invalidate();errno=saved;}
inline void retire()noexcept{const int saved=errno;invalidate();errno=saved;}
}

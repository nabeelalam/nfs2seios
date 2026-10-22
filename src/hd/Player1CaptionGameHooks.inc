// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include "SoftwareFontProvider.h"
#ifndef NFS_PLAYER1_CAPTION_TEXT_TEST
#include "ContinuousLogoGameHooks.inc"
#endif
#include <atomic>
#include <exception>
#include <limits>
#include <new>
#include <cstring>
// Player 1 authority is exact lexical40C160/incoming40BBE0 form off_4D3050.
// Main selector2 resolves through its actual table entry to resource2.
// Original caption "player one" is right aligned168, measured68, left100, y198.
namespace nfs_player1_caption {
struct CaptionPolicy final {
 static constexpr uint32_t selector=2,resource=2;
 static constexpr unsigned textLength=10;
 static constexpr int32_t measureWidth=68,textLeft=100,textY=198;
 static uint32_t formAddress()noexcept{return uint32_t(guest_address(off_4D3050));}
 static uint32_t entryAddress()noexcept{return uint32_t(guest_address(dword_4D3E4C));}
 static const char*text()noexcept{return "player one";}
 static const char*unsupportedTextReason()noexcept{return "sidebar-player-one-text";}
};
}
#define NFS_CACHED_MENU_CAPTION_NAMESPACE nfs_player1_caption
#include "CachedMenuCaptionGameHooks.inc"
#undef NFS_CACHED_MENU_CAPTION_NAMESPACE

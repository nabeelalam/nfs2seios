// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include "SoftwareFontProvider.h"
#ifndef NFS_PLAYER2_CAPTION_TEXT_TEST
#include "ContinuousLogoGameHooks.inc"
#endif
#include <atomic>
#include <exception>
#include <limits>
#include <new>
#include <cstring>
// Player 2 authority is exact lexical40C160/incoming40BBE0 form off_4D3100.
// Main selector3 resolves through its actual table entry to resource3.
// Original caption "player two" is right aligned168, measured70, left98, y234.
namespace nfs_player2_caption {
struct CaptionPolicy final {
 static constexpr uint32_t selector=3,resource=3;
 static constexpr unsigned textLength=10;
 static constexpr int32_t measureWidth=70,textLeft=98,textY=234;
 static uint32_t formAddress()noexcept{return uint32_t(guest_address(off_4D3100));}
 static uint32_t entryAddress()noexcept{return uint32_t(guest_address(dword_4D3E60));}
 static const char*text()noexcept{return "player two";}
 static const char*unsupportedTextReason()noexcept{return "sidebar-player-two-text";}
};
}
#define NFS_CACHED_MENU_CAPTION_NAMESPACE nfs_player2_caption
#include "CachedMenuCaptionGameHooks.inc"
#undef NFS_CACHED_MENU_CAPTION_NAMESPACE

// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include "SoftwareFontProvider.h"
#ifndef NFS_SETUP_CAPTION_TEXT_TEST
#include "ContinuousLogoGameHooks.inc"
#endif
#include <atomic>
#include <exception>
#include <limits>
#include <new>
#include <cstring>
// Setup authority is exact lexical40C160/incoming40BBE0 form off_4D2790.
// Main selector0 resolves through the actual table to resource0.
// Original caption "game setup" is right aligned168, measured75, left93, y126.
namespace nfs_setup_caption {
struct CaptionPolicy final {
 static constexpr uint32_t selector=0,resource=0;
 static constexpr unsigned textLength=10;
 static constexpr int32_t measureWidth=75,textLeft=93,textY=126;
 static uint32_t formAddress()noexcept{return uint32_t(guest_address(off_4D2790));}
 static uint32_t entryAddress()noexcept{return uint32_t(guest_address(dword_4D3E24));}
 static const char*text()noexcept{return "game setup";}
 static const char*unsupportedTextReason()noexcept{return "sidebar-game-setup-text";}
};
}
#define NFS_CACHED_MENU_CAPTION_NAMESPACE nfs_setup_caption
#include "CachedMenuCaptionGameHooks.inc"
#undef NFS_CACHED_MENU_CAPTION_NAMESPACE

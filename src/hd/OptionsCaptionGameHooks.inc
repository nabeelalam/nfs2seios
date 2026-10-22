// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFramePacket.h"
#include "SoftwareFontProvider.h"
#ifndef NFS_OPTIONS_CAPTION_TEXT_TEST
#include "ContinuousLogoGameHooks.inc"
#endif
#include <atomic>
#include <exception>
#include <limits>
#include <new>
#include <cstring>
// Options authority is exact lexical40C160/incoming40BBE0 form off_4D3A88.
// Main selector6 resolves through the actual table to resource7.
// Original caption "options" is right aligned168, measured48, left120, y364.
namespace nfs_options_caption {
struct CaptionPolicy final {
 static constexpr uint32_t selector=6,resource=7;
 static constexpr unsigned textLength=7;
 static constexpr int32_t measureWidth=48,textLeft=120,textY=364;
 static uint32_t formAddress()noexcept{return uint32_t(guest_address(off_4D3A88));}
 static uint32_t entryAddress()noexcept{return uint32_t(guest_address(dword_4D3E9C));}
 static const char*text()noexcept{return "options";}
 static const char*unsupportedTextReason()noexcept{return "sidebar-options-text";}
};
}
#define NFS_CACHED_MENU_CAPTION_NAMESPACE nfs_options_caption
#include "CachedMenuCaptionGameHooks.inc"
#undef NFS_CACHED_MENU_CAPTION_NAMESPACE

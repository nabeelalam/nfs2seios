// SPDX-License-Identifier: MIT
#pragma once
#include "hd/OpponentSceneLiveGameHooks.inc"

namespace nfs_opponent_initial_full_live {

// Called from the original 640-pixel refresh copy, after the typed preview
// has been authenticated. The two registered side panes alternate in place.
inline bool captureRefreshSidebar(uint32_t expectedBitmap, uint64_t lifetime,
                                  nfs_background_scene_cpu::Plane& plane) noexcept {
    plane.valid = 0;
    try {
        nfs::GuestCopyTransaction transaction;
        const auto selection = transaction.load<int32_t>(dword_4D2314 + 2);
        const auto index = static_cast<uint32_t>(selection) >> 16;
        if (index > 1 || !expectedBitmap || !lifetime)
            return false;

        const auto pane = transaction.load<uint32_t>(
            index == 0 ? dword_4D231C : dword_4D2320);
        if (!pane || transaction.load<uint32_t>(pane + 0x20) != expectedBitmap)
            return false;

        nfs_background_scene_cpu::Target target{};
        nfs_logo_publisher::target(transaction, target, pane);
        nfs_soft::SurfaceIdentity identity{};
        if (target.owner != pane || target.w[8] != expectedBitmap ||
            !nfs_opponent_scene_cpu::captionIdentity(target, lifetime, identity))
            return false;

        constexpr size_t bytes = nfs_background_scene_cpu::Side * sizeof(uint16_t);
        const auto* bitmap = reinterpret_cast<const uint8_t*>(
            nfs_host_address(expectedBitmap, 16 + bytes));
        for (size_t pixel = 0; pixel < nfs_background_scene_cpu::Side; ++pixel)
            plane.pixels[pixel] = nfs_background_scene_cpu::le16(bitmap + 16 + pixel * 2);

        plane.target = target;
        plane.width = 343;
        plane.height = 480;
        plane.valid = 1;
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace nfs_opponent_initial_full_live
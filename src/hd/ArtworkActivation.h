// SPDX-License-Identifier: MIT
#pragma once
#include "ArtworkPack.h"
namespace nfs_hd {
// Call on startup before engine workers, renderer contexts or provider getters.
// The expected pin must come from trusted application configuration, not a pack.
bool activateArtworkPack(const std::string& root,const Digest& expected) noexcept;
// Build-time NFS_HD_ARTWORK_SHA256 enables the optional separate pack. Empty
// configuration keeps original artwork without reading or allocating a pack.
bool activateConfiguredArtwork(const std::string& root) noexcept;
bool activateConfiguredArtwork(const std::string& root,const std::string& bundledTemplate) noexcept;
}

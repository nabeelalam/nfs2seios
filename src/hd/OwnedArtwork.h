// SPDX-License-Identifier: MIT
#pragma once
#include "ArtworkPack.h"
namespace nfs_hd {
// Read-only reconstruction from trusted authored template + separately supplied owner.
// Returns no provider/feature activation; caller retains and installs authenticated pack.
std::shared_ptr<const ArtworkPack>loadBundledOwnedArtwork(const std::string&ownerRoot,const std::string&templatePath,const Digest&expected,std::string*error=nullptr)noexcept;
}

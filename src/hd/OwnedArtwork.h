// SPDX-License-Identifier: MIT
#pragma once
#include "ArtworkPack.h"
namespace nfs_hd {
// Hash the actual bounded owner archive through the same guarded path resolver.
// Output is assigned only on success; no configured/expected digest is returned.
bool readOwnerArchiveDigest(const std::string&ownerRoot,Digest&actual)noexcept;
// Read-only reconstruction from trusted authored template + separately supplied owner.
// Returns no provider/feature activation; caller retains and installs authenticated pack.
std::shared_ptr<const ArtworkPack>loadBundledOwnedArtwork(const std::string&ownerRoot,const std::string&templatePath,const Digest&expected,std::string*error=nullptr)noexcept;
}

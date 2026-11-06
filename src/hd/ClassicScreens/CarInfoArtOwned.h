// SPDX-License-Identifier: MIT
#pragma once
#include "CarInfoArtProvider.h"
#include <vector>
namespace nfs_car_info {
// Internal startup transfer. Metadata pixels are deliberately unused: offsets
// are checked against the candidate-owned allocation before any pixel access.
struct OwnedImage { NfsCarInfoArtImage image{};size_t offset=0; };
// Transfer exclusive vector ownership; caller must retain no mutable aliases.
int installOwned(std::vector<unsigned char> payload,
                 const std::vector<OwnedImage>&images) noexcept;
}

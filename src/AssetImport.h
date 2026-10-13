// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>
#include <string>
namespace nfs {
struct AssetImportResult {uint64_t files=0,bytes=0;};
// Caller holds the selected folder's access grant for this entire operation.
// Copies only GAMEDATA/FEDATA and pinned support files into a new owned root.
// Throws on failure; does not replace an existing destination.
AssetImportResult import_assets(const std::string &source,
    const std::string &support_directory,const std::string &destination);
}

// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>
#include <string>
namespace nfs {
struct AssetImportResult {uint64_t files=0,bytes=0;};
// Caller holds the selected folder's access grant for this entire operation.
// Copies only GAMEDATA/FEDATA and startup files into a new owned root.
// If both bundled startup files are absent, requires them in the selected root.
// Throws on failure; does not replace an existing destination.
AssetImportResult import_assets(const std::string &source,
    const std::string &support_directory,const std::string &destination);
}

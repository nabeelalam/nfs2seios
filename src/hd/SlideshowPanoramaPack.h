// SPDX-License-Identifier: MIT
#pragma once
#include "SlideshowPanoramaProvider.h"
#include <iosfwd>
#include <string>
namespace nfs_slideshow_panorama {
// Version 1, all integers little endian; no padding, paths or original pixels.
// Header (32 bytes): magic[8]="NFSSPAN\0", u32 version=1, count=1..98,
// recordBytes=216, reserved=0; u64 totalCompressedBytes.
// Directory: count records, each archive/entry/center/footer/side/composed SHA256
// (six raw 32-byte digests), u32 width,height,centerWidth,flags=1 (reviewed),
// u64 compressedBytes. Payload follows directory: each exact zlib side stream
// in record order. No gaps or trailing bytes. Side stream layout is Entry's.
// The reviewed flag is format metadata, not external approval or native authority.
// Metadata/compressed bytes only: no inflation until Registry::acquire.
// Failure preserves the registry and all existing leases. Streams are consumed.
bool loadPack(std::istream&,Registry&) noexcept;
// Unpinned path overload is for explicit development use, not external approval.
bool loadPack(const std::string& path,Registry&) noexcept;
// Release callers bind the entire exact reviewed file; zero/missing pins reject.
bool loadPack(const std::string& path,const Digest& expectedFileSha,
              uint64_t expectedFileBytes,Registry&) noexcept;
}

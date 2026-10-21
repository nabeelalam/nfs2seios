// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>
namespace nfs_logo_capture {
enum Error:uint32_t{Read=1,Changed=2,Order=4,Nested=8,Unwind=16,Capacity=32,Identity=64,Palette=128,Rows=256,Owner=512,Unsupported=1024,Overflow=2048,Incomplete=4096};
}

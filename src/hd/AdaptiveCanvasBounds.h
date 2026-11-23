// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
// Authored widescreen exports may round ideal height to the nearest whole pixel.
// Keep every pixel: admit at most half a vertical pixel, only at real-art sizes.
static int nfs_authored_wide_canvas(uint32_t width,uint32_t height){
 if(!width||!height||width>4096||height>4096)return 0;
 int64_t delta=(int64_t)width*9-(int64_t)height*16;
 if(!delta)return 1;
 return width>=640&&height>=360&&delta>=-8&&delta<=8;
}

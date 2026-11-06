// SPDX-License-Identifier: MIT
#pragma once
#include "SlideshowSession.h"
namespace nfs_slideshow {
// Original499C98 maps format126 to class2 and format120 to class3.
// The class2->3 entry in both original conversion tables is4A6A03:
// retain blue, double the masked red/green bits, discard the source alpha bit.
inline uint16_t original1555To565(uint16_t value)noexcept {
    return uint16_t(((value&0x7fe0u)<<1)|(value&0x001fu));
}
// Archive selection alone does not establish what the game actually drew.
// Compare the complete original background before collecting control text.
inline bool sourceSurfaceMatches(const ImageFrame&image,
                                 const std::vector<uint16_t>&surface) noexcept {
    if(image.originalWords.size()!=640u*480u || surface.size()!=image.originalWords.size())return false;
    if(image.sourceFormat==120)return surface==image.originalWords;
    if(image.sourceFormat!=126)return false;
    for(size_t k=0;k<surface.size();++k)
        if(surface[k]!=original1555To565(image.originalWords[k]))return false;
    return true;
}
}

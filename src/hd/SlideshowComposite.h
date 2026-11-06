// SPDX-License-Identifier: MIT
#pragma once
#include "SlideshowSession.h"
#include <memory>
namespace nfs_slideshow {
// Overlay must be independently captured from actual original drawing commands.
// A background/final-frame pixel difference is insufficient to establish alpha.
struct CompositeFrame {
    std::shared_ptr<const ImageFrame> image;
    uint64_t epoch=0,serial=0;
    uint32_t targetOwner=0,targetPixels=0,targetWidth=0,targetHeight=0;
    std::vector<uint16_t> original;
    std::vector<unsigned char> overlayRgba;
    bool overlayCommandsProven=false;
    bool ready() const noexcept {
        return image && epoch && image->epoch==epoch && serial &&
            targetOwner && targetPixels && targetWidth==640 && targetHeight==480 &&
            original.size()==640u*480u && overlayCommandsProven &&
            overlayRgba.size()==640u*480u*4u &&
            image->rgba.size()==size_t(image->width)*image->height*4u;
    }
};
} // namespace nfs_slideshow

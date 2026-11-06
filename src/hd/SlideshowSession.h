// SPDX-License-Identifier: MIT
#pragma once
#include "SlideshowRuntimeImage.h"
#include "SlideshowSourceIdentity.h"
#include "SlideshowFooterCatalog.h"
#include <memory>
namespace nfs_slideshow {
struct ImageFrame {
    uint64_t epoch;
    uint32_t width,height,sourceFormat;
    // Immutable original pixels bind the selected remaster to the displayed scene.
    std::vector<uint16_t> originalWords;
    std::array<unsigned char,32> sourceSha,entrySha,rgbaSha;
    std::vector<unsigned char> rgba;
    nfs_slideshow_footer::ImagePtr footer;
};
// Game-thread-owned session. Renderer borrows immutable frames, never guest memory.
// Call changed() at actual archive retirement/change, close() on slideshow exit.
class Session {
    uint64_t epoch_=1;
    std::shared_ptr<const ImageFrame> frame_;
public:
    void changed() noexcept {frame_.reset();if(++epoch_==0)epoch_=1;}
    void close() noexcept {changed();}
    uint64_t epoch() const noexcept {return epoch_;}
    std::shared_ptr<const ImageFrame> frame() const noexcept {return frame_;}
    bool select(const void* original,size_t bytes,const SourcePin* sources,size_t count,
                const RuntimeImagePin* images,size_t imageCount,const char* path,
                bool preview=false,std::shared_ptr<const nfs_slideshow_footer::Catalog> footers={}) noexcept {
        // Clear before validating: a refused new image must not display the old car.
        changed();
        try {
            const auto* source=identify(original,bytes,sources,count);
            if(!source || !images || !imageCount || imageCount>98)return false;
            const RuntimeImagePin* image=nullptr;
            for(size_t i=0;i<imageCount;++i) {
                if(images[i].sourceSha==source->archiveSha && images[i].entrySha==source->entrySha) {
                    if(image)return false;
                    image=&images[i];
                }
            }
            if(!image || (!preview&&!footers))return false;
            // Shipping artwork requires the separately reviewed deterministic
            // footer. Previewing an unfinished whole image is explicit only.
            auto candidate=std::make_shared<ImageFrame>();
            candidate->epoch=epoch_;candidate->width=image->pixels.width;
            candidate->height=image->pixels.height;candidate->sourceSha=source->archiveSha;
            candidate->entrySha=source->entrySha;candidate->rgbaSha=image->pixels.rgbaSha;
            if(footers){candidate->footer=footers->lookup(source->archiveSha,source->entrySha);if(!candidate->footer)return false;}
            candidate->sourceFormat=source->format;
            candidate->originalWords.resize(size_t(source->width)*source->height);
            const auto* sourcePixels=static_cast<const unsigned char*>(original)+source->entryOffset+16;
            for(size_t k=0;k<candidate->originalWords.size();++k)
                candidate->originalWords[k]=uint16_t(sourcePixels[k*2])|uint16_t(uint16_t(sourcePixels[k*2+1])<<8);
            if(!loadImage(path,*image,source->archiveSha,source->entrySha,candidate->rgba,preview))
                return false;
            frame_=std::move(candidate);
            return true;
        } catch (...) {return false;}
    }
};
} // namespace nfs_slideshow

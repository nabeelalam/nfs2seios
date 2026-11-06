// SPDX-License-Identifier: MIT
#pragma once
#include "TextureDigest.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

namespace nfs_slideshow_footer {
using Digest=std::array<unsigned char,32>;

// Owns one complete4:3 premultiplied RGBA8 canvas. Instances can only be
// published as const shared ownership; no renderer borrow owns caller memory.
class FooterImage final {
    const uint32_t width_,height_;
    const std::vector<unsigned char> rgba_;
    const Digest sha_;
    FooterImage(uint32_t width,uint32_t height,
                std::vector<unsigned char> rgba,const Digest&sha):
        width_(width),height_(height),rgba_(std::move(rgba)),sha_(sha){}
public:
    // Enum constants have no out-of-line storage/ODR requirement in C++14.
    enum : uint32_t {MaximumDimension=4096};
    enum : size_t {MaximumBytes=size_t(64)*1024*1024};
    FooterImage(const FooterImage&)=delete;
    FooterImage&operator=(const FooterImage&)=delete;
    static std::shared_ptr<const FooterImage> create(
        uint32_t width,uint32_t height,std::vector<unsigned char> rgba,
        const Digest&expectedSha) noexcept {
        try {
            if(!width||!height||width>MaximumDimension||height>MaximumDimension||
               uint64_t(width)*3!=uint64_t(height)*4)return {};
            const uint64_t bytes=uint64_t(width)*height*4;
            if(bytes>MaximumBytes||bytes!=rgba.size())return {};
            // Validate the owned candidate, not an independently mutable input
            // pointer. Transparent pixels must consequently have zero RGB.
            for(size_t k=0;k<rgba.size();k+=4)
                if(rgba[k]>rgba[k+3]||rgba[k+1]>rgba[k+3]||rgba[k+2]>rgba[k+3])return {};
            Digest actual{};
            if(!nfs_texture_sha256(rgba.data(),rgba.size(),actual.data())||actual!=expectedSha)return {};
            return std::shared_ptr<const FooterImage>(
                new FooterImage(width,height,std::move(rgba),actual));
        }catch(...){return {};}
    }
    uint32_t width()const noexcept{return width_;}
    uint32_t height()const noexcept{return height_;}
    const std::vector<unsigned char>&rgba()const noexcept{return rgba_;}
    const Digest&sha()const noexcept{return sha_;}
};
using ImagePtr=std::shared_ptr<const FooterImage>;
struct Binding {Digest archiveSha,entrySha;uint32_t footerIndex;};

// Immutable original-identity routing. No names, embedded original hashes,
// file loading or mutable global installation belong to this contract.
class Catalog final {
    const std::vector<ImagePtr> images_;
    const std::vector<Binding> bindings_;
    const size_t bytes_;
    Catalog(std::vector<ImagePtr>images,std::vector<Binding>bindings,size_t bytes):
        images_(std::move(images)),bindings_(std::move(bindings)),bytes_(bytes){}
    static bool nonzero(const Digest&sha)noexcept {
        for(unsigned char value:sha)if(value)return true;
        return false;
    }
public:
    // A slide may need its own faithful body-mark/footer variant. Bound count
    // by original slide cardinality while retaining the hard decoded byte cap.
    enum : size_t {MaximumImages=98,MaximumBindings=98,
                   MaximumBytes=size_t(256)*1024*1024};
    Catalog(const Catalog&)=delete;
    Catalog&operator=(const Catalog&)=delete;
    // A caller may impose a smaller startup memory budget, never raise this
    // contract's hard bound. Passing vectors copies handles/identities only;
    // the exact incoming immutable image owners and their pixels are retained.
    static std::shared_ptr<const Catalog> create(
        const std::vector<ImagePtr>&images,const std::vector<Binding>&bindings,
        size_t byteBudget=MaximumBytes) noexcept {
        try {
            if(images.empty()||images.size()>MaximumImages||bindings.empty()||
               bindings.size()>MaximumBindings||!byteBudget||byteBudget>MaximumBytes)return {};
            size_t bytes=0;
            for(size_t k=0;k<images.size();++k){
                const auto&image=images[k];if(!image)return {};
                if(image->rgba().size()>byteBudget-bytes)return {};
                bytes+=image->rgba().size();
                // Duplicate slots must not consume the limited catalog or
                // disguise its footprint, even through a distinct shared owner.
                for(size_t j=0;j<k;++j)
                    if(image==images[j]||(image->sha()==images[j]->sha()&&
                       image->width()==images[j]->width()&&image->height()==images[j]->height()))return {};
            }
            for(size_t k=0;k<bindings.size();++k){
                const auto&binding=bindings[k];
                if(binding.footerIndex>=images.size()||!nonzero(binding.archiveSha)||
                   !nonzero(binding.entrySha))return {};
                for(size_t j=0;j<k;++j)
                    if(binding.archiveSha==bindings[j].archiveSha&&
                       binding.entrySha==bindings[j].entrySha)return {};
            }
            return std::shared_ptr<const Catalog>(new Catalog(images,bindings,bytes));
        }catch(...){return {};}
    }
    ImagePtr lookup(const Digest&archiveSha,const Digest&entrySha)const noexcept {
        for(const auto&binding:bindings_)
            if(binding.archiveSha==archiveSha&&binding.entrySha==entrySha)
                return images_[binding.footerIndex];
        return {};
    }
    size_t imageCount()const noexcept{return images_.size();}
    size_t bindingCount()const noexcept{return bindings_.size();}
    size_t bytes()const noexcept{return bytes_;}
};
} // namespace nfs_slideshow_footer

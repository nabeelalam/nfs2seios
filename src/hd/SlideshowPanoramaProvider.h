// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

namespace nfs_slideshow_panorama {
using Digest=std::array<unsigned char,32>;
struct Key {
    Digest archiveSha{},entrySha{},centerRgbaSha{},footerRgbaSha{};
    bool operator==(const Key&)const noexcept;
};
// CPU artwork contract only. This is NOT source/native-pass/GPU authority.
// The protected center has x=(width-centerWidth)/2, y=0, height=height;
// it is copied at 1:1 from the separately accepted center supplied to acquire.
// One exact zlib stream contains left strip row-major RGBA, then right strip
// row-major RGBA. No center pixels are stored in the stream. All pixels opaque.
struct Entry {
    Key key;
    uint32_t width=0,height=0,centerWidth=0;
    Digest sideSha{},composedSha{};
    bool reviewed=false;
    std::vector<unsigned char> compressedSides;
};
struct Lifetime {std::atomic<bool>alive{true};};
class Registry;
class Panorama final {
    friend class Registry;
    Key key_;
    uint32_t width_=0,height_=0,centerWidth_=0;
    uint64_t generation_=0;
    Digest sha_{};
    std::shared_ptr<const Lifetime>life_;
    std::vector<unsigned char>rgba_;
    Panorama()=default;
public:
    Panorama(const Panorama&)=delete;
    Panorama&operator=(const Panorama&)=delete;
    bool current()const noexcept{return life_&&life_->alive.load(std::memory_order_acquire);}
    const Key&key()const noexcept{return key_;}
    uint64_t generation()const noexcept{return generation_;}
    uint32_t width()const noexcept{return width_;}
    uint32_t height()const noexcept{return height_;}
    uint32_t centerWidth()const noexcept{return centerWidth_;}
    uint32_t centerLeft()const noexcept{return (width_-centerWidth_)/2;}
    const Digest&sha()const noexcept{return sha_;}
    const std::vector<unsigned char>&rgba()const noexcept{return rgba_;}
};
class Registry final {
    struct Catalog;
    mutable std::mutex mutex_;
    std::shared_ptr<const Catalog>catalog_;
    uint64_t nextGeneration_=1;
public:
    enum : size_t {MaximumEntries=98,MaximumDimension=4096,
        MaximumImageBytes=64u*1024u*1024u,
        MaximumCompressedEntryBytes=32u*1024u*1024u,
        MaximumCompressedBytes=256u*1024u*1024u};
    Registry()=default;
    ~Registry();
    Registry(const Registry&)=delete;
    Registry&operator=(const Registry&)=delete;
    // Owns compressed entries only. Invalid replacement preserves active art.
    // Successful replacement/clear revokes old leases; generation never wraps.
    bool install(std::vector<Entry> entries)noexcept;
    void clear()noexcept;
    // Caller must separately authenticate the key/footer and accepted center.
    // Dimensions and SHA are checked here; no guest or file reads occur.
    std::shared_ptr<const Panorama>acquire(const Key&,uint32_t centerWidth,
        uint32_t centerHeight,const std::vector<unsigned char>&center)const noexcept;
};
}

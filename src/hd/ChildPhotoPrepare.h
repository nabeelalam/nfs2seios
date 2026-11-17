// SPDX-License-Identifier: MIT
#pragma once
#include "PlayerPhotoArtProvider.h"
#include "ClassPictureArtProvider.h"
#include "LocationPhotoArtProvider.h"
#include <memory>
namespace nfs_child_photo {
struct Image{const unsigned char*pixels=nullptr;uint32_t width=0,height=0;};
struct Art{NfsPlayerPhotoArtView photo{};NfsClassPictureArtView klass{};bool isClass=false;uint64_t generation=0;Image image{};~Art(){if(photo.owner&&photo.release)photo.release(photo.owner);if(klass.owner&&klass.release)klass.release(klass.owner);}
 bool current()const noexcept{return generation&&generation==(isClass?nfs_class_picture_art_generation():nfs_player_photo_art_generation());}
 // The providers validate digest and premultiplied alpha before publishing
 // immutable pixels. Their retained owner keeps those same bytes alive.
 static std::shared_ptr<const Art> acquire(unsigned index) {
    if (index >= 15) return {};
    auto art = std::make_shared<Art>();
    art->isClass = index >= 12;
    size_t bytes = 0;
    if (art->isClass) {
        auto &view = art->klass;
        if (!nfs_class_picture_art_acquire(index - 12, &view) || view.schema != 1 ||
            view.count != 1 || !view.owner || !view.retain || !view.release) return {};
        const auto &image = view.images[0];
        if (image.sourceIndex != index - 12 || image.format != 1) return {};
        art->image = {image.pixels, image.width, image.height};
        art->generation = view.generation;
        bytes = image.bytes;
    } else {
        auto &view = art->photo;
        if (!nfs_player_photo_art_acquire(index, &view) || view.schema != 1 ||
            view.count != 1 || !view.owner || !view.retain || !view.release) return {};
        const auto &image = view.images[0];
        if (image.sourceIndex != index || image.format != 1) return {};
        art->image = {image.pixels, image.width, image.height};
        art->generation = view.generation;
        bytes = image.bytes;
    }
    const auto &image = art->image;
    if (!art->current() || !image.pixels || image.width != (art->isClass ? 1224u : 1228u) ||
        image.height != 584 || bytes != size_t(image.width) * image.height * 4) return {};
    return art;
 }
};
struct LocationArt {
    NfsLocationPhotoArtView view{};
    Image image{};
    uint64_t generation = 0;

    ~LocationArt() { if (view.owner && view.release) view.release(view.owner); }
    bool current() const noexcept {
        return generation && generation == nfs_location_photo_art_generation();
    }
    static std::shared_ptr<const LocationArt> acquire(unsigned index) {
        if (index >= 8) return {};
        auto art = std::make_shared<LocationArt>();
        auto &view = art->view;
        if (!nfs_location_photo_art_acquire(index, &view) || view.schema != 1 ||
            view.count != 1 || !view.owner || !view.retain || !view.release) return {};
        const auto &image = view.images[0];
        const unsigned height = index == 2 ? 588 : 584;
        art->generation = view.generation;
        if (!art->current() || image.sourceIndex != index || image.format != 1 ||
            !image.pixels || image.width != 1228 || image.height != height ||
            image.bytes != size_t(image.width) * height * 4) return {};
        art->image = {image.pixels, image.width, image.height};
        return art;
    }
};

}

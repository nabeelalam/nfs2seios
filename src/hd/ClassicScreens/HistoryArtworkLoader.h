// SPDX-License-Identifier: MIT
#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
int nfs_history_art_load_file(const char*path,char*error,size_t capacity);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <memory>
#include <array>
#include <stdint.h>
#include <string>
namespace nfs_history_wide {class Artwork;}
namespace nfs_history_art_scope {
bool configurePanorama(uint32_t source,std::string path,const std::array<unsigned char,32>&pin) noexcept;
std::shared_ptr<const nfs_history_wide::Artwork> panorama(uint32_t source,uint64_t base) noexcept;
void configure(std::string path);
std::shared_ptr<void> acquire() noexcept;
}
#endif

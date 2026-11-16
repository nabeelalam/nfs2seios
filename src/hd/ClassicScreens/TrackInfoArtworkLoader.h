// SPDX-License-Identifier: MIT
#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
int nfs_track_info_art_load_file(const char*path,char*error,size_t capacity);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <memory>
#include <string>
namespace nfs_track_info_art_scope {
void configure(std::string path);
std::shared_ptr<void> acquire() noexcept;
}
#endif

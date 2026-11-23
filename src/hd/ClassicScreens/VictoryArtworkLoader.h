// SPDX-License-Identifier: MIT
#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
// Register production metadata only. Pixel decode remains selected-image lazy.
int nfs_victory_art_load_bundled_file(const char*path,char*error,size_t capacity);
#ifdef __cplusplus
}
#endif

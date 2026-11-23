// SPDX-License-Identifier: MIT
#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
// Optional pinned eleven-layer pack; diagnostics never include local paths.
int nfs_credits_art_load_file(const char*,char*error,size_t capacity);
#ifdef __cplusplus
}
#endif

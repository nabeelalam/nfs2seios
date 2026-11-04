// SPDX-License-Identifier: MIT
#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Pure C++/zlib startup loader. Returns1 only after immutable installation.
   Missing/unconfigured/invalid optional data leaves the original intro intact.
   Diagnostics contain no file paths. Install the primary HdArtProvider first. */
int nfs_intro_art_load_file(const char*path,char*error,size_t capacity);
#ifdef __cplusplus
}
#endif

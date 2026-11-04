// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
typedef struct NfsIntroLiveView {uint32_t schema,width,height,locale;uint64_t request,content,artGeneration;const uint16_t*original;size_t pixels;void*owner;void(*retain)(void*);void(*release)(void*);}NfsIntroLiveView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_intro_begin(const NfsIntroLiveView*);
int nfs_private_intro_finish(uint64_t);
int nfs_private_intro_cancel(uint64_t);
#ifdef __cplusplus
}
#endif

// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "CreditsWholeRuntimeAbi.h"
typedef struct NfsCreditsCacheView {
 uint32_t schema,slot,locale;uint64_t content;uintptr_t sourceDomain;
 NfsCreditsWholeRect sprite[2];const uint16_t*original565;size_t pixels;uint32_t addresses[300];
 void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(const void*);
}NfsCreditsCacheView;
typedef struct NfsCreditsCacheDrawView {NfsCreditsCacheView cache;uint64_t request;int32_t x,y,step;float clip[4];}NfsCreditsCacheDrawView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_credits_cache_begin(const NfsCreditsCacheDrawView*);
int nfs_private_credits_cache_finish(uint64_t request);
int nfs_private_credits_cache_cancel(uint64_t request);
int nfs_private_credits_whole_submit(const NfsCreditsWholeView*);
int nfs_private_credits_cache_reset(void);
#ifdef __cplusplus
}
#endif

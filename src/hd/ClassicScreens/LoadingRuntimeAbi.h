// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
typedef struct NfsLoadingWordView {uint32_t index;int32_t left,top;uint32_t width,height;}NfsLoadingWordView;
typedef struct NfsLoadingLiveView {uint32_t schema,width,height;uint64_t request,content;const uint16_t*original,*clean;size_t pixels;NfsLoadingWordView word;void*owner;void(*retain)(void*);void(*release)(void*);}NfsLoadingLiveView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_loading_begin(const NfsLoadingLiveView*);
int nfs_private_loading_finish(uint64_t);
int nfs_private_loading_cancel(uint64_t);
#ifdef __cplusplus
}
#endif

// PRIVATE experiment ABI. Immutable borrows plus paired lifetime callbacks.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "hd/SoftwareTextLayerAbi.h"
typedef struct NfsVictoryWordView {uint32_t index;int32_t left,top;uint32_t width,height;} NfsVictoryWordView;
typedef struct NfsVictoryLiveView {
 uint32_t schema,width,height;uint64_t request,content;
 const uint16_t*original;const uint16_t*clean;size_t pixels;
 NfsSoftwareTextLayerView text;
 const NfsVictoryWordView*words;size_t wordCount;
 void*owner;void(*retain)(void*);void(*release)(void*);
} NfsVictoryLiveView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_victory_begin(const NfsVictoryLiveView*);
int nfs_private_victory_finish(uint64_t);
int nfs_private_victory_cancel(uint64_t);
#ifdef __cplusplus
}
#endif

// PRIVATE experiment ABI. Immutable borrows plus paired lifetime callbacks.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "hd/SoftwareTextLayerAbi.h"
typedef struct NfsResultsLiveView {
 uint32_t schema,width,height,headingIndex;uint64_t request,content;
 const uint16_t*original;const uint16_t*clean;size_t pixels;
 NfsSoftwareTextLayerView text;
 void*owner;void(*retain)(void*);void(*release)(void*);
} NfsResultsLiveView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_results_begin(const NfsResultsLiveView*);
int nfs_private_results_finish(uint64_t);
int nfs_private_results_cancel(uint64_t);
#ifdef __cplusplus
}
#endif

// Dedicated history ABI. Immutable borrows plus paired lifetime callbacks.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "hd/SoftwareTextLayerAbi.h"
#include "HistoryArtProvider.h"
typedef struct NfsHistoryLiveView {
 uint32_t schema,width,height,formType,archiveIndex,languageRole;uint64_t request,content;
 const uint16_t*original;const uint16_t*clean;size_t pixels;
 NfsSoftwareTextLayerView text;NfsHistoryArtView art;
 void*owner;void(*retain)(void*);void(*release)(void*);
} NfsHistoryLiveView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_history_begin(const NfsHistoryLiveView*);
int nfs_private_history_finish(uint64_t);
int nfs_private_history_cancel(uint64_t);
#ifdef __cplusplus
}
#endif

// PRIVATE experiment ABI. Immutable borrows plus paired lifetime callbacks.
#pragma once
#include <stdint.h>
#include "RecordsBackdropAbi.h"
#include <stddef.h>
#include "hd/SoftwareTextLayerAbi.h"
typedef struct NfsRecordsLiveView {
 uint32_t schema,width,height;uint64_t request,content;
 const uint16_t*original;const uint16_t*clean;size_t pixels;
 NfsSoftwareTextLayerView text;
 void*owner;void(*retain)(void*);void(*release)(void*);
 NfsRecordsBackdropView backdrop;
} NfsRecordsLiveView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_records_begin(const NfsRecordsLiveView*);
int nfs_private_records_stage(uint64_t);
int nfs_private_records_commit(uint64_t);
int nfs_private_records_finish(uint64_t);
int nfs_private_records_cancel(uint64_t);
#ifdef __cplusplus
}
#endif

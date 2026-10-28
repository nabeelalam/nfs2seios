// PRIVATE Personal Statistics display ABI. Immutable source-owned borrows.
#pragma once
#include <stdint.h>
#include "RecordsBackdropAbi.h"
#include <stddef.h>
#include "hd/SoftwareTextLayerAbi.h"
typedef struct NfsStatsGraphVertex {float vertex[4],shape[4],color[4],clip[4];} NfsStatsGraphVertex;
typedef struct NfsStatsDrawBatch {uint32_t kind;size_t first,count;} NfsStatsDrawBatch;
typedef struct NfsStatsUiView {
 uint32_t schema,width,height;uint64_t request,content;
 const uint16_t*original;const uint16_t*clean;size_t pixels;
 const NfsStatsGraphVertex*graph;size_t graphVertices;
 NfsSoftwareTextLayerView text;
 const NfsStatsDrawBatch*batches;size_t batchCount;
 void*owner;void(*retain)(void*);void(*release)(void*);
 NfsRecordsBackdropView backdrop;
} NfsStatsUiView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_stats_ui_begin(const NfsStatsUiView*);
int nfs_private_stats_ui_stage(uint64_t request);
int nfs_private_stats_ui_commit(uint64_t request);
int nfs_private_stats_ui_finish(uint64_t request);
int nfs_private_stats_ui_cancel(uint64_t request);
#ifdef __cplusplus
}
#endif

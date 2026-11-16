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
 uintptr_t sourceGame;uint64_t sourceEpoch;int(*current)(void*);
} NfsRecordsLiveView;
/* Actual lexical Records4267C0 postcommit cursor, independent source request. */
typedef struct NfsRecordsCursorView {uint64_t request;NfsRecordsLiveView source;uint32_t descriptor,words[7];int32_t x,y,edge;}NfsRecordsCursorView;
typedef struct NfsRecordsCursorQuad {uint64_t request;uint32_t descriptor,words[7],tmu,address,mask,indices[4],sourceVertices[4][8],nativeVertices[4][15];}NfsRecordsCursorQuad;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_records_cursor_begin(const NfsRecordsCursorView*);
int nfs_private_records_cursor_quad(const NfsRecordsCursorQuad*);
int nfs_private_records_cursor_end(uint64_t,int sourceValid);
int nfs_private_records_begin(const NfsRecordsLiveView*);
int nfs_private_records_stage(uint64_t);
int nfs_private_records_commit(uint64_t);
int nfs_private_records_finish(uint64_t);
int nfs_private_records_cancel(uint64_t);
#ifdef __cplusplus
}
#endif

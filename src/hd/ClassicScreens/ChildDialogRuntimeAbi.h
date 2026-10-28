#pragma once
#include "hd/SoftwareTextLayerAbi.h"
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct NfsLoadSaveChildShape {float bounds[4],clip[4];uint32_t role;} NfsLoadSaveChildShape;
typedef struct NfsLoadSaveChildView {uint32_t schema;uint64_t request,content;const unsigned char*rgba;size_t bytes;NfsSoftwareTextLayerView text;const NfsLoadSaveChildShape*shapes;size_t shapeCount;void*owner;void(*retain)(void*);void(*release)(void*);} NfsLoadSaveChildView;
int nfs_private_loadsave_child_begin(uint64_t);
int nfs_private_loadsave_child_begin_seeded(uint64_t,uintptr_t,uint64_t);
int nfs_private_loadsave_child_finish(const NfsLoadSaveChildView*);
int nfs_private_loadsave_child_cancel(uint64_t);
int nfs_private_loadsave_child_end(uint64_t);
int nfs_private_loadsave_child_restore_parent(uint64_t,uintptr_t,uint64_t);
#ifdef __cplusplus
}
#endif

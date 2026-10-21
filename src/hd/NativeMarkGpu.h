// SPDX-License-Identifier: MIT
#ifndef NFS_NATIVE_MARK_GPU_H
#define NFS_NATIVE_MARK_GPU_H
#include <stdint.h>
#include "NativeMarkProvider.h"
/* Private-to-renderer static hooks. Scalars describe actual original operations;
   original batches flush before upload invalidation and source selection.
   No guest/source pointer survives uploaded(). Provider pixels are immutable. */
static void nfs_native_mark_gpu_created(uintptr_t context,uint32_t program);
static void nfs_native_mark_gpu_retired(int currentOwned);
static void nfs_native_mark_gpu_invalidate(uint32_t address,uint64_t bytes);
static void nfs_native_mark_gpu_uploaded(uint32_t address,uint32_t nativeId,
    uint32_t edge,uint32_t encoding,uint32_t hdTag,const void*prepared);
static void nfs_native_mark_gpu_selected(uint32_t address,uint32_t nativeId,
    uint32_t edge,uint32_t encoding,uint32_t hdTag);
static void nfs_native_mark_gpu_sync(int triangles,float textured,int hdSelected,
    uintptr_t context,uint32_t program,int hdLocation,int*hdUniformCache,int targetHeight);
static void nfs_native_mark_gpu_after_draw(void);
#endif

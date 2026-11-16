// SPDX-License-Identifier: MIT
#pragma once
#include "ChildPhotoAnimationAbi.h"
typedef struct NfsChildPhotoDomain{uint64_t serial,sourceLife,epoch;uintptr_t game,sourceCatalog;uint32_t family,form,archive;}NfsChildPhotoDomain;
static void nfs_cp_created(void);
static void nfs_cp_retired(int);
static void nfs_cp_cancel(void);
static void nfs_cp_select(unsigned,uint32_t,uint32_t,const GrTexInfo*,TextureInfo*);
static void nfs_cp_triangle(const GrVertex*,const GrVertex*,const GrVertex*);
/* Borrowed only during the original batch callback; callers must retain their
 * own typed source owner and consume the returned texture synchronously. */
static int nfs_child_photo_gpu_batch(const NfsChildPhotoDomain*,int,int,unsigned,unsigned,uint32_t*);
static int nfs_child_photo_gpu_finished(const NfsChildPhotoDomain*);

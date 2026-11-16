// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundSceneRuntimeAbi.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct NfsMainUploadedDomain {uintptr_t game,catalog;uint64_t life,ownerEpoch,uploadEvent;uint32_t archive,form;}NfsMainUploadedDomain;
typedef struct NfsMainUploadedSeal {uint64_t token,context;}NfsMainUploadedSeal;
typedef struct NfsMainUploadedView {uint32_t schema;NfsMainUploadedDomain domain;NfsBackgroundSceneFrameView frame;void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(void*);void(*status)(void*,unsigned);}NfsMainUploadedView;
int nfs_private_main_uploaded_begin(const NfsMainUploadedView*,const NfsMainUploadedSeal*);
int nfs_private_main_uploaded_finish(uint64_t,NfsMainUploadedSeal*);
#ifdef __cplusplus
}
#endif

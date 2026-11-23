// SPDX-License-Identifier: MIT
#pragma once
#include "MainUploadedSourceAbi.h"
#include "OptionsSceneRuntimeAbi.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct NfsOptionsUploadedView {
 uint32_t schema;NfsMainUploadedDomain domain;NfsOptionsSceneFrameView frame;
 void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(void*);void(*status)(void*,unsigned);
}NfsOptionsUploadedView;
int nfs_private_options_uploaded_begin(const NfsOptionsUploadedView*,const NfsMainUploadedSeal*);
int nfs_private_options_uploaded_finish(uint64_t,NfsMainUploadedSeal*);
#ifdef __cplusplus
}
#endif

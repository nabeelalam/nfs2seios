// SPDX-License-Identifier: MIT
#pragma once
#include "MainUploadedSourceAbi.h"
#include "SetupSceneRuntimeAbi.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct NfsSetupUploadedView {
 uint32_t schema;NfsMainUploadedDomain domain;NfsSetupSceneFrameView frame;
 void*owner;void(*retain)(void*);void(*release)(void*);int(*current)(void*);void(*status)(void*,unsigned);
}NfsSetupUploadedView;
int nfs_private_setup_uploaded_begin(const NfsSetupUploadedView*,const NfsMainUploadedSeal*);
int nfs_private_setup_uploaded_finish(uint64_t,NfsMainUploadedSeal*);
#ifdef __cplusplus
}
#endif

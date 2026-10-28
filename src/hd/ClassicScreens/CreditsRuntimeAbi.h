// PRIVATE Credits hardware-text pass. Original native draws always continue.
#pragma once
#include <stdint.h>
#include "hd/SoftwareTextLayerAbi.h"
typedef struct NfsCreditsUiView {
 uint32_t schema;uint64_t request,content;
 NfsSoftwareTextLayerView text;
 void*owner;void(*retain)(void*);void(*release)(void*);
} NfsCreditsUiView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_credits_ui_begin(uint64_t request);
int nfs_private_credits_ui_finish(const NfsCreditsUiView*);
int nfs_private_credits_ui_cancel(uint64_t request);
#ifdef __cplusplus
}
#endif

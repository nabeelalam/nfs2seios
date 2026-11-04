// Dedicated car-information ABI. Immutable borrows plus paired lifetime callbacks.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include "hd/SoftwareTextLayerAbi.h"
#include "CarInfoArtProvider.h"
typedef struct NfsCarInfoLiveView {
 uint32_t schema,width,height,formType,carIndex,page;uint64_t request,content;
 const uint16_t*original;const uint16_t*clean;size_t pixels;
 NfsSoftwareTextLayerView text;NfsCarInfoArtView art;
 void*owner;void(*retain)(void*);void(*release)(void*);
} NfsCarInfoLiveView;
#ifdef __cplusplus
extern "C" {
#endif
int nfs_private_car_info_begin(const NfsCarInfoLiveView*);
int nfs_private_car_info_finish(uint64_t);
int nfs_private_car_info_cancel(uint64_t);
#ifdef __cplusplus
}
#endif

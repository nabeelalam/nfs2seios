// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Atomic read-only Main source snapshot. Slot5 is authority ONLY together
   with its exact epoch, native selection/profile and converted source digest. */
typedef struct NfsMenuResponsiveDebugState {
 uint32_t schema,sourceForm,sourceX,sourceY,cursorValid,shownActive,originalOpponentAddress,originalOpponentValid;
 uint64_t sourceEpoch,shownRevision,shownEpoch;
} NfsMenuResponsiveDebugState;
void nfs_menu_responsive_debug_state(NfsMenuResponsiveDebugState*);
/* One atomic production observation; guest copy must finish before this call. */
void nfs_menu_responsive_source_observation(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
#ifdef __cplusplus
}
#endif

// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Main-only, successfully PRESENTED layout. Never a pending render request. */
typedef struct NfsMenuResponsiveLayout {
 uint32_t schema,active; uint64_t revision,ownerEpoch;
 uint32_t drawableWidth,drawableHeight; float logicalWidth,delta;
 float sourceCursorX,sourceCursorY; uint32_t cursorValid;
} NfsMenuResponsiveLayout;
typedef struct NfsMenuResponsivePointer {
 uint64_t revision,ownerEpoch; float x,y; uint32_t active;
} NfsMenuResponsivePointer;
/* Exact original form/mouse observation, supplied by the owned source hook. */
void nfs_menu_responsive_source_form(uint32_t form,uint32_t cursorX,uint32_t cursorY);
int nfs_menu_responsive_source_main(uint64_t* epoch,float* x,float* y);
void nfs_menu_responsive_publish(const NfsMenuResponsiveLayout*);
void nfs_menu_responsive_revoke(void);
int nfs_menu_responsive_snapshot(NfsMenuResponsiveLayout*);
uint64_t nfs_menu_responsive_input_generation(void);
float nfs_menu_responsive_logical_width(void);
/* Routes the motion and click against ONE displayed revision under the lock. */
void nfs_menu_responsive_pointer_route(int32_t,int32_t,int,int32_t*,int32_t*,int*);
int nfs_menu_responsive_pointer_snapshot(NfsMenuResponsivePointer*);
#ifdef __cplusplus
}
#endif

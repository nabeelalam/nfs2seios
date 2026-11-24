// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
void *nfs_host_address(uint32_t address, size_t length);
uint32_t nfs_guest_address(const void *pointer);
#ifdef __cplusplus
}
#endif

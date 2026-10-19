// SPDX-License-Identifier: MIT
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Renderer-owned capability only. Publish false before retiring its resources.
// The engine must retain original hit areas until matching artwork is visible.
void nfs_front_word_set_available(int available);
int nfs_front_word_hit_active(void);

#ifdef __cplusplus
}
#endif

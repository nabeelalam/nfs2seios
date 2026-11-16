// SPDX-License-Identifier: MIT
#include "hd/Main343OriginLifetime.h"
#include "hd/MenuTransitionAbi.h"

extern "C" int nfs_menu_transition_source_life_current(uintptr_t game, uint64_t serial) {
    return nfs_main_sidebar_origin::currentGameLife(game, serial) ? 1 : 0;
}

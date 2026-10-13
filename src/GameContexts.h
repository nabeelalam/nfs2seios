// SPDX-License-Identifier: MIT
#pragma once
#include <stdint.h>
class Game;
extern "C" Game *nfs_create_game_context();
extern "C" void *nfs_resolve_game_context(uint32_t);
extern "C" void nfs_destroy_game_context(void *);
extern "C" void nfs_prepare_game_worker(void *,uint32_t);
extern "C" void nfs_run_game_worker(void *);
extern "C" int32_t nfs_dispatch_game_window(void *,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
extern "C" void nfs_dispatch_game_exit(void *,uint32_t);
extern "C" void nfs_dispatch_game_procedure(void *,uint32_t);
extern "C" void nfs_dispatch_game_regparm2(void *,uint32_t,uint32_t,uint32_t);
extern "C" int32_t nfs_dispatch_game_input(void *,uint32_t,uint32_t,uint32_t);

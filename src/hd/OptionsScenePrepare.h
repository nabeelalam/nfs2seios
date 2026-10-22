// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundScenePrepare.h"
#include "OptionsSceneRuntimeAbi.h"
#include "SoftwareFramePacket.h"
// Fixed Options-only authority; no runtime-selected form policy.
#define NFS_CACHED_MENU_CPU_NAMESPACE nfs_options_scene_cpu
#define NFS_CACHED_MENU_FORM_TAG OptionsForm
#define NFS_CACHED_MENU_FORM_VALUE 0x4D3A88
#define NFS_CACHED_MENU_FRAME_VIEW NfsOptionsSceneFrameView
#define NFS_CACHED_MENU_RUNTIME_SCHEMA NFS_OPTIONS_SCENE_RUNTIME_SCHEMA
#define NFS_CACHED_MENU_TAIL NFS_BACKGROUND_TAIL_OPTIONS_CAPTION
#include "CachedMenuScenePrepare.inc"
#undef NFS_CACHED_MENU_TAIL
#undef NFS_CACHED_MENU_RUNTIME_SCHEMA
#undef NFS_CACHED_MENU_FRAME_VIEW
#undef NFS_CACHED_MENU_FORM_VALUE
#undef NFS_CACHED_MENU_FORM_TAG
#undef NFS_CACHED_MENU_CPU_NAMESPACE

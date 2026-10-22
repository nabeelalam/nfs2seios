// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundScenePrepare.h"
#include "SetupSceneRuntimeAbi.h"
#include "SoftwareFramePacket.h"
// Fixed Setup-only authority; no runtime-selected form policy.
#define NFS_CACHED_MENU_CPU_NAMESPACE nfs_setup_scene_cpu
#define NFS_CACHED_MENU_FORM_TAG SetupForm
#define NFS_CACHED_MENU_FORM_VALUE 0x4D2790
#define NFS_CACHED_MENU_FRAME_VIEW NfsSetupSceneFrameView
#define NFS_CACHED_MENU_RUNTIME_SCHEMA NFS_SETUP_SCENE_RUNTIME_SCHEMA
#define NFS_CACHED_MENU_TAIL NFS_BACKGROUND_TAIL_SETUP_CAPTION
#include "CachedMenuScenePrepare.inc"
#undef NFS_CACHED_MENU_TAIL
#undef NFS_CACHED_MENU_RUNTIME_SCHEMA
#undef NFS_CACHED_MENU_FRAME_VIEW
#undef NFS_CACHED_MENU_FORM_VALUE
#undef NFS_CACHED_MENU_FORM_TAG
#undef NFS_CACHED_MENU_CPU_NAMESPACE

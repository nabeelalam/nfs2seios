// SPDX-License-Identifier: MIT
#pragma once
#include "SetupScenePrepare.h"
#include "SetupCaptionGameHooks.inc"
#include "ContinuousNativeUploadHooks.inc"
#include <memory>
#include <atomic>
// Fixed Setup-only owner and caption; never expands another form owner.
#define NFS_CACHED_MENU_OWNER_SCOPE SetupMenuOwnerScope
#define NFS_CACHED_MENU_SCOPE SetupScope
#define NFS_CACHED_MENU_LIVE_NAMESPACE nfs_setup_scene_live
#define NFS_CACHED_MENU_CPU_NAMESPACE nfs_setup_scene_cpu
#define NFS_CACHED_MENU_CAPTION_NAMESPACE nfs_setup_caption
#define NFS_CACHED_MENU_DESCRIPTOR off_4D2790
#define NFS_CACHED_MENU_FORM_TAG SetupForm
#define NFS_CACHED_MENU_TAIL NFS_BACKGROUND_TAIL_SETUP_CAPTION
#include "CachedMenuSceneLiveGameHooks.inc"
#undef NFS_CACHED_MENU_TAIL
#undef NFS_CACHED_MENU_FORM_TAG
#undef NFS_CACHED_MENU_DESCRIPTOR
#undef NFS_CACHED_MENU_CAPTION_NAMESPACE
#undef NFS_CACHED_MENU_CPU_NAMESPACE
#undef NFS_CACHED_MENU_LIVE_NAMESPACE
#undef NFS_CACHED_MENU_SCOPE
#undef NFS_CACHED_MENU_OWNER_SCOPE

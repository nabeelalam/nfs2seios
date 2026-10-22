// SPDX-License-Identifier: MIT
#pragma once
#include "OptionsScenePrepare.h"
#include "OptionsCaptionGameHooks.inc"
#include "ContinuousNativeUploadHooks.inc"
#include <memory>
#include <atomic>
// Fixed Options-only owner and caption; never expands another form owner.
#define NFS_CACHED_MENU_OWNER_SCOPE OptionsMenuOwnerScope
#define NFS_CACHED_MENU_SCOPE OptionsScope
#define NFS_CACHED_MENU_LIVE_NAMESPACE nfs_options_scene_live
#define NFS_CACHED_MENU_CPU_NAMESPACE nfs_options_scene_cpu
#define NFS_CACHED_MENU_CAPTION_NAMESPACE nfs_options_caption
#define NFS_CACHED_MENU_DESCRIPTOR off_4D3A88
#define NFS_CACHED_MENU_FORM_TAG OptionsForm
#define NFS_CACHED_MENU_TAIL NFS_BACKGROUND_TAIL_OPTIONS_CAPTION
#include "CachedMenuSceneLiveGameHooks.inc"
#undef NFS_CACHED_MENU_TAIL
#undef NFS_CACHED_MENU_FORM_TAG
#undef NFS_CACHED_MENU_DESCRIPTOR
#undef NFS_CACHED_MENU_CAPTION_NAMESPACE
#undef NFS_CACHED_MENU_CPU_NAMESPACE
#undef NFS_CACHED_MENU_LIVE_NAMESPACE
#undef NFS_CACHED_MENU_SCOPE
#undef NFS_CACHED_MENU_OWNER_SCOPE

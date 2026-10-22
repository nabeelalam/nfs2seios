// SPDX-License-Identifier: MIT
#pragma once
#include "SmallLogoPrepare.h"
#include "OriginalPreviewLayerAbi.h"
#include <memory>
#include <vector>
// Pure replay verifier, not a source of guest-read or publication authority.
// The live caller must bind every record to one current original owner/archive
// and completed call sequence before passing coherent owned observations here.
// No recorder, guest-pointer dereference, fallback or filesystem loader exists.
#define NFS_PREVIEW_REPLAY_NAMESPACE nfs_location_preview
#define NFS_PREVIEW_REPLAY_MAX_DRAWS 3
#define NFS_PREVIEW_REPLAY_FORM_CONSTANTS constexpr uint32_t LocationForm=0x404abd;
#define NFS_PREVIEW_REPLAY_EXTRA_INPUT
#define NFS_PREVIEW_REPLAY_BINDING_DECLARATION
#define NFS_PREVIEW_REPLAY_BINDING(i) ((i).form==LocationForm)
#define NFS_PREVIEW_REPLAY_PACKET_FIELDS
#define NFS_PREVIEW_REPLAY_PACKET_ACCESSORS
#define NFS_PREVIEW_REPLAY_PACKET_CAPTURE
#include "PreviewReplay.inc"
#undef NFS_PREVIEW_REPLAY_PACKET_CAPTURE
#undef NFS_PREVIEW_REPLAY_PACKET_ACCESSORS
#undef NFS_PREVIEW_REPLAY_PACKET_FIELDS
#undef NFS_PREVIEW_REPLAY_BINDING
#undef NFS_PREVIEW_REPLAY_BINDING_DECLARATION
#undef NFS_PREVIEW_REPLAY_EXTRA_INPUT
#undef NFS_PREVIEW_REPLAY_FORM_CONSTANTS
#undef NFS_PREVIEW_REPLAY_MAX_DRAWS
#undef NFS_PREVIEW_REPLAY_NAMESPACE

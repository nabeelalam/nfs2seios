// SPDX-License-Identifier: MIT
#pragma once
#include "SmallLogoPrepare.h"
#include "OriginalPreviewLayerAbi.h"
#include "LocationCompletedFlag.h"
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
#define NFS_PREVIEW_REPLAY_PACKET_FIELDS std::vector<unsigned char>authored_;std::array<unsigned char,32>authoredSha_{};
#define NFS_PREVIEW_REPLAY_PACKET_ACCESSORS
#define NFS_PREVIEW_REPLAY_PACKET_CAPTURE
#define NFS_PREVIEW_REPLAY_DISPLAY_VIEW if(!authored_.empty()){v.schema=NFS_AUTHORED_PREVIEW_LAYER_SCHEMA;v.authoredRGBA=authored_.data();v.authoredBytes=authored_.size();v.authoredWidth=rect_.width*4;v.authoredHeight=rect_.height*4;std::copy(authoredSha_.begin(),authoredSha_.end(),v.authoredSha256);}
#define NFS_PREVIEW_REPLAY_DISPLAY_FINALIZE packet->authored_=nfs_location_completed_flag::prepare(i,*packet);if(!packet->authored_.empty()&&!nfs_texture_sha256(packet->authored_.data(),packet->authored_.size(),packet->authoredSha_.data()))return reject(Failure::Allocation);
#include "PreviewReplay.inc"
#undef NFS_PREVIEW_REPLAY_DISPLAY_VIEW
#undef NFS_PREVIEW_REPLAY_DISPLAY_FINALIZE
#undef NFS_PREVIEW_REPLAY_PACKET_CAPTURE
#undef NFS_PREVIEW_REPLAY_PACKET_ACCESSORS
#undef NFS_PREVIEW_REPLAY_PACKET_FIELDS
#undef NFS_PREVIEW_REPLAY_BINDING
#undef NFS_PREVIEW_REPLAY_BINDING_DECLARATION
#undef NFS_PREVIEW_REPLAY_EXTRA_INPUT
#undef NFS_PREVIEW_REPLAY_FORM_CONSTANTS
#undef NFS_PREVIEW_REPLAY_MAX_DRAWS
#undef NFS_PREVIEW_REPLAY_NAMESPACE

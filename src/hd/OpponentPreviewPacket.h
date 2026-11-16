// SPDX-License-Identifier: MIT
#pragma once
#include "SmallLogoPrepare.h"
#include "OpponentPreviewSourceCatalog.h"
#include "OriginalPreviewLayerAbi.h"
#include <memory>
#include <vector>
// Pure replay verifier, not a source of guest-read or publication authority.
// The live caller must bind every record to one current original owner/archive
// and completed call sequence before passing coherent owned observations here.
// No recorder, guest-pointer dereference, fallback or filesystem loader exists.
#define NFS_PREVIEW_REPLAY_NAMESPACE nfs_opponent_preview
#define NFS_PREVIEW_REPLAY_MAX_DRAWS 1
#define NFS_PREVIEW_REPLAY_FORM_CONSTANTS constexpr uint32_t OpponentForm=0x4d3204;
#define NFS_PREVIEW_REPLAY_EXTRA_INPUT unsigned selection=15;bool decodedAuthenticated=false;std::shared_ptr<const nfs_opponent_preview_source::Catalog> sourceCatalog;
#define NFS_PREVIEW_REPLAY_BINDING_DECLARATION
#define NFS_PREVIEW_REPLAY_BINDING(i) ((i).form==OpponentForm&&(i).selection<15&&(i).draws.size()==1&&!std::memcmp((i).draws[0].name.data(),nfs_opponent_preview_source::nameFor((i).selection),4)&&(i).draws[0].args[2]==211&&(i).draws[0].args[3]==316)
#define NFS_PREVIEW_REPLAY_PACKET_FIELDS uint32_t logicalLeft_=0,logicalTop_=0;unsigned selection_=15;bool sourceAuthenticated_=false;std::shared_ptr<const nfs_opponent_preview_source::Catalog> sourceCatalog_;
#define NFS_PREVIEW_REPLAY_PACKET_ACCESSORS uint32_t logicalLeft()const noexcept{return logicalLeft_;}uint32_t logicalTop()const noexcept{return logicalTop_;}unsigned selection()const noexcept{return selection_;}bool sourceAuthenticated()const noexcept{return sourceAuthenticated_;} static std::shared_ptr<const Packet>createEmpty(const Input&i,const Plane&base)noexcept{if(i.form!=OpponentForm||!i.lifetime||!i.serial||!i.archive||i.complete!=1||i.error||i.fillCalls||i.drawCalls||!i.draws.empty()||!planeValid(base)||!planeValid(i.pre)||!planeValid(i.post)||!nfs_logo_capture::same(base.target,i.pre.target)||!nfs_logo_capture::same(base.target,i.post.target)||base.pixels!=i.pre.pixels||base.pixels!=i.post.pixels)return {};try{std::shared_ptr<Packet>p(new Packet);p->target_=base.target;p->lifetime_=i.lifetime;p->serial_=i.serial;p->archive_=i.archive;p->rect_={0,0,1,1};p->post_.assign(base.pixels.begin(),base.pixels.end());p->pixels_={base.pixels[0]};p->coverage_={0};if(!wordsSha(p->pixels_.data(),1,p->sha_)||!wordsSha(base.pixels.data(),Full,p->baseSha_)||!nfs_texture_sha256(p->coverage_.data(),1,p->coverageSha_.data()))return {};return p;}catch(...){return {};}}
#define NFS_PREVIEW_REPLAY_PACKET_CAPTURE packet->logicalLeft_=i.draws[0].args[2];packet->logicalTop_=i.draws[0].args[3];
#define NFS_PREVIEW_REPLAY_DISPLAY_VIEW
#define NFS_PREVIEW_REPLAY_DISPLAY_FINALIZE packet->selection_=i.selection;packet->sourceCatalog_=i.sourceCatalog;if(i.decodedAuthenticated&&i.sourceCatalog){const auto&d=i.draws[0];const auto&m=packet->sources_[0];packet->sourceAuthenticated_=i.sourceCatalog->matches(i.selection,d.name,m.width,m.height,d.sourceBefore.data(),d.sourceBefore.size());}
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

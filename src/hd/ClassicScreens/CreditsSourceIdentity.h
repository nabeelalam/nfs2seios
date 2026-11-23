// SPDX-License-Identifier: MIT
// Owned original-data identity. No game memory or original pixels escape validation.
#pragma once
#include "CreditsSourceCatalog.h"
#include "TextureDigest.h"
#include "hd/Main343OriginLifetime.h"
#include <atomic>
#include <cstring>
#include <memory>
#include <vector>
namespace nfs_credits_ui {
class ArchiveIdentity final {
 ArchiveIdentity()=default;
 static uint32_t le32(const unsigned char*p)noexcept{return uint32_t(p[0])|uint32_t(p[1])<<8|uint32_t(p[2])<<16|uint32_t(p[3])<<24;}
public:
 // The caller owns this complete snapshot; hashing is outside the guest-copy lock.
 static std::shared_ptr<const ArchiveIdentity>create(const std::vector<unsigned char>&bytes)noexcept{
  if(bytes.size()!=NFS_CREDITS_ARCHIVE_BYTES)return {};
  const auto*p=bytes.data();
  if(le32(p)!=0x49504853u||le32(p+4)!=bytes.size()||le32(p+8)!=NFS_CREDITS_RESOURCE_COUNT)return {};
  for(unsigned i=0;i<NFS_CREDITS_RESOURCE_COUNT;++i){const auto&r=nfs_credits_sources[i];
   if(std::memcmp(p+16+8*i,r.name,4)||le32(p+20+8*i)!=r.offset||r.offset>bytes.size()||r.span>bytes.size()-r.offset)return {};
  }
  static const unsigned char expected[32]={0x54,0x2d,0x10,0x24,0x52,0xb4,0x53,0x74,0x99,0xd7,0x1b,0x6a,0x3d,0x11,0x9c,0xd3,0x04,0xe7,0xe5,0x54,0x63,0x72,0x70,0x07,0x1c,0x07,0x8f,0xae,0xff,0x16,0x08,0x71};
  unsigned char sha[32];if(!nfs_texture_sha256(p,bytes.size(),sha)||std::memcmp(sha,expected,32))return {};
  try{return std::shared_ptr<const ArchiveIdentity>(new ArchiveIdentity);}catch(...){return {};}
 }
};
// Only a verified archive can acquire authority. Retained renderer packets read
// atomic lifetimes, never mutable game state or a process-wide guessed flag.
class SourceLease final {
 std::atomic<bool>alive_{true};
 std::shared_ptr<const ArchiveIdentity>identity_;
 std::shared_ptr<const nfs_main_sidebar_origin::GameLife>gameLife_;
 uintptr_t game_=0;uint32_t archive_=0;
 SourceLease()=default;
public:
 static std::shared_ptr<SourceLease>create(uintptr_t game,uint32_t archive,std::shared_ptr<const ArchiveIdentity>identity)noexcept{
  if(!game||!archive||uint64_t(archive)+NFS_CREDITS_ARCHIVE_BYTES>UINT64_C(0x100000000)||!identity)return {};
  auto life=nfs_main_sidebar_origin::acquireGameLife(game);if(!life||!life->current())return {};
  try{auto p=std::shared_ptr<SourceLease>(new SourceLease);p->game_=game;p->archive_=archive;p->identity_=std::move(identity);p->gameLife_=std::move(life);return p;}catch(...){return {};}
 }
 bool current(uintptr_t game,uint32_t archive)const noexcept{return game==game_&&archive==archive_&&alive_.load(std::memory_order_acquire)&&gameLife_&&gameLife_->current();}
 void retire()noexcept{alive_.store(false,std::memory_order_release);}
};
} // namespace nfs_credits_ui

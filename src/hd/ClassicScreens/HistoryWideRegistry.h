// SPDX-License-Identifier: MIT
#pragma once
#include "HistoryWideArtwork.h"
#include <array>
#include <mutex>
namespace nfs_history_wide {
class Registry final {
 std::mutex mutex_;std::array<std::shared_ptr<const Artwork>,NFS_HISTORY_ARCHIVE_COUNT>art_{};
 static bool slot(uint32_t source,unsigned&out)noexcept{if(source>=NFS_HISTORY_SOURCE_COUNT||nfs_history_sources[source].role)return false;out=nfs_history_sources[source].archive;return out<NFS_HISTORY_ARCHIVE_COUNT;}
public:
 bool install(std::shared_ptr<const Artwork>art,uint64_t liveBase,uint32_t source,uint64_t(*generation)()=nullptr){
  unsigned at=0;Artwork::View v{};if(!art||!slot(source,at)||!art->view(liveBase,source,v))return false;
  std::lock_guard<std::mutex>lock(mutex_);if(generation&&generation()!=liveBase)return false;art_[at]=std::move(art);return true;
 }
 std::shared_ptr<const Artwork>acquire(uint64_t liveBase,uint32_t source){
  unsigned at=0;if(!slot(source,at))return {};std::lock_guard<std::mutex>lock(mutex_);
  auto art=art_[at];Artwork::View v{};return art&&art->view(liveBase,source,v)?art:std::shared_ptr<const Artwork>{};
 }
 bool current(const std::shared_ptr<const Artwork>&art,uint64_t liveBase,uint32_t source){
  if(!art)return false;auto now=acquire(liveBase,source);return now&&now==art;
 }
 void clear(){std::lock_guard<std::mutex>lock(mutex_);for(auto&art:art_)art.reset();}
};
inline Registry&registry(){static Registry instance;return instance;}
}

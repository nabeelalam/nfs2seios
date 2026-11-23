// SPDX-License-Identifier: MIT
#pragma once
#include "HistoryWidePack.h"
#include "HistoryWideRegistry.h"
#include <atomic>
#include <fstream>
#include <string>
namespace nfs_history_wide {
class Files final {
 using Generation=uint64_t(*)();
 struct Entry {std::string path;std::array<unsigned char,32>pin{};uint64_t attempted=0;};
 Generation generation_;Registry&registry_;std::mutex mutex_;std::array<Entry,NFS_HISTORY_ARCHIVE_COUNT>entries_{};std::atomic<unsigned>configured_{0};
 static bool slot(uint32_t source,unsigned&out)noexcept{if(source>=NFS_HISTORY_SOURCE_COUNT||nfs_history_sources[source].role)return false;out=nfs_history_sources[source].archive;return out<NFS_HISTORY_ARCHIVE_COUNT;}
public:
 explicit Files(Generation generation,Registry&registry):generation_(generation),registry_(registry){}
 // Startup configuration only. No file paths are emitted in diagnostics.
 bool configure(uint32_t source,std::string path,const std::array<unsigned char,32>&pin)noexcept{
  try{unsigned at=0,nonzero=0;for(auto c:pin)nonzero|=c;
   if(!generation_||generation_()||!slot(source,at)||path.empty()||!nonzero)return false;
   std::lock_guard<std::mutex>lock(mutex_);if(generation_()||!entries_[at].path.empty())return false;
   entries_[at].path=std::move(path);entries_[at].pin=pin;configured_.fetch_or(1u<<at,std::memory_order_release);return true;
  }catch(...){return false;}
 }
 std::shared_ptr<const Artwork>acquire(uint32_t source,uint64_t base)noexcept{
  try{unsigned at=0;if(!slot(source,at)||!(configured_.load(std::memory_order_acquire)&(1u<<at))||!base||!generation_||generation_()!=base)return {};
   auto cached=registry_.acquire(base,source);if(cached)return cached;
   Entry input;{std::lock_guard<std::mutex>lock(mutex_);auto&e=entries_[at];if(e.path.empty()||e.attempted==base)return {};e.attempted=base;input=e;}
   std::ifstream file(input.path,std::ios::binary);auto art=load(file,input.pin.data(),base);
   if(!art||!registry_.install(art,base,source,generation_))return {};
   return registry_.acquire(base,source);
  }catch(...){return {};}
 }
};
}

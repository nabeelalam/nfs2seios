// SPDX-License-Identifier: MIT
#pragma once
#include "BackgroundScenePrepare.h"
#include <memory>
#include <mutex>
#include <atomic>
namespace nfs_background_scene_runtime {
// Installation validates structure, not provenance. Caller must supply a trusted
// immutable catalogue and relinquish all mutable aliases before workers start.
struct CatalogRegistry {
 std::mutex mutex;
 std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog> value;
 std::atomic<bool> configured{false};bool acquired=false;
};
inline CatalogRegistry& registry(){static CatalogRegistry value;return value;}
inline bool installCatalog(std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog> value)noexcept {
 if(!value||!nfs_background_scene_cpu::validCatalog(*value))return false;
 try{auto&r=registry();std::lock_guard<std::mutex>lock(r.mutex);if(r.acquired)return false;r.value=std::move(value);r.configured.store(true,std::memory_order_release);return true;}catch(...){return false;}
}
inline bool catalogConfigured()noexcept{return registry().configured.load(std::memory_order_acquire);}
inline std::shared_ptr<const nfs_background_scene_cpu::SourceCatalog> acquireCatalog()noexcept {
 try{auto&r=registry();std::lock_guard<std::mutex>lock(r.mutex);if(!r.value)return {};r.acquired=true;return r.value;}catch(...){return {};}
}
}

// SPDX-License-Identifier: MIT
#pragma once
#include "SoftwareFontCatalog.h"
#include <mutex>
namespace nfs_soft {
namespace font_provider_detail {
struct Registry {std::mutex mutex;std::shared_ptr<const FontCatalog> owner;bool acquired=false;};
inline Registry&registry(){static Registry value;return value;}
}
// Trusted startup caller authenticates provenance before installation. A numeric
// identity alone grants no permission. FontCatalog owns immutable copied inputs.
inline bool installFontCatalog(std::shared_ptr<const FontCatalog> owner)noexcept{
 if(!owner)return false;
 try{auto&r=font_provider_detail::registry();std::lock_guard<std::mutex>lock(r.mutex);if(r.acquired)return false;r.owner=std::move(owner);return true;}catch(...){return false;}
}
// An absent catalog does not freeze startup. The first successful acquisition
// freezes replacement and retains an immutable shared owner for deferred work.
inline std::shared_ptr<const FontCatalog> acquireFontCatalog()noexcept{
 try{auto&r=font_provider_detail::registry();std::lock_guard<std::mutex>lock(r.mutex);if(!r.owner)return {};r.acquired=true;return r.owner;}catch(...){return {};}
}
}

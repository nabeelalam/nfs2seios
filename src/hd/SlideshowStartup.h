// SPDX-License-Identifier: MIT
#pragma once
#include <string>
#include <mutex>
#include <utility>
#include "SlideshowFooterCatalog.h"
namespace nfs_slideshow {
struct Startup {std::string path;bool preview=false;std::shared_ptr<const nfs_slideshow_footer::Catalog> footers;};
inline std::mutex&startupMutex(){static std::mutex value;return value;}
inline Startup&startupValue(){static Startup value;return value;}
// Set before worker creation. No decoded image or original game data is global.
inline void setStartup(std::string path,bool preview=false){std::lock_guard<std::mutex>lock(startupMutex());startupValue().path=std::move(path);startupValue().preview=preview;}
inline void setStartupFooters(std::shared_ptr<const nfs_slideshow_footer::Catalog> value){std::lock_guard<std::mutex>lock(startupMutex());startupValue().footers=std::move(value);}
inline Startup startup(){std::lock_guard<std::mutex>lock(startupMutex());return startupValue();}
}

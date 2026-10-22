// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
namespace nfs_main_sidebar_origin {
class GameLife final {
 std::atomic<bool>alive_{true};
 friend std::shared_ptr<const GameLife>acquireGameLife(uintptr_t)noexcept;
 friend void retireGame(uintptr_t)noexcept;
public:
 bool current()const noexcept{return alive_.load(std::memory_order_acquire);}
};
namespace lifetime_detail {
struct Entry{uintptr_t game=0;std::weak_ptr<GameLife>life;};
struct Registry{std::mutex mutex;std::array<Entry,64>entries{};};
inline Registry&registry(){static Registry value;return value;}
}
// Registration occurs only outside guest-copy leases. Reads use an atomic token.
inline std::shared_ptr<const GameLife>acquireGameLife(uintptr_t game)noexcept{
 if(!game)return {};
 try{auto&r=lifetime_detail::registry();std::lock_guard<std::mutex>lock(r.mutex);lifetime_detail::Entry*free=nullptr;
  for(auto&e:r.entries){auto p=e.life.lock();if(p&&p->current()){if(e.game==game)return p;}else if(!free)free=&e;}
  if(!free)return {};auto p=std::make_shared<GameLife>();free->game=game;free->life=p;return p;
 }catch(...){return {};}
}
// May run on a different thread from the source owner. Reused addresses receive
// a fresh token; an escaped old proof can never become current again.
inline void retireGame(uintptr_t game)noexcept{
 if(!game)return;
 try{auto&r=lifetime_detail::registry();std::lock_guard<std::mutex>lock(r.mutex);
  for(auto&e:r.entries)if(e.game==game){if(auto p=e.life.lock())p->alive_.store(false,std::memory_order_release);e.life.reset();e.game=0;}
 }catch(...){std::terminate();}
}
}

// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
namespace nfs_main_sidebar_origin {
class GameLife final {
 std::atomic<bool>alive_{true};uint64_t serial_=0;
 friend std::shared_ptr<const GameLife>acquireGameLife(uintptr_t)noexcept;
 friend void retireGame(uintptr_t)noexcept;
public:
 uint64_t serial()const noexcept{return serial_;}
 bool current()const noexcept{return alive_.load(std::memory_order_acquire);}
};
namespace lifetime_detail {
struct Entry{uintptr_t game=0;std::weak_ptr<GameLife>life;};
struct Registry{std::mutex mutex;std::array<Entry,64>entries{};uint64_t nextSerial=0;};
inline Registry&registry(){static Registry value;return value;}
}
// Registration occurs only outside guest-copy leases. Reads use an atomic token.
inline std::shared_ptr<const GameLife>acquireGameLife(uintptr_t game)noexcept{
 if(!game)return {};
 try{auto&r=lifetime_detail::registry();std::lock_guard<std::mutex>lock(r.mutex);lifetime_detail::Entry*free=nullptr;
  for(auto&e:r.entries){auto p=e.life.lock();if(p&&p->current()){if(e.game==game)return p;}else if(!free)free=&e;}
  if(!free||r.nextSerial==UINT64_MAX)return {};auto p=std::make_shared<GameLife>();p->serial_=++r.nextSerial;free->game=game;free->life=p;return p;
 }catch(...){return {};}
}
// Exact host lifetime lookup; never guest memory or a guessed source-valid flag.
inline bool currentGameLife(uintptr_t game,uint64_t serial)noexcept{
 if(!game||!serial)return false;
 try{auto&r=lifetime_detail::registry();std::lock_guard<std::mutex>lock(r.mutex);
  for(const auto&e:r.entries)if(e.game==game){auto p=e.life.lock();if(p&&p->serial()==serial&&p->current())return true;}
 }catch(...){return false;}return false;
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

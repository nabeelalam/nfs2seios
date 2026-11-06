// SPDX-License-Identifier: MIT
#pragma once
#include <array>
#include <cerrno>
#include <cstdint>
#include <cstring>
namespace nfs_slideshow_live {
enum class Event {Enter,Exit,ArchiveBefore,ArchiveAfter,BackgroundBefore,BackgroundAfter,OverlayBefore,OverlayAfter,PresentBefore,PresentAfter};
struct Snapshot {Event event;Game* game;uint32_t form,archive,car,picture;};
using Sink=void(*)(const Snapshot&);
inline Sink& sink(){static thread_local Sink value=nullptr;return value;}
// Install on the game worker before entering the original screen.
inline void install(Sink value) noexcept {sink()=value;}
struct Preserve {
 Game& game;std::array<unsigned char,sizeof(CPU)> cpu;int error;
 explicit Preserve(Game&g):game(g),error(errno){std::memcpy(cpu.data(),static_cast<CPU*>(&g),sizeof(CPU));}
 ~Preserve(){std::memcpy(static_cast<CPU*>(&game),cpu.data(),sizeof(CPU));errno=error;}
};
inline void emit(Game&g,Event event,uint32_t form,uint32_t archive,uint32_t car,uint32_t picture) noexcept {
 auto callback=sink();if(!callback)return;Preserve preserve(g);
 try{callback({event,&g,form,archive,car,picture});}catch(...){sink()=nullptr;}
}
struct Scope {
 Game&game;Event after;uint32_t form,archive,car,picture;
 Scope(Game&g,Event before,Event end,uint32_t f,uint32_t a,uint32_t c,uint32_t p) noexcept:
 game(g),after(end),form(f),archive(a),car(c),picture(p){emit(game,before,form,archive,car,picture);}
 ~Scope(){emit(game,after,form,archive,car,picture);}
};
} // namespace nfs_slideshow_live

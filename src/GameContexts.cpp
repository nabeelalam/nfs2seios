// SPDX-License-Identifier: MIT
#include "GameContexts.h"
#include "Game.h"
#include "GuestAccess.h"
#include "hd/Main343OriginLifetime.h"
#ifdef NFS_SDL_RUNTIME
#include "SDLThreadCleanup.h"
#endif
#include <new>
#include <stdexcept>
extern "C" Game *nfs_create_game_context() {
    void *storage=malloc32(sizeof(Game));
    if(!storage) return nullptr;
    try { return new(storage) Game(); }
    catch (...) { free32(storage);throw; }
}
extern "C" void *nfs_resolve_game_context(uint32_t address) {
    return address?nfs_host_address(address,sizeof(Game)):nullptr;
}
extern "C" void nfs_destroy_game_context(void *context) {
    if(context) {nfs_main_sidebar_origin::retireGame(reinterpret_cast<uintptr_t>(context));static_cast<Game *>(context)->~Game();free32(context);}
}
extern "C" void nfs_prepare_game_worker(void *context,uint32_t function_id) {
    auto &game=*static_cast<Game *>(context);
    game.push32(int32_t(function_id)); // Original StartAddress LPVOID argument.
    game.push32(0); // Reserved guest return slot; execution stays native.
}
extern "C" void nfs_run_game_worker(void *context) {
#ifdef NFS_SDL_RUNTIME
    nfs::SDLThreadCleanup cleanup;
#endif
    auto &game=*static_cast<Game *>(context);
    const uint32_t expected=uint32_t(game.esp)+8;
    game._StartAddress();
    game.esp+=4; // Caller consumes the dispatcher's return slot.
    if(uint32_t(game.esp)!=expected)throw std::runtime_error("Unbalanced game worker entry stack");
}
extern "C" int32_t nfs_dispatch_game_window(void *context,uint32_t function,uint32_t window,uint32_t message,uint32_t wparam,uint32_t lparam){
    if(!context)throw std::invalid_argument("Missing window callback context");
    auto callback=nfs_lookup_guest_function(function);
    auto &game=*static_cast<Game *>(context);const int32_t expected=game.esp;
    try{
        game.push32(int32_t(lparam));game.push32(int32_t(wparam));game.push32(int32_t(message));game.push32(int32_t(window));
        game.push32(0);callback(context);game.esp+=4;
        if(game.esp!=expected)throw std::runtime_error("Unbalanced game window callback stack");
        return game.eax;
    }catch(...){game.esp=expected;throw;}
}
extern "C" void nfs_dispatch_game_procedure(void *context,uint32_t function){
    if(!context)throw std::invalid_argument("Missing exit callback context");
    auto callback=nfs_lookup_guest_function(function);
    auto &game=*static_cast<Game *>(context);const int32_t expected=game.esp;
    try{game.push32(0);callback(context);game.esp+=4;if(game.esp!=expected)throw std::runtime_error("Unbalanced game exit callback stack");}
    catch(...){game.esp=expected;throw;}
}
extern "C" void nfs_dispatch_game_exit(void *context,uint32_t function){nfs_dispatch_game_procedure(context,function);}
extern "C" void nfs_dispatch_game_regparm2(void *context,uint32_t function,uint32_t first,uint32_t second){
    if(!context)throw std::invalid_argument("Missing audio callback context");
    auto callback=nfs_lookup_guest_function(function);auto &game=*static_cast<Game *>(context);const int32_t expected=game.esp;
    game.eax=int32_t(first);game.edx=int32_t(second);
    try{game.push32(0);callback(context);game.esp+=4;if(game.esp!=expected)throw std::runtime_error("Unbalanced game audio callback stack");}
    catch(...){game.esp=expected;throw;}
}
extern "C" int32_t nfs_dispatch_game_input(void *context,uint32_t function,uint32_t instance,uint32_t reference){
    if(!context)throw std::invalid_argument("Missing input enumeration context");
    auto callback=nfs_lookup_guest_function(function);auto &game=*static_cast<Game *>(context);const int32_t expected=game.esp;
    try{game.push32(int32_t(reference));game.push32(int32_t(instance));game.push32(0);callback(context);game.esp+=4;
        if(game.esp!=expected)throw std::runtime_error("Unbalanced game input callback stack");return game.eax;}
    catch(...){game.esp=expected;throw;}
}

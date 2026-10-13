// SPDX-License-Identifier: MIT
#include "GameAssets.h"
#include "GuestAccess.h"
#include "RuntimeServices.h"
#include <stdexcept>
extern "C" [[noreturn]] void nfs_required_game_file_failed(uint32_t address){
    std::string path;bool terminated=false;
    for(uint32_t i=0;i<260 && uint64_t(address)+i<=UINT32_MAX;++i){const auto ch=guest_load<char>(address+i);if(!ch){terminated=true;break;}path+=ch;}
    if(!terminated)throw std::out_of_range("Unterminated required game filename");
    nfs::runtime_services().message("Could not load required file '"+path+"'. It may be missing or the game file loader failed.","Need for Speed II SE",0);
    nfs::runtime_services().exit(1);
}

// SPDX-License-Identifier: MIT
#pragma once
#include "SandboxPaths.h"
#include "HandleTable.h"
namespace nfs {
struct GameFindData {uint32_t words[11]{};char name[260]{};char alternate[14]{};uint8_t padding[2]{};};
static_assert(sizeof(GameFindData)==320,"Original WIN32_FIND_DATAA");
class DirectoryBridge {
    struct Search {std::vector<DirectoryEntry> entries;size_t position=0;};
    SandboxPaths &paths;HandleTable<Search,0x90000000> searches;
    static GameFindData record(const DirectoryEntry &entry);
public:
    explicit DirectoryBridge(SandboxPaths &sandbox):paths(sandbox){}
    uint32_t first(const std::string &pattern,GameFindData &output);
    bool next(uint32_t handle,GameFindData &output);
    bool close(uint32_t handle){return searches.close(handle);}
    SandboxPaths &sandbox(){return paths;}
};
void bind_directory_bridge(DirectoryBridge *bridge);
}
extern "C" int32_t FindFirstFileA_wrap(int32_t,int32_t);
extern "C" int32_t FindNextFileA_wrap(int32_t,int32_t);
extern "C" int32_t FindClose_wrap(int32_t);
extern "C" int32_t GetCurrentDirectoryA_wrap(int32_t,int32_t);
extern "C" int32_t SetCurrentDirectoryA_wrap(int32_t);
extern "C" int32_t DeleteFileA_wrap(int32_t);

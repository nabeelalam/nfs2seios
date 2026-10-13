// SPDX-License-Identifier: MIT
#pragma once
#include "HandleTable.h"
#include <unordered_map>
#include <mutex>
namespace nfs {
class FileBridge;
using MappingClose=bool(*)(uint32_t);
inline MappingClose &mapping_close_hook(){static MappingClose value=nullptr;return value;}
inline bool close_mapping_handle(uint32_t id){return mapping_close_hook() && mapping_close_hook()(id);}
class FileMappings {
    struct Mapping;
    struct View {std::shared_ptr<Mapping> mapping;void *private_copy=nullptr;size_t size=0;uint32_t offset=0,references=1;bool write=false;};
    FileBridge &files;
    HandleTable<Mapping,0xa0000000> handles;
    std::mutex mutex;
    std::unordered_map<uint32_t,View> views;
    const size_t maximum;
public:
    explicit FileMappings(FileBridge &bridge,size_t budget=128*1024*1024):files(bridge),maximum(budget){}
    ~FileMappings();
    uint32_t create(uint32_t file,uint32_t protect,uint32_t size);
    uint32_t map(uint32_t handle,uint32_t access,uint32_t offset,uint32_t size);
    bool unmap(uint32_t address);
    bool close(uint32_t handle){return handles.close(handle);}
    bool flush_and_unmap_all(); // Call only after workers/cleanup stop using views.
};
void bind_file_mappings(FileMappings *mappings);
}
extern "C" int32_t CreateFileMappingA_wrap(int32_t,int32_t,int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t MapViewOfFile_wrap(int32_t,int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t UnmapViewOfFile_wrap(int32_t);

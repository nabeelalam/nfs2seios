// SPDX-License-Identifier: MIT
#pragma once
#include "HandleTable.h"
#include "SandboxPaths.h"
#include <cstdio>
#include <string>
#include <unordered_map>
namespace nfs {
class FileBridge {
    friend class FileMappings;
    struct Completion {uint32_t bytes,error;};
    struct File {std::FILE *stream;std::mutex mutex;bool readable,writable,overlapped;std::unordered_map<uint32_t,Completion> completions;File(std::FILE *p,bool r,bool w,bool async):stream(p),readable(r),writable(w),overlapped(async){} ~File(){std::fclose(stream);} };
    HandleTable<File> files;
    SandboxPaths paths;
public:
    explicit FileBridge(std::string directory):paths(directory){}
    SandboxPaths &sandbox(){return paths;}
    static bool relative_path(const std::string &input, std::string &output);
    uint32_t open(const std::string &path, const char *mode,bool overlapped=false);
    bool close(uint32_t id) { return files.close(id); }
    bool read(uint32_t id, void *buffer, size_t size, uint32_t &transferred);
    bool write(uint32_t id, const void *buffer, size_t size, uint32_t &transferred);
    uint32_t size(uint32_t id, uint32_t *high);
    uint32_t seek(uint32_t id, int32_t distance, uint32_t method);
    bool flush(uint32_t id);
    int scan_int32(uint32_t id, int32_t &value);
    bool truncate(uint32_t id);
    bool positioned_io(uint32_t id,void *buffer,size_t size,uint32_t record,uint32_t offset,bool write,uint32_t &transferred,uint32_t &error);
    bool completion(uint32_t id,uint32_t record,uint32_t &transferred,uint32_t &error);
};
// Runtime-owned object; bind once before game threads, unbind after shutdown.
void bind_file_bridge(FileBridge *bridge);
}
extern "C" int32_t CreateFileA_wrap(int32_t,int32_t,int32_t,int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t CloseHandle_wrap(int32_t);
extern "C" int32_t ReadFile_wrap(int32_t,int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t WriteFile_wrap(int32_t,int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t GetFileSize_wrap(int32_t,int32_t);
extern "C" int32_t SetFilePointer_wrap(int32_t,int32_t,int32_t,int32_t);
extern "C" int32_t FlushFileBuffers_wrap(int32_t);
extern "C" int32_t GetLastError_wrap();
extern "C" int32_t fopen_wrap(int32_t,int32_t);
extern "C" int32_t fclose_wrap(int32_t);
extern "C" int32_t fscanf_wrap(int32_t,int32_t,...);
extern "C" int32_t SetEndOfFile_wrap(int32_t);
extern "C" int32_t GetOverlappedResult_wrap(int32_t,int32_t,int32_t,int32_t);

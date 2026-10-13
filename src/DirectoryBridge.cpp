// SPDX-License-Identifier: MIT
#include "DirectoryBridge.h"
#include "GuestAccess.h"
#include "PlatformError.h"
#include <cstring>
#include <stdexcept>
namespace nfs {
GameFindData DirectoryBridge::record(const DirectoryEntry &entry){GameFindData result;result.words[0]=entry.directory?0x10:0x20;result.words[5]=uint32_t(entry.modified);result.words[6]=uint32_t(entry.modified>>32);result.words[7]=uint32_t(entry.size>>32);result.words[8]=uint32_t(entry.size);std::memcpy(result.name,entry.name.c_str(),entry.name.size()+1);return result;}
uint32_t DirectoryBridge::first(const std::string &pattern,GameFindData &output){std::unique_ptr<Search> search(new Search);if(!paths.list(pattern,search->entries) || search->entries.empty()){platform_error()=2;return 0;}output=record(search->entries[0]);search->position=1;auto handle=searches.insert(std::move(search));platform_error()=handle?0:8;return handle;}
bool DirectoryBridge::next(uint32_t handle,GameFindData &output){return searches.with(handle,[&](Search *search){if(!search){platform_error()=6;return false;}if(search->position==search->entries.size()){platform_error()=18;return false;}output=record(search->entries[search->position++]);platform_error()=0;return true;});}
static DirectoryBridge *active=nullptr;
void bind_directory_bridge(DirectoryBridge *bridge){active=bridge;}
static DirectoryBridge &current(){if(!active)throw std::logic_error("Directory runtime unbound");return *active;}
}
static std::string path_string(uint32_t address){std::string value;for(uint32_t i=0;i<260 && uint64_t(address)+i<=UINT32_MAX;++i){const char ch=guest_load<char>(address+i);if(!ch)return value;value+=ch;}throw std::out_of_range("Unterminated directory path");}
extern "C" int32_t FindFirstFileA_wrap(int32_t pattern,int32_t output){nfs_host_address(uint32_t(output),320);nfs::GameFindData record;auto handle=nfs::current().first(path_string(uint32_t(pattern)),record);if(handle)guest_store(uint32_t(output),record);return handle?int32_t(handle):-1;}
extern "C" int32_t FindNextFileA_wrap(int32_t handle,int32_t output){nfs_host_address(uint32_t(output),320);nfs::GameFindData record;if(!nfs::current().next(uint32_t(handle),record))return 0;guest_store(uint32_t(output),record);return 1;}
extern "C" int32_t FindClose_wrap(int32_t handle){const bool ok=nfs::current().close(uint32_t(handle));nfs::platform_error()=ok?0:6;return ok;}
extern "C" int32_t GetCurrentDirectoryA_wrap(int32_t size,int32_t output){const auto path=nfs::current().sandbox().current();if(uint32_t(size)<=path.size())return int32_t(path.size()+1);nfs::check_worker_stop();std::lock_guard<std::mutex> lock(guest_copy_mutex());std::memcpy(nfs_host_address(uint32_t(output),path.size()+1),path.c_str(),path.size()+1);return int32_t(path.size());}
extern "C" int32_t SetCurrentDirectoryA_wrap(int32_t input){const bool ok=nfs::current().sandbox().change(path_string(uint32_t(input)));nfs::platform_error()=ok?0:3;return ok;}
extern "C" int32_t DeleteFileA_wrap(int32_t input){const bool ok=nfs::current().sandbox().remove_file(path_string(uint32_t(input)));nfs::platform_error()=ok?0:2;return ok;}

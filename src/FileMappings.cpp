// SPDX-License-Identifier: MIT
#include "FileMappings.h"
#include "FileBridge.h"
#include "GuestAccess.h"
#include "PlatformError.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
extern "C" void *malloc32(size_t);
extern "C" void free32(void *);
namespace nfs {
struct FileMappings::Mapping {
    std::shared_ptr<FileBridge::File> file;
    std::unique_ptr<void,void(*)(void *)> bytes{nullptr,free32};
    uint32_t length=0,protect;
    explicit Mapping(uint32_t protection):protect(protection){}
};
uint32_t FileMappings::create(uint32_t id,uint32_t protect,uint32_t requested){
    // Original resource loads pass SEC_COMMIT with PAGE_READONLY. For a
    // data-file mapping this attribute has no effect (CreateFileMappingA).
    // Keep rejecting other attributes rather than discarding unknown flags.
    protect &= ~UINT32_C(0x08000000);
    if(protect!=2 && protect!=4 && protect!=8){platform_error()=50;return 0;}
    auto file=files.files.share(id);if(!file){platform_error()=6;return 0;}
    if(!file->readable || (protect==4 && !file->writable)){platform_error()=5;return 0;}
    std::lock_guard<std::mutex> lock(file->mutex);
    const long position=std::ftell(file->stream);if(position<0 || std::fseek(file->stream,0,SEEK_END)){platform_error()=30;return 0;}
    const long end=std::ftell(file->stream);
    if(std::fseek(file->stream,position,SEEK_SET) || end<0 || uint64_t(end)>UINT32_MAX){platform_error()=30;return 0;}
    const uint32_t length=requested?requested:uint32_t(end);
    if(!length || length>maximum || (length>uint64_t(end) && protect!=4)){platform_error()=87;return 0;}
    std::unique_ptr<Mapping> pending(new Mapping(protect));pending->file=file;pending->length=length;
    pending->bytes.reset(malloc32(length));if(!pending->bytes){platform_error()=8;return 0;}
    std::memset(pending->bytes.get(),0,length);
    const size_t count=std::min(size_t(length),size_t(end));
    bool ok=!std::fseek(file->stream,0,SEEK_SET) && std::fread(pending->bytes.get(),1,count,file->stream)==count;
    if(ok && length>uint64_t(end))ok=!std::fseek(file->stream,long(length-1),SEEK_SET) && std::fputc(0,file->stream)!=EOF && !std::fflush(file->stream);
    ok=!std::fseek(file->stream,position,SEEK_SET) && ok;
    if(!ok){platform_error()=30;return 0;}
    const uint32_t handle=handles.insert(std::move(pending));platform_error()=handle?0:8;return handle;
}
uint32_t FileMappings::map(uint32_t id,uint32_t access,uint32_t offset,uint32_t requested){
    auto mapping=handles.share(id);if(!mapping){platform_error()=6;return 0;}
    if(access==0xf001f)access=6;
    if((access!=1 && access!=2 && access!=4 && access!=6) || ((access==2 || access==6) && mapping->protect!=4)){platform_error()=5;return 0;}
    if(offset>=mapping->length || (requested && requested>mapping->length-offset)){platform_error()=87;return 0;}
    const size_t length=requested?requested:mapping->length-offset;
    std::lock_guard<std::mutex> lock(mutex);
    if(views.size()==256){platform_error()=8;return 0;}
    void *copy=nullptr;void *pointer=static_cast<uint8_t *>(mapping->bytes.get())+offset;
    std::unique_ptr<void,void(*)(void *)> guard(nullptr,free32);
    if(access==1){guard.reset(malloc32(length));if(!guard){platform_error()=8;return 0;}copy=guard.get();std::memcpy(copy,pointer,length);pointer=copy;}
    const uint32_t address=nfs_guest_address(pointer);
    auto found=views.find(address);
    if(found!=views.end()){
        if(found->second.references==UINT32_MAX){platform_error()=8;return 0;}
        ++found->second.references;found->second.size=std::max(found->second.size,length);found->second.write|=access==2 || access==6;
    }else{views.emplace(address,View{mapping,copy,length,offset,1,access==2 || access==6});guard.release();}
    platform_error()=0;return address;
}
bool FileMappings::unmap(uint32_t address){
    std::lock_guard<std::mutex> lock(mutex);auto found=views.find(address);if(found==views.end()){platform_error()=487;return false;}
    auto &view=found->second;
    if(view.references>1){--view.references;platform_error()=0;return true;}
    if(view.write){
        auto &file=*view.mapping->file;std::lock_guard<std::mutex> bytes_lock(guest_copy_mutex());std::lock_guard<std::mutex> file_lock(file.mutex);
        const long position=std::ftell(file.stream);
        bool ok=position>=0 && !std::fseek(file.stream,long(view.offset),SEEK_SET) && std::fwrite(static_cast<uint8_t *>(view.mapping->bytes.get())+view.offset,1,view.size,file.stream)==view.size && !std::fflush(file.stream);
        if(position>=0)ok=!std::fseek(file.stream,position,SEEK_SET) && ok;
        if(!ok){platform_error()=29;return false;}
    }
    if(view.private_copy)free32(view.private_copy);views.erase(found);platform_error()=0;return true;
}
FileMappings::~FileMappings(){
    // Normal runtime shutdown explicitly unmaps views and reports write errors.
    for(auto &view:views)if(view.second.private_copy)free32(view.second.private_copy);
}
bool FileMappings::flush_and_unmap_all(){
    for(;;){uint32_t address;
        {std::lock_guard<std::mutex> lock(mutex);if(views.empty())return true;address=views.begin()->first;}
        if(!unmap(address))return false;
    }
}
static FileMappings *active=nullptr;
static bool close(uint32_t id){return active && active->close(id);}
void bind_file_mappings(FileMappings *mappings){active=mappings;mapping_close_hook()=mappings?close:nullptr;}
}
extern "C" int32_t CreateFileMappingA_wrap(int32_t file,int32_t security,int32_t protect,int32_t high,int32_t low,int32_t name){
    if(!nfs::active || security || high || name || file==-1){nfs::platform_error()=50;return 0;}
    return int32_t(nfs::active->create(uint32_t(file),uint32_t(protect),uint32_t(low)));
}
extern "C" int32_t MapViewOfFile_wrap(int32_t mapping,int32_t access,int32_t high,int32_t low,int32_t size){
    if(!nfs::active || high){nfs::platform_error()=50;return 0;}
    return int32_t(nfs::active->map(uint32_t(mapping),uint32_t(access),uint32_t(low),uint32_t(size)));
}
extern "C" int32_t UnmapViewOfFile_wrap(int32_t address){if(!nfs::active){nfs::platform_error()=50;return 0;}return nfs::active->unmap(uint32_t(address));}

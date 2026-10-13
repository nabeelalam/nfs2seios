// SPDX-License-Identifier: MIT
#include "FileBridge.h"
#include "EventBridge.h"
#include "ThreadBridge.h"
#include "PlatformError.h"
#include "GuestAccess.h"
#include "FileMappings.h"
#include <cstring>
#include <climits>
#include <stdexcept>
#include <cstdarg>
#include <cctype>
#include <cstdlib>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
namespace nfs {
bool FileBridge::relative_path(const std::string &input, std::string &output) {
    output.clear();
    if (input.empty() || input[0]=='/' || input[0]=='\\' || input.find(':')!=std::string::npos) return false;
    std::string part;
    const auto commit = [&] {
        if (part=="..") return false;
        if (!part.empty() && part!=".") { if (!output.empty()) output+='/'; output+=part; }
        part.clear(); return true;
    };
    for (unsigned char ch : input) {
        if (ch=='/' || ch=='\\') { if (!commit()) return false; }
        else if (ch<32) return false;
        else part += char(ch>='A' && ch<='Z' ? ch+32 : ch);
    }
    return commit() && !output.empty();
}
uint32_t FileBridge::open(const std::string &path, const char *mode,bool overlapped) {
    std::string native;
    if (!paths.resolve(path,native)) {
        if(std::getenv("NFS_TRACE_FILES"))std::fprintf(stderr,"Game file rejected: %s\n",path.c_str());
        return 0;
    }
    std::FILE *stream = std::fopen(native.c_str(), mode);
    if(std::getenv("NFS_TRACE_FILES"))std::fprintf(stderr,"Game file %s (%s): %s\n",path.c_str(),mode,stream?"opened":"unavailable");
    if (!stream) return 0;
    std::unique_ptr<std::FILE, int(*)(std::FILE *)> guard(stream, std::fclose);
    std::unique_ptr<File> resource(new File(stream,mode[0]=='r' || std::strchr(mode,'+'),mode[0]!='r' || std::strchr(mode,'+'),overlapped));
    guard.release();
    return files.insert(std::move(resource));
}
bool FileBridge::read(uint32_t id, void *buffer, size_t size, uint32_t &transferred) {
    transferred=0;
    return files.with(id, [&](File *file) {
        if (!file) return false;
        std::lock_guard<std::mutex> lock(file->mutex);
        if(file->overlapped)return false;
        if (!size) return true;
        transferred=uint32_t(std::fread(buffer,1,size,file->stream));
        return !std::ferror(file->stream);
    });
}
bool FileBridge::write(uint32_t id, const void *buffer, size_t size, uint32_t &transferred) {
    transferred=0;
    return files.with(id, [&](File *file) {
        if (!file) return false;
        std::lock_guard<std::mutex> lock(file->mutex);
        if(file->overlapped)return false;
        if (!size) return true;
        transferred=uint32_t(std::fwrite(buffer,1,size,file->stream));
        return transferred==size && !std::ferror(file->stream);
    });
}
uint32_t FileBridge::size(uint32_t id, uint32_t *high) {
    if (high) *high=0;
    return files.with(id, [&](File *file) -> uint32_t {
        if (!file) return UINT32_MAX;
        std::lock_guard<std::mutex> lock(file->mutex);
        long position=std::ftell(file->stream);
        if (position<0 || std::fseek(file->stream,0,SEEK_END)) return UINT32_MAX;
        long length=std::ftell(file->stream);
        if (std::fseek(file->stream,position,SEEK_SET) || length<0 || uint64_t(length)>UINT32_MAX) return UINT32_MAX;
        return uint32_t(length);
    });
}
uint32_t FileBridge::seek(uint32_t id, int32_t distance, uint32_t method) {
    return files.with(id, [&](File *file) -> uint32_t {
        if (!file || method>2) return UINT32_MAX;
        std::lock_guard<std::mutex> lock(file->mutex);
        const int origins[]={SEEK_SET,SEEK_CUR,SEEK_END};
        if (std::fseek(file->stream,distance,origins[method])) return UINT32_MAX;
        long offset=std::ftell(file->stream);
        return offset<0 || uint64_t(offset)>UINT32_MAX ? UINT32_MAX : uint32_t(offset);
    });
}
bool FileBridge::flush(uint32_t id) {
    return files.with(id, [](File *file) {if(!file)return false;std::lock_guard<std::mutex> lock(file->mutex);return !std::fflush(file->stream);});
}
int FileBridge::scan_int32(uint32_t id, int32_t &value) {
    return files.with(id, [&](File *file) {
        if (!file) return EOF;
        std::lock_guard<std::mutex> lock(file->mutex);
        int ch=std::fgetc(file->stream);
        while (ch!=EOF && std::isspace(static_cast<unsigned char>(ch))) ch=std::fgetc(file->stream);
        if (ch==EOF) return EOF;
        const bool negative=ch=='-';
        if (ch=='-' || ch=='+') ch=std::fgetc(file->stream);
        uint64_t magnitude=0;
        bool digits=false, overflow=false;
        const uint64_t limit=negative?uint64_t(INT32_MAX)+1:uint64_t(INT32_MAX);
        while (ch>='0' && ch<='9') {
            digits=true;
            if (!overflow) {
                magnitude=magnitude*10+unsigned(ch-'0');
                overflow=magnitude>limit;
            }
            ch=std::fgetc(file->stream);
        }
        if (ch!=EOF) std::ungetc(ch,file->stream);
        if (!digits || overflow) return 0;
        value=int32_t(negative?-int64_t(magnitude):int64_t(magnitude)); return 1;
    });
}
bool FileBridge::truncate(uint32_t id){
    return files.with(id,[](File *file){
        if(!file || !file->writable)return false;std::lock_guard<std::mutex> lock(file->mutex);
        const long position=std::ftell(file->stream);if(position<0 || std::fflush(file->stream))return false;
#ifdef _WIN32
        const bool ok=!_chsize_s(_fileno(file->stream),position);
#else
        const bool ok=!ftruncate(fileno(file->stream),position);
#endif
        if(ok)std::clearerr(file->stream);return ok;
    });
}
bool FileBridge::positioned_io(uint32_t id,void *buffer,size_t size,uint32_t record,uint32_t offset,bool writing,uint32_t &transferred,uint32_t &error){
    transferred=0;error=6;
    return files.with(id,[&](File *file){
        if(!file)return false;std::lock_guard<std::mutex> lock(file->mutex);
        if((writing && !file->writable) || (!writing && !file->readable)){error=5;return false;}
        if(!file->completions.count(record) && file->completions.size()==256){error=8;return false;}
        if(uint64_t(offset)>LONG_MAX){error=50;return false;}
        auto completion=file->completions.emplace(record,Completion{0,997}).first;
        const long position=std::ftell(file->stream);error=0;std::clearerr(file->stream);
        if(position<0 || std::fseek(file->stream,long(offset),SEEK_SET))error=writing?29:30;
        else if(size){
            transferred=uint32_t(writing?std::fwrite(buffer,1,size,file->stream):std::fread(buffer,1,size,file->stream));
            if(std::ferror(file->stream) || (writing && transferred!=size))error=writing?29:30;
            else if(!writing && !transferred)error=38;
        }
        if(position>=0 && std::fseek(file->stream,position,SEEK_SET))error=writing?29:30;
        completion->second={transferred,error};return !error;
    });
}
bool FileBridge::completion(uint32_t id,uint32_t record,uint32_t &transferred,uint32_t &error){
    transferred=0;error=6;return files.with(id,[&](File *file){if(!file)return false;std::lock_guard<std::mutex> lock(file->mutex);const auto found=file->completions.find(record);if(found==file->completions.end()){error=87;return false;}transferred=found->second.bytes;error=found->second.error;return !error;});
}
static FileBridge *active = nullptr;
void bind_file_bridge(FileBridge *bridge) { active = bridge; }
}
static std::string game_string(uint32_t address, size_t maximum=260) {
    std::string result;
    for (size_t i=0; i<maximum; ++i) {
        if (uint64_t(address)+i>UINT32_MAX) break;
        const char ch=guest_load<int8_t>(address+uint32_t(i));
        if (!ch) return result;
        result+=ch;
    }
    throw std::out_of_range("Unterminated guest string");
}
extern "C" int32_t GetLastError_wrap() { return int32_t(nfs::platform_error()); }
extern "C" int32_t CreateFileA_wrap(int32_t path,int32_t access,int32_t,int32_t security,int32_t creation,int32_t flags,int32_t template_file) {
    if (!nfs::active || security || template_file) { nfs::platform_error()=50; return -1; }
    const char *mode=nullptr;
    if (uint32_t(access)==0x80000000 && creation==3) mode="rb";
    if (uint32_t(access)==0x40000000 && creation==2) mode="wb";
    if (uint32_t(access)==0xc0000000 && creation==2) mode="w+b";
    if (uint32_t(access)==0xc0000000 && creation==3) mode="r+b";
    if (!mode) { nfs::platform_error()=50; return -1; }
    uint32_t handle=nfs::active->open(game_string(uint32_t(path)),mode,(uint32_t(flags)&0x40000000)!=0);
    nfs::platform_error()=handle?0:2; return handle?int32_t(handle):-1;
}
extern "C" int32_t CloseHandle_wrap(int32_t handle) {
    bool ok=(nfs::active && nfs::active->close(uint32_t(handle))) || nfs::close_event_handle(uint32_t(handle)) || nfs::close_thread_handle(uint32_t(handle)) || nfs::close_mapping_handle(uint32_t(handle));
    nfs::platform_error()=ok?0:6; return ok;
}
struct GameOverlapped {uint32_t internal,transferred,offset,high,event;};
static_assert(sizeof(GameOverlapped)==20,"Original OVERLAPPED record");
static int32_t completed_io(int32_t handle,int32_t buffer,int32_t size,int32_t output,int32_t overlapped,bool write){
    if(output)nfs_host_address(uint32_t(output),4);
    auto record=guest_load<GameOverlapped>(uint32_t(overlapped));
    if(record.high){nfs::platform_error()=50;return 0;}
    void *native=size?nfs_host_address(uint32_t(buffer),uint32_t(size)):nullptr;
    uint32_t transferred=0,error=0;
    const bool ok=nfs::active->positioned_io(uint32_t(handle),native,uint32_t(size),uint32_t(overlapped),record.offset,write,transferred,error);
    record.internal=ok?0:(error==38?0xc0000011u:0xc0000001u);record.transferred=transferred;
    guest_store(uint32_t(overlapped),record);if(output)guest_store<uint32_t>(uint32_t(output),transferred);
    if(record.event)SetEvent_wrap(int32_t(record.event&~1u));
    nfs::platform_error()=error;return ok;
}
extern "C" int32_t ReadFile_wrap(int32_t handle,int32_t buffer,int32_t size,int32_t output,int32_t overlapped) {
    if (!nfs::active) { nfs::platform_error()=50; return 0; }
    if(overlapped)return completed_io(handle,buffer,size,output,overlapped,false);
    uint32_t transferred=0;
    if (output) nfs_host_address(uint32_t(output),4);
    void *native=size?nfs_host_address(uint32_t(buffer),uint32_t(size)):nullptr;
    bool ok=nfs::active->read(uint32_t(handle),native,uint32_t(size),transferred);
    if (output) guest_store<uint32_t>(uint32_t(output),transferred);
    nfs::platform_error()=ok?0:6; return ok;
}
extern "C" int32_t WriteFile_wrap(int32_t handle,int32_t buffer,int32_t size,int32_t output,int32_t overlapped) {
    if (!nfs::active) { nfs::platform_error()=50; return 0; }
    if(overlapped)return completed_io(handle,buffer,size,output,overlapped,true);
    uint32_t transferred=0;
    if (output) nfs_host_address(uint32_t(output),4);
    const void *native=size?nfs_host_address(uint32_t(buffer),uint32_t(size)):nullptr;
    bool ok=nfs::active->write(uint32_t(handle),native,uint32_t(size),transferred);
    if (output) guest_store<uint32_t>(uint32_t(output),transferred);
    nfs::platform_error()=ok?0:6; return ok;
}
extern "C" int32_t GetFileSize_wrap(int32_t handle,int32_t high) {
    uint32_t upper=0;
    uint32_t size=nfs::active?nfs::active->size(uint32_t(handle),&upper):UINT32_MAX;
    if (high) guest_store<uint32_t>(uint32_t(high),upper);
    nfs::platform_error()=size==UINT32_MAX?6:0; return int32_t(size);
}
extern "C" int32_t SetFilePointer_wrap(int32_t handle,int32_t offset,int32_t high,int32_t method) {
    if (high && guest_load<int32_t>(uint32_t(high))) { nfs::platform_error()=50; return -1; }
    uint32_t position=nfs::active?nfs::active->seek(uint32_t(handle),offset,uint32_t(method)):UINT32_MAX;
    if (high) guest_store<uint32_t>(uint32_t(high),0);
    nfs::platform_error()=position==UINT32_MAX?6:0; return int32_t(position);
}
extern "C" int32_t FlushFileBuffers_wrap(int32_t handle) {
    bool ok=nfs::active && nfs::active->flush(uint32_t(handle)); nfs::platform_error()=ok?0:6; return ok;
}
extern "C" int32_t SetEndOfFile_wrap(int32_t handle){const bool ok=nfs::active && nfs::active->truncate(uint32_t(handle));nfs::platform_error()=ok?0:5;return ok;}
extern "C" int32_t GetOverlappedResult_wrap(int32_t handle,int32_t overlapped,int32_t output,int32_t){
    nfs_host_address(uint32_t(overlapped),20);nfs_host_address(uint32_t(output),4);
    uint32_t transferred=0,error=6;const bool ok=nfs::active && nfs::active->completion(uint32_t(handle),uint32_t(overlapped),transferred,error);
    guest_store<uint32_t>(uint32_t(output),transferred);nfs::platform_error()=error;return ok;
}
extern "C" int32_t fopen_wrap(int32_t path,int32_t mode) {
    const std::string native_mode=game_string(uint32_t(mode),8);
    if (native_mode!="r" && native_mode!="rb" && native_mode!="w" && native_mode!="wb" && native_mode!="a" && native_mode!="ab") return 0;
    return nfs::active?int32_t(nfs::active->open(game_string(uint32_t(path)),native_mode.c_str())):0;
}
extern "C" int32_t fclose_wrap(int32_t handle) {
    const bool ok=nfs::active && nfs::active->close(uint32_t(handle));
    nfs::platform_error()=ok?0:6; return ok?0:EOF;
}
extern "C" int32_t fscanf_wrap(int32_t handle,int32_t format,...) {
    const std::string pattern=game_string(uint32_t(format),16);
    // The translated source currently uses one decimal conversion per call.
    // Resolve its 32-bit guest output explicitly; never pass it to native varargs.
    if (pattern!="%d" && pattern!="%d\n") { nfs::platform_error()=50; return EOF; }
    va_list arguments; va_start(arguments,format);
    const uint32_t output=uint32_t(va_arg(arguments,int32_t));
    va_end(arguments);
    nfs_host_address(output,4);
    int32_t value=0;
    const int result=nfs::active?nfs::active->scan_int32(uint32_t(handle),value):EOF;
    if (result==1) guest_store<int32_t>(output,value);
    return result;
}

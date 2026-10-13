// SPDX-License-Identifier: MIT
#include "TrackRecords.h"
#include "GuestAccess.h"
#include "GameContexts.h"
#include "PlatformError.h"
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <limits>
#include <stdexcept>
extern "C" void *malloc32(size_t);
extern "C" void free32(void *);
namespace nfs {
namespace {
struct Lines {
    const std::string &text;size_t at=0;
    bool next(std::string &line){if(at==text.size())return false;const auto end=text.find('\n',at);const auto length=(end==std::string::npos?text.size():end)-at;if(length>256)return false;line=text.substr(at,length);if(!line.empty() && line.back()=='\r')line.pop_back();at=end==std::string::npos?text.size():end+1;return true;}
};
bool number(const std::string &line,int64_t low,int64_t high,int64_t &value,int base=10){errno=0;char *end=nullptr;const auto result=std::strtoll(line.c_str(),&end,base);if(end==line.c_str() || errno==ERANGE)return false;while(*end==' ' || *end=='\t')++end;if(*end || result<low || result>high)return false;value=result;return true;}
bool entry(Lines &lines,TrackRecordEntry &record){std::string line;if(!lines.next(line))return false;const auto separator=line.find_first_of("\t\r");if(separator!=std::string::npos)line.resize(separator);std::memcpy(record.name,line.data(),line.size()<9?line.size():9);int64_t value=0;if(!lines.next(line)||!number(line,INT16_MIN,INT16_MAX,value,0))return false;record.car=int16_t(value);if(!lines.next(line)||!number(line,INT32_MIN,INT32_MAX,value))return false;record.time=int32_t(value);if(!lines.next(line)||!number(line,INT32_MIN,INT32_MAX,value))return false;record.mode=int32_t(value);return true;}
struct FileGuard {FileBridge &files;uint32_t id;~FileGuard(){if(id)files.close(id);}};
FileBridge *active=nullptr;
struct PathBuffer {void *pointer=malloc32(260);~PathBuffer(){if(pointer)free32(pointer);}};
std::string path(void *game,uint32_t track,PathBuffer &buffer){if(!buffer.pointer)throw std::bad_alloc();std::memset(buffer.pointer,0,260);const auto address=nfs_guest_address(buffer.pointer);nfs_game_record_path(game,track,address);std::string result;for(unsigned i=0;i<260;++i){const auto ch=guest_load<char>(address+i);if(!ch)return result;result+=ch;}throw std::out_of_range("Unterminated original record path");}
}
bool parse_track_records(const std::string &text,TrackRecords &records){
    if(text.size()>65536 || text.find('\0')!=std::string::npos)return false;
    TrackRecords parsed{};Lines lines{text};std::string line;int64_t version;
    if(!lines.next(line)||!lines.next(line)||!number(line,INT32_MIN,INT32_MAX,version)||!lines.next(line)||!entry(lines,parsed[0]))return false;
    for(unsigned group=0;group<3;++group){if(!lines.next(line))return false;for(unsigned i=0;i<10;++i)if(!entry(lines,parsed[1+group*10+i]))return false;}
    records=parsed;return true;
}
bool convert_track_records(FileBridge &files,const std::string &source,const std::string &destination,bool clear){
    TrackRecords records{};
    if(!clear){
        FileGuard input{files,files.open(source,"rb")};
        if(input.id){
            const auto size=files.size(input.id,nullptr);if(size>65536){platform_error()=13;return false;}
            std::string text(size,'\0');uint32_t bytes=0;
            if((size && (!files.read(input.id,&text[0],size,bytes)||bytes!=size)) || !parse_track_records(text,records)){platform_error()=13;return false;}
        }
    }
    FileGuard output{files,files.open(destination,"wb")};if(!output.id){platform_error()=5;return false;}
    uint32_t bytes=0;const bool ok=files.write(output.id,records.data(),sizeof(records),bytes) && bytes==sizeof(records) && files.flush(output.id);
    platform_error()=ok?0:5;return ok;
}
void bind_track_record_files(FileBridge *files){active=files;}
}
extern "C" int32_t fetchTrackRecords(int32_t context,int32_t track,int32_t clear){
    if(!nfs::active)throw std::logic_error("Track record files unbound");
    void *game=nfs_resolve_game_context(uint32_t(context));if(!game)throw std::invalid_argument("Missing original record context");
    nfs::PathBuffer buffer;const auto source=clear?std::string{}:nfs::path(game,uint32_t(track)+20,buffer);const auto destination=nfs::path(game,uint32_t(track),buffer);
    nfs::convert_track_records(*nfs::active,source,destination,clear!=0);return 0;
}

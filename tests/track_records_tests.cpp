#include "TrackRecords.h"
#include "GuestAccess.h"
#include "HeapArena.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
static nfs::GameMemory memory;static nfs::HeapArena heap;static uint8_t storage[65536];static int context;
extern "C" void *malloc32(size_t size){return heap.allocate(size);}
extern "C" void free32(void *p){if(!heap.release(p))throw std::out_of_range("Record release");}
extern "C" void *nfs_host_address(uint32_t a,size_t size){auto p=memory.resolve(a,size);if(!p)throw std::out_of_range("Record span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t a=0;if(!memory.encode(p,a))throw std::out_of_range("Record pointer");return a;}
extern "C" void *nfs_resolve_game_context(uint32_t a){return a==1?&context:nullptr;}
extern "C" void nfs_game_record_path(void *game,uint32_t track,uint32_t output){if(game!=&context || (track!=0 && track!=20))throw std::runtime_error("Record path callback arguments");const char *path=track==20?"records-native.ssf":"records-native.stf";for(size_t i=0;i<=std::strlen(path);++i)guest_store<char>(output+uint32_t(i),path[i]);}
static std::string fixture(){std::string text="Original records\r\n1\r\nBest lap\r\n";for(unsigned i=0;i<31;++i){if(i>0 && (i-1)%10==0)text+="Race laps\r\n";text+="Driver "+std::to_string(i)+"\r\n"+std::to_string(int(i)-10)+"\r\n"+std::to_string(100000+i)+"\r\n"+std::to_string(i%3)+"\r\n";}return text;}
int main(int argc,char **argv){
    CHECK(argc==2 && heap.initialize(memory,0x10000000,storage,sizeof(storage)));
    nfs::TrackRecords records{};const auto text=fixture();CHECK(nfs::parse_track_records(text,records));
    CHECK(std::strcmp(records[0].name,"Driver 0")==0 && records[0].car==-10 && records[30].time==100030 && records[30].mode==0);
    const auto before=records;CHECK(!nfs::parse_track_records("bad",records) && std::memcmp(&records,&before,sizeof(records))==0);
    auto hexadecimal=text;const auto car_at=hexadecimal.find("-10\r\n");hexadecimal.replace(car_at,3,"0x12");nfs::TrackRecords alternate{};CHECK(nfs::parse_track_records(hexadecimal,alternate) && alternate[0].car==18);
    hexadecimal.replace(car_at,4,"012");CHECK(nfs::parse_track_records(hexadecimal,alternate) && alternate[0].car==10);
    auto malformed=text;const auto at=malformed.find("-10\r\n");malformed.replace(at,3,"32768");CHECK(!nfs::parse_track_records(malformed,records));
    nfs::FileBridge files(argv[1]);nfs::bind_track_record_files(&files);
    struct Cleanup {std::string root;~Cleanup(){std::remove((root+"/records-native.ssf").c_str());std::remove((root+"/records-native.stf").c_str());nfs::bind_track_record_files(nullptr);}} cleanup{argv[1]};
    const auto write=[&](const std::string &name,const std::string &data){auto id=files.open(name,"wb");uint32_t bytes=0;const bool ok=id && files.write(id,data.data(),data.size(),bytes) && files.flush(id);if(id)files.close(id);return ok;};
    CHECK(write("records-native.ssf",text));const auto available=heap.available_bytes();CHECK(fetchTrackRecords(1,0,0)==0 && heap.available_bytes()==available);
    auto id=files.open("records-native.stf","rb");CHECK(id && files.size(id,nullptr)==620);uint32_t bytes=0;nfs::TrackRecords binary{};CHECK(files.read(id,binary.data(),sizeof(binary),bytes) && bytes==620);files.close(id);
    CHECK(std::memcmp(binary.data(),records.data(),620)==0);
    CHECK(write("records-native.ssf",malformed));CHECK(!nfs::convert_track_records(files,"records-native.ssf","records-native.stf",false));
    id=files.open("records-native.stf","rb");CHECK(files.read(id,binary.data(),620,bytes));files.close(id);CHECK(std::memcmp(binary.data(),records.data(),620)==0);
    CHECK(fetchTrackRecords(1,0,1)==0);id=files.open("records-native.stf","rb");CHECK(files.read(id,binary.data(),620,bytes));files.close(id);for(const auto *p=reinterpret_cast<const uint8_t *>(binary.data());p<reinterpret_cast<const uint8_t *>(binary.data())+620;++p)CHECK(*p==0);
    CHECK(nfs::convert_track_records(files,"missing-record.ssf","records-native.stf",false));
    std::puts("PASS: original SSF to exact 620-byte STF records, CRLF/signed fields, clear/missing behavior, malformed input preserves existing records, mapped path callback and heap retirement");
}

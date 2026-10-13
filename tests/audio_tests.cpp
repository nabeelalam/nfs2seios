// SPDX-License-Identifier: MIT
#include "AudioBridge.h"
#include "GameContexts.h"
#include "GuestAccess.h"
#include "WorkerControl.h"
#include <atomic>
#include <future>
#include <vector>
#include <cstdio>
#include <cstring>
#include <stdexcept>
class Game {};
static Game main_game;static int stream_tag;
alignas(16) static uint8_t sample_buffer[1024];
static std::atomic<unsigned> created{0},destroyed{0},frames{0},fade_calls{0},resumes{0};
static bool occupied=false,open_fails=false,immediate=false,callback_fails=false,self_stop=false;
static uintptr_t current_token;
static nfs::AudioBridge *bridge;
static std::promise<void> *entered=nullptr;static std::shared_future<void> gate;
extern "C" Game *nfs_create_game_context(){++created;return new Game;}
extern "C" void nfs_destroy_game_context(void *p){delete static_cast<Game *>(p);++destroyed;}
extern "C" void *nfs_resolve_game_context(uint32_t id){if(id!=0x80000000)throw std::out_of_range("Audio service context");return &main_game;}
extern "C" void *malloc32(size_t size){if(occupied || size!=1024)return nullptr;occupied=true;return sample_buffer;}
extern "C" void free32(void *p){if(p!=sample_buffer || !occupied)throw std::runtime_error("Audio buffer lifetime");occupied=false;}
extern "C" uint32_t nfs_guest_address(const void *p){if(p!=sample_buffer)throw std::out_of_range("Audio guest pointer");return 0x90000000;}
extern "C" void *nfs_host_address(uint32_t id,size_t size){if(id!=0x90000000 || size>1024)throw std::out_of_range("Audio guest span");return sample_buffer;}
static void noop(void *){}
extern "C" NativeGameFunction nfs_lookup_guest_function(uint32_t id){if(id!=1 && id!=2)throw std::out_of_range("Audio function ID");return noop;}
extern "C" void nfs_dispatch_game_procedure(void *p,uint32_t id){if(p!=&main_game || id!=2)throw std::runtime_error("Audio fade ABI");++fade_calls;}
extern "C" void nfs_dispatch_game_regparm2(void *p,uint32_t id,uint32_t output,uint32_t count){
    if(p==&main_game || id!=1 || output!=0x90000000 || count!=256)throw std::runtime_error("Audio sample ABI");
    if(self_stop)bridge->stop();if(callback_fails)throw std::runtime_error("Mixer fixture");
    if(entered){entered->set_value();gate.wait();nfs::check_worker_stop();}
    const auto start=frames.fetch_add(256);
    for(unsigned i=0;i<256;++i){const int16_t l=int16_t(start+i),r=int16_t(-int(start+i));std::memcpy(sample_buffer+i*4,&l,2);std::memcpy(sample_buffer+i*4+2,&r,2);}
}
extern "C" void *nfs_native_audio_open(uintptr_t token){current_token=token;if(immediate){uint8_t first[4];if(nfs_audio_render(token,first,4)!=4)throw std::runtime_error("Immediate audio callback rejected");}return open_fails?nullptr:&stream_tag;}
extern "C" bool nfs_native_audio_resume(void *p){if(p!=&stream_tag)throw std::runtime_error("Bad native audio stream");++resumes;return true;}
extern "C" bool nfs_native_audio_pause(void *p){return p==&stream_tag;}
extern "C" void nfs_native_audio_close(void *p){if(p!=&stream_tag)throw std::runtime_error("Bad native audio close");}
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(){
    nfs::AudioBridge audio;bridge=&audio;nfs::bind_audio_bridge(&audio);
    CHECK(iSNDdllversion_()==0x60002 && iSNDdirectcaps_(0)==0x23e0f);
    CHECK(!iSNDdirectsetfunctions_wrap(1,0,0,2,0));CHECK(!iSNDdirectstart_(0,0));const uintptr_t old=current_token;
    CHECK(!iSNDdirectserve_(int32_t(0x80000000u)) && !iSNDdirectserve_(int32_t(0x80000000u)) && resumes==1 && fade_calls==2);
    std::vector<uint8_t> output(4111);size_t offset=0;for(size_t count:{size_t(3),size_t(1021),size_t(17),size_t(3070)}){CHECK(nfs_audio_render(old,output.data()+offset,count)==count);offset+=count;}
    for(size_t i=0;i<output.size();++i){const unsigned frame=unsigned(i/4);const uint16_t value=uint16_t((i/2)%2?-int(frame):int(frame));CHECK(output[i]==uint8_t(value>>((i%2)*8)));}
    CHECK(!iSNDdirectstop_() && created==destroyed && !occupied);uint8_t stale[4]{0xcc,0xcc,0xcc,0xcc};CHECK(!nfs_audio_render(old,stale,4) && stale[0]==0xcc);
    immediate=true;CHECK(audio.start() && current_token!=old);audio.stop();immediate=false;CHECK(created==destroyed && !occupied);
    open_fails=true;CHECK(!audio.start() && created==destroyed && !occupied);open_fails=false;
    CHECK(audio.start());callback_fails=true;CHECK(!nfs_audio_render(current_token,stale,4));bool failed=false;try{audio.stop();}catch(const std::runtime_error &){failed=true;}CHECK(failed && created==destroyed && !occupied);callback_fails=false;
    CHECK(audio.start());self_stop=true;CHECK(!nfs_audio_render(current_token,stale,4));failed=false;try{audio.stop();}catch(const std::logic_error &){failed=true;}CHECK(failed && created==destroyed);self_stop=false;
    CHECK(audio.start());std::promise<void> mixing,release;entered=&mixing;gate=release.get_future().share();
    const auto token=current_token;std::promise<size_t> pulled;std::thread callback([&]{uint8_t bytes[4];pulled.set_value(nfs_audio_render(token,bytes,4));});mixing.get_future().wait();
    std::promise<void> stopped;auto done=stopped.get_future();std::thread stopping([&]{audio.stop();stopped.set_value();});
    CHECK(done.wait_for(std::chrono::milliseconds(10))==std::future_status::timeout);
    release.set_value();CHECK(pulled.get_future().get()==0);callback.join();done.get();stopping.join();entered=nullptr;
    CHECK(created==destroyed && !occupied);
    nfs::bind_audio_bridge(nullptr);
    std::puts("PASS: stereo PCM byte continuity, 256-frame mapped sample ABI, service/resume, immediate/stale callbacks, failed open, mixer errors, self-drain rejection and context retirement after in-flight stop");
}

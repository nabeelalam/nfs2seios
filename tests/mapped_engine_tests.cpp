#include "GameMemory.h"
#include "GuestAccess.h"
#include "Game.h"
#include "BSS.h"
#include "DATA.h"
#include "FunctionIds.h"
#include "HeapArena.h"
#include "MemoryWrappers.h"
#include "SystemWrappers.h"
#include "EventBridge.h"
#include "CriticalSectionBridge.h"
#include "ThreadBridge.h"
#include "GameContexts.h"
#include "TimerBridge.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include "ExitCallbacks.h"
#include "RendererSurface.h"
#undef true
#undef false
#include "AudioBridge.h"
#include "InputAxes.h"
#include "InputDevices.h"

static nfs::GameMemory address_space;
static nfs::HeapArena heap;
alignas(16) static uint8_t heap_storage[8*1024*1024];
static nfs::Functions functions;
static uintptr_t timer_token=0;
static unsigned graphics_shutdowns=0;
static uintptr_t audio_token=0;static int audio_stream;static unsigned audio_fades=0;
extern "C" void *nfs_native_audio_open(uintptr_t token){audio_token=token;return &audio_stream;}
extern "C" bool nfs_native_audio_resume(void *p){return p==&audio_stream;}
extern "C" bool nfs_native_audio_pause(void *p){return p==&audio_stream;}
extern "C" void nfs_native_audio_close(void *){}
// Non-drawing boundaries are witnesses; the hardware record is production code.
extern "C" int32_t grGlideInit(){return 0;}
extern "C" int32_t grSstSelect(int32_t){return 0;}
extern "C" int32_t grSstQueryHardware(int32_t output){return nfs_glide_query_hardware(static_cast<GrHwConfiguration *>(nfs_host_address(uint32_t(output),sizeof(GrHwConfiguration))));}
extern "C" int32_t grGlideShutdown(){++graphics_shutdowns;nfs_glide_lfb_reset();return 0;}
extern "C" uint64_t nfs_native_ticks(){return 0;}
extern "C" uint32_t nfs_native_add_timer(uintptr_t token){timer_token=token;return 1;}
extern "C" void nfs_native_remove_timer(uint32_t){}
extern "C" bool verify_original_relocations();
extern "C" void *nfs_host_address(uint32_t address, size_t length) {
    if (address>=0x10000000 && address<0x10800000 && !heap.contains(address,length))
        throw std::out_of_range("Unallocated game heap span");
    void *host = address_space.resolve(address, length);
    if (!host) throw std::out_of_range("Unmapped game address/span");
    return host;
}
extern "C" uint32_t nfs_guest_address(const void *host) {
    uint32_t guest = 0;
    if (!address_space.encode(host, guest)) throw std::out_of_range("Unregistered native pointer");
    return guest;
}
extern "C" NativeGameFunction nfs_lookup_guest_function(uint32_t id) {
    const auto fn = functions.find(id);
    if (!fn) throw std::out_of_range("Unregistered game function ID");
    return fn;
}
extern "C" bool nfs_register_guest_function(uint32_t id,NativeGameFunction function){return functions.add(id,function);}
static unsigned input_enumerations=0;
static void original_input_enumerate_thunk(void *context){static_cast<Game *>(context)->_sub_4217F0();}
static void input_enumerate_thunk(void *context){
    auto &game=*static_cast<Game *>(context);const auto esp=uint32_t(game.esp);
    const auto instance=guest_load<uint32_t>(esp+4);
    if(guest_load<uint32_t>(esp)!=0 || guest_load<uint32_t>(esp+8)!=0x1234 || guest_load<uint32_t>(instance)!=580 || guest_load<uint32_t>(instance+8)!=input_enumerations)throw std::runtime_error("Original input enumeration arguments");
    ++input_enumerations;game.eax=1;game.esp+=8;
}
static void upper_thunk(void *game) { auto *context=static_cast<Game *>(game); if(uint32_t(context->esp)!=nfs_guest_address(context->stack)+(1<<20)-12 || guest_load<uint32_t>(uint32_t(context->esp)+4)!=0 || guest_load<uint32_t>(uint32_t(context->esp)+8)!=game_id_toupper_) throw std::runtime_error("Invalid original worker frame"); context->eax='z';context->_toupper_();if(context->eax!='Z') throw std::runtime_error("Worker game callback failed"); }
static void window_thunk(void *context){
    auto &game=*static_cast<Game *>(context);const uint32_t esp=uint32_t(game.esp);
    if(guest_load<uint32_t>(esp)!=0 || guest_load<uint32_t>(esp+4)!=0xb1000001u ||
       guest_load<uint32_t>(esp+8)!=0x401 || guest_load<uint32_t>(esp+12)!=UINT32_MAX ||
       guest_load<uint32_t>(esp+16)!=0x81234567u)throw std::runtime_error("Invalid original window callback frame");
    game.eax=321;game.esp+=16;
}
static void data_write_thunk(void *game) { static_cast<Game *>(game)->_sub_401080(); }
static void graphics_shutdown_thunk(void *game){static_cast<Game *>(game)->_sub_4972BC();}
static void audio_sample_thunk(void *context){
    auto &game=*static_cast<Game *>(context);
    if(game.edx!=256 || uint32_t(game.esp)!=nfs_guest_address(game.stack)+(1<<20)-4 || guest_load<uint32_t>(game.esp)!=0)throw std::runtime_error("Audio sample call frame");
    for(unsigned i=0;i<512;++i)guest_store<int16_t>(uint32_t(game.eax)+i*2,int16_t(i));
}
static void audio_fade_thunk(void *){++audio_fades;}
extern "C" void *malloc32(size_t size) {
    return heap.allocate(size);
}
extern "C" void free32(void *host) { if (!heap.release(host)) throw std::out_of_range("Invalid heap allocation"); }
extern "C" uint32_t nfs_native_page_size() { return 16384; }
extern "C" uint32_t nfs_game_heap_capacity() { return uint32_t(heap.total_bytes()); }
extern "C" uint32_t nfs_game_heap_available() { return uint32_t(heap.available_bytes()); }
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #x); return 1; } } while (0)
int main() {
    CHECK(sizeof(void *) == 8);
    CHECK(address_space.map(0x00400000, reinterpret_cast<uint8_t *>(&_data), sizeof(_data)));
    CHECK(address_space.map(0x02000000, reinterpret_cast<uint8_t *>(&_bss), sizeof(_bss)));
    CHECK(verify_original_relocations());
    CHECK(reinterpret_cast<uintptr_t>(&_data) > UINT32_MAX);
    CHECK(Application::to32i(off_409E10) == int32_t(nfs_guest_address(aTrk6)));
    CHECK(Application::to32i(off_409E10 + 4) == int32_t(nfs_guest_address(aTrk5)));
    CHECK(std::strcmp(static_cast<const char *>(nfs_host_address(uint32_t(Application::to32i(off_409E10)), 5)), "trk6") == 0);
    CHECK(functions.add(game_id_sub_401080, data_write_thunk));
    CHECK(heap.initialize(address_space,0x10000000,heap_storage,sizeof(heap_storage)));
    uint8_t a[128]{}, b[128]{};
    CHECK(reinterpret_cast<uintptr_t>(a) > UINT32_MAX);
    CHECK(address_space.map(0x80000000, a, sizeof(a)));
    CHECK(address_space.map(0x90000000, b, sizeof(b)));
    Game game;
    const int32_t initial_stack = game.esp;
    {
        nfs::InputDevices devices(nfs_dispatch_game_input);nfs::bind_input_devices(&devices);
        CHECK(nfs_register_input_functions());CHECK(functions.add(0xffff0008u,input_enumerate_thunk));
        CHECK(DirectInputCreateA_wrap(0,0,0x500,int32_t(0x80000003),0)==0);
        const auto root=guest_load<uint32_t>(0x80000003);
        game.push32(0);game.push32(0x1234);game.push32(int32_t(0xffff0008u));game.push32(4);game.push32(int32_t(root));
        game.call(guest_load<uint32_t>(guest_load<uint32_t>(root)+16));
        CHECK(game.eax==0 && input_enumerations==2 && game.esp==initial_stack);
        CHECK(functions.add(game_id_sub_4217F0,original_input_enumerate_thunk));
        guest_store<int32_t>(dword_563D54,int32_t(0xb1000001u));guest_store<int32_t>(dword_4D4C5C,0);guest_store<int32_t>(dword_4D4C58,0);
        game.push32(0);game.push32(int32_t(root));game.push32(int32_t(game_id_sub_4217F0));game.push32(4);game.push32(int32_t(root));
        game.call(nfs::input_function_id(nfs::InputMethod::enumerate));
        CHECK(game.eax==0 && game.esp==initial_stack && guest_load<int32_t>(dword_4D4C5C)==2);
        for(unsigned slot=0;slot<2;++slot){
            const auto original_device=guest_load<uint32_t>(dword_4E7348+4*slot);CHECK(original_device);
            game.push32(int32_t(original_device));game.call(nfs::input_function_id(nfs::InputMethod::release));CHECK(game.eax==0 && game.esp==initial_stack);
            guest_store<uint32_t>(dword_4E7348+4*slot,0);
        }
        guest_store<int32_t>(dword_4D4C5C,0);guest_store<int32_t>(dword_563D54,0);
        guest_store(0x80000020,std::array<uint32_t,4>{0x6f1d2b70,0,0,0});
        game.push32(0);game.push32(int32_t(0x80000043));game.push32(int32_t(0x80000020));game.push32(int32_t(root));
        game.call(guest_load<uint32_t>(guest_load<uint32_t>(root)+12));
        CHECK(game.eax==0 && game.esp==initial_stack);const auto device=guest_load<uint32_t>(0x80000043);
        nfs::GameJoystickState state;state.axes[0]=65535;state.buttons[0]=0x80;devices.publish(0,&state);
        game.push32(int32_t(0x90000003));game.push32(80);game.push32(int32_t(device));
        game.call(guest_load<uint32_t>(guest_load<uint32_t>(device)+36));
        CHECK(game.eax==0 && game.esp==initial_stack && guest_load<nfs::GameJoystickState>(0x90000003).axes[0]==65535);
        game.push32(int32_t(device));game.call(nfs::input_function_id(nfs::InputMethod::release));CHECK(game.eax==0 && game.esp==initial_stack);
        game.push32(int32_t(root));game.call(nfs::input_function_id(nfs::InputMethod::release));CHECK(game.eax==0 && game.esp==initial_stack);
        nfs::bind_input_devices(nullptr);
    }
    guest_store<int32_t>(dword_557540,0x4000);
    game.eax=0x684e;game.ebx=123;game.ecx=456;game.edx=789;
    game._sub_477850();
    CHECK(game.eax==160 && game.ebx==123 && game.ecx==456 && game.edx==789 && game.esp==initial_stack);
    nfs::set_game_axis_deadzone(true);game.eax=0x684e;game._sub_477850();
    CHECK(game.eax==131 && game.esp==initial_stack);
    nfs::set_game_axis_deadzone(false);
    game.call(game_id_sub_401080); // ID dispatch to original translated DATA write.
    CHECK(game.esp == initial_stack);
    CHECK(Application::to32i(dword_4D1FEC) == 0x15e);
    for (size_t offset = 0; offset < 8; ++offset) {
        for (size_t length = 0; length < 96; ++length) {
            std::memset(a, 0xcc, sizeof(a));
            game.eax = int32_t(0x80000000u + offset); game.edx = 0x9b; game.ebx = int32_t(length);
            game._memset_();
            CHECK(game.esp == initial_stack);
            CHECK(uint32_t(game.eax) == 0x80000000u + offset);
            for (size_t i = 0; i < sizeof(a); ++i) CHECK(a[i] == (i >= offset && i < offset+length ? 0x9b : 0xcc));
        }
    }
    std::strcpy(reinterpret_cast<char *>(b + 1), "original game text");
    std::memset(a, 0xcc, sizeof(a));
    game.eax = int32_t(0x80000003u); game.edx = int32_t(0x90000001u); game.ebx = 32;
    game._strncpy_();
    CHECK(std::memcmp(a + 3, b + 1, 18) == 0);
    for (unsigned i = 21; i < 35; ++i) CHECK(a[i] == 0);
    CHECK(game.esp == initial_stack);
    game.eax = int32_t(0x80000003u); game.edx = int32_t(0x90000001u);
    game._strcmp_(); CHECK(game.eax == 0 && game.esp == initial_stack);
    b[1] = 'z';
    game.eax = int32_t(0x80000003u); game.edx = int32_t(0x90000001u);
    game._strcmp_(); CHECK(game.eax < 0 && game.esp == initial_stack);
    for (int ch = 0; ch < 256; ++ch) {
        game.eax = ch; game._toupper_();
        CHECK(game.eax == (ch >= 'a' && ch <= 'z' ? ch-32 : ch));
    }
    game.___CHP(); CHECK(game.esp == initial_stack);
    // Execute the original allocator's page rounding / system-info / calloc
    // path, rather than only testing replacement allocation exports.
    std::memset(a,0xcc,sizeof(a));
    GetSystemInfo_wrap(int32_t(0x80000003u));
    CHECK(a[2]==0xcc && a[39]==0xcc && guest_load<uint32_t>(0x80000007u)==16384);
    std::memset(a,0xcc,sizeof(a));
    GlobalMemoryStatus_wrap(int32_t(0x80000003u));
    CHECK(guest_load<uint32_t>(0x80000003u)==32 && a[2]==0xcc && a[35]==0xcc);
    guest_store<int32_t>(dword_4DD3A8,0);
    guest_store<uint32_t>(0x80000003u,17);
    game.eax=int32_t(0x80000003u);
    game.ebx=0x12345678; game.ecx=0x76543210;
    game._sub_49CAD4();
    CHECK(game.esp==initial_stack && Application::to32i(dword_4DD3A8)==16384);
    CHECK(game.ebx==0x12345678 && game.ecx==0x76543210);
    CHECK(guest_load<uint32_t>(0x80000003u)==16384);
    const uint32_t allocation=uint32_t(game.eax);
    CHECK(allocation && heap.contains(allocation,16384));
    auto *native=static_cast<uint8_t *>(nfs_host_address(allocation,16384));
    CHECK(reinterpret_cast<uintptr_t>(native)>UINT32_MAX);
    for (unsigned i=0;i<16384;++i) CHECK(native[i]==0);
    CHECK(free_wrap(int32_t(allocation))==0);
    // Exercise the original stack and register calling conventions through the
    // actual formatter adapter, including an unaligned 64-bit floating argument.
    std::strcpy(reinterpret_cast<char *>(b+1), "%s%02d.qfs");
    std::strcpy(reinterpret_cast<char *>(b+32), "trk");
    game.esp=initial_stack-64;
    const uint32_t format_stack=uint32_t(game.esp);
    guest_store<uint32_t>(format_stack+4,0x80000003u);
    guest_store<uint32_t>(format_stack+8,0x90000001u);
    guest_store<uint32_t>(format_stack+12,0x90000020u);
    guest_store<int32_t>(format_stack+16,6);
    game.ecx=0x12345678; game.edx=0x76543210;
    game._sprintf_();
    CHECK(game.eax==9 && game.ecx==0x12345678 && game.edx==0x76543210);
    CHECK(uint32_t(game.esp)==format_stack && std::strcmp(reinterpret_cast<char *>(a+3),"trk06.qfs")==0);
    std::strcpy(reinterpret_cast<char *>(b+1),"%5.2f");
    guest_store<double>(0x90000023u,3.125);
    game.eax=int32_t(0x80000003u); game.edx=int32_t(0x90000001u); game.ebx=int32_t(0x90000023u);
    game._vsprintf_();
    CHECK(game.eax==5 && game.ecx==0x12345678 && uint32_t(game.esp)==format_stack);
    CHECK(std::strcmp(reinterpret_cast<char *>(a+3)," 3.12")==0);
    game.esp=initial_stack;
    nfs::EventBridge events; nfs::bind_event_bridge(&events);
    game.ecx=0x12345678; game.edx=0x76543210;
    game._sub_489E0C();
    CHECK(game.eax && game.ecx==0x12345678 && game.edx==0x76543210 && game.esp==initial_stack);
    const uint32_t event_id=uint32_t(game.eax);
    CHECK(events.wait(&event_id,1,false,0)==258);
    game.edx=int32_t(event_id); game._sub_489E20();
    CHECK(game.eax==1 && game.ecx==0x12345678 && game.esp==initial_stack);
    CHECK(events.wait(&event_id,1,false,0)==0 && events.wait(&event_id,1,false,0)==258);
    CHECK(events.close(event_id)); nfs::bind_event_bridge(nullptr);
    nfs::CriticalSectionBridge sections; nfs::bind_critical_sections(&sections);
    InitializeCriticalSection_wrap(int32_t(0x80000003u));
    game.eax=int32_t(0x80000003u); game.edx=0x76543210; game.ecx=0x12345678;
    game._sub_489F48();
    CHECK(game.eax==1 && game.edx==0x76543210 && game.ecx==0x12345678 && game.esp==initial_stack);
    game.eax=int32_t(0x80000003u); game._sub_489F60();
    CHECK(game.eax==0 && game.edx==0x76543210 && game.ecx==0x12345678 && game.esp==initial_stack);
    DeleteCriticalSection_wrap(int32_t(0x80000003u)); nfs::bind_critical_sections(nullptr);
    // A real Game/CPU/stack allocation enters a registered translated routine
    // on a native worker and is released after the worker returns.
    CHECK(functions.add(game_id_toupper_,upper_thunk));
    const size_t free_before_worker=heap.available_bytes();
    Game *worker_context=nfs_create_game_context();CHECK(worker_context);
    CHECK(reinterpret_cast<uintptr_t>(worker_context)>UINT32_MAX);
    nfs::ThreadBridge workers; nfs::bind_thread_bridge(&workers);
    const int32_t worker_handle=CreateThread_wrap(0,0,int32_t(game_id_toupper_),int32_t(nfs_guest_address(worker_context)),4,int32_t(0x80000003u));
    CHECK(worker_handle && guest_load<uint32_t>(0x80000003u));
    CHECK(ResumeThread_wrap(worker_handle)==1);
    workers.join_all();nfs::bind_thread_bridge(nullptr);
    CHECK(heap.available_bytes()==free_before_worker);
    // Exercise production four-argument window marshalling on the real CPU stack.
    CHECK(functions.add(0xffff0004u,window_thunk));
    CHECK(nfs_dispatch_game_window(&game,0xffff0004u,0xb1000001u,0x401,UINT32_MAX,0x81234567u)==321 && game.esp==initial_stack);
    bool bad_window_function=false;try{nfs_dispatch_game_window(&game,1,0,0,0,0);}catch(const std::out_of_range &){bad_window_function=true;}
    CHECK(bad_window_function && game.esp==initial_stack);
    void *reservation=heap.allocate(heap.available_bytes()-4096);CHECK(reservation);
    const size_t before_failed_context=heap.available_bytes();bool allocation_failed=false;
    try {nfs_create_game_context();} catch(const std::bad_alloc &) {allocation_failed=true;}
    CHECK(allocation_failed && heap.available_bytes()==before_failed_context);
    free32(reservation);CHECK(heap.available_bytes()==free_before_worker);
    nfs::ThreadBridge startup_threads;nfs::bind_thread_bridge(&startup_threads);
    guest_store<int32_t>(dword_564268,0);guest_store<int32_t>(dword_4DB6A8,0);
    game.ecx=0x12345678;game.edx=0x76543210;
    const size_t free_before_pool=heap.available_bytes();
    game._sub_489A88();
    CHECK(game.esp==initial_stack && game.ecx==0x12345678 && game.edx==0x76543210);
    CHECK(Application::to32i(dword_564268)==1 && uint32_t(Application::to32i(dword_564264))==uint32_t(GetCurrentThreadId_wrap()));
    const uint32_t main_handle=uint32_t(Application::to32i(TargetHandle));
    CHECK(main_handle && startup_threads.duplicate(main_handle));
    // The original initializer links 32 records at 24-byte strides in a
    // page-rounded calloc block. Check the whole list and FREE markers.
    uint32_t record=uint32_t(Application::to32i(dword_4DB6A8)),pool_low=UINT32_MAX;
    for(int i=0;i<32;++i){CHECK(record && heap.contains(record,24));pool_low=record<pool_low?record:pool_low;CHECK(guest_load<uint32_t>(record+4)==0x46524545u);record=guest_load<uint32_t>(record);}
    CHECK(!record && free_before_pool-heap.available_bytes()==16384);
    startup_threads.join_all();nfs::bind_thread_bridge(nullptr);
    free32(nfs_host_address(pool_low,24));CHECK(heap.available_bytes()==free_before_pool);
    nfs::EventBridge timer_events;nfs::bind_event_bridge(&timer_events);
    const uint32_t wake1=timer_events.create(false,false),wake2=timer_events.create(false,false),wake3=timer_events.create(false,false);
    guest_store<uint32_t>(dword_5637A0,655360);guest_store<uint32_t>(dword_5637CC,wake1);
    guest_store<uint32_t>(dword_5637D8,wake2);guest_store<uint32_t>(dword_4DDA70,wake3);guest_store<uint32_t>(dword_4DB1B0,1);
    CHECK(startTimer()==0 && nfs_timer_dispatch(timer_token)==1);
    CHECK(timer_events.wait(&wake1,1,false,0)==0 && timer_events.wait(&wake2,1,false,0)==258 && timer_events.wait(&wake3,1,false,0)==0);
    guest_store<uint32_t>(dword_4DB1B0,0);CHECK(nfs_timer_dispatch(timer_token)==1);
    CHECK(timer_events.wait(&wake2,1,false,0)==0);CHECK(stopTimer()==0 && !nfs_timer_dispatch(timer_token));
    guest_store<uint32_t>(dword_5637CC,0);guest_store<uint32_t>(dword_5637D8,0);guest_store<uint32_t>(dword_4DDA70,0);
    timer_events.shutdown();nfs::bind_event_bridge(nullptr);
    nfs::ExitCallbacks graphics_exit;nfs::bind_exit_callbacks(&graphics_exit);
    CHECK(functions.add(game_id_sub_4972BC,graphics_shutdown_thunk));
    guest_store<int32_t>(dword_4DCFE4,0);
    game._sub_497244();
    CHECK(game.eax==1 && game.esp==initial_stack && guest_load<int32_t>(dword_56EEA4)==1 && guest_load<int32_t>(dword_4DCFE4)==1);
    CHECK(guest_load<float>(flt_56E9DC+4)==0.0f && guest_load<float>(flt_56E9DC+1024)==255.0f);
    nfs::WorkerControl shutdown_control;shutdown_control.stop=true;nfs::current_worker_control()=&shutdown_control;
    graphics_exit.drain(&game);CHECK(nfs::worker_stop_ready());nfs::current_worker_control()=nullptr;
    CHECK(graphics_shutdowns==1 && guest_load<int32_t>(dword_4DCFE4)==0 && game.esp==initial_stack);
    graphics_exit.drain(&game);CHECK(graphics_shutdowns==1);nfs::bind_exit_callbacks(nullptr);
    CHECK(functions.add(game_id_sub_4A7CBC,audio_sample_thunk) && functions.add(game_id_sub_4A5170,audio_fade_thunk));
    nfs::AudioBridge audio;nfs::bind_audio_bridge(&audio);guest_store<int32_t>(dword_4DB67C,0);
    game.ecx=0x12345678;game.edx=0x76543210;game.ebx=0x23456789;game._sub_488718();
    CHECK(game.eax==0 && game.ecx==0x12345678 && game.edx==0x76543210 && game.ebx==0x23456789 && game.esp==initial_stack && guest_load<int32_t>(dword_4DB67C)==1);
    const size_t free_before_audio=heap.available_bytes();CHECK(audio.start());audio.serve(&game);
    int16_t pcm[512]{};CHECK(nfs_audio_render(audio_token,pcm,sizeof(pcm))==sizeof(pcm));
    for(unsigned i=0;i<512;++i)CHECK(pcm[i]==int16_t(i));
    audio.stop();CHECK(audio_fades==1 && game.esp==initial_stack && heap.available_bytes()==free_before_audio);nfs::bind_audio_bridge(nullptr);
    bool rejected = false;
    try { Application::to32i(int32_t(0x8000007fu)); } catch (const std::out_of_range &) { rejected = true; }
    CHECK(rejected);
    rejected = false;
    try { game.call(0xf0000001); } catch (const std::out_of_range &) { rejected = true; }
    CHECK(rejected && game.esp == initial_stack);
    std::printf("PASS: 1783 DATA relocations, function-ID dispatch, real DATA write, 768 memset cases, unaligned strings, FPU stack, original page-rounded allocation, formatter conventions and original event/critical-section routines and native Game worker lifetime and original synchronization startup, timer event fanout, graphics capability startup, original exit callback and sound callback registration/mapped stereo PCM; native data %p >4GB\n", static_cast<void *>(a));
}

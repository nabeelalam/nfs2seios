#include <stdint.h>
extern "C" uint64_t nfs_glide_palette_upload_count(void);
// SPDX-License-Identifier: MIT
#include "GuestGlide.h"
#include "RendererSurface.h"
#undef true
#undef false
#include "GuestAccess.h"
#include "HeapArena.h"
#include "WindowBridge.h"
#include "MainThreadBridge.h"
#include "GraphicsLifecycle.h"
#include "FramePresentation.h"
#include "HdAtlas.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengles2.h>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <exception>
#include <future>
#include <vector>
extern "C" {
int32_t keepAspectRatio=1,needRecreateGl=0,windowResized=0,linearFiltering=0,fixedFramebufferSize=1,framebufferLinearFiltering=1;
int32_t vSync=0,winWidth=640,winHeight=480,initialWinWidth=640,initialWinHeight=480;
float dpr=1;
SDL_Window *sdlWin=nullptr;
}
static nfs::GameMemory memory;static nfs::HeapArena heap;
alignas(16) static uint8_t storage[4*1024*1024];
extern "C" void *malloc32(size_t size){return heap.allocate(size);}
extern "C" void free32(void *p){if(!heap.release(p))throw std::runtime_error("Bad GPU surface free");}
extern "C" void *nfs_host_address(uint32_t id,size_t size){auto *p=memory.resolve(id,size);if(!p)throw std::out_of_range("GPU guest span");return p;}
extern "C" uint32_t nfs_guest_address(const void *p){uint32_t id;if(!memory.encode(p,id))throw std::out_of_range("GPU guest pointer");return id;}
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %d: %s SDL=%s GL=0x%x\n",__LINE__,#x,SDL_GetError(),glGetError());return 1;}}while(0)
static GrLfbInfo_t lock(uint32_t address,int type,int buffer,int origin){
    GrLfbInfo_t info{};info.size=20;guest_store(address,info);
    if(!grLfbLock(type,buffer,0,origin,0,int32_t(address)))throw std::runtime_error("GPU LFB lock failed");
    return guest_load<GrLfbInfo_t>(address);
}
int main(int argc,char **argv){
    alignas(16) uint8_t records[256];std::memset(records,0xcc,sizeof(records));
    CHECK(memory.map(0x80000000,records,sizeof(records)) && heap.initialize(memory,0x10000000,storage,sizeof(storage)));
    CHECK(reinterpret_cast<uintptr_t>(storage)>UINT32_MAX && SDL_Init(SDL_INIT_VIDEO));
    nfs::MainThreadBridge main(nfs::sdl_main_thread_scheduler());nfs::bind_main_thread_bridge(&main);
    auto operations=nfs::sdl_window_operations();sdlWin=static_cast<SDL_Window *>(operations.create());CHECK(sdlWin);
    CHECK(grSstQueryHardware(int32_t(0x80000003u))==1 && guest_load<GrHwConfiguration>(0x80000003u).num_sst==1);
    CHECK(records[2]==0xcc && records[3+sizeof(GrHwConfiguration)]==0xcc);
    CHECK(grSstWinOpen(1,0,0,0,0,2,1)==1);
    std::printf("GLES renderer: %s / %s\n",glGetString(GL_RENDERER),glGetString(GL_VERSION));
    grGammaCorrectionValue(1.0f);grRenderBuffer(GR_BUFFER_BACKBUFFER);grBufferClear(0x0000ff,255,65535);
    const auto before=heap.available_bytes();
    auto info=lock(0x800000a3u,GR_LFB_WRITE_ONLY,GR_BUFFER_BACKBUFFER,GR_ORIGIN_UPPER_LEFT);
    CHECK(info.size==20 && info.strideInBytes==1280 && info.writeMode==0);
    auto *pixels=static_cast<uint16_t *>(nfs_host_address(info.lfbPtr,NFS_SURFACE_BYTES));
    const uint16_t untouched=pixels[640*200+200];CHECK(untouched!=0);
    for(int y=0;y<480;++y){pixels[y*640]=0xf800;pixels[y*640+639]=0x07e0;}
    pixels[0]=0x001f;pixels[479*640]=0xffff;
    CHECK(grLfbUnlock(1,1)==1 && heap.available_bytes()==before);
    info=lock(0x800000a3u,0,1,0);pixels=static_cast<uint16_t *>(nfs_host_address(info.lfbPtr,NFS_SURFACE_BYTES));
    CHECK(pixels[0]==0x001f && pixels[479*640]==0xffff && pixels[640*50]==0xf800 && pixels[640*50+639]==0x07e0 && pixels[640*200+200]==untouched);
    CHECK(grLfbUnlock(0,1)==1);
    grBufferSwap(0);
    for(int buffer=0;buffer<2;++buffer){
        grRenderBuffer(buffer);GLint texture=0,filter=0,previous=0;
        glGetIntegerv(GL_TEXTURE_BINDING_2D,&previous);
        glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&texture);
        glBindTexture(GL_TEXTURE_2D,GLuint(texture));glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,&filter);
        glBindTexture(GL_TEXTURE_2D,GLuint(previous));CHECK(filter==GL_LINEAR);
    }
    grRenderBuffer(1);
    info=lock(0x800000a3u,0,0,1);pixels=static_cast<uint16_t *>(nfs_host_address(info.lfbPtr,NFS_SURFACE_BYTES));
    CHECK(pixels[0]==0xffff && pixels[479*640]==0x001f);CHECK(grLfbUnlock(0,0)==1);
    // Front writes present without exchanging or overwriting the back surface.
    grRenderBuffer(1);grBufferClear(0x00ff00,255,65535);
    info=lock(0x800000a3u,1,0,0);pixels=static_cast<uint16_t *>(nfs_host_address(info.lfbPtr,NFS_SURFACE_BYTES));pixels[640*100+100]=0xffe0;
    CHECK(grLfbUnlock(1,0)==1);
    info=lock(0x800000a3u,0,0,0);pixels=static_cast<uint16_t *>(nfs_host_address(info.lfbPtr,NFS_SURFACE_BYTES));CHECK(pixels[640*100+100]==0xffe0);CHECK(grLfbUnlock(0,0)==1);
    info=lock(0x800000a3u,0,1,0);pixels=static_cast<uint16_t *>(nfs_host_address(info.lfbPtr,NFS_SURFACE_BYTES));
    if(pixels[640*100+100]!=0x07e0)std::fprintf(stderr,"Back surface pixel: expected=0x07e0 actual=0x%04x\n",pixels[640*100+100]);
    CHECK(pixels[640*100+100]==0x07e0);CHECK(grLfbUnlock(0,1)==1);
    nfs::GraphicsLifecycle lifecycle;const auto original_context=SDL_GL_GetCurrentContext();CHECK(original_context);
    lifecycle.set_suspended(true);CHECK(SDL_GL_GetCurrentContext()==nullptr);
    bool rejected=false;try{grBufferClear(0,255,65535);}catch(const std::logic_error &){rejected=true;}CHECK(rejected);
    lifecycle.set_suspended(false);CHECK(SDL_GL_GetCurrentContext()==original_context);
    info=lock(0x800000a3u,0,0,0);pixels=static_cast<uint16_t *>(nfs_host_address(info.lfbPtr,NFS_SURFACE_BYTES));CHECK(pixels[640*100+100]==0xffe0);CHECK(grLfbUnlock(0,0)==1);
    info=lock(0x800000a3u,0,1,0);pixels=static_cast<uint16_t *>(nfs_host_address(info.lfbPtr,NFS_SURFACE_BYTES));CHECK(pixels[640*100+100]==0x07e0);CHECK(grLfbUnlock(0,1)==1);
    CHECK(glGetError()==GL_NO_ERROR && heap.available_bytes()==before);

    void *palette_memory=malloc32(2048),*index_memory=malloc32(16);CHECK(palette_memory && index_memory);
    const uint32_t palette_a=nfs_guest_address(palette_memory),palette_b=palette_a+1024,index=nfs_guest_address(index_memory);
    GuTexPalette red{},blue{},green{},yellow{};
    red.data[1]=0xffff0000;blue.data[1]=0xff0000ff;green.data[1]=0xff00ff00;yellow.data[1]=0xffffff00;
    guest_store(palette_a,green);guest_store(palette_b,blue);guest_store<uint8_t>(index,1);
    void *temporary_palette=malloc32(1024);CHECK(temporary_palette);
    const auto temporary_id=nfs_guest_address(temporary_palette);guest_store(temporary_id,red);
    GrTexInfo texture{};texture.largeLod=texture.smallLod=8;texture.format=GR_TEXFMT_P_8;texture.data=index;
    const uint32_t metadata=0x800000c3u;guest_store(metadata,texture);
    grTexDownloadMipMap(0,0,3,int32_t(metadata));
    const auto pixel=[&](unsigned r,unsigned g,unsigned b,int px=0,int py=0){
        GLint texture_id=0,previous=0;GLuint frame=0;uint8_t color[4]{};
        glGetIntegerv(GL_TEXTURE_BINDING_2D,&texture_id);glGetIntegerv(GL_FRAMEBUFFER_BINDING,&previous);
        glGenFramebuffers(1,&frame);glBindFramebuffer(GL_FRAMEBUFFER,frame);
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,GLuint(texture_id),0);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Palette texture framebuffer incomplete");
        glReadPixels(px,py,1,1,GL_RGBA,GL_UNSIGNED_BYTE,color);
        glBindFramebuffer(GL_FRAMEBUFFER,GLuint(previous));glDeleteFramebuffers(1,&frame);
        if(color[0]!=r || color[1]!=g || color[2]!=b || color[3]!=255){std::fprintf(stderr,"Palette actual RGB=%u,%u,%u expected=%u,%u,%u\n",color[0],color[1],color[2],r,g,b);throw std::runtime_error("Palette pixel mismatch");}
    };
    std::atomic<bool> done{false};std::exception_ptr failure;
    std::thread producer([&]{try{
        grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(temporary_id));
        guest_store(temporary_id,green); // Snapshot must outlive/ignore later guest mutation.
        free32(temporary_palette); // Guest lifetime ends before any queued native read.
        grTexSource(0,0,3,int32_t(metadata));nfs::flush_renderer_commands();
    }catch(...){failure=std::current_exception();}done=true;});
    while(!done){SDL_PumpEvents();SDL_Delay(1);}producer.join();if(failure)std::rethrow_exception(failure);
    pixel(255,0,0);
    grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_b));grTexSource(0,0,3,int32_t(metadata));pixel(0,0,255);
    grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_a));grTexSource(0,0,3,int32_t(metadata));pixel(0,255,0);
    // Explicit same-address palette downloads must update changed colours.
    guest_store(palette_a,yellow);grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_a));
    grTexSource(0,0,3,int32_t(metadata));pixel(255,255,0);
    const auto unchanged_uploads=nfs_glide_palette_upload_count();
    grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_a));grTexSource(0,0,3,int32_t(metadata));pixel(255,255,0);
    CHECK(nfs_glide_palette_upload_count()==unchanged_uploads);
    guest_store(palette_a,green); // Mutating caller data without download cannot update GPU table.
    grTexSource(0,0,3,int32_t(metadata));pixel(255,255,0);
    CHECK(nfs_glide_palette_upload_count()==unchanged_uploads);
    grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_a));grTexSource(0,0,3,int32_t(metadata));pixel(0,255,0);
    CHECK(nfs_glide_palette_upload_count()==unchanged_uploads+1);
    guest_store(palette_a,yellow);grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_a));
    grTexDownloadMipMap(0,0,3,int32_t(metadata));grTexSource(0,0,3,int32_t(metadata));pixel(255,255,0);
    // Global renderer state follows FIFO admission across producers: A's
    // texture-source operation must observe B's intervening palette command.
    guest_store(palette_a,red);grTexDownloadMipMap(0,0,3,int32_t(metadata));
    std::promise<void> admitted_a,admitted_b;
    auto gate_a=admitted_a.get_future().share(),gate_b=admitted_b.get_future().share();
    std::atomic<unsigned> finished{0};std::exception_ptr failure_a,failure_b;
    std::thread producer_a([&]{try{
        grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_a));admitted_a.set_value();gate_b.get();
        grTexSource(0,0,3,int32_t(metadata));nfs::flush_renderer_commands();
    }catch(...){failure_a=std::current_exception();try{admitted_a.set_exception(failure_a);}catch(...){}}++finished;});
    std::thread producer_b([&]{try{
        gate_a.get();grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_b));admitted_b.set_value();
    }catch(...){failure_b=std::current_exception();try{admitted_b.set_exception(failure_b);}catch(...){}}++finished;});
    while(finished!=2){SDL_PumpEvents();SDL_Delay(1);}producer_a.join();producer_b.join();
    if(failure_a)std::rethrow_exception(failure_a);if(failure_b)std::rethrow_exception(failure_b);pixel(0,0,255);
    // Separate textures retain unchanged palette generations across A/B/A/B.
    guest_store(palette_a,red);grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_a));grTexSource(0,0,3,int32_t(metadata));pixel(255,0,0);
    const auto two_texture_uploads=nfs_glide_palette_upload_count();
    grTexDownloadMipMap(0,1024,3,int32_t(metadata));grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_b));grTexSource(0,1024,3,int32_t(metadata));pixel(0,0,255);
    CHECK(nfs_glide_palette_upload_count()==two_texture_uploads+1);
    grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_a));grTexSource(0,0,3,int32_t(metadata));pixel(255,0,0);
    grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_b));grTexSource(0,1024,3,int32_t(metadata));pixel(0,0,255);
    CHECK(nfs_glide_palette_upload_count()==two_texture_uploads+1);
    // The public bridge accepts 256x256 P8 textures (largeLod zero).
    void *full_indices=malloc32(256*256);CHECK(full_indices);
    std::memset(full_indices,1,256*256);static_cast<uint8_t *>(full_indices)[256*256-1]=2;
    blue.data[2]=0xffff0000;guest_store(palette_b,blue);
    texture.largeLod=texture.smallLod=0;texture.data=nfs_guest_address(full_indices);guest_store(metadata,texture);
    grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_b));grTexDownloadMipMap(0,0,3,int32_t(metadata));grTexSource(0,0,3,int32_t(metadata));
    pixel(0,0,255);pixel(255,0,0,255,255);free32(full_indices);
    // Bounded-cache eviction cannot reuse a generation from a retired record.
    void *eviction_tables[129]{};
    for(unsigned i=0;i<129;++i){eviction_tables[i]=malloc32(sizeof(GuTexPalette));CHECK(eviction_tables[i]);guest_store(nfs_guest_address(eviction_tables[i]),green);grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(nfs_guest_address(eviction_tables[i])));}
    const auto before_eviction_reload=nfs_glide_palette_upload_count();
    grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_a));grTexSource(0,0,3,int32_t(metadata));pixel(255,0,0);
    CHECK(nfs_glide_palette_upload_count()==before_eviction_reload+1);
    // Native context recreation must restore texture data and invalidate the
    // GPU palette conversion even when submitted palette bytes are unchanged.
    const auto uploads_before_recreate=nfs_glide_palette_upload_count();
    needRecreateGl=1;grBufferSwap(0);CHECK(needRecreateGl==0 && glGetError()==GL_NO_ERROR);
    grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(palette_a));grTexSource(0,0,3,int32_t(metadata));pixel(255,0,0);
    CHECK(nfs_glide_palette_upload_count()==uploads_before_recreate+1 && glGetError()==GL_NO_ERROR);
    std::puts("PASS: native GLES context destroy/recreate restores texture and invalidates palette conversion");
    // Resolution changes reallocate both buffers at swap, while original
    // RGB565 readback coordinates remain 640x480 at every render density.
    for(int height:{720,960,480}){
        fixedFramebufferSize=1;initialWinWidth=height*4/3;initialWinHeight=height;windowResized=1;
        grBufferSwap(0);CHECK(nfs_render_width()==height*4/3 && nfs_render_height()==height);
        grBufferClear(0xff0000,255,0xffff);
        auto scaled=lock(0x80000003,GR_LFB_READ_ONLY,GR_BUFFER_BACKBUFFER,GR_ORIGIN_UPPER_LEFT);
        auto *pixels=static_cast<uint16_t *>(nfs_host_address(scaled.lfbPtr,NFS_SURFACE_BYTES));
        CHECK(pixels[0]==0xf800 && pixels[640*480-1]==0xf800);
        CHECK(grLfbUnlock(GR_LFB_READ_ONLY,GR_BUFFER_BACKBUFFER) && glGetError()==GL_NO_ERROR);
    }
    CHECK(SDL_SetWindowSize(sdlWin,2360,1640));SDL_PumpEvents();SDL_SyncWindow(sdlWin);SDL_PumpEvents();
    SDL_GetWindowSizeInPixels(sdlWin,&winWidth,&winHeight);
    fixedFramebufferSize=0;windowResized=1;grBufferSwap(0);
    std::printf("Native target %dx%d; drawable %dx%d\n",nfs_render_width(),nfs_render_height(),winWidth,winHeight);
    CHECK(nfs_render_height()==1640 && nfs_render_width()==2186 && glGetError()==GL_NO_ERROR);
    grBufferClear(0xff0000,255,0xffff);
    auto native_pixels=lock(0x80000003,GR_LFB_READ_ONLY,GR_BUFFER_BACKBUFFER,GR_ORIGIN_UPPER_LEFT);
    auto *native_readback=static_cast<uint16_t *>(nfs_host_address(native_pixels.lfbPtr,NFS_SURFACE_BYTES));
    CHECK(native_readback[0]==0xf800 && native_readback[640*480-1]==0xf800);
    CHECK(grLfbUnlock(GR_LFB_READ_ONLY,GR_BUFFER_BACKBUFFER));
    fixedFramebufferSize=1;initialWinWidth=640;initialWinHeight=480;windowResized=1;grBufferSwap(0);
    CHECK(nfs_render_width()==640 && nfs_render_height()==480 && glGetError()==GL_NO_ERROR);
    std::puts("PASS: 480/720/960/native-height render targets, both-buffer reallocation, original RGB565 readback and return to original resolution");
    // A synthetic two-pixel atlas becomes four pixels without changing CPU data.
    // No original game artwork is embedded in this fixture.
    std::vector<unsigned char> hd_pack(32+8+64,0);
    std::memcpy(hd_pack.data(),"NFSHDA1\0",8);
    auto hd_word=[&](unsigned at,uint32_t value){for(unsigned i=0;i<4;++i)hd_pack[at+i]=(value>>(8*i))&255;};
    hd_word(8,2);hd_word(12,4);hd_word(16,8);hd_word(20,64);
    for(unsigned i=0;i<4;++i){hd_pack[32+2*i]=1;hd_pack[33+2*i]=248;}
    for(unsigned i=0;i<16;++i){hd_pack[40+4*i+1]=255;hd_pack[40+4*i+3]=255;}
    hd_pack[40]=17;hd_pack[41]=33;hd_pack[42]=99;hd_pack[43]=0;
    const auto hd_path="hd-atlas-fixture-"+std::to_string(SDL_GetTicksNS())+".nfsatlas";
    FILE *hd_file=std::fopen(hd_path.c_str(),"wbx");CHECK(hd_file);
    CHECK(std::fwrite(hd_pack.data(),1,hd_pack.size(),hd_file)==hd_pack.size());CHECK(!std::fclose(hd_file));
    CHECK(nfs_glide_hd_atlas_load_file(hd_path.c_str()));CHECK(!std::remove(hd_path.c_str()));
    for(unsigned i=0;i<4;++i)guest_store<uint16_t>(index+2*i,0xfc00); // Original ARGB1555 red
    texture.largeLod=texture.smallLod=7;texture.format=GR_TEXFMT_ARGB_1555;texture.data=index;guest_store(metadata,texture);
    grTexDownloadMipMap(0,2048,3,int32_t(metadata));grTexSource(0,2048,3,int32_t(metadata));
    CHECK(nfs_glide_hd_atlas_upload_count()==1);pixel(0,255,0,3,3);
    // Exact straight RGBA including a transparent texel survives the upload.
    GLint hd_texture=0,hd_previous=0;GLuint hd_frame=0;unsigned char hd_rgba[4]{};
    glGetIntegerv(GL_TEXTURE_BINDING_2D,&hd_texture);glGetIntegerv(GL_FRAMEBUFFER_BINDING,&hd_previous);
    glGenFramebuffers(1,&hd_frame);glBindFramebuffer(GL_FRAMEBUFFER,hd_frame);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,GLuint(hd_texture),0);
    CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);
    glReadPixels(0,0,1,1,GL_RGBA,GL_UNSIGNED_BYTE,hd_rgba);
    CHECK(!std::memcmp(hd_rgba,hd_pack.data()+40,4));
    glBindFramebuffer(GL_FRAMEBUFFER,GLuint(hd_previous));glDeleteFramebuffers(1,&hd_frame);
    CHECK(guest_load<uint16_t>(index)==0xfc00);
    // Reusing the same address for different pixels must restore the original.
    for(unsigned i=0;i<4;++i)guest_store<uint16_t>(index+2*i,0x801f);
    grTexDownloadMipMap(0,2048,3,int32_t(metadata));grTexSource(0,2048,3,int32_t(metadata));pixel(0,0,255);
    CHECK(nfs_glide_hd_atlas_upload_count()==1);
    for(unsigned i=0;i<4;++i)guest_store<uint16_t>(index+2*i,0xfc00);
    grTexDownloadMipMap(0,2048,3,int32_t(metadata));grTexSource(0,2048,3,int32_t(metadata));pixel(0,255,0,3,3);
    CHECK(nfs_glide_hd_atlas_upload_count()==2);
    needRecreateGl=1;grBufferSwap(0);grTexSource(0,2048,3,int32_t(metadata));pixel(0,255,0,3,3);
    CHECK(nfs_glide_hd_atlas_upload_count()==3 && glGetError()==GL_NO_ERROR);
    // Missing optional data clears matching; a fresh download is ordinary red.
    CHECK(!nfs_glide_hd_atlas_load_file(hd_path.c_str()));
    grTexDownloadMipMap(0,2048,3,int32_t(metadata));grTexSource(0,2048,3,int32_t(metadata));pixel(255,0,0);
    nfs_glide_hd_atlas_clear();nfs_glide_hd_atlas_clear();
    std::puts("PASS: optional HD texture RGBA/dimensions, exact content identity, same-address fallback, CPU immutability and context recreation");
    if(argc==3){
        // Private original-data extension: ordinary CI never needs these files.
        NfsHdAtlas expected{};FILE *file=std::fopen(argv[2],"rb");CHECK(file);
        CHECK(nfs_hd_atlas_read(&expected,file));CHECK(!std::fclose(file));
        CHECK(expected.original_size==256 && expected.replacement_size==1024);
        void *raw_memory=malloc32(131072);CHECK(raw_memory);auto *raw=static_cast<unsigned char *>(raw_memory);
        file=std::fopen(argv[1],"rb");CHECK(file && std::fread(raw,1,131072,file)==131072 && std::fgetc(file)==EOF);CHECK(!std::fclose(file));
        CHECK(!std::memcmp(raw,expected.original,131072));
        // Feed original ARGB1555 through the real download conversion.
        for(unsigned i=0;i<65536;++i){uint16_t value;std::memcpy(&value,raw+2*i,2);value=(value>>1)|((value&1)<<15);std::memcpy(raw+2*i,&value,2);}
        texture.largeLod=texture.smallLod=0;texture.format=GR_TEXFMT_ARGB_1555;texture.data=nfs_guest_address(raw_memory);guest_store(metadata,texture);
        nfs_glide_hd_builtin_enable(1);grTexDownloadMipMap(0,1048576,3,int32_t(metadata));grTexSource(0,1048576,3,int32_t(metadata));
        CHECK(nfs_glide_hd_atlas_upload_count()==1);
        auto check_auto_pixels=[&](){
            GLint bound=0,previous=0;GLuint frame=0;std::vector<unsigned char> actual(4194304);
            glGetIntegerv(GL_TEXTURE_BINDING_2D,&bound);glGetIntegerv(GL_FRAMEBUFFER_BINDING,&previous);
            glGenFramebuffers(1,&frame);glBindFramebuffer(GL_FRAMEBUFFER,frame);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,GLuint(bound),0);
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Automatic atlas FBO incomplete");
            glReadPixels(0,0,1024,1024,GL_RGBA,GL_UNSIGNED_BYTE,actual.data());
            glBindFramebuffer(GL_FRAMEBUFFER,GLuint(previous));glDeleteFramebuffers(1,&frame);
            if(std::memcmp(actual.data(),expected.rgba,actual.size()))throw std::runtime_error("Automatic GPU pixels differ from accepted atlas");
        };
        check_auto_pixels();needRecreateGl=1;grBufferSwap(0);grTexSource(0,1048576,3,int32_t(metadata));
        CHECK(nfs_glide_hd_atlas_upload_count()==2);check_auto_pixels();
        nfs_glide_hd_builtin_enable(0);guest_store<uint16_t>(texture.data,0x801f);
        grTexDownloadMipMap(0,1048576,3,int32_t(metadata));grTexSource(0,1048576,3,int32_t(metadata));pixel(0,0,255);
        CHECK(nfs_glide_hd_atlas_upload_count()==0 && glGetError()==GL_NO_ERROR);
        nfs_glide_hd_atlas_clear();nfs_hd_atlas_free(&expected);free32(raw_memory);
        std::puts("PASS: automatic original-data GPU atlas byte identity, context restoration and disabled fallback without private pack loading");
    }
    for(auto table:eviction_tables)free32(table);
    free32(index_memory);free32(palette_memory);CHECK(heap.available_bytes()==before && glGetError()==GL_NO_ERROR);
    std::puts("PASS: owned palette snapshot, deferred guest mutation, generation A/B/A, same-address explicit updates, unchanged-content cache and texture invalidation RGB pixels");
    grGlideShutdown();operations.destroy(sdlWin);sdlWin=nullptr;nfs::bind_main_thread_bridge(nullptr);SDL_Quit();
    std::puts("PASS: actual GLES pixels, RGB565 partial writes/stride, row origins, front/back buffers, presentation, context suspend/restore with preserved pixels, guest canaries and cleanup");
}

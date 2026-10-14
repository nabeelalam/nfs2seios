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
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengles2.h>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <thread>
#include <atomic>
#include <exception>
#include <future>
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
int main(){
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
    for(auto table:eviction_tables)free32(table);
    free32(index_memory);free32(palette_memory);CHECK(heap.available_bytes()==before && glGetError()==GL_NO_ERROR);
    std::puts("PASS: owned palette snapshot, deferred guest mutation, generation A/B/A, same-address explicit updates, unchanged-content cache and texture invalidation RGB pixels");
    grGlideShutdown();operations.destroy(sdlWin);sdlWin=nullptr;nfs::bind_main_thread_bridge(nullptr);SDL_Quit();
    std::puts("PASS: actual GLES pixels, RGB565 partial writes/stride, row origins, front/back buffers, presentation, context suspend/restore with preserved pixels, guest canaries and cleanup");
}

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
    grGlideShutdown();operations.destroy(sdlWin);sdlWin=nullptr;nfs::bind_main_thread_bridge(nullptr);SDL_Quit();
    std::puts("PASS: actual GLES pixels, RGB565 partial writes/stride, row origins, front/back buffers, presentation, context suspend/restore with preserved pixels, guest canaries and cleanup");
}

#include <stdint.h>
extern "C" uint64_t nfs_glide_palette_upload_count(void);
extern "C" void native_grGlideShutdown(void);
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
#include "TextureDigest.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengles2.h>
#include <cstdio>
#include <cstdlib>
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
#ifdef NFS_TEST_GL_CALLS
static unsigned unbound_hd_gl_calls;
extern "C" void __real_glDeleteTextures(GLsizei,const GLuint *);
extern "C" void __real_glUniform1f(GLint,GLfloat);
extern "C" void __real_glActiveTexture(GLenum);
extern "C" void __wrap_glDeleteTextures(GLsizei n,const GLuint *ids){
    if(!SDL_GL_GetCurrentContext())++unbound_hd_gl_calls;
    __real_glDeleteTextures(n,ids);
}
extern "C" void __wrap_glUniform1f(GLint location,GLfloat value){
    if(!SDL_GL_GetCurrentContext())++unbound_hd_gl_calls;
    __real_glUniform1f(location,value);
}
extern "C" void __wrap_glActiveTexture(GLenum unit){
    if(!SDL_GL_GetCurrentContext())++unbound_hd_gl_calls;
    __real_glActiveTexture(unit);
}
#endif
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
    CHECK(nfs_glide_hd_timing_mode()==-1 && !nfs_glide_hd_timing_set_visible(1));
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
            GLint active=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
            if(active!=GL_TEXTURE0)throw std::runtime_error("HD upload did not restore unit zero");
            glActiveTexture(GL_TEXTURE1);
            glGetIntegerv(GL_TEXTURE_BINDING_2D,&bound);glGetIntegerv(GL_FRAMEBUFFER_BINDING,&previous);
            glActiveTexture(GL_TEXTURE0);
            glGenFramebuffers(1,&frame);glBindFramebuffer(GL_FRAMEBUFFER,frame);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,GLuint(bound),0);
            if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Automatic atlas FBO incomplete");
            glReadPixels(0,0,1024,1024,GL_RGBA,GL_UNSIGNED_BYTE,actual.data());
            glBindFramebuffer(GL_FRAMEBUFFER,GLuint(previous));glDeleteFramebuffers(1,&frame);
            if(std::memcmp(actual.data(),expected.rgba,actual.size()))throw std::runtime_error("Automatic GPU pixels differ from accepted atlas");
        };
        check_auto_pixels();needRecreateGl=1;grBufferSwap(0);grTexSource(0,1048576,3,int32_t(metadata));
        CHECK(nfs_glide_hd_atlas_upload_count()==2);check_auto_pixels();
        // Compare actual shader sampling at native size, not just uploaded bytes.
        void *quad_memory=malloc32(4*sizeof(GrVertex));CHECK(quad_memory);
        const uint32_t quad_address=nfs_guest_address(quad_memory);
        fixedFramebufferSize=1;initialWinWidth=2186;initialWinHeight=1640;windowResized=1;grBufferSwap(0);
        auto sampled_scene=[&](unsigned digit=0,unsigned speed=10){
            grBufferClear(0x19436b,255,65535);grClipWindow(0,0,640,480);
            grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);grFogMode(GR_FOG_DISABLE);
            grAlphaBlendFunction(GR_BLEND_ONE,GR_BLEND_ZERO,GR_BLEND_ONE,GR_BLEND_ZERO);
            grAlphaCombine(0,0,0,GR_COMBINE_OTHER_TEXTURE,0);
            grTexSource(0,1048576,3,int32_t(metadata));
            auto quad=[&](float x,float y,float w,float h,float u,float v,float uw,float vh){
                for(unsigned i=0;i<4;++i){
                    GrVertex a{};a.x=786432+x+((i&1)?w:0);a.y=786432+y+((i&2)?h:0);
                    a.r=a.g=a.b=a.a=255;a.oow=1;
                    a.tmuvtx[0].sow=u+((i&1)?uw:0);a.tmuvtx[0].tow=v+((i&2)?vh:0);
                    guest_store(quad_address+i*sizeof(a),a);
                }
                grDrawTriangle(quad_address,quad_address+sizeof(GrVertex),quad_address+2*sizeof(GrVertex));
                grDrawTriangle(quad_address+sizeof(GrVertex),quad_address+3*sizeof(GrVertex),quad_address+2*sizeof(GrVertex));
            };
            // Entire original atlas rejects numeral routing, including the round
            // indicator. A separate whole glyph positively exercises HD routing.
            quad(8,8,256,256,0,0,255,255);
            const float shrink=255.0f/256.0f;
            quad(320,40,96,208,(48.5f+12*digit)*shrink,21.5f*shrink,11.5f*shrink,25.5f*shrink);
            static const float speedBounds[10][4]={{43.330078125f,47.314453125f,72.71484375f,70.72265625f},{72.216796875f,47.314453125f,100.60546875f,70.72265625f},{101.103515625f,47.314453125f,130.48828125f,70.72265625f},{129.990234375f,47.314453125f,159.375f,70.72265625f},{159.873046875f,47.314453125f,189.2578125f,70.72265625f},{188.759765625f,47.314453125f,218.14453125f,70.72265625f},{217.646484375f,47.314453125f,246.03515625f,70.72265625f},{73.212890625f,71.220703125f,101.6015625f,94.62890625f},{102.099609375f,71.220703125f,131.484375f,94.62890625f},{130.986328125f,71.220703125f,160.37109375f,94.62890625f}};
            static const float speedWidth[10]={120,116,120,120,120,120,116,116,120,120};
            if(speed<10){const float *sb=speedBounds[speed];quad(450,40,speedWidth[speed],96,sb[0],sb[1],sb[2]-sb[0],sb[3]-sb[1]);}
            // Lines reuse triangle storage. Their selector must be cleared even
            // when the preceding primitive was an HD numeral.
            auto line_a=guest_load<GrVertex>(quad_address);
            auto line_b=guest_load<GrVertex>(quad_address+sizeof(GrVertex));
            line_a.x=786448;line_b.x=786680;line_a.y=line_b.y=786732;
            guest_store(quad_address,line_a);guest_store(quad_address+sizeof(GrVertex),line_b);
            grDrawLine(quad_address,quad_address+sizeof(GrVertex));
            grTexSource(0,1048576,3,int32_t(metadata));
            std::vector<unsigned char> pixels(2186*1640*4);
            glReadPixels(0,0,2186,1640,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
            return pixels;
        };
        nfs_glide_hd_builtin_enable(0);grTexDownloadMipMap(0,1048576,3,int32_t(metadata));
        const auto sampled_original=sampled_scene();
        nfs_glide_hd_builtin_enable(1);grTexDownloadMipMap(0,1048576,3,int32_t(metadata));
        const auto sampled_hd=sampled_scene();
        for(unsigned which=0;which<2;++which){
            const auto &pixels=which?sampled_hd:sampled_original;
            FILE *proof=std::fopen(which?"sampled-hd.rgba":"sampled-original.rgba","wb");
            CHECK(proof && std::fwrite(pixels.data(),1,pixels.size(),proof)==pixels.size());CHECK(!std::fclose(proof));
        }
        size_t atlas_differences=0,glyph_differences=0;
        for(unsigned y=0;y<1640;++y)for(unsigned x=0;x<2186;++x){
            const size_t offset=(y*2186+x)*4;
            if(!std::memcmp(sampled_original.data()+offset,sampled_hd.data()+offset,4))continue;
            if(x<1000)++atlas_differences;else ++glyph_differences;
        }
        std::printf("Native sampled differences: atlas=%zu glyph=%zu\n",atlas_differences,glyph_differences);
        CHECK(atlas_differences==0 && glyph_differences>100 && glGetError()==GL_NO_ERROR);
        const auto uploads_before_switch=nfs_glide_hd_atlas_upload_count();
        CHECK(nfs_glide_hd_timing_mode()==1 && nfs_glide_hd_timing_set_visible(0));
        CHECK(nfs_glide_hd_timing_mode()==0 && sampled_scene()==sampled_original);
        std::atomic<int> worker_switch{-1};std::thread switch_worker([&]{worker_switch=nfs_glide_hd_timing_set_visible(1);});switch_worker.join();
        CHECK(worker_switch==0 && nfs_glide_hd_timing_mode()==0);
        CHECK(nfs_glide_hd_timing_set_visible(1) && sampled_scene()==sampled_hd);
        // A UI switch must finish already buffered geometry in its old mode.
        auto queue_glyph=[&](){grDrawTriangle(quad_address,quad_address+sizeof(GrVertex),quad_address+2*sizeof(GrVertex));};
        auto read_scene=[&](){std::vector<unsigned char> p(2186*1640*4);glReadPixels(0,0,2186,1640,GL_RGBA,GL_UNSIGNED_BYTE,p.data());return p;};
        queue_glyph();CHECK(nfs_glide_hd_timing_set_visible(0));const auto pending_switched=read_scene();CHECK(pending_switched!=sampled_hd);
        CHECK(nfs_glide_hd_timing_set_visible(1));sampled_scene();queue_glyph();grTexSource(0,1048576,3,int32_t(metadata));
        CHECK(read_scene()==pending_switched && nfs_glide_hd_atlas_upload_count()==uploads_before_switch);
        sampled_scene();
        lifecycle.set_suspended(true);CHECK(SDL_GL_GetCurrentContext()==nullptr);
        CHECK(!nfs_glide_hd_timing_set_visible(0) && nfs_glide_hd_timing_mode()==1);
        lifecycle.set_suspended(false);CHECK(SDL_GL_GetCurrentContext()!=nullptr);
        CHECK(sampled_scene()==sampled_hd && glGetError()==GL_NO_ERROR);
        CHECK(nfs_glide_hd_timing_set_visible(0) && sampled_scene()==sampled_original);
        CHECK(nfs_glide_hd_timing_set_visible(1) && sampled_scene()==sampled_hd);
        std::puts("PASS: live original/HD comparison, identical pixels both ways, no reuploads, buffered old-mode flush and worker/unbound rejection");
        const auto before_all_digits=nfs_glide_hd_atlas_upload_count();
        for(unsigned digit=0;digit<10;++digit){
            CHECK(nfs_glide_hd_timing_set_visible(0));const auto original=sampled_scene(digit);
            CHECK(nfs_glide_hd_timing_set_visible(1));const auto hd=sampled_scene(digit);
            size_t changed=0;
            for(unsigned y=0;y<1640;++y)for(unsigned x=0;x<2186;++x){
                size_t at=(y*2186+x)*4;if(!std::memcmp(original.data()+at,hd.data()+at,4))continue;
                CHECK(x>=1093 && x<=1421 && y>=792 && y<=1505);++changed;
            }
            CHECK(changed>100 && glGetError()==GL_NO_ERROR);
            std::printf("PASS: original biased digit %u actual sampled HD changed pixels=%zu\n",digit,changed);
        }
        CHECK(nfs_glide_hd_atlas_upload_count()==before_all_digits);
        for(unsigned speed=0;speed<10;++speed){
            CHECK(nfs_glide_hd_timing_set_visible(0));const auto original=sampled_scene(0,speed);
            CHECK(nfs_glide_hd_timing_set_visible(1));const auto hd=sampled_scene(0,speed);
            size_t changed=0;
            for(unsigned y=0;y<1640;++y)for(unsigned x=0;x<2186;++x){
                const size_t at=(y*2186+x)*4;
                if(x<1500){CHECK(!std::memcmp(original.data()+at,sampled_original.data()+at,4));CHECK(!std::memcmp(hd.data()+at,sampled_hd.data()+at,4));}
                else if(std::memcmp(original.data()+at,hd.data()+at,4))++changed;
            }
            CHECK(changed>100 && glGetError()==GL_NO_ERROR);
            std::printf("PASS: gold family digit=%u changed=%zu\n",speed,changed);
            char path[64];std::snprintf(path,sizeof(path),"gold-%u.rgba",speed);FILE *file=std::fopen(path,"wb");CHECK(file);
            for(unsigned y=1170;y<1510;++y)CHECK(std::fwrite(hd.data()+(y*2186+1500)*4,1,500*4,file)==500*4);
            CHECK(!std::fclose(file));
        }
        sampled_scene();
        CHECK(nfs_glide_hd_atlas_upload_count()==before_all_digits);
        if(const char *font_case_path=std::getenv("NFS_HUD_FONT_CASES")){
        const unsigned font_codes[38]={65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,48,49,50,51,52,53,54,55,56,57,46,58};
        void *font_memory=malloc32(38*128),*font_meta_memory=malloc32(38*sizeof(GrTexInfo));CHECK(font_memory && font_meta_memory);
        GrTexInfo info[38];unsigned widths[38],edges[38];unsigned char originals[38][128]{};
        unsigned char scene_digest[38][32];const uint32_t font_base=1572864,font_data=nfs_guest_address(font_memory),font_meta=nfs_guest_address(font_meta_memory);
        FILE *font_file=std::fopen(font_case_path,"rb");CHECK(font_file);unsigned font_count=0;CHECK(std::fread(&font_count,4,1,font_file)==1 && font_count==38);
        const auto font_uploads=nfs_glide_hd_atlas_upload_count();
        for(unsigned d=0;d<38;++d){unsigned h[4];CHECK(std::fread(h,4,4,font_file)==4 && h[0]==font_codes[d] && h[1]>=16 && h[1]<=144 && (h[2]==4 || h[2]==8) && h[3]==h[2]*h[2]*2);
            unsigned char source[144];CHECK(std::fread(source,1,h[1],font_file)==h[1] && std::fread(originals[d],1,h[3],font_file)==h[3]);
            widths[d]=source[4]|(unsigned(source[5])<<8);edges[d]=h[2];CHECK(widths[d]==edges[d]);
            for(unsigned j=0;j<edges[d]*edges[d];++j){uint16_t v;std::memcpy(&v,originals[d]+2*j,2);guest_store<uint16_t>(font_data+d*128+2*j,(v>>1)|((v&1)<<15));}
            info[d]=texture;info[d].largeLod=info[d].smallLod=edges[d]==4?6:5;info[d].data=font_data+d*128;
            guest_store(font_meta+d*sizeof(GrTexInfo),info[d]);grTexDownloadMipMap(0,font_base+d*4096,3,int32_t(font_meta+d*sizeof(GrTexInfo)));
        }
        CHECK(std::fgetc(font_file)==EOF && !std::fclose(font_file));CHECK(nfs_glide_hd_atlas_upload_count()==font_uploads);
        auto check_font_original_gpu=[&](unsigned d)->int{
            grTexSource(0,font_base+d*4096,3,int32_t(font_meta+d*sizeof(GrTexInfo)));
            GLint tex=0,previous=0;GLuint f=0;unsigned char pixels[256];glGetIntegerv(GL_TEXTURE_BINDING_2D,&tex);glGetIntegerv(GL_FRAMEBUFFER_BINDING,&previous);
            glGenFramebuffers(1,&f);glBindFramebuffer(GL_FRAMEBUFFER,f);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,GLuint(tex),0);
            CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE);glReadPixels(0,0,edges[d],edges[d],GL_RGBA,GL_UNSIGNED_BYTE,pixels);
            for(unsigned j=0;j<edges[d]*edges[d];++j){uint16_t v=((pixels[j*4]>>3)<<11)|((pixels[j*4+1]>>3)<<6)|((pixels[j*4+2]>>3)<<1)|(pixels[j*4+3]>>7),expected;std::memcpy(&expected,originals[d]+j*2,2);CHECK(v==expected);}
            glBindFramebuffer(GL_FRAMEBUFFER,GLuint(previous));glDeleteFramebuffers(1,&f);return 0;
        };
        auto font_scene=[&](unsigned d,int malformed=0){
            sampled_scene();grTexSource(0,font_base+d*4096,3,int32_t(font_meta+d*sizeof(GrTexInfo)));
            auto draw_font=[&](float x,float y,float scale){
                for(unsigned i=0;i<4;++i){GrVertex a{};a.x=786432+x+((i&1)?widths[d]*scale:0);a.y=786432+y+((i&2)?8*scale:0);a.r=a.g=a.b=a.a=255;a.oow=1;
                    a.tmuvtx[0].sow=(i&1)?255:127.5f/edges[d];a.tmuvtx[0].tow=(i&2)?255:15.9375f;
                    if(malformed==1 && !(i&1))a.tmuvtx[0].sow+=1;if(malformed==2)a.tmuvtx[0].sow=100;
                    if(malformed==3){a.oow=2;a.tmuvtx[0].sow*=2;a.tmuvtx[0].tow*=2;}
                    guest_store(quad_address+i*sizeof(a),a);}
                grDrawTriangle(quad_address,quad_address+sizeof(GrVertex),quad_address+2*sizeof(GrVertex));grDrawTriangle(quad_address+sizeof(GrVertex),quad_address+3*sizeof(GrVertex),quad_address+2*sizeof(GrVertex));
            };
            draw_font(450,40,8);draw_font(450,200,1);grTexSource(0,font_base+d*4096,3,int32_t(font_meta+d*sizeof(GrTexInfo)));
            std::vector<unsigned char> p(2186*1640*4);glReadPixels(0,0,2186,1640,GL_RGBA,GL_UNSIGNED_BYTE,p.data());return p;
        };
        for(unsigned d=0;d<38;++d){CHECK(check_font_original_gpu(d)==0);
            CHECK(nfs_glide_hd_timing_set_visible(0));const auto original=font_scene(d);CHECK(nfs_glide_hd_timing_set_visible(1));const auto hd=font_scene(d);
            size_t changes=0,native_changes=0;
            for(unsigned y=0;y<1640;++y)for(unsigned x=0;x<2186;++x){size_t at=(y*2186+x)*4;
                if(x<1500){CHECK(!std::memcmp(original.data()+at,sampled_original.data()+at,4));CHECK(!std::memcmp(hd.data()+at,sampled_hd.data()+at,4));}
                else if(std::memcmp(original.data()+at,hd.data()+at,4)){++changes;if(y>=925 && y<=960)++native_changes;}}
            CHECK(changes>100 && native_changes>0 && nfs_texture_sha256(hd.data(),hd.size(),scene_digest[d]) && glGetError()==GL_NO_ERROR);
            char name[64];if(d<26)std::snprintf(name,sizeof(name),"letter-%c.rgba",65+d);else std::snprintf(name,sizeof(name),"record-%u.rgba",d-26);FILE *f=std::fopen(name,"wb");CHECK(f);
            for(unsigned y=1170;y<1510;++y)CHECK(std::fwrite(hd.data()+(y*2186+1500)*4,1,500*4,f)==500*4);CHECK(!std::fclose(f));
            if(d==0){f=std::fopen("letter-A-full.rgba","wb");CHECK(f && std::fwrite(hd.data(),1,hd.size(),f)==hd.size());CHECK(!std::fclose(f));}
            if(d==0 || d==8 || d==25 || d==27 || d==29 || d==36 || d==37){
                CHECK(nfs_glide_hd_timing_set_visible(0) && font_scene(d)==original);CHECK(nfs_glide_hd_timing_set_visible(1) && font_scene(d)==hd);
                for(int malformed=1;malformed<=3;++malformed){CHECK(nfs_glide_hd_timing_set_visible(0));const auto o=font_scene(d,malformed);CHECK(nfs_glide_hd_timing_set_visible(1));const auto h=font_scene(d,malformed);
                    for(unsigned y=0;y<1640;++y)CHECK(!std::memcmp(o.data()+(y*2186+1500)*4,h.data()+(y*2186+1500)*4,(2186-1500)*4));}
                uint16_t saved=guest_load<uint16_t>(info[d].data);guest_store<uint16_t>(info[d].data,saved^1);grTexDownloadMipMap(0,font_base+d*4096,3,int32_t(font_meta+d*sizeof(GrTexInfo)));
                CHECK(nfs_glide_hd_timing_set_visible(0));const auto o=font_scene(d);CHECK(nfs_glide_hd_timing_set_visible(1));const auto h=font_scene(d);
                for(unsigned y=0;y<1640;++y)CHECK(!std::memcmp(o.data()+(y*2186+1500)*4,h.data()+(y*2186+1500)*4,(2186-1500)*4));
                guest_store<uint16_t>(info[d].data,saved);grTexDownloadMipMap(0,font_base+d*4096,3,int32_t(font_meta+d*sizeof(GrTexInfo)));CHECK(font_scene(d)==hd);
            }
            CHECK(nfs_glide_hd_atlas_upload_count()==font_uploads);
            if(d<26)std::printf("PASS uppercase %c edge=%u changed=%zu native=%zu\n",65+d,edges[d],changes,native_changes);
            else std::printf("PASS record %u edge=%u changed=%zu native=%zu\n",d-26,edges[d],changes,native_changes);
            if(d==26)CHECK(!std::memcmp(scene_digest[14],scene_digest[26],32));
        }

        // Match the actual untextured split HUD trapezoid while a recognized
        // glyph remains selected. No font sampling may recolor this panel.
        auto flat_panel=[&](unsigned d,int visible){
            nfs_glide_hd_timing_set_visible(visible);grBufferClear(0,255,65535);
            grClipWindow(0,0,640,480);grTexSource(0,font_base+d*4096,3,int32_t(font_meta+d*sizeof(GrTexInfo)));
            grAlphaCombine(0,0,0,0,0);
            const float x[4]={363,640,331,640},y[4]={0,0,32,32};
            for(unsigned i=0;i<4;++i){GrVertex v{};v.x=786432+x[i];v.y=786432+y[i];v.r=v.g=v.b=64;v.a=255;v.oow=1;guest_store(quad_address+i*sizeof(v),v);}
            grDrawTriangle(quad_address,quad_address+sizeof(GrVertex),quad_address+2*sizeof(GrVertex));
            grDrawTriangle(quad_address+sizeof(GrVertex),quad_address+3*sizeof(GrVertex),quad_address+2*sizeof(GrVertex));
            grAlphaCombine(0,0,0,GR_COMBINE_OTHER_TEXTURE,0);
            std::vector<unsigned char> p(2186*1640*4);glReadPixels(0,0,2186,1640,GL_RGBA,GL_UNSIGNED_BYTE,p.data());return p;
        };
        for(unsigned d:{0u,8u,25u,27u,36u,37u}){
            const auto original=flat_panel(d,0),hd=flat_panel(d,1);CHECK(original==hd);
            const auto at=(1620u*2186+1875u)*4;
            CHECK(hd[at]==64 && hd[at+1]==64 && hd[at+2]==64 && hd[at+3]==255 && glGetError()==GL_NO_ERROR);
        }
        std::puts("PASS exact native HUD flat-panel color across Original/HD with A/I/Z selected");

        for(unsigned d=0;d<38;++d){GrTexInfo overlap=info[d];overlap.largeLod=overlap.smallLod=7;
            guest_store(font_meta+d*sizeof(GrTexInfo),overlap);grTexDownloadMipMap(0,font_base+d*4096+4,3,int32_t(font_meta+d*sizeof(GrTexInfo)));guest_store(font_meta+d*sizeof(GrTexInfo),info[d]);}
        needRecreateGl=1;grBufferSwap(0);
        for(unsigned d=0;d<38;++d){const auto hd=font_scene(d);unsigned char digest[32];CHECK(nfs_texture_sha256(hd.data(),hd.size(),digest) && !std::memcmp(digest,scene_digest[d],32));CHECK(check_font_original_gpu(d)==0);}
        free32(font_memory);free32(font_meta_memory);sampled_scene();
        std::puts("PASS all38original GPUs exact; uppercase and record negative controls, zero/O alias, no extra upload, all38CPU-RAM overlaps/context restored");

        }

        // Overlapping CPU texture RAM must not corrupt the matched original GPU
        // image when context restoration reuploads it from the owned snapshot.
        GrTexInfo overlap=texture;overlap.largeLod=overlap.smallLod=7;
        guest_store(metadata,overlap);grTexDownloadMipMap(0,1048580,3,int32_t(metadata));
        guest_store(metadata,texture);needRecreateGl=1;grBufferSwap(0);
        CHECK(sampled_scene()==sampled_hd && glGetError()==GL_NO_ERROR);
        free32(quad_memory);
        std::printf("PASS: native 2186x1640 actual shader sampling: zero non-target changed pixels, %zu changed glyph pixels; overlapping RAM/context restoration identical\n",glyph_differences);
        nfs_glide_hd_builtin_enable(0);guest_store<uint16_t>(texture.data,0x801f);
        grTexDownloadMipMap(0,1048576,3,int32_t(metadata));grTexSource(0,1048576,3,int32_t(metadata));pixel(0,0,255);
        CHECK(nfs_glide_hd_atlas_upload_count()==0 && glGetError()==GL_NO_ERROR);
        nfs_glide_hd_atlas_clear();nfs_hd_atlas_free(&expected);free32(raw_memory);
        std::puts("PASS: automatic original-data GPU atlas byte identity, context restoration and disabled fallback without private pack loading");
    }
    for(auto table:eviction_tables)free32(table);
    free32(index_memory);free32(palette_memory);CHECK(heap.available_bytes()==before && glGetError()==GL_NO_ERROR);
    std::puts("PASS: owned palette snapshot, deferred guest mutation, generation A/B/A, same-address explicit updates, unchanged-content cache and texture invalidation RGB pixels");
    if(argc==3){
        // Permanent background shutdown must release the still-owned auxiliary
        // texture through context destruction without issuing unbound GL calls.
        lifecycle.set_suspended(true);CHECK(SDL_GL_GetCurrentContext()==nullptr);
        main.retire_renderer();
        lifecycle.abandon_suspended();CHECK(SDL_GL_GetCurrentContext()==nullptr);
        native_grGlideShutdown();nfs_glide_hd_atlas_clear();
#ifdef NFS_TEST_GL_CALLS
        CHECK(unbound_hd_gl_calls==0);
#endif
        std::puts("PASS: permanent suspended HD context abandonment and idempotent shutdown without unbound HD GL operations");
    }else grGlideShutdown();
    operations.destroy(sdlWin);sdlWin=nullptr;nfs::bind_main_thread_bridge(nullptr);SDL_Quit();
    std::puts("PASS: actual GLES pixels, RGB565 partial writes/stride, row origins, front/back buffers, presentation, context suspend/restore with preserved pixels, guest canaries and cleanup");
}

#include "GuestAccess.h"
#include "GameMemory.h"
#include "NativeGlide.h"
#undef true
#undef false
#include "GuestGlide.h"
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include "MainThreadHarness.h"
static_assert(sizeof(GrVertex)==60,"Original Glide vertex layout");
static_assert(sizeof(GrTexInfo)==20 && sizeof(GrLfbInfo_t)==20,"Original Glide pointer layouts");
static nfs::GameMemory memory;
static bool saw_vertex, saw_texture, saw_palette;
static uint8_t *framebuffer;
extern "C" void *nfs_host_address(uint32_t address,size_t length) {
    void *p=memory.resolve(address,length);
    if (!p) throw std::out_of_range("Unmapped Glide span");
    return p;
}
extern "C" uint32_t nfs_guest_address(const void *pointer) {
    uint32_t address=0;
    if (!memory.encode(pointer,address)) throw std::out_of_range("Unregistered Glide pointer");
    return address;
}
// Native witnesses test the actual generated export bodies, without a GPU.
extern "C" void native_grDrawTriangle(const GrVertex *a,const GrVertex *b,const GrVertex *c) {
    saw_vertex=reinterpret_cast<uintptr_t>(a)>UINT32_MAX && a->x==77.0f && b->r==31.0f && c->tmuvtx[1].tow==9.0f;
}
extern "C" void native_grTexDownloadMipMap(GrChipID_t,uint32_t,uint32_t,GrTexInfo *info) {
    const auto *bytes=static_cast<const uint8_t *>(nfs_host_address(info->data,1024));
    saw_texture=info->data==0x90000000 && info->largeLod==3 && bytes[0]==0x9b;
}
extern "C" void native_grTexSource(GrChipID_t,uint32_t,uint32_t,GrTexInfo *info) { saw_texture=saw_texture && info->largeLod==3; }
extern "C" BOOL native_grLfbLock(GrLock_t,GrBuffer_t,GrLfbWriteMode_t,GrOriginLocation_t,BOOL,GrLfbInfo_t *info) {
    if(info->size!=20)throw std::runtime_error("Lost caller LFB size");
    info->size=20; info->lfbPtr=nfs_guest_address(framebuffer); info->strideInBytes=1280;
    info->writeMode=GR_LFBWRITEMODE_565; info->origin=GR_ORIGIN_UPPER_LEFT; return 1;
}
extern "C" void native_grFogTable(const GrFog_t table[GR_FOG_TABLE_SIZE]) { if(table[63]!=63) throw std::runtime_error("Bad fog table"); }
extern "C" void native_guFogGenerateExp(GrFog_t table[GR_FOG_TABLE_SIZE],float density) { table[63]=uint8_t(density); }
extern "C" void native_grTexDownloadTable(GrChipID_t,GrTexTable_t,void *data) { saw_palette=reinterpret_cast<uintptr_t>(data)>UINT32_MAX; }
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);return 1;}}while(0)
int main() {
    MainThreadHarness main_thread;
    alignas(16) uint8_t records[512], pixels[2048];
    std::memset(records,0xcc,sizeof(records)); std::memset(pixels,0x9b,sizeof(pixels));
    framebuffer=pixels;
    CHECK(memory.map(0x80000000,records,sizeof(records)) && memory.map(0x90000000,pixels,sizeof(pixels)));
    GrVertex vertex{}; vertex.x=77; vertex.r=31; vertex.tmuvtx[1].tow=9;
    guest_store<GrVertex>(0x80000003u,vertex); guest_store<GrVertex>(0x80000043u,vertex); guest_store<GrVertex>(0x80000083u,vertex);
    CHECK(grDrawTriangle(int32_t(0x80000003u),int32_t(0x80000043u),int32_t(0x80000083u))==0 && saw_vertex);
    GrTexInfo texture{}; texture.smallLod=3; texture.largeLod=3; texture.format=GR_TEXFMT_P_8; texture.data=0x90000000;
    guest_store<GrTexInfo>(0x800000c3u,texture);
    CHECK(grTexDownloadMipMap(0,0,3,int32_t(0x800000c3u))==0 && saw_texture);
    CHECK(grTexSource(0,0,3,int32_t(0x800000c3u))==0 && saw_texture);
    CHECK(grTexDownloadTable(0,GR_TEXTABLE_PALETTE,int32_t(0x90000001u))==0 && saw_palette);
    GrLfbInfo_t input_lfb{};input_lfb.size=20;guest_store(0x80000103u,input_lfb);
    CHECK(grLfbLock(1,0,0,0,0,int32_t(0x80000103u))==1);
    const auto lfb=guest_load<GrLfbInfo_t>(0x80000103u);
    CHECK(lfb.size==20 && lfb.lfbPtr==0x90000000 && lfb.strideInBytes==1280);
    CHECK(records[258]==0xcc && records[279]==0xcc);
    for(unsigned i=0;i<64;++i) records[320+i]=uint8_t(i);
    CHECK(grFogTable(int32_t(0x80000140u))==0);
    CHECK(guFogGenerateExp(int32_t(0x80000140u),12.0f)==0 && records[383]==12);
    bool rejected=false;
    texture.data=0x900007ff; guest_store<GrTexInfo>(0x800000c3u,texture);
    try {grTexDownloadMipMap(0,0,3,int32_t(0x800000c3u));} catch(const std::out_of_range &){rejected=true;}
    CHECK(rejected);
    std::puts("PASS: original Glide layouts, unaligned high-address vertices, nested guest texture pointer, palette/fog spans, framebuffer output/canaries, bounds");
}

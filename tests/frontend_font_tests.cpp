// SPDX-License-Identifier: MIT
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>
#include "TextureDigest.h"
struct Tmu { float sow, tow, oow; };
struct GrVertex { float x,y,z,r,g,b,ooz,a,oow; Tmu tmuvtx[2]; };
static int nfs_hd_builtin_enabled=1, nfs_hd_visible=1, nfs_hd_selected=41;
static int nfs_front_font_texture=1, nfs_hd_cell_layout_available=1;
#include "FrontendFontPixels.inc"
#include "FrontendFontLayout.inc"
#include "FrontendFontGeometry.inc"
#define CHECK(v) do { if (!(v)) { std::fprintf(stderr,"Frontend check failed: %d\n",__LINE__); std::abort(); } } while (0)

int main() {
    const size_t size=4194304;
    std::vector<unsigned char> memory(size+32,0x5b);
    auto pixels=memory.data()+16;
    CHECK(!nfs_front_font_expand(nullptr,size));
    CHECK(!nfs_front_font_expand(pixels,size-1));
    for (auto c:memory) CHECK(c==0x5b);
    CHECK(nfs_front_font_expand(pixels,size));
    const unsigned char expected[]={0x6d,0xcf,0x0b,0x04,0x6e,0x5a,0xc9,0x2b,0xb3,0xa2,0x9f,0xc6,0xa3,0xc8,0x0c,0xc8,0x60,0xf4,0xd4,0x8c,0x8f,0xa0,0xd0,0xc8,0x9f,0xcc,0xdc,0x4c,0x22,0xa5,0x74,0x79};
    unsigned char digest[32];
    CHECK(nfs_texture_sha256(pixels,size,digest) && !std::memcmp(digest,expected,32));
    for (unsigned i=0;i<16;++i) CHECK(memory[i]==0x5b && memory[size+16+i]==0x5b);
    unsigned cases=0;
    for (const auto &glyph:nfs_front_word_glyphs) {
        const auto &cell=nfs_front_cells[glyph.cell-1];
        const float bx=cell.large?1:(cell.code==69?.5f:1);
        const float by=cell.large?3:(cell.code>=65 && cell.code<=90?-.5f:0);
        for (unsigned omitted=0;omitted<4;++omitted) {
            GrVertex v[3]{}, before[3], out[3]{};
            unsigned at=0;
            for (unsigned corner=0;corner<4;++corner) if (corner!=omitted) {
                auto &p=v[at++];
                p.x=786432+glyph.x+((corner&1)?glyph.w:0);
                p.y=786432+glyph.y+((corner&2)?glyph.h:0);
                p.z=37; p.r=247; p.g=204; p.b=19; p.a=253; p.ooz=41; p.oow=1;
                p.tmuvtx[0].sow=(cell.x+bx+((corner&1)?cell.width:.5f))*255/256;
                p.tmuvtx[0].tow=(cell.y+by+((corner&2)?51:.5f))*255/256;
                p.tmuvtx[1].sow=99;
            }
            std::memcpy(before,v,sizeof v);
            CHECK(nfs_front_font_geometry(v,v+1,v+2,out)==glyph.cell);
            CHECK(!std::memcmp(before,v,sizeof v));
            for (unsigned i=0;i<3;++i) {
                CHECK(out[i].x>=786432 && out[i].x<=786600);
                CHECK(out[i].y>=786432 && out[i].y<=786912);
                CHECK(out[i].tmuvtx[0].sow>=0 && out[i].tmuvtx[0].sow<=256);
                CHECK(out[i].tmuvtx[0].tow>=0 && out[i].tmuvtx[0].tow<=256);
                out[i].x=v[i].x; out[i].y=v[i].y;
                out[i].tmuvtx[0].sow=v[i].tmuvtx[0].sow;
                out[i].tmuvtx[0].tow=v[i].tmuvtx[0].tow;
                CHECK(!std::memcmp(out+i,v+i,sizeof(GrVertex)));
            }
            for (int *flag:{&nfs_hd_builtin_enabled,&nfs_hd_visible,&nfs_front_font_texture,&nfs_hd_cell_layout_available}) {
                *flag=0; CHECK(!nfs_front_font_geometry(v,v+1,v+2,out)); *flag=1;
            }
            nfs_hd_selected=1; CHECK(!nfs_front_font_geometry(v,v+1,v+2,out)); nfs_hd_selected=41;
            for (float bad:{0.0f,-1.0f,2.0f,std::numeric_limits<float>::quiet_NaN()}) {
                v[0].oow=bad; CHECK(!nfs_front_font_geometry(v,v+1,v+2,out));
            }
            std::memcpy(v,before,sizeof v);
            v[0].tmuvtx[0].sow=std::numeric_limits<float>::infinity();
            CHECK(!nfs_front_font_geometry(v,v+1,v+2,out));
            std::memcpy(v,before,sizeof v);
            for (auto &p:v) p.x+=1;
            CHECK(!nfs_front_font_geometry(v,v+1,v+2,out));
            ++cases;
        }
    }
    std::printf("PASS authored atlas digest/canaries, %u complete glyph triangles, immutable attributes and fallback guards\n",cases);
}

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
static int nfs_front_extended_ready=1;
#include "FrontendFontPixels.inc"
#include "FrontendFontLayout.inc"
#include "FrontendFontGeometry.inc"
#define CHECK(v) do { if (!(v)) { std::fprintf(stderr,"Frontend check failed: %d\n",__LINE__); std::abort(); } } while (0)

int main() {
    const size_t size=NFS_FRONT_FONT_RGBA_BYTES;
    CHECK(NFS_FRONT_FONT_WIDTH==1024 && NFS_FRONT_FONT_HEIGHT==2048 && size==8388608);
    CHECK(NFS_FRONT_FONT_PIXELS==NFS_FRONT_FONT_WIDTH*NFS_FRONT_FONT_HEIGHT && size==NFS_FRONT_FONT_PIXELS*4);
    std::vector<unsigned char> memory(size+32,0x5b);
    auto pixels=memory.data()+16;
    CHECK(!nfs_front_font_expand(nullptr,size));
    CHECK(!nfs_front_font_expand(pixels,size-1));
    for (auto c:memory) CHECK(c==0x5b);
    CHECK(nfs_front_font_expand(pixels,size));
    const unsigned char expected[]={0xd5,0xfd,0x21,0xe3,0x92,0x25,0xad,0xf8,0x0d,0xfb,0xa2,0xf7,0x6d,0x6a,0x18,0xfa,0x0c,0x49,0x8c,0x69,0xeb,0x9d,0xb2,0xc8,0x6d,0x78,0x1d,0x2b,0xce,0xa5,0x31,0x34};
    unsigned char digest[32];
    CHECK(nfs_texture_sha256(pixels,size,digest) && !std::memcmp(digest,expected,32));
    for (unsigned i=0;i<16;++i) CHECK(memory[i]==0x5b && memory[size+16+i]==0x5b);
    unsigned cases=0,ambiguous=0;
    for (const auto &glyph:nfs_front_word_glyphs) {
        nfs::FrontendTextScope scope(glyph.word);
        nfs_hd_selected=glyph.cell==121?42:41;
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
                CHECK(out[i].x>=786432 && out[i].x<=787072);
                CHECK(out[i].y>=786432 && out[i].y<=786912);
                CHECK(out[i].tmuvtx[0].sow>=0 && out[i].tmuvtx[0].sow<=256);
                CHECK(out[i].tmuvtx[0].tow>=0 && out[i].tmuvtx[0].tow<=256);
                const bool right=v[i].x>786432+glyph.x,bottom=v[i].y>786432+glyph.y;
                const float edge=right?glyph.right:glyph.left;
                const float sx=glyph.outputScaleX>0?glyph.outputScaleX:1,sy=glyph.outputScaleY>0?glyph.outputScaleY:1;
                CHECK(std::fabs(out[i].x-(786432+glyph.outputX+edge*glyph.width*sx))<.063f);
                CHECK(std::fabs(out[i].y-(786432+glyph.y+glyph.inkTop*sy+(bottom?glyph.inkHeight*sy:0)))<.063f);
                CHECK(std::fabs(out[i].tmuvtx[0].sow-(glyph.atlasX+edge*glyph.width))<.001f);
                CHECK(std::fabs(out[i].tmuvtx[0].tow-(glyph.atlasY+(bottom?glyph.inkHeight*nfs_front_font_v_scale:0)))<.001f);
                out[i].x=v[i].x; out[i].y=v[i].y;
                out[i].tmuvtx[0].sow=v[i].tmuvtx[0].sow;
                out[i].tmuvtx[0].tow=v[i].tmuvtx[0].tow;
                CHECK(!std::memcmp(out+i,v+i,sizeof(GrVertex)));
            }
            if (glyph.word) {
                nfs::FrontendTextScope missing(0);
                CHECK(!nfs_front_font_geometry(v,v+1,v+2,out));
                nfs_front_text_context_set(UINT64_C(0xdeadbeef));
                CHECK(!nfs_front_font_geometry(v,v+1,v+2,out));
            }
            for (int *flag:{&nfs_hd_builtin_enabled,&nfs_hd_visible,&nfs_front_font_texture,&nfs_hd_cell_layout_available}) {
                *flag=0; CHECK(!nfs_front_font_geometry(v,v+1,v+2,out)); *flag=1;
            }
            nfs_hd_selected=1; CHECK(!nfs_front_font_geometry(v,v+1,v+2,out)); nfs_hd_selected=glyph.cell==121?42:41;
            if (glyph.word==UINT64_C(0x385db5487d40488d)) {
                nfs_front_extended_ready=0; CHECK(!nfs_front_font_geometry(v,v+1,v+2,out)); nfs_front_extended_ready=1;
            }
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
        for (const auto &other:nfs_front_word_glyphs)
            if(glyph.word && other.word && glyph.word!=other.word && glyph.cell==other.cell && glyph.x==other.x && glyph.y==other.y && glyph.w==other.w && glyph.h==other.h){++ambiguous;break;}
    }
    CHECK(cases==7900 && ambiguous==294 && nfs_front_text_context()==0);
    std::printf("PASS authored atlas digest/canaries, %u complete glyph triangles, %u ambiguous positions, immutable attributes and whole-label fallback guards\n",cases,ambiguous);
}

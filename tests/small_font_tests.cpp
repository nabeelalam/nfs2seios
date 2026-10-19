// SPDX-License-Identifier: MIT
#include "BuiltinSmallFont.h"
#include "SmallFontPixels.inc"
#include "TextureDigest.h"
#include <cstdio>
#include <vector>
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr,"Small font check failed at %d\n",__LINE__); return 1; } } while(0)

int main() {
    const size_t capacity=4194304;
    std::vector<unsigned char> destination(capacity+32,0xa5),before=destination,glyphs;
    auto rejects=[&](const unsigned char *input,size_t bytes,unsigned char *output,size_t size) {
        return !nfs_small_font_expand(input,bytes,output,size) && destination==before;
    };
    unsigned char zero[6]={},short_run[6]={1,0,1,2,3,4};
    unsigned char overflow[12]={255,255,0,0,0,0,255,255,0,0,0,0};
    CHECK(rejects(nullptr,sizeof(nfs_small_font_pixels_rle),destination.data()+16,capacity));
    CHECK(rejects(nfs_small_font_pixels_rle,sizeof(nfs_small_font_pixels_rle),nullptr,capacity));
    CHECK(rejects(nfs_small_font_pixels_rle,sizeof(nfs_small_font_pixels_rle),destination.data()+16,capacity-1));
    CHECK(rejects(nfs_small_font_pixels_rle,sizeof(nfs_small_font_pixels_rle)-1,destination.data()+16,capacity));
    CHECK(rejects(nfs_small_font_pixels_rle,0,destination.data()+16,capacity));
    CHECK(rejects(zero,sizeof(zero),destination.data()+16,capacity));
    CHECK(rejects(short_run,sizeof(short_run),destination.data()+16,capacity));
    CHECK(rejects(overflow,sizeof(overflow),destination.data()+16,capacity));
    CHECK(nfs_small_font_expand(nfs_small_font_pixels_rle,sizeof(nfs_small_font_pixels_rle),destination.data()+16,capacity));
    for(unsigned d=0;d<26;++d) {
        unsigned alpha=0;
        for(unsigned y=0;y<64;++y)for(unsigned x=0;x<64;++x) {
            size_t at=16+((size_t)(512+(d/16)*64+y)*1024+(d%16)*64+x)*4;
            glyphs.insert(glyphs.end(),destination.begin()+at,destination.begin()+at+4);
            alpha+=destination[at+3];
            if(d==8 && x>=32)for(unsigned c=0;c<4;++c)CHECK(destination[at+c]==0);
        }
        CHECK(d==22 ? alpha==0 : alpha>0); // W owns the separately checked wide strip.
    }
    unsigned char digest[32];
    CHECK(glyphs.size()==425984 && nfs_texture_sha256(glyphs.data(),glyphs.size(),digest));
    CHECK(!memcmp(digest,nfs_small_font_raster_digest,32));
    for(unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x) {
        bool owned=(y>=512 && y<576)||(y>=576 && y<640 && x<640);
        if(!owned)for(unsigned c=0;c<4;++c)CHECK(destination[16+((size_t)y*1024+x)*4+c]==0xa5);
    }
    for(unsigned i=0;i<16;++i)CHECK(destination[i]==0xa5 && destination[16+capacity+i]==0xa5);
    std::puts("PASS all26 uppercase tiles/digest, narrow I, unchanged atlas/canaries and atomic invalid input");
}

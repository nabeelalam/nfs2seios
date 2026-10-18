// SPDX-License-Identifier: MIT
#include "BuiltinTiming.h"
#undef NDEBUG
#include <cassert>
#include <cstdio>
#include <vector>
int main(int argc,char **argv){
    std::vector<unsigned char> rgba(4194304,0xa5);
    assert(nfs_timing_expand(nfs_timing_pixels_rle,sizeof(nfs_timing_pixels_rle),rgba.data(),rgba.size()));
    std::vector<unsigned char> glyphs;
    for(unsigned d=0;d<10;++d)for(unsigned y=0;y<104;++y){size_t at=((size_t)(84+y)*1024+192+d*48)*4;glyphs.insert(glyphs.end(),rgba.begin()+at,rgba.begin()+at+192);}
    unsigned char digest[32];assert(nfs_texture_sha256(glyphs.data(),glyphs.size(),digest));assert(!memcmp(digest,nfs_timing_raster_digest,32));
    for(unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x)if(!(192<=x && x<672 && 84<=y && y<188))for(unsigned b=0;b<4;++b)assert(rgba[((size_t)y*1024+x)*4+b]==0xa5);
    const auto before=rgba;unsigned char zero[6]={},overflow[6]={255,255,0,0,0,0};
    assert(!nfs_timing_expand(zero,sizeof(zero),rgba.data(),rgba.size()));assert(rgba==before);
    assert(!nfs_timing_expand(overflow,sizeof(overflow),rgba.data(),rgba.size()));assert(rgba==before);
    assert(!nfs_timing_expand(nfs_timing_pixels_rle,sizeof(nfs_timing_pixels_rle)-1,rgba.data(),rgba.size()));assert(rgba==before);
    assert(!nfs_timing_expand(nfs_timing_pixels_rle,sizeof(nfs_timing_pixels_rle)-6,rgba.data(),rgba.size()));assert(rgba==before);
    assert(!nfs_timing_expand(nfs_timing_pixels_rle,sizeof(nfs_timing_pixels_rle),rgba.data(),rgba.size()-1));assert(rgba==before);
    NfsHdAtlas atlas{};unsigned char black[131072]={};assert(!nfs_timing_build(&atlas,256,black) && !atlas.original);
    assert(!nfs_timing_build(&atlas,128,black) && !nfs_timing_build(&atlas,256,nullptr));
    if(argc==3){
        FILE *f=fopen(argv[1],"rb");assert(f);std::vector<unsigned char> raw(131073);assert(fread(raw.data()+1,1,131072,f)==131072);fclose(f);
        assert(nfs_timing_build(&atlas,256,raw.data()+1));NfsHdAtlas accepted{};
        f=fopen(argv[2],"rb");assert(f && nfs_hd_atlas_read(&accepted,f));fclose(f);
        assert(atlas.original_size==accepted.original_size && atlas.replacement_size==accepted.replacement_size);
        assert(!memcmp(atlas.original,accepted.original,131072) && !memcmp(atlas.rgba,accepted.rgba,4194304));
        auto *owned=atlas.rgba;auto sample=nfs_timing_sample(raw.data()+1);raw[4001]^=1;
        assert(nfs_timing_sample(raw.data()+1)==sample); // Deliberately outside the fast prefilter.
        assert(!nfs_timing_build(&atlas,256,raw.data()+1) && atlas.rgba==owned);
        assert(!memcmp(atlas.rgba,accepted.rgba,4194304));nfs_hd_atlas_free(&accepted);
        puts("PASS automatic generation exactly equals accepted original-game pack; full digest rejects unsampled mutation");
    }
    nfs_hd_atlas_free(&atlas);puts("PASS new-art RLE integrity, shared gutters, malformed bounds and unsupported originals");
}

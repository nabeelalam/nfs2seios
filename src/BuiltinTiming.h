// SPDX-License-Identifier: MIT
#ifndef NFS_BUILTIN_TIMING_H
#define NFS_BUILTIN_TIMING_H
#include "HdAtlas.h"
#include "TextureDigest.h"
#include "TimingFontPixels.inc"
#include "BuiltinSpeed.h"
#include "BuiltinSmallFont.h"
#include "SmallFontPixels.inc"
#include "BuiltinRecordFont.h"
#include "RecordFontPixels.inc"
#include "LogoPixels.inc"
#include "PauseFontPixels.inc"
#include "GaugeMaskPixels.inc"
#include "GearPixels.inc"
#include "WideLabelPixels.inc"
static uint64_t nfs_timing_sample(const unsigned char *data){
    uint64_t value=UINT64_C(14695981039346656037);
    for(unsigned block=0;block<131072;block+=4096)for(unsigned i=0;i<16;++i)value=(value^data[block+i])*UINT64_C(1099511628211);
    return value;
}
static int nfs_timing_expand(const unsigned char *rle,size_t bytes,unsigned char *rgba,size_t capacity){
    if(!rle || !rgba || bytes%6 || capacity<4194304)return 0;
    unsigned count=0;
    for(size_t i=0;i<bytes;i+=6){unsigned run=rle[i]|((unsigned)rle[i+1]<<8);if(!run || run>49920-count)return 0;count+=run;}
    if(count!=49920)return 0;
    unsigned position=0;
    for(size_t i=0;i<bytes;i+=6){
        unsigned run=rle[i]|((unsigned)rle[i+1]<<8);
        for(unsigned j=0;j<run;++j,++position){unsigned digit=position/4992,cell=position%4992,x=192+digit*48+cell%48,y=84+cell/48;memcpy(rgba+((size_t)y*1024+x)*4,rle+i+2,4);}
    }
    return 1;
}
static int nfs_timing_build(NfsHdAtlas *atlas,unsigned size,const void *original){
    if(!atlas || size!=256 || !original || nfs_timing_sample((const unsigned char *)original)!=nfs_timing_sample_expected)return 0;
    unsigned char digest[32];
    if(!nfs_texture_sha256(original,131072,digest) || memcmp(digest,nfs_timing_original_digest,32))return 0;
    NfsHdAtlas next={256,1024,NULL,NULL};
    next.original=(unsigned char *)malloc(131072);next.rgba=(unsigned char *)malloc(4194304);
    if(!next.original || !next.rgba){nfs_hd_atlas_free(&next);return 0;}
    memcpy(next.original,original,131072);const unsigned char *src=next.original;
    for(unsigned y=0;y<1024;++y)for(unsigned x=0;x<1024;++x){
        unsigned index=((y/4)*256+x/4)*2,value=src[index]|((unsigned)src[index+1]<<8);unsigned char *pixel=next.rgba+((size_t)y*1024+x)*4;
        pixel[0]=((value>>11)&31)*255/31;pixel[1]=((value>>6)&31)*255/31;pixel[2]=((value>>1)&31)*255/31;pixel[3]=(value&1)?255:0;
    }
    if(!nfs_timing_expand(nfs_timing_pixels_rle,sizeof(nfs_timing_pixels_rle),next.rgba,4194304) ||
       !nfs_speed_expand(nfs_speed_pixels_rle,sizeof(nfs_speed_pixels_rle),next.rgba,4194304) ||
       !nfs_small_font_expand(nfs_small_font_pixels_rle,sizeof(nfs_small_font_pixels_rle),next.rgba,4194304) ||
       !nfs_record_font_expand(nfs_record_font_pixels_rle,sizeof(nfs_record_font_pixels_rle),next.rgba,4194304) || !nfs_logo_expand(next.rgba,4194304) || !nfs_pause_expand(next.rgba,4194304) || !nfs_gauge_mask_expand(next.rgba,4194304) || !nfs_gear_expand(next.rgba,4194304) || !nfs_wide_label_expand(next.rgba,4194304)){nfs_hd_atlas_free(&next);return 0;}
    nfs_hd_atlas_free(atlas);*atlas=next;return 1;
}
#endif

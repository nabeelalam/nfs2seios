// SPDX-License-Identifier: MIT
#ifndef NFS_BUILTIN_SPEED_H
#define NFS_BUILTIN_SPEED_H
#include "HdAtlas.h"
#include "SpeedFontPixels.inc"
/* Validate the complete new-art stream before touching the destination. These
 * reserved auxiliary cells never replace pixels in the original GPU texture. */
static int nfs_speed_expand(const unsigned char *rle,size_t bytes,unsigned char *rgba,size_t capacity){
    if(!rle || !rgba || bytes%6 || capacity<4194304)return 0;
    unsigned count=0;
    for(size_t i=0;i<bytes;i+=6){
        unsigned run=rle[i]|((unsigned)rle[i+1]<<8);
        if(!run || run>111360-count)return 0;
        count+=run;
    }
    if(count!=111360)return 0;
    unsigned position=0;
    for(size_t i=0;i<bytes;i+=6){
        unsigned run=rle[i]|((unsigned)rle[i+1]<<8);
        for(unsigned j=0;j<run;++j,++position){
            unsigned digit=position/11136,cell=position%11136;
            unsigned x=(digit%8)*116+cell%116,y=300+(digit/8)*96+cell/116;
            memcpy(rgba+((size_t)y*1024+x)*4,rle+i+2,4);
        }
    }
    return 1;
}
#endif

// SPDX-License-Identifier: MIT
#ifndef NFS_BUILTIN_RECORD_FONT_H
#define NFS_BUILTIN_RECORD_FONT_H
#include <stddef.h>
#include <string.h>
// Eleven new tiles; zero shares the already reviewed uppercase O tile.
static int nfs_record_font_expand(const unsigned char *rle,size_t bytes,
                                  unsigned char *rgba,size_t capacity){
    const unsigned pixels=11u*4096u;
    if(!rle || !rgba || bytes%6 || capacity<4194304)return 0;
    unsigned count=0;
    for(size_t i=0;i<bytes;i+=6){
        unsigned run=rle[i]|((unsigned)rle[i+1]<<8);
        if(!run || run>pixels-count)return 0;
        count+=run;
    }
    if(count!=pixels)return 0;
    unsigned position=0;
    for(size_t i=0;i<bytes;i+=6){
        unsigned run=rle[i]|((unsigned)rle[i+1]<<8);
        for(unsigned j=0;j<run;++j,++position){
            unsigned tile=26+position/4096,cell=position%4096;
            unsigned x=(tile%16)*64+cell%64,y=512+(tile/16)*64+cell/64;
            memcpy(rgba+((size_t)y*1024+x)*4,rle+i+2,4);
        }
    }
    return 1;
}
#endif

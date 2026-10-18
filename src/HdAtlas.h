// SPDX-License-Identifier: MIT
#ifndef NFS_HD_ATLAS_H
#define NFS_HD_ATLAS_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Private user-generated pack: 32-byte LE header, exact original GPU RGBA5551
 * bytes, then straight-alpha RGBA8 replacement. No original artwork is shipped.
 * One square atlas, bounded to 256 original / 2048 replacement pixels per edge.
 */
typedef struct {unsigned original_size,replacement_size;unsigned char *original,*rgba;} NfsHdAtlas;
static void nfs_hd_atlas_free(NfsHdAtlas *atlas){
    free(atlas->original);free(atlas->rgba);memset(atlas,0,sizeof(*atlas));
}
static uint32_t nfs_hd_le32(const unsigned char *p){
    return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
}
static int nfs_hd_power_two(uint32_t value){return value && !(value&(value-1));}
static int nfs_hd_atlas_read(NfsHdAtlas *atlas,FILE *file){
    unsigned char h[32];NfsHdAtlas next={0,0,NULL,NULL};int ok=0;
    nfs_hd_atlas_free(atlas);
    if(!file || fread(h,1,sizeof(h),file)!=sizeof(h) || memcmp(h,"NFSHDA1\0",8))return 0;
    const uint32_t original=nfs_hd_le32(h+8),replacement=nfs_hd_le32(h+12);
    const uint32_t original_bytes=nfs_hd_le32(h+16),rgba_bytes=nfs_hd_le32(h+20);
    if(!nfs_hd_power_two(original) || original>256 || !nfs_hd_power_two(replacement) ||
       replacement<=original || replacement>2048 || replacement/original>8 ||
       original_bytes!=original*original*2 || rgba_bytes!=replacement*replacement*4 ||
       nfs_hd_le32(h+24) || nfs_hd_le32(h+28))return 0;
    next.original=(unsigned char *)malloc(original_bytes);next.rgba=(unsigned char *)malloc(rgba_bytes);
    if(next.original && next.rgba && fread(next.original,1,original_bytes,file)==original_bytes &&
       fread(next.rgba,1,rgba_bytes,file)==rgba_bytes && fgetc(file)==EOF && !ferror(file)){
        next.original_size=original;next.replacement_size=replacement;*atlas=next;ok=1;
    }
    if(!ok)nfs_hd_atlas_free(&next);
    return ok;
}
static int nfs_hd_atlas_matches(const NfsHdAtlas *atlas,unsigned size,const void *data){
    return atlas->original && size==atlas->original_size && data &&
           !memcmp(atlas->original,data,(size_t)size*size*2);
}
#ifdef __cplusplus
extern "C" {
#endif
/* Configure only before a session starts; clear after its renderer has stopped.
 * Pack contents remain resident across GL context recreation. */
int nfs_glide_hd_atlas_load_file(const char *path);
void nfs_glide_hd_atlas_clear(void);
void nfs_glide_hd_builtin_enable(int enabled);
uint64_t nfs_glide_hd_atlas_upload_count(void);
#ifdef __cplusplus
}
#endif
#endif

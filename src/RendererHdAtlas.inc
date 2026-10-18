// SPDX-License-Identifier: MIT
#include "HdAtlas.h"
#include "BuiltinTiming.h"
static NfsHdAtlas nfs_hd_atlas;
static uint64_t nfs_hd_uploads;
static int nfs_hd_builtin_enabled;
int nfs_glide_hd_atlas_load_file(const char *path){
    nfs_hd_atlas_free(&nfs_hd_atlas);nfs_hd_uploads=0;nfs_hd_builtin_enabled=0;
    if(!path)return 0;
    FILE *file=fopen(path,"rb");if(!file)return 0;
    int result=nfs_hd_atlas_read(&nfs_hd_atlas,file);fclose(file);return result;
}
void nfs_glide_hd_builtin_enable(int enabled){nfs_hd_atlas_free(&nfs_hd_atlas);nfs_hd_uploads=0;nfs_hd_builtin_enabled=(enabled!=0);}
void nfs_glide_hd_atlas_clear(void){nfs_hd_atlas_free(&nfs_hd_atlas);nfs_hd_builtin_enabled=0;}
uint64_t nfs_glide_hd_atlas_upload_count(void){return nfs_hd_uploads;}
static int nfs_hd_upload_texture(TextureInfo *ti){
    if(ti->fmt!=GR_TEXFMT_ARGB_1555)return 0;
    if(nfs_hd_builtin_enabled && !nfs_hd_atlas.original)nfs_timing_build(&nfs_hd_atlas,ti->size,ti->data);
    if(!nfs_hd_atlas_matches(&nfs_hd_atlas,ti->size,ti->data))return 0;
    GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);
    if(limit<(GLint)nfs_hd_atlas.replacement_size)return 0;
    /* The original CPU texture and UVs stay intact. Normalized sampling places
     * the larger image in exactly the same screen rectangle in either view. */
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,nfs_hd_atlas.replacement_size,
                 nfs_hd_atlas.replacement_size,0,GL_RGBA,GL_UNSIGNED_BYTE,nfs_hd_atlas.rgba);
    ++nfs_hd_uploads;return 1;
}

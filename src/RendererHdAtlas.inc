// SPDX-License-Identifier: MIT
#include "HdAtlas.h"
#include "BuiltinTiming.h"
#include "HdGlyphSelect.h"
#include "SmallFontRuntime.inc"
#include <SDL3/SDL_init.h>
static NfsHdAtlas nfs_hd_atlas;
static uint64_t nfs_hd_uploads;
static int nfs_hd_builtin_enabled;
static int nfs_hd_visible=1;
static inline void drawTriangles(void);
int nfs_glide_hd_timing_mode(void){return nfs_hd_builtin_enabled?nfs_hd_visible:-1;}
int nfs_glide_hd_timing_set_visible(int visible){
    if(!nfs_hd_builtin_enabled || !SDL_IsMainThread() || !SDL_GL_GetCurrentContext())return 0;
    if(nfs_hd_visible!=(visible!=0)){
        drawTriangles(); // Finish queued geometry with its original selection.
        nfs_hd_visible=(visible!=0);
    }
    return 1;
}
static GLuint nfs_hd_aux_texture;
static int nfs_hd_selected,nfs_hd_uniform_sent;
static GLint nfs_hd_active_location;
static void nfs_hd_program_created(GLuint program){
    nfs_hd_active_location=glGetUniformLocation(program,"uHdActive");
    glUniform1i(glGetUniformLocation(program,"uHdTextureSampler"),1);
    glUniform1f(nfs_hd_active_location,0);nfs_hd_uniform_sent=0;nfs_hd_selected=0;
}
static void nfs_hd_sync_uniform(void){
    int active=nfs_hd_selected && nfs_hd_builtin_enabled && nfs_hd_visible && nfs_hd_aux_texture;
    if(active!=nfs_hd_uniform_sent){glUniform1f(nfs_hd_active_location,(GLfloat)active);nfs_hd_uniform_sent=active;}
}
static void nfs_hd_context_retired(int bound){
    if(bound && nfs_hd_aux_texture)glDeleteTextures(1,&nfs_hd_aux_texture);
    nfs_hd_aux_texture=0;nfs_hd_selected=0;nfs_hd_uniform_sent=-1;
}
static int nfs_hd_triangle_selected(const GrVertex *a,const GrVertex *b,const GrVertex *c){
    if(!nfs_hd_selected || !nfs_hd_builtin_enabled || !nfs_hd_visible || !nfs_hd_aux_texture)return 0;
    const GrVertex *vertices[3]={a,b,c};float uv[6];
    for(unsigned i=0;i<3;++i){
        float q=vertices[i]->oow;if(!(q>0))return 0;
        uv[2*i]=vertices[i]->tmuvtx[0].sow/q;uv[2*i+1]=vertices[i]->tmuvtx[0].tow/q;
    }
    if(nfs_hd_selected>=2 && nfs_hd_selected<=27){
        if(a->oow!=1 || b->oow!=1 || c->oow!=1)return 0;
        unsigned d=nfs_hd_selected-2;
        return nfs_hd_small_font_triangle(uv,nfs_small_font_edge[d])?12+d:0;
    }
    return nfs_hd_digit_triangle(uv);
}
int nfs_glide_hd_atlas_load_file(const char *path){
    nfs_hd_atlas_free(&nfs_hd_atlas);nfs_hd_uploads=0;nfs_hd_builtin_enabled=0;nfs_small_font_valid=0;
    if(!path)return 0;
    FILE *file=fopen(path,"rb");if(!file)return 0;
    int result=nfs_hd_atlas_read(&nfs_hd_atlas,file);fclose(file);return result;
}
void nfs_glide_hd_builtin_enable(int enabled){nfs_hd_atlas_free(&nfs_hd_atlas);nfs_hd_uploads=0;nfs_hd_builtin_enabled=(enabled!=0);nfs_hd_visible=1;nfs_small_font_valid=0;}
void nfs_glide_hd_atlas_clear(void){nfs_hd_atlas_free(&nfs_hd_atlas);nfs_hd_builtin_enabled=0;nfs_small_font_valid=0;}
uint64_t nfs_glide_hd_atlas_upload_count(void){return nfs_hd_uploads;}
static int nfs_hd_upload_texture(TextureInfo *ti){
    ti->hdDigits=0;
    if(ti->fmt!=GR_TEXFMT_ARGB_1555)return 0;
    if(nfs_hd_builtin_enabled && nfs_small_font_match(ti))return 0;
    if(nfs_hd_builtin_enabled && !nfs_hd_atlas.original)nfs_timing_build(&nfs_hd_atlas,ti->size,ti->data);
    if(!nfs_hd_atlas_matches(&nfs_hd_atlas,ti->size,ti->data))return 0;
    GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);
    if(limit<(GLint)nfs_hd_atlas.replacement_size)return 0;
    if(nfs_hd_builtin_enabled){
        if(!nfs_hd_aux_texture){
            glActiveTexture(GL_TEXTURE1);glGenTextures(1,&nfs_hd_aux_texture);glBindTexture(GL_TEXTURE_2D,nfs_hd_aux_texture);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,nfs_hd_atlas.replacement_size,nfs_hd_atlas.replacement_size,0,GL_RGBA,GL_UNSIGNED_BYTE,nfs_hd_atlas.rgba);
            glActiveTexture(GL_TEXTURE0);++nfs_hd_uploads;
        }
        ti->hdDigits=1;return 0; // Keep the original GPU atlas and its filtering.
    }
    /* The original CPU texture and UVs stay intact. Normalized sampling places
     * the larger image in exactly the same screen rectangle in either view. */
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,nfs_hd_atlas.replacement_size,
                 nfs_hd_atlas.replacement_size,0,GL_RGBA,GL_UNSIGNED_BYTE,nfs_hd_atlas.rgba);
    ++nfs_hd_uploads;return 1;
}

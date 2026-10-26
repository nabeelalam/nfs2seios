#include "FrontendWordMode.h"
static void nfs_front_hit_refresh(void);
// SPDX-License-Identifier: MIT
#include "HdAtlas.h"
#include "BuiltinTiming.h"
#include "HdGlyphSelect.h"
#include "SmallFontRuntime.inc"
#include "PauseFontRuntime.inc"
#include "LabelCellLayout.inc"
#include <SDL3/SDL_init.h>
static unsigned char nfs_logo_original[8192];
static int nfs_logo_valid;
static int nfs_logo_match(TextureInfo *ti){
 if(ti->fmt!=GR_TEXFMT_ARGB_4444 || ti->size!=64)return 0;unsigned char digest[32];
 if(!nfs_texture_sha256(ti->data,8192,digest) || memcmp(digest,nfs_logo_original_digest,32))return 0;
 if(ti->data!=nfs_logo_original)memcpy(nfs_logo_original,ti->data,8192);nfs_logo_valid=1;ti->hdDigits=39;return 1;
}
static int nfs_logo_triangle(const float uv[6]){
 unsigned corners=0;for(unsigned i=0;i<3;++i){float u=uv[2*i],v=uv[2*i+1];
 if(!((u>=1.9821875f && u<=2.0021875f)||(u>=254.99f && u<=255.01f)) || !((v>=1.9821875f && v<=2.0021875f)||(v>=254.99f && v<=255.01f)))return 0;
 corners|=1u<<((u>128)+(v>128)*2);}
 return corners==7||corners==11||corners==13||corners==14;
}
static int nfs_alternate_logo_triangle(const float uv[6]){
 unsigned corners=0;for(unsigned i=0;i<3;++i){unsigned x=2,y=2;
  if(fabsf(uv[2*i]-.498046875f)<=.01f)x=0;else if(fabsf(uv[2*i]-47.8125f)<=.01f)x=1;
  if(fabsf(uv[2*i+1]-21.416015625f)<=.01f)y=0;else if(fabsf(uv[2*i+1]-43.828125f)<=.01f)y=1;
  if(x>1 || y>1)return 0;corners|=1u<<(x+y*2);
 }
 return corners==7 || corners==11 || corners==13 || corners==14;
}
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
    nfs_front_hit_refresh();return 1;
}
static GLuint nfs_hd_aux_texture,nfs_hd_bound_aux_texture;
static int nfs_hd_selected,nfs_hd_uniform_sent;
static GLint nfs_hd_active_location;
#include "SilverSlashRuntime.inc"
#include "FrontendFontRuntime.inc"
static void nfs_front_hit_refresh(void){int next=nfs_hd_builtin_enabled && nfs_hd_visible && nfs_hd_cell_layout_available && nfs_front_font_texture && nfs_front_source_matched[0];nfs_front_word_set_available(next!=0);}

static void nfs_hd_program_created(GLuint program){
    GLint cell=glGetUniformLocation(program,"uHdCellLayout");nfs_hd_cell_layout_available=cell>=0;if(cell>=0)glUniform1f(cell,1);
    nfs_hd_active_location=glGetUniformLocation(program,"uHdActive");
    glUniform1i(glGetUniformLocation(program,"uHdTextureSampler"),1);
    glUniform1f(nfs_hd_active_location,0);nfs_hd_uniform_sent=0;nfs_hd_selected=0;nfs_front_hit_refresh();
}
static void nfs_hd_sync_uniform(void){
    GLuint auxiliary=nfs_front_font_source_page(nfs_hd_selected)>=0?nfs_front_font_texture:nfs_hd_aux_texture;
    int active=nfs_hd_selected && nfs_hd_builtin_enabled && nfs_hd_visible && auxiliary;
    if(active && nfs_hd_bound_aux_texture!=auxiliary){glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,auxiliary);glActiveTexture(GL_TEXTURE0);nfs_hd_bound_aux_texture=auxiliary;}
    if(active!=nfs_hd_uniform_sent){glUniform1f(nfs_hd_active_location,(GLfloat)active);nfs_hd_uniform_sent=active;}
}
static void nfs_hd_context_retired(int bound){
    if(bound && nfs_hd_aux_texture)glDeleteTextures(1,&nfs_hd_aux_texture);
    if(bound && nfs_front_font_texture)glDeleteTextures(1,&nfs_front_font_texture);
    nfs_front_font_texture=0;nfs_front_font_uploaded_height=0;nfs_hd_bound_aux_texture=0;nfs_hd_aux_texture=0;nfs_hd_selected=0;nfs_hd_uniform_sent=-1;for(unsigned p=0;p<2;++p){nfs_front_source_matched[p]=0;nfs_front_source_owner[p]=NULL;}nfs_front_extended_ready=0;nfs_front_hit_refresh();
}

#include "StartLightRuntime.inc"
static int nfs_hd_triangle_selected(const GrVertex *a,const GrVertex *b,const GrVertex *c){
    if(!nfs_hd_selected || !nfs_hd_builtin_enabled || !nfs_hd_visible || !nfs_hd_aux_texture)return 0;
    const GrVertex *vertices[3]={a,b,c};float uv[6];
    for(unsigned i=0;i<3;++i){
        float q=vertices[i]->oow;if(!(q>0))return 0;
        uv[2*i]=vertices[i]->tmuvtx[0].sow/q;uv[2*i+1]=vertices[i]->tmuvtx[0].tow/q;
    }
    if(nfs_front_font_source_page(nfs_hd_selected)>=0)return 0;
    // hd_selected1 comes only from the existing exact builtin timing-atlas SHA.
    // The uniform availability guard also preserves basic/lowp original fallback.
    if(nfs_hd_selected==1 && nfs_hd_cell_layout_available){
        NfsWarningWordVertex warning[3];
        for(unsigned i=0;i<3;++i){const GrVertex*v=vertices[i];NfsWarningWordVertex value={v->x-VertexSnap,v->y-VertexSnap,v->oow,v->r,v->g,v->b,v->a};warning[i]=value;}
        const int selected=nfs_warning_word_triangle(warning,uv);if(selected)return selected;
    }
    if(nfs_hd_selected==1 && nfs_hd_cell_layout_available && nfs_start_light_triangle(a,b,c,uv))return 71;
    if(nfs_hd_selected==1 && a->oow==1 && b->oow==1 && c->oow==1){int gear=nfs_gear_triangle(uv);if(gear)return gear;if(nfs_hd_cell_layout_available && nfs_clock_punctuation_triangle(uv))return 69;}
    if(nfs_hd_selected==1 && a->oow==1 && b->oow==1 && c->oow==1){int gauge=nfs_gauge_mask_triangle(uv);if(gauge)return gauge;}
    if(nfs_hd_selected==1 && a->oow==1 && b->oow==1 && c->oow==1 && nfs_alternate_logo_triangle(uv))return 56;
    if(nfs_hd_selected==40)return a->oow==1 && b->oow==1 && c->oow==1?nfs_pause_triangle(uv):0;
    if(nfs_hd_selected==39)return a->oow==1 && b->oow==1 && c->oow==1 && nfs_logo_triangle(uv)?49:0;
    if(nfs_hd_selected>=2 && nfs_hd_selected<=27){
        if(a->oow!=1 || b->oow!=1 || c->oow!=1)return 0;
        unsigned d=nfs_hd_selected-2;
        return nfs_hd_small_font_triangle(uv,nfs_small_font_edge[d])?12+d:0;
    }
    if(nfs_hd_selected>=28 && nfs_hd_selected<=38){
        if(a->oow!=1 || b->oow!=1 || c->oow!=1)return 0;
        unsigned d=nfs_hd_selected-28;
        return nfs_hd_small_font_triangle(uv,nfs_record_font_edge[d])?38+d:0;
    }
    return nfs_hd_digit_triangle(uv);
}
int nfs_glide_hd_atlas_load_file(const char *path){
    nfs_hd_atlas_free(&nfs_hd_atlas);nfs_hd_uploads=0;nfs_hd_builtin_enabled=0;nfs_small_font_valid=0;nfs_record_font_valid=0;nfs_logo_valid=0;nfs_pause_valid=0;
    nfs_front_hit_refresh();if(!path)return 0;
    FILE *file=fopen(path,"rb");if(!file)return 0;
    int result=nfs_hd_atlas_read(&nfs_hd_atlas,file);fclose(file);return result;
}
void nfs_glide_hd_builtin_enable(int enabled){nfs_hd_atlas_free(&nfs_hd_atlas);nfs_hd_uploads=0;nfs_hd_builtin_enabled=(enabled!=0);nfs_hd_visible=1;nfs_small_font_valid=0;nfs_record_font_valid=0;nfs_logo_valid=0;nfs_pause_valid=0;nfs_front_hit_refresh();}
void nfs_glide_hd_atlas_clear(void){nfs_hd_atlas_free(&nfs_hd_atlas);nfs_hd_builtin_enabled=0;nfs_small_font_valid=0;nfs_record_font_valid=0;nfs_logo_valid=0;nfs_pause_valid=0;nfs_front_hit_refresh();}
uint64_t nfs_glide_hd_atlas_upload_count(void){return nfs_hd_uploads;}
static int nfs_hd_upload_texture(TextureInfo *ti){
    ti->hdDigits=0;
    if(nfs_front_font_upload(ti))return 0;
    if(nfs_hd_builtin_enabled && nfs_pause_match(ti))return 0;
    if(nfs_hd_builtin_enabled && nfs_logo_match(ti))return 0;
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
            nfs_hd_bound_aux_texture=nfs_hd_aux_texture;glActiveTexture(GL_TEXTURE0);++nfs_hd_uploads;
        }
        ti->hdDigits=1;return 0; // Keep the original GPU atlas and its filtering.
    }
    /* The original CPU texture and UVs stay intact. Normalized sampling places
     * the larger image in exactly the same screen rectangle in either view. */
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,nfs_hd_atlas.replacement_size,
                 nfs_hd_atlas.replacement_size,0,GL_RGBA,GL_UNSIGNED_BYTE,nfs_hd_atlas.rgba);
    ++nfs_hd_uploads;return 1;
}

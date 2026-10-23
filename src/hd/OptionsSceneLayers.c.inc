// SPDX-License-Identifier: MIT
#include "OptionsSceneLayers.h"
#ifndef NFS_OPTIONS_SCENE_EXTERNAL_GRAPHICS_IMPLEMENTATION
#include "GraphicsSceneLayers.c.inc"
#endif
#define NFS_OPTIONS_LAYER_GL(call) do{if(!nfs_graphics_scene_current(t))return 0;call;if(!nfs_graphics_scene_current(t))return 0;}while(0)
static int nfs_options_scene_ready(NfsOptionsSceneGpu*s,const NfsBackgroundSceneStage*t){
 NfsBackgroundSceneCompositor*b=s->background;GLboolean yes;unsigned i;
 if(b->generation!=t->generation||b->ownerGeneration!=t->generation||!b->program||!b->copyProgram||b->matrix<0||b->sampler<0||b->opacity<0||b->copySampler<0||b->logo.generation!=t->generation||b->logo.ownerGeneration!=t->generation||!b->logo.program||!b->logo.texture||!b->logo.artIdentity||b->logo.matrix<0||b->logo.sampler<0||memcmp(b->logo.regions,nfs_logo_regions,sizeof nfs_logo_regions))return 0;
 {GLuint programs[4]={b->program,b->copyProgram,b->logo.program,s->caption->program};GLuint textures[7]={b->textures[0],b->textures[1],b->textures[2],b->textures[3],b->textures[4],b->logo.texture,s->caption->texture};
 for(i=0;i<5;++i)if(!b->art[i].identity||!b->textures[i])return 0;
 for(i=0;i<4;++i){NFS_OPTIONS_LAYER_GL(yes=glIsProgram(programs[i]));if(!yes)return 0;}
 for(i=0;i<7;++i){NFS_OPTIONS_LAYER_GL(yes=glIsTexture(textures[i]));if(!yes)return 0;}}
 return 1;
}
static int nfs_options_scene_target(NfsOptionsSceneGpu*s,const NfsBackgroundSceneStage*t){
 /* The shared validator is source-neutral; using caption for both read-only
    font aliases checks the same attachment without requesting a640text layer. */
 NfsGraphicsSceneGpu g;g.background=s->background;g.sidebarCaption=s->caption;g.text=s->caption;return nfs_graphics_scene_target(&g,t);
}
static int nfs_options_scene_draw_background(NfsOptionsSceneGpu*s,const NfsBackgroundSceneStage*t,const NfsOptionsSceneLayers*v){
 NfsBackgroundSceneCompositor*b=s->background;NfsLogoVertex logos[18];size_t count=0;float bounds[4];GLenum error;static const int32_t full[4]={0,0,640,480};
 if(!nfs_logo_layers(v->background.logos,3,logos,&count,bounds))return 0;
 for(unsigned i=0;i<8;++i){NFS_OPTIONS_LAYER_GL(glDisable(nfs_frame_caps[i]));}NFS_OPTIONS_LAYER_GL(glColorMask(1,1,1,1));NFS_OPTIONS_LAYER_GL(glActiveTexture(GL_TEXTURE0));NFS_OPTIONS_LAYER_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_OPTIONS_LAYER_GL(glDepthMask(GL_FALSE));
 if(!nfs_graphics_scene_quad(b,t,v->background.mapRole,0,0,640,480,0,0,b->art[v->background.mapRole].width,b->art[v->background.mapRole].height,full,256,0)||!nfs_graphics_scene_quad(b,t,2,0,0,343,480,0,0,1372,1920,v->background.panelClip,256,1))return 0;
 for(unsigned i=0;i<v->background.wordCount;++i){const NfsBackgroundSceneWordLayer*w=v->background.words+i;const NfsBackgroundSceneWord*r=b->regions+w->sourceRole-1;if(!nfs_graphics_scene_quad(b,t,r->textureRole,(float)w->left,(float)w->top,(float)r->logicalWidth,(float)r->logicalHeight,r->x,r->y,r->width,r->height,w->clip,w->scalar256,1))return 0;}
 if(count){NFS_OPTIONS_LAYER_GL(glUseProgram(b->logo.program));NFS_OPTIONS_LAYER_GL(glUniformMatrix4fv(b->logo.matrix,1,GL_FALSE,t->sampling.projection));NFS_OPTIONS_LAYER_GL(glUniform1i(b->logo.sampler,0));NFS_OPTIONS_LAYER_GL(glBindTexture(GL_TEXTURE_2D,b->logo.texture));NFS_OPTIONS_LAYER_GL(glEnableVertexAttribArray(0));NFS_OPTIONS_LAYER_GL(glEnableVertexAttribArray(1));NFS_OPTIONS_LAYER_GL(glDisableVertexAttribArray(2));NFS_OPTIONS_LAYER_GL(glDisableVertexAttribArray(3));NFS_OPTIONS_LAYER_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),logos));NFS_OPTIONS_LAYER_GL(glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),&logos[0].coefficient));if(!nfs_graphics_scene_blend(t,1))return 0;NFS_OPTIONS_LAYER_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count));NFS_OPTIONS_LAYER_GL(error=glGetError());if(error!=GL_NO_ERROR)return 0;}
 return 1;
}
static int nfs_options_scene_draw(NfsOptionsSceneGpu*s,const NfsBackgroundSceneStage*t,const NfsOptionsSceneLayers*v){return nfs_options_scene_draw_background(s,t,v)&&nfs_graphics_scene_text(s->caption,t,&v->caption);}
static int nfs_options_scene_abandon(NfsOptionsSceneGpu*s){nfs_bg_scene_context_retired(s->background,s->background->generation,0);nfs_soft_gpu_context_retired(s->caption,s->caption->generation,0);return 0;}
int nfs_options_scene_draw_layers_into_staging(NfsOptionsSceneGpu*s,const NfsBackgroundSceneStage*t,const NfsOptionsSceneLayers*v){
 NfsBgSaved saved;int ok;
 if(!s||!s->background||!s->caption||!v||!nfs_graphics_scene_current(t)||!nfs_bg_stage_layers(&v->background)||!nfs_graphics_scene_font(s->caption,&v->caption,343,t->generation))return 0;
 if(!nfs_options_scene_ready(s,t)||!nfs_options_scene_target(s,t))return nfs_graphics_scene_current(t)?0:nfs_options_scene_abandon(s);
 if(!nfs_graphics_scene_save(&saved,t))return nfs_options_scene_abandon(s);
 ok=nfs_options_scene_draw(s,t,v);if(!nfs_graphics_scene_current(t))return nfs_options_scene_abandon(s);
 if(!nfs_graphics_scene_restore(&saved,t))return nfs_options_scene_abandon(s);
 return nfs_graphics_scene_current(t)?ok:nfs_options_scene_abandon(s);
}
#undef NFS_OPTIONS_LAYER_GL

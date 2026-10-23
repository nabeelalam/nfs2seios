// SPDX-License-Identifier: MIT
#include "AudioSceneLayers.h"
#ifndef NFS_AUDIO_SCENE_EXTERNAL_BACKGROUND_IMPLEMENTATION
#include "BackgroundSceneCompositor.c.inc"
#endif
#ifndef NFS_AUDIO_SCENE_EXTERNAL_BAR_IMPLEMENTATION
#define NFS_AUDIO_BAR_EXTERNAL_SCENE_IMPLEMENTATION
#include "AudioBarGpu.c.inc"
#undef NFS_AUDIO_BAR_EXTERNAL_SCENE_IMPLEMENTATION
#endif
#include "NestedCaptionGpu.c.inc"
#include "AudioDialogWordGpu.c.inc"
static int nfs_audio_scene_font(const NfsSoftwareTextGpu*s,const NfsSoftwareTextLayerView*l,unsigned logical){
 NfsSoftwareTextStage stage;float bounds[4];if(!s||!l||l->logicalWidth!=logical||l->logicalHeight!=480||l->font.schema!=1||l->font.width!=1024||l->font.height!=512||l->font.bytes!=1024u*512u||!l->font.alpha||!l->font.identity||s->fontIdentity!=l->font.identity||memcmp(s->fontSha256,l->font.sha256,32)||!nfs_soft_gpu_ready(s))return 0;
 memset(&stage,0,sizeof stage);stage.logicalWidth=logical;stage.logicalHeight=480;return nfs_soft_gpu_vertices(&stage,l->vertices,l->vertexCount,bounds);
}
static int nfs_audio_scene_text(NfsSoftwareTextGpu*s,const NfsBackgroundSceneStage*t,const NfsSoftwareTextLayerView*l){
 NfsBgSaved saved;int ok;if(!nfs_bg_stage_owner(t)||s->generation!=t->generation)return 0;nfs_bg_save(&saved);nfs_frame_draw_state();glDepthMask(GL_FALSE);glUseProgram(s->program);glUniformMatrix4fv(s->matrix,1,GL_FALSE,t->sampling.projection);glUniform1i(s->sampler,0);glBindTexture(GL_TEXTURE_2D,s->texture);glEnableVertexAttribArray(0);glEnableVertexAttribArray(1);glDisableVertexAttribArray(2);glDisableVertexAttribArray(3);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),l->vertices);glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),&l->vertices[0].r);glEnable(GL_BLEND);glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);glDrawArrays(GL_TRIANGLES,0,(GLsizei)l->vertexCount);
 if(!nfs_bg_stage_owner(t))return 0;ok=glGetError()==GL_NO_ERROR;nfs_bg_restore(&saved);return nfs_bg_stage_owner(t)&&ok;
}
static int nfs_audio_scene_word(NfsNestedCaptionGpu*s,const NfsBackgroundSceneStage*t,const NfsCompactCaptionWordView*v){NfsBgSaved saved;int ok;if(!nfs_nested_caption_save(&saved,t))return 0;ok=nfs_nested_caption_draw(s,v,t);if(!nfs_nested_caption_current(t))return 0;if(!nfs_nested_caption_restore(&saved,t))return 0;return ok;}
static int nfs_audio_scene_abandon(NfsAudioSceneGpu*s){if(s->dialogWords)nfs_audio_dialog_forget(s->dialogWords);if(s->sidebarWord)nfs_nested_caption_forget(s->sidebarWord);if(s->background)nfs_bg_scene_context_retired(s->background,s->background->generation,0);if(s->sidebarCaption)nfs_soft_gpu_context_retired(s->sidebarCaption,s->sidebarCaption->generation,0);if(s->text)nfs_soft_gpu_context_retired(s->text,s->text->generation,0);if(s->bars)nfs_audio_bar_context_retired(s->bars,s->bars->generation,0);return 0;}
int nfs_audio_scene_draw_layers_into_staging(NfsAudioSceneGpu*s,const NfsBackgroundSceneStage*t,const NfsAudioSceneLayers*v){
 NfsAudioBarStage bars;GLint type=0,name=0;
 if(!s||!s->background||!s->bars||!s->sidebarCaption||!s->text||!v||!nfs_audio_dialog_valid(&v->dialogWords,&v->text)||(!nfs_audio_dialog_empty(&v->dialogWords)&&!s->dialogWords)||!nfs_nested_caption_valid(&v->sidebarWord,3)||(!nfs_nested_caption_empty(&v->sidebarWord)&&!s->sidebarWord)||!nfs_bg_stage_owner(t)||!nfs_bg_stage_target(t)||s->background->generation!=t->generation||s->bars->generation!=t->generation||s->bars->contextIdentity!=t->contextIdentity||s->sidebarCaption->generation!=t->generation||s->text->generation!=t->generation||!nfs_bg_stage_layers(&v->background)||!nfs_bg_ready(s->background)||!nfs_audio_bar_ready(s->bars)||!nfs_audio_bar_vertices(v->bars,v->barVertices)||!nfs_audio_scene_font(s->sidebarCaption,&v->sidebarCaption,343)||!nfs_audio_scene_font(s->text,&v->text,640))return 0;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);if(type!=GL_TEXTURE||!name||(GLuint)name==s->bars->texture||(GLuint)name==s->sidebarCaption->texture||(GLuint)name==s->text->texture||(!nfs_nested_caption_empty(&v->sidebarWord)&&(GLuint)name==s->sidebarWord->texture)||glGetError()!=GL_NO_ERROR)return 0;
 if(!nfs_audio_dialog_empty(&v->dialogWords)){unsigned i;for(i=0;i<8;++i)if((GLuint)name==s->dialogWords->textures[i])return 0;if(nfs_audio_dialog_alias(s->dialogWords,s->background,s->sidebarCaption,s->text,s->bars,s->sidebarWord,(GLuint)name,0)||!nfs_audio_dialog_ready(s->dialogWords,&v->dialogWords,&v->text,t))return nfs_bg_stage_owner(t)?0:nfs_audio_scene_abandon(s);}
 if(!nfs_nested_caption_empty(&v->sidebarWord)&&(nfs_nested_caption_alias(s->sidebarWord,s->background,s->sidebarCaption,s->text,s->bars->texture,s->bars->program,0,0)||!nfs_nested_caption_ready(s->sidebarWord,&v->sidebarWord,t)))return nfs_bg_stage_owner(t)?0:nfs_audio_scene_abandon(s);
 if(!nfs_bg_scene_draw_layers_into_staging(s->background,t,&v->background))return nfs_bg_stage_owner(t)?0:nfs_audio_scene_abandon(s);
 if(!(nfs_nested_caption_empty(&v->sidebarWord)?nfs_audio_scene_text(s->sidebarCaption,t,&v->sidebarCaption):nfs_audio_scene_word(s->sidebarWord,t,&v->sidebarWord)))return nfs_bg_stage_owner(t)?0:nfs_audio_scene_abandon(s);
 memset(&bars,0,sizeof bars);bars.generation=t->generation;bars.framebuffer=t->framebuffer;bars.width=t->width;bars.height=t->height;bars.sampling=t->sampling;
 if(!nfs_audio_bar_draw_into_staging(s->bars,&bars,v->bars,v->barVertices))return nfs_bg_stage_owner(t)?0:nfs_audio_scene_abandon(s);
 if(!(nfs_audio_dialog_empty(&v->dialogWords)?nfs_audio_scene_text(s->text,t,&v->text):nfs_audio_dialog_draw(s->dialogWords,&v->dialogWords,s->text,&v->text,t)))return nfs_bg_stage_owner(t)?0:nfs_audio_scene_abandon(s);
 return nfs_bg_stage_owner(t)?1:nfs_audio_scene_abandon(s);
}

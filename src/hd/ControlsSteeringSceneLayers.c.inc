// SPDX-License-Identifier: MIT
/* Child Controls-owned composition mechanics only. Caller must independently join
   genuine current source/FIFO/native-upload authority and prepare all resources.
   The exact OPTIONS caption view is mandatory. A refusal retains caller original
   fallback; any late failure discards this entire unpublished staging image. */
#include "ControlsSteeringSceneLayers.h"
#ifndef NFS_CONTROLS_STEERING_SCENE_EXTERNAL_BACKGROUND_IMPLEMENTATION
#include "BackgroundSceneCompositor.c.inc"
#endif
#include "NestedCaptionGpu.c.inc"
#include "ControlsSteeringWordGpu.c.inc"
#define NFS_CONTROLS_STEERING_BAR_EXTERNAL_SCENE_IMPLEMENTATION 1
#include "ControlsSteeringBarGpu.c"
#undef NFS_CONTROLS_STEERING_BAR_EXTERNAL_SCENE_IMPLEMENTATION
static int nfs_controls_steering_scene_current(const NfsBackgroundSceneStage*t){return t&&t->generation&&t->contextIdentity&&t->framebuffer&&t->ownerCurrent&&t->ownerCurrent(t->ownerUser,t);}
#define NFS_CONTROLS_STEERING_GL(call) do{if(!nfs_controls_steering_scene_current(t))return 0;call;if(!nfs_controls_steering_scene_current(t))return 0;}while(0)
/* These two local helpers only require the supplied trusted owner callback,
   generation and context. They do not demand a particular current FBO: resource
   preparation may use a context/thread-only callback and change bindings before
   restoration. The public draw entry must retain its stricter registered-stage
   callback throughout; it separately validates framebuffer/viewport/aliases.
   A false return means owner loss, not permission to clean up through GL. */
static int nfs_controls_steering_scene_save(NfsBgSaved*v,const NfsBackgroundSceneStage*t){NfsFrameSaved*s=&v->common;unsigned i;NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&s->program));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_ACTIVE_TEXTURE,&s->active));NFS_CONTROLS_STEERING_GL(glActiveTexture(GL_TEXTURE0));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_TEXTURE_BINDING_2D,&s->texture));NFS_CONTROLS_STEERING_GL(glActiveTexture(s->active));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&s->buffer));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&s->fbo));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_RENDERBUFFER_BINDING,&s->rbo));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_VIEWPORT,s->viewport));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_PACK_ALIGNMENT,&s->pack));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_UNPACK_ALIGNMENT,&s->unpack));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_BLEND_SRC_RGB,&s->sr));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_BLEND_DST_RGB,&s->dr));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_BLEND_SRC_ALPHA,&s->sa));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_BLEND_DST_ALPHA,&s->da));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_BLEND_EQUATION_RGB,&s->eqr));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&s->eqa));NFS_CONTROLS_STEERING_GL(glGetBooleanv(GL_COLOR_WRITEMASK,s->mask));
 for(i=0;i<8;++i){NFS_CONTROLS_STEERING_GL(s->caps[i]=glIsEnabled(nfs_frame_caps[i]));}for(i=0;i<4;++i){NfsSoftwareTextGpuAttrib*a=s->attributes+i;NFS_CONTROLS_STEERING_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_ENABLED,&a->enabled));NFS_CONTROLS_STEERING_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_SIZE,&a->size));NFS_CONTROLS_STEERING_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_TYPE,&a->type));NFS_CONTROLS_STEERING_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,&a->normalized));NFS_CONTROLS_STEERING_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_STRIDE,&a->stride));NFS_CONTROLS_STEERING_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING,&a->buffer));NFS_CONTROLS_STEERING_GL(glGetVertexAttribPointerv(i,GL_VERTEX_ATTRIB_ARRAY_POINTER,&a->pointer));NFS_CONTROLS_STEERING_GL(glGetVertexAttribfv(i,GL_CURRENT_VERTEX_ATTRIB,a->current));}NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_DEPTH_FUNC,&v->depthFunc));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_SCISSOR_BOX,v->scissor));NFS_CONTROLS_STEERING_GL(glGetBooleanv(GL_DEPTH_WRITEMASK,&v->depthMask));NFS_CONTROLS_STEERING_GL(glGetFloatv(GL_DEPTH_CLEAR_VALUE,&v->depthClear));return 1;}
static int nfs_controls_steering_scene_restore(const NfsBgSaved*v,const NfsBackgroundSceneStage*t){const NfsFrameSaved*s=&v->common;unsigned i;NFS_CONTROLS_STEERING_GL(glDepthFunc(v->depthFunc));NFS_CONTROLS_STEERING_GL(glDepthMask(v->depthMask));NFS_CONTROLS_STEERING_GL(glClearDepthf(v->depthClear));NFS_CONTROLS_STEERING_GL(glScissor(v->scissor[0],v->scissor[1],v->scissor[2],v->scissor[3]));for(i=0;i<4;++i){const NfsSoftwareTextGpuAttrib*a=s->attributes+i;NFS_CONTROLS_STEERING_GL(glBindBuffer(GL_ARRAY_BUFFER,a->buffer));NFS_CONTROLS_STEERING_GL(glVertexAttribPointer(i,a->size,a->type,a->normalized,a->stride,a->pointer));NFS_CONTROLS_STEERING_GL(glVertexAttrib4fv(i,a->current));if(a->enabled){NFS_CONTROLS_STEERING_GL(glEnableVertexAttribArray(i));}else{NFS_CONTROLS_STEERING_GL(glDisableVertexAttribArray(i));}}NFS_CONTROLS_STEERING_GL(glBindBuffer(GL_ARRAY_BUFFER,s->buffer));NFS_CONTROLS_STEERING_GL(glActiveTexture(GL_TEXTURE0));NFS_CONTROLS_STEERING_GL(glBindTexture(GL_TEXTURE_2D,s->texture));NFS_CONTROLS_STEERING_GL(glActiveTexture(s->active));NFS_CONTROLS_STEERING_GL(glUseProgram(s->program));NFS_CONTROLS_STEERING_GL(glBindFramebuffer(GL_FRAMEBUFFER,s->fbo));NFS_CONTROLS_STEERING_GL(glBindRenderbuffer(GL_RENDERBUFFER,s->rbo));NFS_CONTROLS_STEERING_GL(glViewport(s->viewport[0],s->viewport[1],s->viewport[2],s->viewport[3]));NFS_CONTROLS_STEERING_GL(glPixelStorei(GL_PACK_ALIGNMENT,s->pack));NFS_CONTROLS_STEERING_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,s->unpack));NFS_CONTROLS_STEERING_GL(glColorMask(s->mask[0],s->mask[1],s->mask[2],s->mask[3]));NFS_CONTROLS_STEERING_GL(glBlendEquationSeparate(s->eqr,s->eqa));NFS_CONTROLS_STEERING_GL(glBlendFuncSeparate(s->sr,s->dr,s->sa,s->da));for(i=0;i<8;++i){if(s->caps[i]){NFS_CONTROLS_STEERING_GL(glEnable(nfs_frame_caps[i]));}else{NFS_CONTROLS_STEERING_GL(glDisable(nfs_frame_caps[i]));}}return 1;}

static int nfs_controls_steering_scene_font(const NfsSoftwareTextGpu*s,const NfsSoftwareTextLayerView*l,unsigned width,uint64_t generation){
 NfsSoftwareTextStage stage;float bounds[4];
 if(!s||!l||s->generation!=generation||s->ownerGeneration!=generation||!s->program||!s->texture||s->matrix<0||s->sampler<0||l->logicalWidth!=width||l->logicalHeight!=480||l->font.schema!=1||l->font.width!=1024||l->font.height!=512||l->font.bytes!=1024u*512u||!l->font.alpha||!l->font.identity||s->fontIdentity!=l->font.identity||memcmp(s->fontSha256,l->font.sha256,32))return 0;
 memset(&stage,0,sizeof stage);stage.logicalWidth=width;stage.logicalHeight=480;
 return nfs_soft_gpu_vertices(&stage,l->vertices,l->vertexCount,bounds);
}
static int nfs_controls_steering_scene_ready(NfsControlsSteeringSceneGpu*s,const NfsBackgroundSceneStage*t){
 NfsBackgroundSceneCompositor*b=s->background;GLboolean yes;unsigned i;
 if(b->generation!=t->generation||b->ownerGeneration!=t->generation||!b->program||!b->copyProgram||b->matrix<0||b->sampler<0||b->opacity<0||b->copySampler<0||b->logo.generation!=t->generation||b->logo.ownerGeneration!=t->generation||!b->logo.program||!b->logo.texture||!b->logo.artIdentity||b->logo.matrix<0||b->logo.sampler<0||memcmp(b->logo.regions,nfs_logo_regions,sizeof nfs_logo_regions))return 0;
 GLuint programs[4]={b->program,b->logo.program,s->sidebarCaption->program,s->text->program};
 GLuint textures[8]={b->textures[0],b->textures[1],b->textures[2],b->textures[3],b->textures[4],b->logo.texture,s->sidebarCaption->texture,s->text->texture};
 for(i=0;i<5;++i)if(!b->art[i].identity||!b->textures[i])return 0;
 for(i=0;i<4;++i){NFS_CONTROLS_STEERING_GL(yes=glIsProgram(programs[i]));if(!yes)return 0;}
 for(i=0;i<8;++i){NFS_CONTROLS_STEERING_GL(yes=glIsTexture(textures[i]));if(!yes)return 0;}
 return 1;
}
static int nfs_controls_steering_scene_target(NfsControlsSteeringSceneGpu*s,const NfsBackgroundSceneStage*t){
 GLint fbo=0,vp[4],sc[4],maxTexture=0,maxViewport[2]={0,0},type=0,name=0;GLenum status;GLboolean enabled;
 if(!t->width||!t->height||t->width>NFS_FRAME_MAX_DIMENSION||t->height>NFS_FRAME_MAX_DIMENSION||(uint64_t)t->width*t->height>NFS_FRAME_MAX_PIXELS||!nfs_frame_sampling_valid(t->width,t->height,&t->sampling)||t->framebuffer==s->background->fbo[0]||t->framebuffer==s->background->fbo[1])return 0;
 NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTexture));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_MAX_VIEWPORT_DIMS,maxViewport));
 if(t->width>(unsigned)maxTexture||t->height>(unsigned)maxTexture||t->width>(unsigned)maxViewport[0]||t->height>(unsigned)maxViewport[1])return 0;
 NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_VIEWPORT,vp));
 if((GLuint)fbo!=t->framebuffer||memcmp(vp,t->sampling.viewport,sizeof vp))return 0;
 NFS_CONTROLS_STEERING_GL(enabled=glIsEnabled(GL_SCISSOR_TEST));if(enabled){NFS_CONTROLS_STEERING_GL(glGetIntegerv(GL_SCISSOR_BOX,sc));if(sc[0]>0||sc[1]>0||sc[2]<0||sc[3]<0||(int64_t)sc[0]+sc[2]<t->width||(int64_t)sc[1]+sc[3]<t->height)return 0;}
 NFS_CONTROLS_STEERING_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));if(status!=GL_FRAMEBUFFER_COMPLETE)return 0;
 NFS_CONTROLS_STEERING_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CONTROLS_STEERING_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));
 if(type!=GL_TEXTURE||!name||(GLuint)name==s->sidebarCaption->texture||(GLuint)name==s->text->texture||(GLuint)name==s->background->logo.texture)return 0;
 for(unsigned i=0;i<5;++i)if((GLuint)name==s->background->textures[i])return 0;
 NFS_CONTROLS_STEERING_GL(status=glGetError());return status==GL_NO_ERROR;
}
static int nfs_controls_steering_scene_blend(const NfsBackgroundSceneStage*t,int enabled){
 if(enabled){NFS_CONTROLS_STEERING_GL(glEnable(GL_BLEND));NFS_CONTROLS_STEERING_GL(glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD));NFS_CONTROLS_STEERING_GL(glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA));}else{NFS_CONTROLS_STEERING_GL(glDisable(GL_BLEND));}return 1;
}
static int nfs_controls_steering_scene_quad(NfsBackgroundSceneCompositor*s,const NfsBackgroundSceneStage*t,unsigned role,float left,float top,float width,float height,unsigned x,unsigned y,unsigned tw,unsigned th,const int32_t clip[4],unsigned scalar,int blend){
 float l=fmaxf(left,(float)clip[0]),topClip=fmaxf(top,(float)clip[1]),r=fminf(left+width,(float)clip[2]),b=fminf(top+height,(float)clip[3]);GLenum error;GLfloat verts[24],u0,u1,v0,v1;
 if(l>=r||topClip>=b||!scalar)return nfs_controls_steering_scene_current(t);
 u0=(x+(l-left)*tw/width)/s->art[role].width;u1=(x+(r-left)*tw/width)/s->art[role].width;v0=(y+(topClip-top)*th/height)/s->art[role].height;v1=(y+(b-top)*th/height)/s->art[role].height;
 {const GLfloat made[]={l,topClip,u0,v0,r,topClip,u1,v0,l,b,u0,v1,l,b,u0,v1,r,topClip,u1,v0,r,b,u1,v1};memcpy(verts,made,sizeof verts);}
 NFS_CONTROLS_STEERING_GL(glUseProgram(s->program));NFS_CONTROLS_STEERING_GL(glUniformMatrix4fv(s->matrix,1,GL_FALSE,t->sampling.projection));NFS_CONTROLS_STEERING_GL(glUniform1i(s->sampler,0));NFS_CONTROLS_STEERING_GL(glUniform1f(s->opacity,scalar/256.f));NFS_CONTROLS_STEERING_GL(glBindTexture(GL_TEXTURE_2D,s->textures[role]));
 NFS_CONTROLS_STEERING_GL(glEnableVertexAttribArray(0));for(unsigned i=1;i<4;++i){NFS_CONTROLS_STEERING_GL(glDisableVertexAttribArray(i));}NFS_CONTROLS_STEERING_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,verts));
 if(!nfs_controls_steering_scene_blend(t,blend))return 0;NFS_CONTROLS_STEERING_GL(glDrawArrays(GL_TRIANGLES,0,6));NFS_CONTROLS_STEERING_GL(error=glGetError());return error==GL_NO_ERROR;
}
static int nfs_controls_steering_scene_draw(NfsControlsSteeringSceneGpu*s,const NfsBackgroundSceneStage*t,const NfsControlsSteeringSceneLayers*v){
 NfsBackgroundSceneCompositor*b=s->background;NfsLogoVertex logos[18];size_t count=0;float bounds[4];GLenum error;static const int32_t full[4]={0,0,640,480};
 if(!nfs_logo_layers(v->background.logos,3,logos,&count,bounds))return 0;
 for(unsigned i=0;i<8;++i){NFS_CONTROLS_STEERING_GL(glDisable(nfs_frame_caps[i]));}NFS_CONTROLS_STEERING_GL(glColorMask(1,1,1,1));NFS_CONTROLS_STEERING_GL(glActiveTexture(GL_TEXTURE0));NFS_CONTROLS_STEERING_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_CONTROLS_STEERING_GL(glDepthMask(GL_FALSE));
 if(!nfs_controls_steering_scene_quad(b,t,v->background.mapRole,0,0,640,480,0,0,b->art[v->background.mapRole].width,b->art[v->background.mapRole].height,full,256,0)||!nfs_controls_steering_scene_quad(b,t,2,0,0,343,480,0,0,1372,1920,v->background.panelClip,256,1))return 0;
 for(unsigned i=0;i<v->background.wordCount;++i){const NfsBackgroundSceneWordLayer*w=v->background.words+i;const NfsBackgroundSceneWord*r=b->regions+w->sourceRole-1;if(!nfs_controls_steering_scene_quad(b,t,r->textureRole,(float)w->left,(float)w->top,(float)r->logicalWidth,(float)r->logicalHeight,r->x,r->y,r->width,r->height,w->clip,w->scalar256,1))return 0;}
 if(count){NFS_CONTROLS_STEERING_GL(glUseProgram(b->logo.program));NFS_CONTROLS_STEERING_GL(glUniformMatrix4fv(b->logo.matrix,1,GL_FALSE,t->sampling.projection));NFS_CONTROLS_STEERING_GL(glUniform1i(b->logo.sampler,0));NFS_CONTROLS_STEERING_GL(glBindTexture(GL_TEXTURE_2D,b->logo.texture));NFS_CONTROLS_STEERING_GL(glEnableVertexAttribArray(0));NFS_CONTROLS_STEERING_GL(glEnableVertexAttribArray(1));NFS_CONTROLS_STEERING_GL(glDisableVertexAttribArray(2));NFS_CONTROLS_STEERING_GL(glDisableVertexAttribArray(3));NFS_CONTROLS_STEERING_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),logos));NFS_CONTROLS_STEERING_GL(glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),&logos[0].coefficient));if(!nfs_controls_steering_scene_blend(t,1))return 0;NFS_CONTROLS_STEERING_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count));NFS_CONTROLS_STEERING_GL(error=glGetError());if(error!=GL_NO_ERROR)return 0;}
 return nfs_nested_caption_draw(s->sidebarWord,&v->sidebarWord,t)&&nfs_controls_steering_word_draw(s->childWords,&v->childWords,s->text,&v->text,t)&&nfs_controls_steering_bar_gpu_draw_into_staging(s->bar,&v->chart,t);
}
static int nfs_controls_steering_scene_word_ready(NfsControlsSteeringSceneGpu*s,const NfsBackgroundSceneStage*t,const NfsControlsSteeringSceneLayers*v){GLint type=0,name=0;if(nfs_nested_caption_alias(s->sidebarWord,s->background,s->sidebarCaption,s->text,0,0,0,0)||!nfs_nested_caption_ready(s->sidebarWord,&v->sidebarWord,t))return 0;NFS_CONTROLS_STEERING_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CONTROLS_STEERING_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));return type==GL_TEXTURE&&name&&(GLuint)name!=s->sidebarWord->texture;}
static int nfs_controls_steering_scene_child_words_ready(NfsControlsSteeringSceneGpu*s,const NfsBackgroundSceneStage*t,const NfsControlsSteeringSceneLayers*v){
 GLint type=0,name=0;unsigned i;
 if(nfs_controls_steering_word_alias(s->childWords,s->background,s->sidebarCaption,s->text,s->sidebarWord,0,0)||!nfs_controls_steering_word_ready(s->childWords,&v->childWords,&v->text,t))return 0;
 NFS_CONTROLS_STEERING_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CONTROLS_STEERING_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));
 if(type!=GL_TEXTURE||!name)return 0;
 for(i=0;i<NFS_CONTROLS_STEERING_WORD_IMAGES;++i)if((GLuint)name==s->childWords->textures[i])return 0;
 return 1;
}
// All bar resources are preflighted against other owned layers and the current
// unpublished target before any draw. This never prepares or reads image bytes.
static int nfs_controls_steering_scene_bar_ready(NfsControlsSteeringSceneGpu*s,const NfsBackgroundSceneStage*t,const NfsControlsSteeringSceneLayers*v){
 NfsControlsSteeringBarGpu*b=s->bar;NfsBackgroundSceneCompositor*g=s->background;unsigned i,j;GLboolean yes;GLint type=0,name=0;
 if(!b||!nfs_sbar_valid(&v->chart)||b->generation!=t->generation||b->contextIdentity!=t->contextIdentity||!b->program||b->matrix<0||b->sampler<0||!b->textures[v->chart.level]||b->rgba[v->chart.level]!=v->chart.rgba)return 0;
 if(b->program==g->program||b->program==g->copyProgram||b->program==g->logo.program||b->program==s->sidebarCaption->program||b->program==s->text->program||b->program==s->sidebarWord->program||b->program==s->childWords->program)return 0;
 NFS_CONTROLS_STEERING_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_CONTROLS_STEERING_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||!name)return 0;
 for(i=0;i<4;++i)if(b->textures[i]){GLuint a=b->textures[i];if(a==(GLuint)name||a==g->logo.texture||a==g->previewTexture||a==s->sidebarCaption->texture||a==s->text->texture||a==s->sidebarWord->texture)return 0;for(j=0;j<5;++j)if(a==g->textures[j])return 0;for(j=0;j<2;++j)if(a==g->color[j])return 0;for(j=0;j<NFS_CONTROLS_STEERING_WORD_IMAGES;++j)if(a==s->childWords->textures[j])return 0;for(j=0;j<i;++j)if(a==b->textures[j])return 0;}
 NFS_CONTROLS_STEERING_GL(yes=glIsProgram(b->program));if(!yes)return 0;NFS_CONTROLS_STEERING_GL(yes=glIsTexture(b->textures[v->chart.level]));return yes!=0;
}
static int nfs_controls_steering_scene_abandon(NfsControlsSteeringSceneGpu*s){if(s->bar)nfs_controls_steering_bar_gpu_forget(s->bar);if(s->childWords)nfs_controls_steering_word_forget(s->childWords);if(s->sidebarWord)nfs_nested_caption_forget(s->sidebarWord);nfs_bg_scene_context_retired(s->background,s->background->generation,0);nfs_soft_gpu_context_retired(s->sidebarCaption,s->sidebarCaption->generation,0);nfs_soft_gpu_context_retired(s->text,s->text->generation,0);return 0;}
int nfs_controls_steering_scene_draw_layers_into_staging(NfsControlsSteeringSceneGpu*s,const NfsBackgroundSceneStage*t,const NfsControlsSteeringSceneLayers*v){
 NfsBgSaved saved;int ok;
 if(!s||!s->background||!s->sidebarCaption||!s->text||!v||!s->childWords||!s->bar||!nfs_controls_steering_word_valid(&v->childWords,&v->text)||!s->sidebarWord||nfs_nested_caption_empty(&v->sidebarWord)||!nfs_nested_caption_valid(&v->sidebarWord,3)||!nfs_controls_steering_scene_current(t)||!nfs_bg_stage_layers(&v->background)||!nfs_controls_steering_scene_font(s->sidebarCaption,&v->sidebarCaption,343,t->generation)||!nfs_controls_steering_scene_font(s->text,&v->text,640,t->generation))return 0;
 if(!nfs_controls_steering_scene_child_words_ready(s,t,v))return nfs_controls_steering_scene_current(t)?0:nfs_controls_steering_scene_abandon(s);
 if(!nfs_controls_steering_scene_bar_ready(s,t,v))return nfs_controls_steering_scene_current(t)?0:nfs_controls_steering_scene_abandon(s);
 if(!nfs_controls_steering_scene_word_ready(s,t,v))return nfs_controls_steering_scene_current(t)?0:nfs_controls_steering_scene_abandon(s);
 if(!nfs_controls_steering_scene_ready(s,t)||!nfs_controls_steering_scene_target(s,t))return nfs_controls_steering_scene_current(t)?0:nfs_controls_steering_scene_abandon(s);
 if(!nfs_controls_steering_scene_save(&saved,t))return nfs_controls_steering_scene_abandon(s);
 ok=nfs_controls_steering_scene_draw(s,t,v);
 if(!nfs_controls_steering_scene_current(t))return nfs_controls_steering_scene_abandon(s);
 if(!nfs_controls_steering_scene_restore(&saved,t))return nfs_controls_steering_scene_abandon(s);
 return nfs_controls_steering_scene_current(t)?ok:nfs_controls_steering_scene_abandon(s);
}
#undef NFS_CONTROLS_STEERING_GL

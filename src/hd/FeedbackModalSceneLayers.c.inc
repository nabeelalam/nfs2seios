// SPDX-License-Identifier: MIT
#include "FeedbackModalSceneLayers.h"
/* Include after the existing ControlsSceneLayers implementation. It supplies
   neutral guarded stage/state/readiness mechanics, not Controls source authority.
   Do not include that unguarded implementation a second time here. */
#include "FeedbackModalBackdropGpu.c.inc"
#include "FeedbackModalWordGpu.c.inc"
typedef struct NfsFeedbackModalSceneInventory {GLuint programs[128],textures[128];size_t programCount,textureCount;} NfsFeedbackModalSceneInventory;
static int nfs_feedback_modal_scene_add(GLuint*names,size_t*count,GLuint name,int unique){size_t i;if(!name)return 1;for(i=0;i<*count;++i)if(names[i]==name)return !unique;if(*count>=128)return 0;names[(*count)++]=name;return 1;}
static int nfs_feedback_modal_scene_inventory(NfsFeedbackModalSceneGpu*s,const NfsFeedbackModalSceneAliases*a,NfsFeedbackModalSceneInventory*out,NfsFeedbackModalSceneInventory*owned){
 NfsControlsSceneGpu*p;NfsBackgroundSceneCompositor*b;unsigned i;size_t k;
 if(!s||!(p=s->parent)||!(b=p->background)||!p->sidebarCaption||!p->text||!p->sidebarWord||!p->dialogWords||!s->backdrop||!s->words||!a||a->programCount>64||a->textureCount>64||(a->programCount&&!a->programs)||(a->textureCount&&!a->textures))return 0;
 memset(owned,0,sizeof *owned);
#define NFS_FMS_P(name,strict) do{if(!nfs_feedback_modal_scene_add(owned->programs,&owned->programCount,(name),(strict)))return 0;}while(0)
#define NFS_FMS_T(name,strict) do{if(!nfs_feedback_modal_scene_add(owned->textures,&owned->textureCount,(name),(strict)))return 0;}while(0)
 /* Independently owned active parent resources must be distinct. Inactive
    background caches may refer to an already enumerated parent allocation. */
 NFS_FMS_P(b->program,1);NFS_FMS_P(b->copyProgram,1);NFS_FMS_P(b->logo.program,1);NFS_FMS_P(p->sidebarCaption->program,1);NFS_FMS_P(p->text->program,1);NFS_FMS_P(p->sidebarWord->program,1);NFS_FMS_P(p->dialogWords->program,1);
 for(i=0;i<5;++i){NFS_FMS_T(b->textures[i],1);}NFS_FMS_T(b->logo.texture,1);NFS_FMS_T(p->sidebarCaption->texture,1);NFS_FMS_T(p->text->texture,1);NFS_FMS_T(p->sidebarWord->texture,1);for(i=0;i<5;++i){NFS_FMS_T(p->dialogWords->textures[i],1);}
 NFS_FMS_P(b->caption.program,0);NFS_FMS_P(b->locationCaption.program,0);NFS_FMS_T(b->caption.texture,0);NFS_FMS_T(b->locationCaption.texture,0);NFS_FMS_T(b->previewTexture,0);for(i=0;i<2;++i){NFS_FMS_T(b->color[i],0);}
 /* Popup programs/textures must be distinct from the complete parent inventory
    and each other, including inactive parent cache/native resources. */
 NFS_FMS_P(s->backdrop->program,1);NFS_FMS_P(s->words->program,1);for(i=0;i<2;++i){NFS_FMS_T(s->backdrop->textures[i],1);NFS_FMS_T(s->words->textures[i],1);}
 *out=*owned;
 for(k=0;k<a->programCount;++k)if(!nfs_feedback_modal_scene_add(out->programs,&out->programCount,a->programs[k],1))return 0;
 for(k=0;k<a->textureCount;++k)if(!nfs_feedback_modal_scene_add(out->textures,&out->textureCount,a->textures[k],1))return 0;
#undef NFS_FMS_P
#undef NFS_FMS_T
 return 1;
}
static void nfs_feedback_modal_scene_except(const NfsFeedbackModalSceneInventory*all,GLuint program,const GLuint*textures,NfsFeedbackModalSceneInventory*out){size_t i;memset(out,0,sizeof *out);for(i=0;i<all->programCount;++i)if(all->programs[i]!=program)out->programs[out->programCount++]=all->programs[i];for(i=0;i<all->textureCount;++i)if(all->textures[i]!=textures[0]&&all->textures[i]!=textures[1])out->textures[out->textureCount++]=all->textures[i];}
static int nfs_feedback_modal_scene_abandon_popup(NfsFeedbackModalSceneGpu*s){if(s){if(s->backdrop)nfs_feedback_modal_backdrop_gpu_forget(s->backdrop);if(s->words)nfs_feedback_modal_word_forget(s->words);}return 0;}
#define NFS_FMS_GL(call) do{if(!nfs_controls_scene_current(t))goto lost;call;if(!nfs_controls_scene_current(t))goto lost;}while(0)
int nfs_feedback_modal_scene_preflight(NfsFeedbackModalSceneGpu*s,const NfsBackgroundSceneStage*t,const NfsFeedbackModalSceneLayers*v,const NfsFeedbackModalSceneAliases*a){
 NfsFeedbackModalSceneInventory all,owned,backdropNames,wordNames;NfsFeedbackModalBackdropGpuAliases backdropAliases;NfsFeedbackModalWordGpuAliases wordAliases;NfsControlsSceneGpu*p;GLint type=0,name=0;
 if(!v||v->schema!=NFS_FEEDBACK_MODAL_SCENE_LAYERS_SCHEMA||!nfs_feedback_modal_scene_inventory(s,a,&all,&owned)||!nfs_controls_scene_current(t))return 0;p=s->parent;
 /* All metadata and both finite popup domains are checked before parent drawing. */
 if(nfs_feedback_modal_word_empty(&v->words)||!nfs_feedback_modal_word_valid(&v->words,&v->text)||nfs_feedback_modal_backdrop_gpu_empty(&v->backdrop)||!nfs_feedback_modal_backdrop_gpu_valid(&v->backdrop)||nfs_controls_dialog_empty(&v->parent.dialogWords)||!nfs_controls_dialog_valid(&v->parent.dialogWords,&v->parent.text)||nfs_nested_caption_empty(&v->parent.sidebarWord)||!nfs_nested_caption_valid(&v->parent.sidebarWord,3)||!nfs_bg_stage_layers(&v->parent.background)||!nfs_controls_scene_font(p->sidebarCaption,&v->parent.sidebarCaption,343,t->generation)||!nfs_controls_scene_font(p->text,&v->parent.text,640,t->generation)||!nfs_controls_scene_font(p->text,&v->text,640,t->generation))return 0;
 nfs_feedback_modal_scene_except(&all,s->backdrop->program,s->backdrop->textures,&backdropNames);backdropAliases.programs=backdropNames.programs;backdropAliases.programCount=backdropNames.programCount;backdropAliases.textures=backdropNames.textures;backdropAliases.textureCount=backdropNames.textureCount;
 nfs_feedback_modal_scene_except(&all,s->words->program,s->words->textures,&wordNames);wordAliases.programs=wordNames.programs;wordAliases.programCount=wordNames.programCount;wordAliases.textures=wordNames.textures;wordAliases.textureCount=wordNames.textureCount;
 if(!nfs_feedback_modal_backdrop_gpu_ready(s->backdrop,&v->backdrop,t,&backdropAliases)||!nfs_feedback_modal_word_ready(s->words,&v->words,&v->text,t,&wordAliases)||!nfs_controls_scene_dialog_ready(p,t,&v->parent)||!nfs_controls_scene_word_ready(p,t,&v->parent)||!nfs_controls_scene_ready(p,t)||!nfs_controls_scene_target(p,t))return nfs_controls_scene_current(t)?0:nfs_feedback_modal_scene_abandon_popup(s);
 NFS_FMS_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_FMS_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));
 if(type!=GL_TEXTURE||!name||nfs_feedback_modal_word_name((GLuint)name,owned.textures,owned.textureCount))return 0;
 return 1;
 lost:return nfs_feedback_modal_scene_abandon_popup(s);
}
int nfs_feedback_modal_scene_draw_layers_into_staging(NfsFeedbackModalSceneGpu*s,const NfsBackgroundSceneStage*t,const NfsFeedbackModalSceneLayers*v,const NfsFeedbackModalSceneAliases*a){
 NfsFeedbackModalSceneInventory all,owned,backdropNames,wordNames;NfsFeedbackModalBackdropGpuAliases backdropAliases;NfsFeedbackModalWordGpuAliases wordAliases;int ok;
 if(!nfs_feedback_modal_scene_preflight(s,t,v,a))return 0;
 /* No untrusted caller code runs between preflight and use. Recreate bounded
    local inventories so all borrowed alias pointers remain synchronous. */
 if(!nfs_feedback_modal_scene_inventory(s,a,&all,&owned))return 0;
 nfs_feedback_modal_scene_except(&all,s->backdrop->program,s->backdrop->textures,&backdropNames);backdropAliases.programs=backdropNames.programs;backdropAliases.programCount=backdropNames.programCount;backdropAliases.textures=backdropNames.textures;backdropAliases.textureCount=backdropNames.textureCount;
 nfs_feedback_modal_scene_except(&all,s->words->program,s->words->textures,&wordNames);wordAliases.programs=wordNames.programs;wordAliases.programCount=wordNames.programCount;wordAliases.textures=wordNames.textures;wordAliases.textureCount=wordNames.textureCount;
 ok=nfs_controls_scene_draw_layers_into_staging(s->parent,t,&v->parent);
 if(!nfs_controls_scene_current(t))return nfs_feedback_modal_scene_abandon_popup(s);if(!ok)return 0;
 ok=nfs_feedback_modal_backdrop_gpu_draw(s->backdrop,&v->backdrop,t,&backdropAliases);
 if(!nfs_controls_scene_current(t))return nfs_feedback_modal_scene_abandon_popup(s);if(!ok)return 0;
 ok=nfs_feedback_modal_word_draw(s->words,&v->words,&v->text,t,&wordAliases);
 return nfs_controls_scene_current(t)?ok:nfs_feedback_modal_scene_abandon_popup(s);
}
#undef NFS_FMS_GL

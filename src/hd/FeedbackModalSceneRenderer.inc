// SPDX-License-Identifier: MIT
/* Actual modal-owned staging and color-only publication. Included after the
   independent observer and existing prepared Controls rendering mechanics.
   Native receipt consumption, preparation, staging and commit remain in one
   renderer callback before the original cursor and swap. No gamma pass. */
#include "FeedbackModalSceneRuntimeAbi.h"
#include "FeedbackModalSceneLayers.c.inc"
static struct {
 uint64_t generation,serial;uintptr_t context,thread;
 GLuint color,fbo;unsigned width,height;
 NfsNativeLogoTargetProof target;
 NfsFeedbackModalWordGpu words;NfsFeedbackModalBackdropGpu backdrop;
} nfs_feedback_scene_renderer;
static void nfs_feedback_scene_forget(void){nfs_feedback_modal_word_forget(&nfs_feedback_scene_renderer.words);nfs_feedback_modal_backdrop_gpu_forget(&nfs_feedback_scene_renderer.backdrop);memset(&nfs_feedback_scene_renderer,0,sizeof nfs_feedback_scene_renderer);}
static int nfs_feedback_scene_context(uint64_t generation,uintptr_t context){return generation&&nfs_feedback_scene_renderer.generation==generation&&nfs_feedback_scene_renderer.context==context&&nfs_feedback_scene_renderer.thread==NFS_FEEDBACK_OBSERVER_THREAD_TOKEN()&&nfs_feedback_observer_current()&&nfs_feedback_observer.generation==generation&&nfs_feedback_observer.context==context;}
static int nfs_feedback_scene_current(void){return nfs_feedback_scene_context(nfs_feedback_scene_renderer.generation,nfs_feedback_scene_renderer.context);}
static int nfs_feedback_scene_context_owner(void*user,const NfsBackgroundSceneStage*t){(void)user;return t&&nfs_feedback_scene_context(t->generation,t->contextIdentity);}
static NfsBackgroundSceneStage nfs_feedback_scene_context_stage(void){NfsBackgroundSceneStage t;memset(&t,0,sizeof t);t.generation=nfs_feedback_scene_renderer.generation;t.contextIdentity=nfs_feedback_scene_renderer.context;t.framebuffer=g_framebuffer;t.ownerCurrent=nfs_feedback_scene_context_owner;return t;}
#define NFS_FEEDBACK_SCENE_GL(call) do{if(!nfs_feedback_scene_current())goto lost;call;if(!nfs_feedback_scene_current())goto lost;}while(0)
static int nfs_feedback_scene_color_target(unsigned w,unsigned h){
 GLuint texture=0,fbo=0;GLint maxSize=0,maxViewport[2]={0,0},type=0,name=0;GLenum status=0,error;GLboolean exists=0;NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_feedback_scene_context_stage();int ok=0;
 if(nfs_feedback_scene_renderer.color==g_framebufferTexture||nfs_feedback_scene_renderer.fbo==g_framebuffer)return 0;
 if(!nfs_feedback_scene_current()||!w||!h||w>NFS_FRAME_MAX_DIMENSION||h>NFS_FRAME_MAX_DIMENSION||(uint64_t)w*h>NFS_FRAME_MAX_PIXELS)return 0;
 NFS_FEEDBACK_SCENE_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxSize));NFS_FEEDBACK_SCENE_GL(glGetIntegerv(GL_MAX_VIEWPORT_DIMS,maxViewport));if(w>(unsigned)maxSize||h>(unsigned)maxSize||w>(unsigned)maxViewport[0]||h>(unsigned)maxViewport[1])return 0;
 if(!nfs_controls_scene_save(&saved,&guard))goto lost;
 if(nfs_feedback_scene_renderer.color&&nfs_feedback_scene_renderer.fbo&&nfs_feedback_scene_renderer.width==w&&nfs_feedback_scene_renderer.height==h){
  NFS_FEEDBACK_SCENE_GL(exists=glIsTexture(nfs_feedback_scene_renderer.color));if(!exists)goto done;NFS_FEEDBACK_SCENE_GL(exists=glIsFramebuffer(nfs_feedback_scene_renderer.fbo));if(!exists)goto done;
  NFS_FEEDBACK_SCENE_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_feedback_scene_renderer.fbo));NFS_FEEDBACK_SCENE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_FEEDBACK_SCENE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));NFS_FEEDBACK_SCENE_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_FEEDBACK_SCENE_GL(error=glGetError());ok=type==GL_TEXTURE&&(GLuint)name==nfs_feedback_scene_renderer.color&&status==GL_FRAMEBUFFER_COMPLETE&&error==GL_NO_ERROR;
  if(!nfs_controls_scene_restore(&saved,&guard))goto lost;return ok;
 }
 NFS_FEEDBACK_SCENE_GL(glActiveTexture(GL_TEXTURE0));NFS_FEEDBACK_SCENE_GL(glGenTextures(1,&texture));if(!texture)goto done;NFS_FEEDBACK_SCENE_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_FEEDBACK_SCENE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_FEEDBACK_SCENE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_FEEDBACK_SCENE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_FEEDBACK_SCENE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_FEEDBACK_SCENE_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL));NFS_FEEDBACK_SCENE_GL(error=glGetError());if(error!=GL_NO_ERROR)goto done;
 NFS_FEEDBACK_SCENE_GL(glGenFramebuffers(1,&fbo));if(!fbo)goto done;NFS_FEEDBACK_SCENE_GL(glBindFramebuffer(GL_FRAMEBUFFER,fbo));NFS_FEEDBACK_SCENE_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture,0));NFS_FEEDBACK_SCENE_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));NFS_FEEDBACK_SCENE_GL(error=glGetError());ok=status==GL_FRAMEBUFFER_COMPLETE&&error==GL_NO_ERROR;
 done:if(!ok){if(fbo){NFS_FEEDBACK_SCENE_GL(glDeleteFramebuffers(1,&fbo));}if(texture){NFS_FEEDBACK_SCENE_GL(glDeleteTextures(1,&texture));}}
 if(!nfs_controls_scene_restore(&saved,&guard))goto lost;if(!ok)return 0;
 if(nfs_feedback_scene_renderer.fbo){NFS_FEEDBACK_SCENE_GL(glDeleteFramebuffers(1,&nfs_feedback_scene_renderer.fbo));}if(nfs_feedback_scene_renderer.color){NFS_FEEDBACK_SCENE_GL(glDeleteTextures(1,&nfs_feedback_scene_renderer.color));}
 nfs_feedback_scene_renderer.fbo=fbo;nfs_feedback_scene_renderer.color=texture;nfs_feedback_scene_renderer.width=w;nfs_feedback_scene_renderer.height=h;return 1;
 lost:nfs_feedback_scene_forget();return 0;
}

static int nfs_feedback_scene_stage_owner(void*user,const NfsBackgroundSceneStage*t){
 GLint fbo=0,type=0,name=0;(void)user;
 if(!t||nfs_feedback_scene_renderer.color==g_framebufferTexture||nfs_feedback_scene_renderer.fbo==g_framebuffer||!nfs_feedback_scene_context(t->generation,t->contextIdentity)||t->framebuffer!=nfs_feedback_scene_renderer.fbo||t->width!=nfs_feedback_scene_renderer.width||t->height!=nfs_feedback_scene_renderer.height)return 0;
 NFS_FEEDBACK_SCENE_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=t->framebuffer)return 0;NFS_FEEDBACK_SCENE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_FEEDBACK_SCENE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));return type==GL_TEXTURE&&(GLuint)name==nfs_feedback_scene_renderer.color;
 lost:return 0;
}

static int nfs_feedback_scene_resource_owner(void*user,const NfsBackgroundResourceOwner*o){
 GLint fbo=0,type=0,name=0;(void)user;if(!o||!nfs_feedback_scene_context(o->generation,o->contextIdentity)||!nfs_feedback_scene_renderer.serial||nfs_feedback_scene_renderer.target.framebuffer!=g_framebuffer)return 0;
 NFS_FEEDBACK_SCENE_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=g_framebuffer)return 0;NFS_FEEDBACK_SCENE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_FEEDBACK_SCENE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));return type==GL_TEXTURE&&(GLuint)name==g_framebufferTexture;
 lost:return 0;
}

static int nfs_feedback_scene_commit_current(void){NfsBackgroundResourceOwner owner={nfs_feedback_scene_renderer.generation,nfs_feedback_scene_renderer.context,nfs_feedback_scene_resource_owner,NULL};return nfs_feedback_scene_renderer.color&&nfs_feedback_scene_renderer.color!=g_framebufferTexture&&nfs_feedback_scene_renderer.fbo!=g_framebuffer&&nfs_feedback_scene_resource_owner(NULL,&owner);}

static int nfs_feedback_scene_copy_color(const NfsNativeLogoTargetProof*target){
 GLenum error;static const GLfloat copy[]={-1,1,0,1,-1,-1,0,0,1,1,1,1,1,1,1,1,-1,-1,0,0,1,-1,1,0};
#define NFS_FEEDBACK_SCENE_COMMIT_GL(call) do{if(!nfs_feedback_scene_commit_current())return 0;call;if(!nfs_feedback_scene_commit_current())return 0;}while(0)
 for(unsigned i=0;i<8;++i){NFS_FEEDBACK_SCENE_COMMIT_GL(glDisable(nfs_frame_caps[i]));}NFS_FEEDBACK_SCENE_COMMIT_GL(glColorMask(1,1,1,1));NFS_FEEDBACK_SCENE_COMMIT_GL(glActiveTexture(GL_TEXTURE0));NFS_FEEDBACK_SCENE_COMMIT_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_FEEDBACK_SCENE_COMMIT_GL(glDepthMask(GL_FALSE));NFS_FEEDBACK_SCENE_COMMIT_GL(glViewport(0,0,target->width,target->height));NFS_FEEDBACK_SCENE_COMMIT_GL(glUseProgram(nfs_bg_compositor.copyProgram));NFS_FEEDBACK_SCENE_COMMIT_GL(glUniform1i(nfs_bg_compositor.copySampler,0));NFS_FEEDBACK_SCENE_COMMIT_GL(glBindTexture(GL_TEXTURE_2D,nfs_feedback_scene_renderer.color));NFS_FEEDBACK_SCENE_COMMIT_GL(glEnableVertexAttribArray(0));for(unsigned i=1;i<4;++i){NFS_FEEDBACK_SCENE_COMMIT_GL(glDisableVertexAttribArray(i));}NFS_FEEDBACK_SCENE_COMMIT_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,copy));NFS_FEEDBACK_SCENE_COMMIT_GL(glDrawArrays(GL_TRIANGLES,0,6));NFS_FEEDBACK_SCENE_COMMIT_GL(error=glGetError());return error==GL_NO_ERROR;
#undef NFS_FEEDBACK_SCENE_COMMIT_GL
}

/* Caller resource names supplement the internally enumerated complete parent
   and popup bundle. Actual original-upload textures are protected too. */
static int nfs_feedback_scene_add(GLuint*names,size_t*count,GLuint name){if(!name)return 1;for(size_t i=0;i<*count;++i)if(names[i]==name)return 1;if(*count>=64)return 0;names[(*count)++]=name;return 1;}
static int nfs_feedback_scene_external(GLuint programs[64],size_t*np,GLuint textures[64],size_t*nt){
 *np=*nt=0;
 if(!nfs_feedback_scene_add(programs,np,g_shaderProgram)||!nfs_feedback_scene_add(programs,np,g_shaderProgramDisp)||!nfs_feedback_scene_add(textures,nt,g_framebufferTexture)||!nfs_feedback_scene_add(textures,nt,g_frontTexture)||!nfs_feedback_scene_add(textures,nt,nfs_feedback_scene_renderer.color)||!nfs_feedback_scene_add(textures,nt,nfs_controls_renderer.color))return 0;
 for(unsigned i=0;i<300;++i)if(!nfs_feedback_scene_add(textures,nt,nfs_feedback_observer.pass.textureIds[i]))return 0;
 return 1;
}
static int nfs_feedback_scene_prepare_aliases(GLuint programs[64],size_t*np,GLuint textures[64],size_t*nt,int words){
 NfsControlsSceneGpu p={&nfs_bg_compositor,&nfs_controls_renderer.caption,&nfs_controls_renderer.text,&nfs_controls_renderer.sidebarWordGpu,&nfs_controls_renderer.dialogWordsGpu};
 if(!nfs_feedback_scene_external(programs,np,textures,nt))return 0;
 GLuint ps[]={p.background->program,p.background->copyProgram,p.background->logo.program,p.sidebarCaption->program,p.text->program,p.sidebarWord->program,p.dialogWords->program,words?nfs_feedback_scene_renderer.backdrop.program:nfs_feedback_scene_renderer.words.program};
 for(unsigned i=0;i<sizeof ps/sizeof ps[0];++i)if(!nfs_feedback_scene_add(programs,np,ps[i]))return 0;
 GLuint ts[]={p.background->logo.texture,p.background->previewTexture,p.sidebarCaption->texture,p.text->texture,p.sidebarWord->texture,words?nfs_feedback_scene_renderer.backdrop.textures[0]:nfs_feedback_scene_renderer.words.textures[0],words?nfs_feedback_scene_renderer.backdrop.textures[1]:nfs_feedback_scene_renderer.words.textures[1]};
 for(unsigned i=0;i<sizeof ts/sizeof ts[0];++i)if(!nfs_feedback_scene_add(textures,nt,ts[i]))return 0;
 for(unsigned i=0;i<5;++i)if(!nfs_feedback_scene_add(textures,nt,p.background->textures[i])||!nfs_feedback_scene_add(textures,nt,p.dialogWords->textures[i]))return 0;
 for(unsigned i=0;i<2;++i)if(!nfs_feedback_scene_add(textures,nt,p.background->color[i]))return 0;
 return 1;
}
/* Explicit retirement happens while the registered old owner is still current.
   A lost owner only forgets metadata; its context owns the abandoned GL names. */
static void nfs_feedback_scene_retired(void){
 GLuint ps[64],ts[64];size_t np=0,nt=0;NfsBackgroundSceneStage guard=nfs_feedback_scene_context_stage();
 if(!nfs_feedback_scene_current()){nfs_feedback_scene_forget();return;}
 if(nfs_feedback_scene_prepare_aliases(ps,&np,ts,&nt,1)){NfsFeedbackModalWordGpuAliases a={ps,np,ts,nt};(void)nfs_feedback_modal_word_retire(&nfs_feedback_scene_renderer.words,&guard,1,&a);}
 if(!nfs_feedback_scene_current()){nfs_feedback_scene_forget();return;}
 if(nfs_feedback_scene_prepare_aliases(ps,&np,ts,&nt,0)){NfsFeedbackModalBackdropGpuAliases a={ps,np,ts,nt};(void)nfs_feedback_modal_backdrop_gpu_retire(&nfs_feedback_scene_renderer.backdrop,&guard,1,&a);}
 if(!nfs_feedback_scene_current()){nfs_feedback_scene_forget();return;}
 if(nfs_feedback_scene_renderer.fbo){NFS_FEEDBACK_SCENE_GL(glDeleteFramebuffers(1,&nfs_feedback_scene_renderer.fbo));}
 if(nfs_feedback_scene_renderer.color){NFS_FEEDBACK_SCENE_GL(glDeleteTextures(1,&nfs_feedback_scene_renderer.color));}
 lost:nfs_feedback_scene_forget();
}
int nfs_private_feedback_modal_scene_compose(const NfsFeedbackModalSceneView*v,NfsFeedbackModalNativeReceipt*out){
 NfsFeedbackModalNativeReceipt receipt;NfsNativeLogoTargetProof target;NfsBackgroundSceneStage guard,stage;NfsControlsSceneGpu parent;NfsFeedbackModalSceneGpu gpu;NfsBgSaved saved;
 GLuint ps[64],ts[64];size_t np=0,nt=0;int ok=0;const char*reason="view";
 if(out)memset(out,0,sizeof *out);
 if(!v||!out||v->schema!=NFS_FEEDBACK_MODAL_SCENE_RUNTIME_SCHEMA||v->width!=640||v->height!=480||!v->serial||!v->modalEpoch||v->layers.schema!=NFS_FEEDBACK_MODAL_SCENE_LAYERS_SCHEMA)return 0;
 if(!nfs_private_feedback_modal_observer_take_receipt(v->serial,v->modalEpoch,&receipt)||!receipt.admitted||receipt.batches!=300||!nfs_feedback_observer_current()||receipt.contextGeneration!=nfs_feedback_observer.generation)return 0;
 *out=receipt;
 if(nfs_feedback_scene_renderer.generation&&nfs_feedback_scene_renderer.generation!=receipt.contextGeneration)nfs_feedback_scene_forget();
 nfs_feedback_scene_renderer.generation=receipt.contextGeneration;nfs_feedback_scene_renderer.context=nfs_feedback_observer.context;nfs_feedback_scene_renderer.thread=nfs_feedback_observer.thread;nfs_feedback_scene_renderer.serial=v->serial;
 reason="target";if(!nfs_feedback_observer_read_target(&target,v->serial)||memcmp(&target,&nfs_feedback_observer.target,sizeof target))goto done;nfs_feedback_scene_renderer.target=target;
 reason="parent-registration";if(!nfs_controls_current()||nfs_controls_renderer.generation!=receipt.contextGeneration||nfs_controls_renderer.context!=nfs_feedback_scene_renderer.context)goto done;
 reason="stage-allocation";if(!nfs_feedback_scene_color_target(target.width,target.height))goto done;
 guard=nfs_feedback_scene_context_stage();
 reason="word-prepare";if(!nfs_feedback_scene_prepare_aliases(ps,&np,ts,&nt,1))goto done;
 {NfsFeedbackModalWordGpuAliases a={ps,np,ts,nt};if(!nfs_feedback_modal_word_prepare(&nfs_feedback_scene_renderer.words,&v->layers.words,&v->layers.text,&guard,&a))goto done;}
 reason="backdrop-prepare";if(!nfs_feedback_scene_prepare_aliases(ps,&np,ts,&nt,0))goto done;
 {NfsFeedbackModalBackdropGpuAliases a={ps,np,ts,nt};if(!nfs_feedback_modal_backdrop_gpu_prepare(&nfs_feedback_scene_renderer.backdrop,&v->layers.backdrop,&guard,&a))goto done;}
 reason="prepare-target";if(!nfs_feedback_observer_read_target(&target,v->serial)||memcmp(&target,&nfs_feedback_scene_renderer.target,sizeof target))goto done;
 reason="save-stage";if(!nfs_controls_scene_save(&saved,&guard))goto lost;
 NFS_FEEDBACK_SCENE_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_feedback_scene_renderer.fbo));NFS_FEEDBACK_SCENE_GL(glViewport(target.sampling.viewport[0],target.sampling.viewport[1],target.sampling.viewport[2],target.sampling.viewport[3]));
 memset(&stage,0,sizeof stage);stage.generation=nfs_feedback_scene_renderer.generation;stage.contextIdentity=nfs_feedback_scene_renderer.context;stage.framebuffer=nfs_feedback_scene_renderer.fbo;stage.width=target.width;stage.height=target.height;stage.sampling=target.sampling;stage.ownerCurrent=nfs_feedback_scene_stage_owner;
 parent.background=&nfs_bg_compositor;parent.sidebarCaption=&nfs_controls_renderer.caption;parent.text=&nfs_controls_renderer.text;parent.sidebarWord=&nfs_controls_renderer.sidebarWordGpu;parent.dialogWords=&nfs_controls_renderer.dialogWordsGpu;gpu.parent=&parent;gpu.backdrop=&nfs_feedback_scene_renderer.backdrop;gpu.words=&nfs_feedback_scene_renderer.words;
 reason="external-aliases";if(!nfs_feedback_scene_external(ps,&np,ts,&nt)){if(!nfs_controls_scene_restore(&saved,&guard))goto lost;goto done;}
 {NfsFeedbackModalSceneAliases a={ps,np,ts,nt};ok=nfs_feedback_modal_scene_draw_layers_into_staging(&gpu,&stage,&v->layers,&a);}
 reason="ordered-whole-stage";if(!nfs_feedback_scene_current())goto lost;if(!nfs_controls_scene_restore(&saved,&guard))goto lost;if(!ok)goto done;ok=0;
 reason="precommit-target";if(!nfs_feedback_observer_read_target(&target,v->serial)||memcmp(&target,&nfs_feedback_scene_renderer.target,sizeof target))goto done;
 reason="save-commit";if(!nfs_controls_scene_save(&saved,&guard))goto lost;
 ok=nfs_feedback_scene_copy_color(&target);if(!nfs_feedback_scene_current())goto lost;if(!nfs_controls_scene_restore(&saved,&guard))goto lost;reason=ok?"color-only-commit-before-cursor":"copy-error";
 done:if(nfs_logo_trace_enabled())fprintf(stderr,"FEEDBACK MODAL SCENE %s reason=%s serial=%llu epoch=%llu size=%ux%u\n",ok?"published":"refused",reason,(unsigned long long)v->serial,(unsigned long long)v->modalEpoch,nfs_feedback_scene_renderer.width,nfs_feedback_scene_renderer.height);nfs_feedback_scene_renderer.serial=0;return ok;
 lost:nfs_feedback_scene_forget();return 0;
}
#undef NFS_FEEDBACK_SCENE_GL

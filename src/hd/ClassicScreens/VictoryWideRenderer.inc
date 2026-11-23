// SPDX-License-Identifier: MIT
// Independent full-window image, sealed only after a complete original pass.
static struct {
 NfsVictoryLiveView source;NfsVictoryArtView art;
 uintptr_t context;uint64_t generation;GLuint fbo,color,photo,nativeFbo,nativeColor,nativeDepth;
 unsigned width,height,ready,selected;
} nfs_victory_wide;
static int nfs_victory_wide_current(void){return nfs_victory_context(nfs_victory_wide.generation,nfs_victory_wide.context)&&nfs_victory_wide.source.request&&nfs_victory_wide.art.generation==nfs_victory_art_generation();}
static int nfs_victory_wide_guard(void*u,const NfsBackgroundSceneStage*t){return u==&nfs_victory_wide&&t&&t->generation==nfs_victory_wide.generation&&t->contextIdentity==nfs_victory_wide.context&&nfs_victory_wide_current();}
static NfsBackgroundSceneStage nfs_victory_wide_stage(void){NfsBackgroundSceneStage s={0};s.generation=nfs_victory_wide.generation;s.contextIdentity=nfs_victory_wide.context;s.framebuffer=nfs_victory_wide.fbo?nfs_victory_wide.fbo:nfs_victory_wide.nativeFbo;s.width=nfs_victory_wide.width;s.height=nfs_victory_wide.height;s.ownerCurrent=nfs_victory_wide_guard;s.ownerUser=&nfs_victory_wide;return s;}
static void nfs_victory_wide_revoke(void){nfs_victory_logo_revoke();NfsVictoryLiveView source=nfs_victory_wide.source;NfsVictoryArtView art=nfs_victory_wide.art;memset(&nfs_victory_wide.source,0,sizeof source);memset(&nfs_victory_wide.art,0,sizeof art);nfs_victory_wide.ready=nfs_victory_wide.selected=0;if(source.owner&&source.release)source.release(source.owner);if(art.owner&&art.release)art.release(art.owner);}
static void nfs_victory_wide_retire(int owner){if(owner&&nfs_victory_context(nfs_victory_wide.generation,nfs_victory_wide.context)){if(nfs_victory_wide.fbo)glDeleteFramebuffers(1,&nfs_victory_wide.fbo);if(nfs_victory_wide.color)glDeleteTextures(1,&nfs_victory_wide.color);if(nfs_victory_wide.photo)glDeleteTextures(1,&nfs_victory_wide.photo);}nfs_victory_wide_revoke();memset(&nfs_victory_wide,0,sizeof nfs_victory_wide);}
#define NFS_VW_GL(call) do{if(!nfs_victory_wide_current())goto lost;call;if(!nfs_victory_wide_current())goto lost;}while(0)
static int nfs_victory_wide_attachment(GLuint framebuffer,GLuint color,GLuint depth){
 GLint prior=0,type=0,name=0;int ok=0;
 NFS_VW_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prior));NFS_VW_GL(glBindFramebuffer(GL_FRAMEBUFFER,framebuffer));
 NFS_VW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_VW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));ok=type==GL_TEXTURE&&(GLuint)name==color;
 if(depth){NFS_VW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_VW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));ok=ok&&type==GL_RENDERBUFFER&&(GLuint)name==depth;}
 NFS_VW_GL(glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)prior));return ok;
 lost:return 0;
}
static int nfs_victory_wide_text_current(void*u,const NfsSoftwareTextStage*t){return u==&nfs_victory_wide&&t&&t->framebuffer==nfs_victory_wide.fbo&&t->width==nfs_victory_wide.width&&t->height==nfs_victory_wide.height&&nfs_victory_wide_current()&&nfs_victory_wide_attachment(t->framebuffer,nfs_victory_wide.color,0);}
static int nfs_victory_wide_prepare(void){
 NfsVictoryArtView art={0};NfsVictoryCanvasLayout layout;NfsFrameLegacySampling overlay;NfsSoftwareTextStage textStage;NfsBgSaved saved;NfsBackgroundSceneStage guard;GLint type=0,color=0,depth=0,maximum=0;int w=0,h=0,savedActive=0,ok=0;GLfloat quad[]={-1,1,0,0,-1,-1,0,1,1,1,1,0,1,1,1,0,-1,-1,0,1,1,-1,1,1};
 const NfsVictoryLiveView*v=&nfs_victory_renderer.view;
 if(!v->request||!v->artwork.present||!nfs_victory_words_owner_current()||!SDL_GetWindowSizeInPixels(sdlWin,&w,&h)||w!=winWidth||h!=winHeight||w<=0||h<=0||w>NFS_FRAME_MAX_DIMENSION||h>NFS_FRAME_MAX_DIMENSION||(uint64_t)(unsigned)w*(unsigned)h>NFS_FRAME_MAX_PIXELS)return 0;
 {int viewport[4]={0,0,w,h};if(!nfs_victory_canvas_layout(viewport,&layout)||layout.aspect!=1)return 0;}
 if(!nfs_victory_art_acquire(1,v->artwork.image,v->artwork.sourceSha256,&art))return 0;
 if(art.schema!=1||art.aspect!=1||art.image!=v->artwork.image||!art.owner||!art.release||!art.pixels||art.bytes!=(size_t)art.width*art.height*4||memcmp(art.sourceSha256,v->artwork.sourceSha256,32)){if(art.owner&&art.release)art.release(art.owner);return 0;}
 {unsigned char sha[32];if(!nfs_texture_sha256(art.pixels,art.bytes,sha)||memcmp(sha,art.rgbaSha256,32)){art.release(art.owner);return 0;}}
 {int viewport[4]={0,0,w,h};if(!nfs_victory_canvas_layout_image(viewport,art.width,art.height,&layout)){art.release(art.owner);return 0;}}
 nfs_victory_wide_retire(1);v->retain(v->owner);nfs_victory_wide.source=*v;nfs_victory_wide.art=art;nfs_victory_wide.context=nfs_victory_renderer.context;nfs_victory_wide.generation=nfs_victory_renderer.generation;nfs_victory_wide.width=w;nfs_victory_wide.height=h;nfs_victory_wide.nativeFbo=nfs_victory_renderer.target.framebuffer;
 guard=nfs_victory_wide_stage();if(!nfs_graphics_scene_save(&saved,&guard))goto lost;savedActive=1;
 {GLint actual=0;NFS_VW_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&actual));if((GLuint)actual!=nfs_victory_wide.nativeFbo)goto done;}
 NFS_VW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_VW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&color));if(type!=GL_TEXTURE||!color)goto done;
 NFS_VW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_VW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&depth));if(type!=GL_RENDERBUFFER||!depth)goto done;nfs_victory_wide.nativeColor=color;nfs_victory_wide.nativeDepth=depth;
 NFS_VW_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maximum));if(w>maximum||h>maximum||art.width>(unsigned)maximum||art.height>(unsigned)maximum)goto done;
 NFS_VW_GL(glActiveTexture(GL_TEXTURE0));NFS_VW_GL(glGenTextures(1,&nfs_victory_wide.color));NFS_VW_GL(glBindTexture(GL_TEXTURE_2D,nfs_victory_wide.color));NFS_VW_GL(nfs_frame_texture_params());NFS_VW_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL));
 NFS_VW_GL(glGenFramebuffers(1,&nfs_victory_wide.fbo));NFS_VW_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_victory_wide.fbo));NFS_VW_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,nfs_victory_wide.color,0));{GLenum status=0;NFS_VW_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));if(status!=GL_FRAMEBUFFER_COMPLETE)goto done;}
 NFS_VW_GL(glGenTextures(1,&nfs_victory_wide.photo));NFS_VW_GL(glBindTexture(GL_TEXTURE_2D,nfs_victory_wide.photo));NFS_VW_GL(nfs_frame_texture_params());NFS_VW_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));NFS_VW_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,art.width,art.height,0,GL_RGBA,GL_UNSIGNED_BYTE,art.pixels));
 if(!nfs_victory_wide.fbo||!nfs_victory_wide.color||!nfs_victory_wide.photo||nfs_victory_wide.fbo==nfs_victory_wide.nativeFbo||nfs_victory_wide.color==nfs_victory_wide.nativeColor||nfs_victory_wide.photo==nfs_victory_wide.nativeColor||nfs_victory_wide.photo==nfs_victory_wide.color||nfs_victory_wide.color==nfs_victory_renderer.color||nfs_victory_wide.photo==nfs_victory_renderer.clean)goto done;
 for(unsigned k=0;k<6;++k){quad[4*k+2]=layout.leftU+quad[4*k+2]*(layout.rightU-layout.leftU);quad[4*k+3]=layout.topV+quad[4*k+3]*(layout.bottomV-layout.topV);}
 NFS_VW_GL(nfs_frame_draw_state());NFS_VW_GL(glViewport(0,0,w,h));NFS_VW_GL(glUseProgram(nfs_victory_renderer.copyProgram));NFS_VW_GL(glUniform1i(nfs_victory_renderer.copySampler,0));NFS_VW_GL(glUniform4f(nfs_victory_renderer.copyFog,0,0,0,0));NFS_VW_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_VW_GL(glEnableVertexAttribArray(0));for(unsigned k=1;k<4;++k){NFS_VW_GL(glDisableVertexAttribArray(k));}NFS_VW_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,quad));NFS_VW_GL(glDrawArrays(GL_TRIANGLES,0,6));
 overlay=nfs_victory_renderer.target.sampling;memcpy(overlay.viewport,layout.textViewport,sizeof layout.textViewport);
 if(!nfs_victory_words_draw(&nfs_victory_renderer.words,v->words,v->wordCount,&overlay))goto done;
 memset(&textStage,0,sizeof textStage);textStage.framebuffer=nfs_victory_wide.fbo;textStage.width=w;textStage.height=h;textStage.sampling=overlay;textStage.logicalWidth=640;textStage.logicalHeight=480;
 {const float logical=480.f*w/h;memset(&textStage.sampling,0,sizeof textStage.sampling);textStage.sampling.magFilter=overlay.magFilter;textStage.sampling.viewport[2]=w;textStage.sampling.viewport[3]=h;textStage.sampling.projection[0]=2.f/logical;textStage.sampling.projection[5]=-2.f/480;textStage.sampling.projection[10]=-2;textStage.sampling.projection[12]=-640.f/logical;textStage.sampling.projection[13]=textStage.sampling.projection[14]=textStage.sampling.projection[15]=1;}
 NFS_VW_GL(glViewport(0,0,w,h));
 if(!nfs_soft_gpu_draw_into_owned_responsive_staging(&nfs_victory_renderer.text,&textStage,v->text.vertices,v->text.vertexCount,&nfs_victory_wide,nfs_victory_wide_text_current))goto done;
 if(!nfs_victory_wide_attachment(nfs_victory_wide.fbo,nfs_victory_wide.color,0))goto done;
 NFS_VW_GL(glViewport(0,0,w,h));if(!nfs_victory_stage_fog(quad))goto done;
 {GLenum error=0;NFS_VW_GL(error=glGetError());ok=error==GL_NO_ERROR&&nfs_victory_wide_attachment(nfs_victory_wide.fbo,nfs_victory_wide.color,0);}
 done:if(savedActive&&!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok||!nfs_victory_wide_attachment(nfs_victory_wide.nativeFbo,nfs_victory_wide.nativeColor,nfs_victory_wide.nativeDepth)){nfs_victory_wide_retire(1);return 0;}nfs_victory_wide.ready=1;if(nfs_logo_trace_enabled())fprintf(stderr,"VICTORY WIDE prepared request=%llu output=%ux%u image=%u native300600=1\n",(unsigned long long)v->request,nfs_victory_wide.width,nfs_victory_wide.height,art.image);return 1;
 lost:nfs_victory_wide_retire(0);return 0;
}
static GLuint nfs_victory_wide_present(GLuint front,GLuint color,int w,int h){if(nfs_victory_logo_pending()){nfs_victory_wide_revoke();return 0;}nfs_victory_wide.selected=0;if(!nfs_victory_wide.ready)return 0;if(!nfs_victory_wide_current()||w!=(int)nfs_victory_wide.width||h!=(int)nfs_victory_wide.height||front!=nfs_victory_wide.nativeFbo||color!=nfs_victory_wide.nativeColor||!nfs_victory_wide_attachment(front,color,nfs_victory_wide.nativeDepth)||!nfs_victory_wide_attachment(nfs_victory_wide.fbo,nfs_victory_wide.color,0)){nfs_victory_wide_revoke();return 0;}nfs_victory_wide.selected=1;return nfs_victory_wide.color;}
static void nfs_victory_wide_after_present(int success){static unsigned lines;if(lines<32&&nfs_victory_wide.selected&&success&&nfs_victory_wide_current()&&nfs_logo_trace_enabled()){++lines;fprintf(stderr,"VICTORY WIDE presented request=%llu output=%ux%u image=%u native300600=1\n",(unsigned long long)nfs_victory_wide.source.request,nfs_victory_wide.width,nfs_victory_wide.height,nfs_victory_wide.art.image);}nfs_victory_wide.selected=0;}
#undef NFS_VW_GL

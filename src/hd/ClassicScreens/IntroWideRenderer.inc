// SPDX-License-Identifier: MIT
// Completed original title content, retained through its actual front-surface swap.
// No native attachment is sampled or written by this independent compositor.
static struct {
 NfsIntroLiveView source;NfsIntroArtView art;
 uintptr_t context;uint64_t generation;GLuint fbo,color,photo,nativeFbo,nativeColor,nativeDepth;
 unsigned width,height,nativeWidth,nativeHeight,ready,selected;GLfloat matrix[16];
} nfs_title_wide;
static int nfs_title_wide_context(void){return nfs_title_wide.generation&&nfs_intro_context(nfs_title_wide.generation,nfs_title_wide.context);}
static int nfs_title_wide_current(void){return nfs_title_wide_context()&&nfs_title_wide.source.request&&nfs_title_wide.source.artGeneration==nfs_intro_art_generation()&&nfs_title_wide.art.generation==nfs_title_wide.source.artGeneration;}
static int nfs_title_wide_guard(void*u,const NfsBackgroundSceneStage*t){return u==&nfs_title_wide&&t&&t->generation==nfs_title_wide.generation&&t->contextIdentity==nfs_title_wide.context&&nfs_title_wide_current();}
static NfsBackgroundSceneStage nfs_title_wide_stage(void){NfsBackgroundSceneStage s={0};s.generation=nfs_title_wide.generation;s.contextIdentity=nfs_title_wide.context;s.framebuffer=nfs_title_wide.fbo?nfs_title_wide.fbo:nfs_title_wide.nativeFbo;s.width=nfs_title_wide.width;s.height=nfs_title_wide.height;s.ownerCurrent=nfs_title_wide_guard;s.ownerUser=&nfs_title_wide;return s;}
static void nfs_title_wide_revoke(void){if(!nfs_title_wide.source.owner&&!nfs_title_wide.art.owner&&!nfs_title_wide.ready&&!nfs_title_wide.selected)return;NfsIntroLiveView source=nfs_title_wide.source;NfsIntroArtView art=nfs_title_wide.art;memset(&nfs_title_wide.source,0,sizeof source);memset(&nfs_title_wide.art,0,sizeof art);nfs_title_wide.ready=nfs_title_wide.selected=0;if(source.owner&&source.release)source.release(source.owner);if(art.owner&&art.release)art.release(art.owner);}
static void nfs_title_wide_retire(int owner){if(owner&&nfs_title_wide_context()){if(nfs_title_wide.fbo)glDeleteFramebuffers(1,&nfs_title_wide.fbo);if(nfs_title_wide_context()&&nfs_title_wide.color)glDeleteTextures(1,&nfs_title_wide.color);if(nfs_title_wide_context()&&nfs_title_wide.photo)glDeleteTextures(1,&nfs_title_wide.photo);}nfs_title_wide_revoke();memset(&nfs_title_wide,0,sizeof nfs_title_wide);}
#define NFS_TITLE_GL(call) do{if(!nfs_title_wide_current())goto lost;call;if(!nfs_title_wide_current())goto lost;}while(0)
static int nfs_title_wide_attachment(GLuint framebuffer,GLuint color,GLuint depth){
 GLint prior=0,type=0,name=0;int ok=0;
 NFS_TITLE_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prior));NFS_TITLE_GL(glBindFramebuffer(GL_FRAMEBUFFER,framebuffer));
 NFS_TITLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));
 NFS_TITLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));ok=type==GL_TEXTURE&&(GLuint)name==color;
 if(depth){NFS_TITLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_TITLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));ok=ok&&type==GL_RENDERBUFFER&&(GLuint)name==depth;}
 NFS_TITLE_GL(glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)prior));return ok;
 lost:return 0;
}
static int nfs_title_wide_draw(GLuint texture,const GLfloat*quad,unsigned mode,unsigned w,unsigned h){
 NFS_TITLE_GL(glActiveTexture(GL_TEXTURE0));NFS_TITLE_GL(glUseProgram(nfs_intro_renderer.program));NFS_TITLE_GL(glUniformMatrix4fv(nfs_intro_renderer.matrix,1,GL_FALSE,nfs_title_wide.matrix));
 NFS_TITLE_GL(glUniform1i(nfs_intro_renderer.sampler,0));NFS_TITLE_GL(glUniform1i(nfs_intro_renderer.kind,mode));NFS_TITLE_GL(glUniform2f(nfs_intro_renderer.pixel,1.f/w,1.f/h));
 NFS_TITLE_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_TITLE_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_TITLE_GL(glEnableVertexAttribArray(0));
 for(unsigned i=1;i<4;++i){NFS_TITLE_GL(glDisableVertexAttribArray(i));}NFS_TITLE_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,quad));NFS_TITLE_GL(glDrawArrays(GL_TRIANGLES,0,6));return 1;
 lost:return 0;
}
static int nfs_title_wide_prepare(void){
 NfsBgSaved saved;NfsBackgroundSceneStage guard;GLfloat quad[24];GLint type=0,color=0,depth=0,maximum=0;int savedActive=0,ok=0,w=0,h=0;
 const NfsIntroArtView*a=&nfs_intro_renderer.art;const NfsIntroLiveView*v=&nfs_intro_renderer.view;
 if(a->schema!=2||!a->widePhotograph.pixels||!v->request||!nfs_intro_context(nfs_intro_renderer.generation,nfs_intro_renderer.context)||a->generation!=v->artGeneration||a->generation!=nfs_intro_art_generation()||!SDL_GetWindowSizeInPixels(sdlWin,&w,&h)||w!=winWidth||h!=winHeight||w<=0||h<=0||w>NFS_FRAME_MAX_DIMENSION||h>NFS_FRAME_MAX_DIMENSION||(uint64_t)(unsigned)w*(unsigned)h>NFS_FRAME_MAX_PIXELS||(int64_t)w*3<=(int64_t)h*4||(int64_t)w*9>(int64_t)h*16+16)return 0;
 nfs_title_wide_retire(1);v->retain(v->owner);a->retain(a->owner);nfs_title_wide.source=*v;nfs_title_wide.art=*a;nfs_title_wide.context=nfs_intro_renderer.context;nfs_title_wide.generation=nfs_intro_renderer.generation;nfs_title_wide.width=w;nfs_title_wide.height=h;nfs_title_wide.nativeFbo=nfs_intro_renderer.target.framebuffer;nfs_title_wide.nativeWidth=nfs_intro_renderer.target.width;nfs_title_wide.nativeHeight=nfs_intro_renderer.target.height;
 guard=nfs_title_wide_stage();if(!nfs_graphics_scene_save(&saved,&guard))goto lost;savedActive=1;
 {GLint actual=0;NFS_TITLE_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&actual));if((GLuint)actual!=nfs_title_wide.nativeFbo)goto done;}
 NFS_TITLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_TITLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&color));if(type!=GL_TEXTURE||!color)goto done;
 NFS_TITLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_TITLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&depth));if(type!=GL_RENDERBUFFER||!depth)goto done;
 nfs_title_wide.nativeColor=color;nfs_title_wide.nativeDepth=depth;
 NFS_TITLE_GL(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maximum));if(w>maximum||h>maximum||a->widePhotograph.width>(unsigned)maximum||a->widePhotograph.height>(unsigned)maximum)goto done;
 NFS_TITLE_GL(glActiveTexture(GL_TEXTURE0));NFS_TITLE_GL(glGenTextures(1,&nfs_title_wide.color));NFS_TITLE_GL(glBindTexture(GL_TEXTURE_2D,nfs_title_wide.color));
 NFS_TITLE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_TITLE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_TITLE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_TITLE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_TITLE_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL));
 NFS_TITLE_GL(glGenFramebuffers(1,&nfs_title_wide.fbo));NFS_TITLE_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_title_wide.fbo));NFS_TITLE_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,nfs_title_wide.color,0));
 {GLenum status=0;NFS_TITLE_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));if(status!=GL_FRAMEBUFFER_COMPLETE)goto done;}
 if(!nfs_title_wide.fbo||!nfs_title_wide.color||nfs_title_wide.fbo==nfs_title_wide.nativeFbo||nfs_title_wide.color==nfs_title_wide.nativeColor||nfs_title_wide.color==nfs_intro_renderer.color)goto done;
 NFS_TITLE_GL(glGenTextures(1,&nfs_title_wide.photo));NFS_TITLE_GL(glBindTexture(GL_TEXTURE_2D,nfs_title_wide.photo));NFS_TITLE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_TITLE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_TITLE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_TITLE_GL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));NFS_TITLE_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));NFS_TITLE_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,a->widePhotograph.width,a->widePhotograph.height,0,GL_RGBA,GL_UNSIGNED_BYTE,a->widePhotograph.pixels));
 if(!nfs_title_wide.photo||nfs_title_wide.photo==nfs_title_wide.color||nfs_title_wide.photo==nfs_title_wide.nativeColor)goto done;
 for(unsigned i=0;i<8;++i)if(nfs_intro_renderer.images[i]&&(nfs_intro_renderer.images[i]==nfs_title_wide.color||nfs_intro_renderer.images[i]==nfs_title_wide.photo))goto done;
 {const float logical=480.f*w/h,shift=(logical-640.f)/2,scale=fmaxf(logical/a->widePhotograph.width,480.f/a->widePhotograph.height);const float pw=a->widePhotograph.width*scale,ph=a->widePhotograph.height*scale;
 memset(nfs_title_wide.matrix,0,sizeof nfs_title_wide.matrix);nfs_title_wide.matrix[0]=2/logical;nfs_title_wide.matrix[5]=-2.f/480;nfs_title_wide.matrix[10]=1;nfs_title_wide.matrix[12]=-1;nfs_title_wide.matrix[13]=1;nfs_title_wide.matrix[15]=1;
 for(unsigned i=0;i<8;++i){NFS_TITLE_GL(glDisable(nfs_frame_caps[i]));}NFS_TITLE_GL(glColorMask(1,1,1,1));NFS_TITLE_GL(glDepthMask(GL_FALSE));NFS_TITLE_GL(glViewport(0,0,w,h));
 nfs_intro_quad(quad,(logical-pw)/2,(480-ph)/2,(logical+pw)/2,(480+ph)/2);if(!nfs_title_wide_draw(nfs_title_wide.photo,quad,0,a->widePhotograph.width,a->widePhotograph.height))goto done;
 NFS_TITLE_GL(glEnable(GL_BLEND));NFS_TITLE_GL(glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD));NFS_TITLE_GL(glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA));
 nfs_intro_quad(quad,a->logoRect.left+shift,a->logoRect.top,a->logoRect.right+shift,a->logoRect.bottom);for(unsigned i=0;i<6;++i){quad[4*i+2]=a->logoUv.left+(a->logoUv.right-a->logoUv.left)*quad[4*i+2];quad[4*i+3]=a->logoUv.top+(a->logoUv.bottom-a->logoUv.top)*quad[4*i+3];}if(!nfs_title_wide_draw(nfs_intro_renderer.images[1],quad,0,a->logo.width,a->logo.height))goto done;
 const NfsIntroSource*c=nfs_intro_sources+v->locale+1;const NfsIntroImage*word=a->copyright+v->locale;nfs_intro_quad(quad,c->left+shift,c->top,c->left+c->width+shift,c->top+c->height);if(!nfs_title_wide_draw(nfs_intro_renderer.images[2+v->locale],quad,1,word->width,word->height))goto done;
 }
 {GLenum error=0;NFS_TITLE_GL(error=glGetError());ok=error==GL_NO_ERROR&&nfs_title_wide_attachment(nfs_title_wide.fbo,nfs_title_wide.color,0);}
 done:if(savedActive&&!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!ok||!nfs_title_wide_attachment(nfs_title_wide.nativeFbo,nfs_title_wide.nativeColor,nfs_title_wide.nativeDepth)){nfs_title_wide_retire(1);return 0;}nfs_title_wide.ready=1;return 1;
 lost:nfs_title_wide_retire(0);return 0;
}
static GLuint nfs_title_wide_present(GLuint front,GLuint color,int w,int h){
 nfs_title_wide.selected=0;if(!nfs_title_wide.ready)return 0;
 if(!nfs_title_wide_current()||w!=(int)nfs_title_wide.width||h!=(int)nfs_title_wide.height||front!=nfs_title_wide.nativeFbo||color!=nfs_title_wide.nativeColor||!nfs_title_wide_attachment(front,color,nfs_title_wide.nativeDepth)){nfs_title_wide_revoke();return 0;}
 if(!nfs_title_wide_attachment(nfs_title_wide.fbo,nfs_title_wide.color,0)){nfs_title_wide_revoke();return 0;}
 nfs_title_wide.selected=1;return nfs_title_wide.color;
}
static void nfs_title_wide_after_present(int success){static unsigned lines;if(lines<32&&nfs_title_wide.selected&&success&&nfs_title_wide_current()&&nfs_logo_trace_enabled()){++lines;fprintf(stderr,"TITLE WIDE presented request=%llu output=%ux%u native300600=1\n",(unsigned long long)nfs_title_wide.source.request,nfs_title_wide.width,nfs_title_wide.height);}nfs_title_wide.selected=0;}
#undef NFS_TITLE_GL

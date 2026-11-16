// Private draft: complete native300/600 commit precedes this optional fullwindow pass.
static struct {
 NfsResultsLiveView view;NfsNativeLogoTargetProof nativeTarget;
 uintptr_t context;uint64_t generation,uploads;unsigned width,height,ready,selected,variant;
 NfsResultsCursor cursor;
 uint64_t cursorRevision;
 unsigned cursorTriangles,cursorBatch,cursorComplete;
 // The provider publishes immutable pixels for the lifetime of the process.
 const unsigned char*backdropPixels;
 unsigned backdropWidth,backdropHeight;
 GLuint fbo,color,backdrop,overlay,nativeColor,nativeDepth;NfsSoftwareTextGpu text;NfsFrameLegacySampling sampling;
}nfs_rsw;
// The lexical cursor receipt is independent of preparing or retiring a frame.
static struct {
 NfsResultsCursor view;
 uint64_t generation;
 uintptr_t context;
 GLuint brandingTexture;
} nfs_rsw_cursor_pending;
static NfsResultsLiveView nfs_rsw_parent;static GLuint nfs_rsw_parentColor,nfs_rsw_parentFbo;static unsigned nfs_rsw_parentWidth,nfs_rsw_parentHeight;
static void nfs_results_wide_parent_clear(void){void*p=nfs_rsw_parent.owner;void(*release)(void*)=nfs_rsw_parent.release;memset(&nfs_rsw_parent,0,sizeof nfs_rsw_parent);nfs_rsw_parentColor=nfs_rsw_parentFbo=nfs_rsw_parentWidth=nfs_rsw_parentHeight=0;if(p&&release)release(p);}
static int nfs_results_wide_parent_matches(uintptr_t game,uint64_t epoch,uint64_t content){GLint prior=0,type=0,name=0;int ok=0;if(!nfs_rsw_parent.current||!nfs_rsw_parent.current(nfs_rsw_parent.owner)||nfs_rsw_parent.sourceGame!=game||nfs_rsw_parent.sourceEpoch!=epoch||nfs_rsw_parent.content!=content||!nfs_results_context(nfs_rsw.generation,nfs_rsw.context)||nfs_rsw_parentColor!=nfs_rsw.color||nfs_rsw_parentFbo!=nfs_rsw.fbo||!glIsTexture(nfs_rsw_parentColor)||!glIsFramebuffer(nfs_rsw_parentFbo))return 0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prior);glBindFramebuffer(GL_FRAMEBUFFER,nfs_rsw_parentFbo);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);ok=type==GL_TEXTURE&&(GLuint)name==nfs_rsw_parentColor&&glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)prior);return ok&&nfs_rsw_parent.current(nfs_rsw_parent.owner)&&nfs_results_context(nfs_rsw.generation,nfs_rsw.context);}

static int nfs_rsw_current(void){return nfs_rsw.ready&&nfs_rsw.view.current&&nfs_rsw.view.current(nfs_rsw.view.owner)&&nfs_rsw.view.sourceGame&&nfs_rsw.view.sourceEpoch&&nfs_rsw.view.content&&nfs_results_context(nfs_rsw.generation,nfs_rsw.context)&&nfs_logo_provenance.uploads==nfs_rsw.uploads;}
static void nfs_results_wide_revoke(void){if(!nfs_rsw.ready&&!nfs_rsw.selected&&!nfs_rsw.view.owner)return;void*p=nfs_rsw.view.owner;void(*release)(void*)=nfs_rsw.view.release;nfs_rsw.ready=nfs_rsw.selected=0;nfs_rsw.cursor.request=0;
 nfs_rsw.cursorTriangles=nfs_rsw.cursorBatch=nfs_rsw.cursorComplete=0;memset(&nfs_rsw.view,0,sizeof nfs_rsw.view);if(p&&release)release(p);}
static void nfs_rsw_trace(const char*stage,const char*reason,int w,int h){
 const int saved=errno;
 const char*enabled=getenv("NFS_PRIVATE_RESULTS_TRACE");
 static unsigned count;
 if(enabled&&enabled[0]=='1'&&!enabled[1]&&count<96){
  ++count;
  fprintf(stderr,"RESULTS WIDE detail stage=%s reason=%s output=%dx%d ready=%u variant=%u content=%llu\n",stage,reason,w,h,nfs_rsw.ready,nfs_rsw.variant,(unsigned long long)nfs_rsw.view.content);
 }
 errno=saved;
}
static void nfs_results_wide_invalidate(const char*reason){
 if(nfs_rsw.ready)nfs_rsw_trace("invalidate",reason,nfs_rsw.width,nfs_rsw.height);
 nfs_results_wide_revoke();
}
static void nfs_results_wide_retire(int owner){memset(&nfs_rsw_cursor_pending,0,sizeof nfs_rsw_cursor_pending);nfs_results_wide_parent_clear();if(owner&&nfs_results_context(nfs_rsw.generation,nfs_rsw.context)){nfs_soft_gpu_context_retired(&nfs_rsw.text,nfs_rsw.generation,1);if(nfs_rsw.fbo)glDeleteFramebuffers(1,&nfs_rsw.fbo);GLuint t[]={nfs_rsw.color,nfs_rsw.backdrop,nfs_rsw.overlay};glDeleteTextures(3,t);}nfs_results_wide_revoke();memset(&nfs_rsw,0,sizeof nfs_rsw);}
static int nfs_rsw_own(void){GLint f=0,type=0,name=0;if(!nfs_rsw_current()||!nfs_rsw.fbo||!nfs_rsw.color)return 0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&f);if((GLuint)f!=nfs_rsw.fbo)return 0;glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);return type==GL_TEXTURE&&(GLuint)name==nfs_rsw.color&&glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE&&nfs_rsw_current();}
static int nfs_rsw_text_current(void*owner,const NfsSoftwareTextStage*t){return owner==&nfs_rsw&&t&&t->framebuffer==nfs_rsw.fbo&&t->width==nfs_rsw.width&&t->height==nfs_rsw.height&&t->logicalWidth==640&&t->logicalHeight==480&&nfs_rsw.text.generation==nfs_rsw.generation&&!memcmp(&t->sampling,&nfs_rsw.sampling,sizeof t->sampling)&&nfs_rsw_own();}
static int nfs_rsw_native_attachment(GLuint front,GLuint color){GLint prior=0,type=0,name=0;int ok=0;if(!nfs_rsw_current()||front!=nfs_rsw.nativeTarget.framebuffer||color!=nfs_rsw.nativeColor||!front||!color||!nfs_rsw.nativeDepth)return 0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prior);glBindFramebuffer(GL_FRAMEBUFFER,front);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);ok=type==GL_TEXTURE&&(GLuint)name==color;glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);ok=ok&&type==GL_RENDERBUFFER&&(GLuint)name==nfs_rsw.nativeDepth&&glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)prior);return ok&&nfs_rsw_current();}
static int nfs_results_wide_prepare(void){
 static const GLfloat full[]={-1,1,0,0,-1,-1,0,1,1,1,1,0,1,1,1,0,-1,-1,0,1,1,-1,1,1};
 static const GLfloat title[]={-.43125f,.9375f,0,0,-.43125f,.616666667f,0,1,.928125f,.9375f,1,0,.928125f,.9375f,1,0,-.43125f,.616666667f,0,1,.928125f,.616666667f,1,1};
 int w=0,h=0,ok=0;GLint type=0,name=0;GLuint color=0,fbo=0;unsigned char*rgba=NULL;NfsBgSaved saved;NfsSoftwareTextStage text;NfsResultsBackdropImage image={0};const char*stage="native-attachment";float logical=0,center=0;GLfloat art[24],overlay[24],heading[24];NfsResultsLiveView*v=&nfs_results_renderer.view;
 nfs_results_wide_parent_clear();nfs_results_wide_revoke();if(v->schema!=3||!v->current||!v->current(v->owner)||!v->backdropMask||!SDL_GetWindowSizeInPixels(sdlWin,&w,&h)||w<=0||h<=0||!nfs_frame_dimensions(w,h))return 0;logical=480.f*w/h;if(logical<=640.01f||logical>854.f)return 0;
 if(nfs_rsw.generation&&(nfs_rsw.generation!=nfs_results_renderer.generation||nfs_rsw.context!=nfs_results_renderer.context))nfs_results_wide_retire(0);
 v->retain(v->owner);nfs_rsw.view=*v;nfs_rsw.generation=nfs_results_renderer.generation;nfs_rsw.context=nfs_results_renderer.context;nfs_rsw.nativeTarget=nfs_results_renderer.target;nfs_rsw.uploads=nfs_logo_provenance.uploads;nfs_rsw.ready=1;nfs_rsw.variant=w*9>=h*14?1u:w*246>=h*341?2u:0u;
 if(!nfs_results_backdrop_image(nfs_rsw.variant,&image)){nfs_rsw_trace("prepare","artwork-unavailable",w,h);nfs_results_wide_revoke();return 0;}
 nfs_bg_save(&saved);glBindFramebuffer(GL_FRAMEBUFFER,nfs_rsw.nativeTarget.framebuffer);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);nfs_rsw.nativeColor=type==GL_TEXTURE?(GLuint)name:0;glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);nfs_rsw.nativeDepth=type==GL_RENDERBUFFER?(GLuint)name:0;
 if(!nfs_rsw_native_attachment(nfs_rsw.nativeTarget.framebuffer,nfs_rsw.nativeColor))goto done;
 stage="allocate-target";
 if(!nfs_rsw.fbo||nfs_rsw.width!=(unsigned)w||nfs_rsw.height!=(unsigned)h){glGenTextures(1,&color);glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,color);nfs_frame_texture_params();glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0);if(!color||!fbo||glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE||!nfs_rsw_current())goto done;if(nfs_rsw.fbo)glDeleteFramebuffers(1,&nfs_rsw.fbo);if(nfs_rsw.color)glDeleteTextures(1,&nfs_rsw.color);nfs_rsw.fbo=fbo;nfs_rsw.color=color;fbo=color=0;}
 nfs_rsw.width=w;nfs_rsw.height=h;if(!nfs_rsw.backdrop)glGenTextures(1,&nfs_rsw.backdrop);if(!nfs_rsw.overlay)glGenTextures(1,&nfs_rsw.overlay);if(!nfs_rsw.backdrop||!nfs_rsw.overlay||nfs_rsw.backdrop==nfs_rsw.nativeColor||nfs_rsw.overlay==nfs_rsw.nativeColor||nfs_rsw.color==nfs_rsw.nativeColor||nfs_rsw.overlay==nfs_rsw.color||nfs_rsw.backdrop==nfs_rsw.color||nfs_rsw.overlay==nfs_rsw.backdrop)goto done;
 stage="allocate-overlay";
 rgba=(unsigned char*)malloc(v->pixels*4);if(!rgba)goto done;for(size_t i=0;i<v->pixels;++i){unsigned char*q=rgba+4*i;unsigned x=v->clean[i],r=x>>11,g=(x>>5)&63,b=x&31;if(v->backdropMask[i])memset(q,0,4);else{q[0]=(r<<3)|(r>>2);q[1]=(g<<2)|(g>>4);q[2]=(b<<3)|(b>>2);q[3]=255;}}
 stage="upload-artwork";
 glActiveTexture(GL_TEXTURE0);
 glPixelStorei(GL_UNPACK_ALIGNMENT,1);
 if(nfs_rsw.backdropPixels!=image.pixels||
    nfs_rsw.backdropWidth!=image.width||nfs_rsw.backdropHeight!=image.height){
  glBindTexture(GL_TEXTURE_2D,nfs_rsw.backdrop);
  nfs_frame_texture_params();
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width,image.height,0,GL_RGBA,GL_UNSIGNED_BYTE,image.pixels);
  if(glGetError()!=GL_NO_ERROR){
   nfs_rsw.backdropPixels=NULL;
   goto done;
  }
  nfs_rsw.backdropPixels=image.pixels;
  nfs_rsw.backdropWidth=image.width;
  nfs_rsw.backdropHeight=image.height;
 }
 glBindTexture(GL_TEXTURE_2D,nfs_rsw.overlay);
 nfs_frame_texture_params();
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,640,480,0,GL_RGBA,GL_UNSIGNED_BYTE,rgba);
 free(rgba);
 rgba=NULL;
 if(glGetError()!=GL_NO_ERROR)goto done;
 stage="text-font";
 if(!nfs_rsw.text.generation&&!nfs_soft_gpu_context_created(&nfs_rsw.text,nfs_rsw.generation))goto done;if(!nfs_results_font_prepare(&nfs_rsw.text,&v->text.font))goto done;
 memset(&nfs_rsw.sampling,0,sizeof nfs_rsw.sampling);nfs_rsw.sampling.magFilter=linearFiltering?GL_LINEAR:GL_NEAREST;nfs_rsw.sampling.viewport[2]=w;nfs_rsw.sampling.viewport[3]=h;nfs_rsw.sampling.projection[0]=2.f/logical;nfs_rsw.sampling.projection[5]=-2.f/480;nfs_rsw.sampling.projection[10]=-2;nfs_rsw.sampling.projection[12]=-640.f/logical;nfs_rsw.sampling.projection[13]=nfs_rsw.sampling.projection[14]=nfs_rsw.sampling.projection[15]=1;
 stage="draw-image";
 glBindFramebuffer(GL_FRAMEBUFFER,nfs_rsw.fbo);if(!nfs_rsw_own())goto done;nfs_frame_draw_state();glViewport(0,0,w,h);glUseProgram(nfs_results_renderer.copyProgram);glUniform1i(nfs_results_renderer.copySampler,0);glUniform4f(nfs_results_renderer.copyFog,0,0,0,0);glActiveTexture(GL_TEXTURE0);glBindBuffer(GL_ARRAY_BUFFER,0);glDisable(GL_BLEND);glEnableVertexAttribArray(0);glDisableVertexAttribArray(1);glDisableVertexAttribArray(2);glDisableVertexAttribArray(3);
 memcpy(art,full,sizeof art);memcpy(overlay,full,sizeof overlay);memcpy(heading,title,sizeof heading);center=640.f/logical;{const float a=(float)image.width/image.height,t=(float)w/h,u=t>a?t/a:1.f,z=a>t?a/t:1.f;for(unsigned k=0;k<6;++k){art[4*k+2]=.5f+(art[4*k+2]-.5f)*u;art[4*k+3]=.5f+(art[4*k+3]-.5f)*z;overlay[4*k]*=center;heading[4*k]*=center;}}
 glBindTexture(GL_TEXTURE_2D,nfs_rsw.backdrop);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,art);glDrawArrays(GL_TRIANGLES,0,6);if(!nfs_rsw_own())goto done;glEnable(GL_BLEND);glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);glBindTexture(GL_TEXTURE_2D,nfs_rsw.overlay);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,overlay);glDrawArrays(GL_TRIANGLES,0,6);
 glUseProgram(nfs_results_renderer.headingProgram);glUniform1i(nfs_results_renderer.headingSampler,0);glUniform1f(nfs_results_renderer.headingRow,(float)(v->headingIndex-1));glBindTexture(GL_TEXTURE_2D,nfs_results_renderer.headingTexture);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,heading);glDrawArrays(GL_TRIANGLES,0,6);if(!nfs_rsw_own())goto done;
 stage="draw-text";
 memset(&text,0,sizeof text);text.framebuffer=nfs_rsw.fbo;text.width=w;text.height=h;text.sampling=nfs_rsw.sampling;text.logicalWidth=640;text.logicalHeight=480;if(!nfs_soft_gpu_draw_into_owned_responsive_staging(&nfs_rsw.text,&text,v->text.vertices,v->text.vertexCount,&nfs_rsw,nfs_rsw_text_current)||!nfs_rsw_own())goto done;
 if(nfs_results_renderer.fogAmount){glUseProgram(nfs_results_renderer.copyProgram);glUniform1i(nfs_results_renderer.copySampler,0);glUniform4f(nfs_results_renderer.copyFog,nfs_results_renderer.fogColor[0]*(nfs_results_renderer.fogAmount/255.f),nfs_results_renderer.fogColor[1]*(nfs_results_renderer.fogAmount/255.f),nfs_results_renderer.fogColor[2]*(nfs_results_renderer.fogAmount/255.f),nfs_results_renderer.fogAmount/255.f);glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,full);glDrawArrays(GL_TRIANGLES,0,6);}
 stage="validate-draw";
 ok=nfs_rsw_own()&&glGetError()==GL_NO_ERROR;
 if(ok&&v->headingIndex==1){v->retain(v->owner);nfs_rsw_parent=*v;nfs_rsw_parentColor=nfs_rsw.color;nfs_rsw_parentFbo=nfs_rsw.fbo;nfs_rsw_parentWidth=w;nfs_rsw_parentHeight=h;}
 done:free(rgba);if(nfs_results_context(nfs_rsw.generation,nfs_rsw.context)){if(fbo)glDeleteFramebuffers(1,&fbo);if(color)glDeleteTextures(1,&color);nfs_bg_restore(&saved);}else ok=0;nfs_rsw_trace("prepare",ok?"complete":stage,w,h);if(!ok)nfs_results_wide_revoke();return ok;
}
static GLuint nfs_results_wide_present(GLuint front,GLuint texture,int w,int h){nfs_rsw.selected=0;if(!nfs_rsw.ready)return 0;
 const char*refusal=NULL;
 if(!nfs_rsw_current())refusal="stale-owner";
 else if(nfs_rsw.cursor.request&&!nfs_rsw.cursorComplete)refusal="cursor-incomplete";
 else if(w!=(int)nfs_rsw.width||h!=(int)nfs_rsw.height)refusal="size-changed";
 else if(nfs_rsw.color==texture||!glIsTexture(nfs_rsw.color)||!glIsFramebuffer(nfs_rsw.fbo))refusal="target-missing";
 else if(!nfs_rsw_native_attachment(front,texture))refusal="front-mismatch";
 if(refusal){nfs_rsw_trace("select",refusal,w,h);return 0;}nfs_rsw.selected=1;return nfs_rsw.color;}
static void nfs_results_wide_after_present(int success){if(nfs_rsw.selected&&success&&nfs_rsw_current()){static unsigned lines;if(nfs_logo_trace_enabled()&&lines++<32)fprintf(stderr,"RESULTS WIDE presented game=%llu epoch=%llu content=%llu heading=%u variant=%u output=%ux%u native300600=1 fresh=1\n",(unsigned long long)nfs_rsw.view.sourceGame,(unsigned long long)nfs_rsw.view.sourceEpoch,(unsigned long long)nfs_rsw.view.content,nfs_rsw.view.headingIndex,nfs_rsw.variant,nfs_rsw.width,nfs_rsw.height);}nfs_results_wide_revoke();}

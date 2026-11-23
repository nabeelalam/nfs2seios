// SPDX-License-Identifier: MIT
/* A prepared image owns GPU pixels only. No guest pointers or borrowed frame
   packets survive the synchronous producer callback. Presentation is separately
   admitted after the original cursor draw; this preparation grants none. */
#include "NestedSceneLayout.h"
static struct {
 uintptr_t context,thread;uint64_t generation,uploads,clearEvent,serial,revision;
 GLuint fbo,color,art[2],nativeColor,nativeDepth;unsigned width,height,ratio,kind,phase,cursorSeen;
 NfsNativeLogoTargetProof target;NfsNestedSceneLayout layout;
} nfs_nested;
static int nfs_nested_context(void){return nfs_nested.context&&g_glCtx&&
 (uintptr_t)g_glCtx==nfs_nested.context&&SDL_GL_GetCurrentContext()==g_glCtx&&
 nfs_nested.thread==(uintptr_t)SDL_GetCurrentThreadID()&&
 nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled&&
 nfs_nested.generation==nfs_logo_provenance.context;}
static int nfs_nested_current(void){return nfs_nested_context()&&nfs_nested.phase&&
 nfs_nested.uploads==nfs_logo_provenance.uploads&&
 nfs_nested.clearEvent==nfs_logo_provenance.clearEvent;}
static void nfs_nested_revoke(void){nfs_nested.phase=0;nfs_nested.serial=0;nfs_nested.cursorSeen=0;}
static void nfs_nested_retire(int owner){
 if(owner&&nfs_nested_context()){
  if(nfs_nested.fbo)glDeleteFramebuffers(1,&nfs_nested.fbo);
  if(nfs_nested_context()&&nfs_nested.color)glDeleteTextures(1,&nfs_nested.color);
  if(nfs_nested_context()&&nfs_nested.art[0])glDeleteTextures(2,nfs_nested.art);
 }
 memset(&nfs_nested,0,sizeof nfs_nested);
}
#define NFS_NESTED_GL(call) do{if(!nfs_nested_current())goto lost;call;if(!nfs_nested_current())goto lost;}while(0)
static int nfs_nested_stage_owner(void*user,const NfsBackgroundSceneStage*t){
 GLint fbo=0,type=0,name=0;
 if(user!=&nfs_nested||!t||!nfs_nested_current()||
 t->generation!=nfs_nested.generation||t->contextIdentity!=nfs_nested.context||
 t->framebuffer!=nfs_nested.fbo||t->width!=nfs_nested.width||t->height!=nfs_nested.height)return 0;
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);if(!nfs_nested_current()||(GLuint)fbo!=t->framebuffer)return 0;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);
 if(!nfs_nested_current())return 0;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);
 return nfs_nested_current()&&type==GL_TEXTURE&&(GLuint)name==nfs_nested.color;
}
static int nfs_nested_allocate(unsigned w,unsigned h,unsigned ratio,const NfsWideMenuLayers*a){
 GLuint color=0,fbo=0,art[2]={0,0};GLenum status=0;int ok=0;
 if(!a||!a->map||!a->panel||!a->width||!a->height||a->width>4096||a->height>4096)return 0;
 if(nfs_nested.width!=w||nfs_nested.height!=h||!nfs_nested.fbo||!nfs_nested.color){
  NFS_NESTED_GL(glGenTextures(1,&color));if(!color)goto done;
  NFS_NESTED_GL(glBindTexture(GL_TEXTURE_2D,color));NFS_NESTED_GL(nfs_frame_texture_params());
  NFS_NESTED_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL));
  NFS_NESTED_GL(glGenFramebuffers(1,&fbo));if(!fbo)goto done;
  NFS_NESTED_GL(glBindFramebuffer(GL_FRAMEBUFFER,fbo));
  NFS_NESTED_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0));
  NFS_NESTED_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));
  if(status!=GL_FRAMEBUFFER_COMPLETE||glGetError()!=GL_NO_ERROR)goto done;
  if(nfs_nested.fbo){NFS_NESTED_GL(glDeleteFramebuffers(1,&nfs_nested.fbo));}
  if(nfs_nested.color){NFS_NESTED_GL(glDeleteTextures(1,&nfs_nested.color));}
  nfs_nested.fbo=fbo;nfs_nested.color=color;nfs_nested.width=w;nfs_nested.height=h;fbo=color=0;
 }
 if(!nfs_nested.art[0]||nfs_nested.ratio!=ratio){
  NFS_NESTED_GL(glGenTextures(2,art));NFS_NESTED_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));
  for(unsigned i=0;i<2;++i){if(!art[i])goto done;
   NFS_NESTED_GL(glBindTexture(GL_TEXTURE_2D,art[i]));NFS_NESTED_GL(nfs_frame_texture_params());
   NFS_NESTED_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,a->width,a->height,0,GL_RGBA,GL_UNSIGNED_BYTE,i?a->panel:a->map));
   if(glGetError()!=GL_NO_ERROR)goto done;
  }
  if(nfs_nested.art[0]){NFS_NESTED_GL(glDeleteTextures(2,nfs_nested.art));}
  memcpy(nfs_nested.art,art,sizeof art);memset(art,0,sizeof art);nfs_nested.ratio=ratio;
 }
 ok=1;
 done:if(fbo){NFS_NESTED_GL(glDeleteFramebuffers(1,&fbo));}if(color){NFS_NESTED_GL(glDeleteTextures(1,&color));}
 if(art[0]||art[1]){NFS_NESTED_GL(glDeleteTextures(2,art));}return ok;
 lost:nfs_nested_retire(0);return 0;
}
static void nfs_nested_note(unsigned kind,unsigned step,int w,int h){
 static unsigned counts[4][10];const int saved=errno;
 if(kind<4&&step<10&&nfs_logo_trace_enabled()&&counts[kind][step]<2){++counts[kind][step];
 fprintf(stderr,"NESTED WIDE checkpoint kind=%u step=%u size=%dx%d phase=%u context=%u\n",kind,step,w,h,nfs_nested.phase,nfs_nested_context());}
 errno=saved;
}
static void nfs_nested_prepare(unsigned kind,uint64_t serial,uint64_t revision,
 const void*view,const NfsNativeLogoTargetProof*target){
 NfsBgSaved saved;NfsNestedSceneLayout layout;NfsWideMenuLayers art={0};
 NfsBackgroundSceneStage stage;NfsNestedBackdropTextures textures;int w=0,h=0,ok=0;
 GLint bound=0,type=0,name=0;unsigned step=1;
 nfs_nested_revoke();
 if(!view||!target||!serial||!revision||!sdlWin||!g_glCtx||SDL_GL_GetCurrentContext()!=g_glCtx||
 !SDL_GetWindowSizeInPixels(sdlWin,&w,&h)||w<=0||h<=0||
 !nfs_nested_scene_layout((unsigned)w,(unsigned)h,GL_LINEAR,&layout)){nfs_nested_note(kind,step,w,h);return;}step=2;
 if(nfs_nested.context&&(!nfs_nested_context()))nfs_nested_retire(0);
 nfs_nested.context=(uintptr_t)g_glCtx;nfs_nested.thread=(uintptr_t)SDL_GetCurrentThreadID();
 nfs_nested.generation=nfs_logo_provenance.context;nfs_nested.uploads=nfs_logo_provenance.uploads;
 nfs_nested.clearEvent=nfs_logo_provenance.clearEvent;nfs_nested.phase=1;
 if(!nfs_nested_current()||target->contextGeneration!=nfs_nested.generation)goto lost;
 step=3;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&bound);
 if(!nfs_nested_current()||(GLuint)bound!=target->framebuffer)goto refused;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);
 if(!nfs_nested_current())goto lost;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);
 if(!nfs_nested_current())goto lost;
 if(type!=GL_TEXTURE||(GLuint)name!=g_framebufferTexture||!g_framebufferTexture)goto refused;
 step=4;if(!nfs_wide_menu_layers(nfs_wide_menu_ratio(layout.logicalWidth),&art))goto refused;step=5;
 nfs_bg_save(&saved);if(!nfs_nested_current())goto lost;
 NFS_NESTED_GL(glActiveTexture(GL_TEXTURE0));
 if(!nfs_nested_allocate(w,h,nfs_wide_menu_ratio(layout.logicalWidth),&art)){
  if(!nfs_nested_current())goto lost;goto restore;
 }
 step=6;nfs_nested.layout=layout;nfs_nested.target=*target;nfs_nested.nativeColor=g_framebufferTexture;
 nfs_nested.nativeDepth=g_framebufferDepth;nfs_nested.serial=serial;nfs_nested.revision=revision;nfs_nested.kind=kind;
 NFS_NESTED_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_nested.fbo));
 NFS_NESTED_GL(glViewport(0,0,w,h));NFS_NESTED_GL(glDisable(GL_SCISSOR_TEST));
 memset(&stage,0,sizeof stage);stage.generation=nfs_nested.generation;stage.contextIdentity=nfs_nested.context;
 stage.framebuffer=nfs_nested.fbo;stage.width=w;stage.height=h;stage.sampling=layout.sidebar;
 stage.ownerCurrent=nfs_nested_stage_owner;stage.ownerUser=&nfs_nested;
 textures.generation=nfs_nested.generation;textures.map=nfs_nested.art[0];textures.panel=nfs_nested.art[1];textures.ratio=nfs_nested.ratio;
 if(kind==1){const NfsAudioSceneFrameView*v=(const NfsAudioSceneFrameView*)view;
  NfsAudioSceneGpu gpu={&nfs_bg_compositor,&nfs_audio_renderer.bars,&nfs_audio_renderer.caption,&nfs_audio_renderer.text,&nfs_audio_renderer.sidebarWordGpu,&nfs_audio_renderer.dialogWordsGpu};
  ok=nfs_audio_scene_draw_layers_into_nested_staging(&gpu,&stage,&v->layers,&textures);
 }else if(kind==2){const NfsGraphicsSceneFrameView*v=(const NfsGraphicsSceneFrameView*)view;
  NfsGraphicsSceneGpu gpu={&nfs_bg_compositor,&nfs_graphics_renderer.caption,&nfs_graphics_renderer.text,&nfs_graphics_renderer.sidebarWordGpu,&nfs_graphics_renderer.dialogWordsGpu};
  ok=nfs_graphics_scene_draw_layers_into_nested_staging(&gpu,&stage,&v->layers,&textures);
 }else if(kind==3){const NfsControlsSceneFrameView*v=(const NfsControlsSceneFrameView*)view;
  NfsControlsSceneGpu gpu={&nfs_bg_compositor,&nfs_controls_renderer.caption,&nfs_controls_renderer.text,&nfs_controls_renderer.sidebarWordGpu,&nfs_controls_renderer.dialogWordsGpu};
  ok=nfs_controls_scene_draw_layers_into_nested_staging(&gpu,&stage,&v->layers,&textures);
 }else if(kind==4){const NfsOptionsBitmapPreludeView*v=(const NfsOptionsBitmapPreludeView*)view;NfsSoftwareTextGpu empty;memset(&empty,0,sizeof empty);
  ok=!nfs_nested_caption_alias(&nfs_options_bitmap_renderer.word,&nfs_bg_compositor,&empty,&empty,nfs_nested.art[0],0,nfs_nested.color,nfs_nested.nativeColor)&&nfs_options_bitmap_renderer.word.texture!=nfs_nested.art[1]&&nfs_nested_caption_ready(&nfs_options_bitmap_renderer.word,&v->caption,&stage)&&nfs_bg_scene_draw_nested_into_staging(&nfs_bg_compositor,&stage,&v->background,&textures)&&nfs_nested_caption_draw_responsive(&nfs_options_bitmap_renderer.word,&v->caption,&stage);
 }
 restore:nfs_nested_note(kind,ok?8:step,w,h);if(!nfs_nested_current())goto lost;nfs_bg_restore(&saved);if(!nfs_nested_current())goto lost;
 nfs_nested.phase=ok?2:0;
 if(ok&&nfs_logo_trace_enabled()){static unsigned lines;if(lines<32){++lines;fprintf(stderr,"NESTED WIDE prepared kind=%u size=%dx%d serial=%llu\n",kind,w,h,(unsigned long long)serial);}}
 return;
 refused:nfs_nested_note(kind,step,w,h);nfs_nested_revoke();return;
 lost:nfs_nested_note(kind,9,w,h);nfs_nested_retire(0);
}
static int nfs_nested_options_bitmap_prepare(const NfsOptionsBitmapPreludeView*v,const NfsNativeLogoTargetProof*t){if(!v||v->schema!=1)return 0;nfs_nested_prepare(4,v->serial,v->content,v,t);return nfs_nested_current()&&nfs_nested.phase==2&&nfs_nested.kind==4&&nfs_nested.serial==v->serial;}
static void nfs_nested_audio_prepare(const NfsAudioSceneFrameView*v,const NfsNativeLogoTargetProof*t){if(v&&v->schema==NFS_AUDIO_SCENE_RUNTIME_SCHEMA)nfs_nested_prepare(1,v->serial,v->contentRevision,v,t);}
static void nfs_nested_graphics_prepare(const NfsGraphicsSceneFrameView*v,const NfsNativeLogoTargetProof*t){if(v&&v->schema==NFS_GRAPHICS_SCENE_RUNTIME_SCHEMA)nfs_nested_prepare(2,v->serial,v->contentRevision,v,t);}
static void nfs_nested_controls_prepare(const NfsControlsSceneFrameView*v,const NfsNativeLogoTargetProof*t){if(v&&v->schema==NFS_CONTROLS_SCENE_RUNTIME_SCHEMA)nfs_nested_prepare(3,v->serial,v->contentRevision,v,t);}
#undef NFS_NESTED_GL

/* Replay only the current authenticated original cursor quad. Unrecognized
   drawing discards this output instead of retaining a stale menu image. */
static void nfs_nested_tail(unsigned count){
 NfsBgSaved saved;GLint fbo=0,program=0,vp[4],sc[4];GLfloat matrix[16];int ok=0;
 if(nfs_nested.phase!=2)return;
 if(!nfs_nested_current()||count!=2||count!=g_trianglesCount||nfs_nested.cursorSeen||
 !nfs_mark_owner()||!nfs_native_marks.sent||nfs_native_marks.program!=g_shaderProgram||
 g_textureEnabled!=1||g_fogEnabled||nfs_hd_selected)goto refused;
 {const int selected=nfs_native_marks.selected;
 if(selected<0||selected>=NFS_NATIVE_MARK_MAX_BINDINGS)goto refused;
 const NfsNativeMarkBinding*b=nfs_native_marks.bindings+selected;
 if(!b->valid||b->generation!=nfs_native_marks.generation||!b->revision||
 b->encoding!=NFS_NATIVE_MARK_SOURCE_RGBA4444_LE||b->edge!=32||b->preparedBytes!=2048||
 b->texture>=NFS_NATIVE_MARK_MAX_TEXTURES||(b->address&3)||b->address>TextureMem-2048)goto refused;
 const TextureInfo*ti=g_textures+(b->address>>2);const NfsNativeMarkTexture*t=nfs_native_marks.textures+b->texture;
 if(ti->id!=b->nativeId||ti->fmt!=GR_TEXFMT_ARGB_4444||ti->size!=32||ti->hdDigits||
 ti->data!=g_textureMem+b->address||!t->texture||!t->art.identity||
 t->generation!=nfs_native_marks.generation||(t->art.logicalWidth!=32||t->art.logicalHeight!=32)||
 t->texture==nfs_nested.color||t->texture==nfs_nested.nativeColor)goto refused;
 }
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_CURRENT_PROGRAM,&program);glGetIntegerv(GL_VIEWPORT,vp);
 if(!nfs_nested_current()||(GLuint)fbo!=nfs_nested.target.framebuffer||(GLuint)program!=g_shaderProgram||
 g_framebufferTexture!=nfs_nested.nativeColor||g_framebufferDepth!=nfs_nested.nativeDepth||
 g_framebufferHeight!=(int)nfs_nested.target.height||
 memcmp(vp,nfs_nested.target.sampling.viewport,sizeof vp)||
 memcmp(g_matrix,nfs_nested.target.sampling.projection,sizeof g_matrix))goto refused;
 {static const unsigned corners[]={0,1,2,0,2,3};
 static const float us[]={0,255.f/256.f,255.f/256.f,0},vs[]={0,0,255.f/256.f,255.f/256.f};
 const float left=g_vertices[0].vertex[0].x,top=g_vertices[0].vertex[0].y;
 const float edge=g_vertices[0].vertex[1].x-left;
 if(!isfinite(left)||!isfinite(top)||!isfinite(edge)||edge<=0||edge>64||left< -64||left>704||top< -64||top>544)goto refused;
 for(unsigned k=0;k<6;++k){const unsigned c=corners[k],i=k/3,j=k%3;
  if(g_vertices[i].vertex[j].x!=left+((c==1||c==2)?edge:0)||
  g_vertices[i].vertex[j].y!=top+(c>=2?edge:0)||g_vertices[i].vertex[j].z!=1||
  g_textureCoord[i].vertex[j].s!=us[c]||g_textureCoord[i].vertex[j].t!=vs[c]||
  g_textureCoord[i].vertex[j].r!=0||g_textureCoord[i].vertex[j].q!=1)goto refused;
 }}
 nfs_bg_save(&saved);memcpy(matrix,g_matrix,sizeof matrix);glGetIntegerv(GL_SCISSOR_BOX,sc);
#define NFS_NESTED_CURSOR_GL(call) do{if(!nfs_nested_current())goto done;call;if(!nfs_nested_current())goto done;}while(0)
 NFS_NESTED_CURSOR_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_nested.fbo));
 {NfsBackgroundSceneStage stage={0};stage.generation=nfs_nested.generation;stage.contextIdentity=nfs_nested.context;
 stage.framebuffer=nfs_nested.fbo;stage.width=nfs_nested.width;stage.height=nfs_nested.height;
 if(!nfs_nested_stage_owner(&nfs_nested,&stage))goto done;}
 NFS_NESTED_CURSOR_GL(glViewport(0,0,nfs_nested.width,nfs_nested.height));
 NFS_NESTED_CURSOR_GL(glDisable(GL_DEPTH_TEST));NFS_NESTED_CURSOR_GL(glDepthMask(GL_FALSE));
 NFS_NESTED_CURSOR_GL(glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nfs_nested.layout.content.projection));
 NFS_NESTED_CURSOR_GL(glVertexAttribPointer(g_aPositionLoc,3,GL_FLOAT,GL_FALSE,0,g_vertices));
 {const float*m=nfs_nested.layout.content.projection;float l=0,r=640,top=0,bottom=480;
 if(saved.common.caps[4]){const GLint*v=nfs_nested.target.sampling.viewport;
 l=fmaxf(0,((sc[0]-v[0])*2.f/v[2]-1-matrix[12])/matrix[0]);
 r=fminf(640,((sc[0]+sc[2]-v[0])*2.f/v[2]-1-matrix[12])/matrix[0]);
 top=fmaxf(0,((sc[1]+sc[3]-v[1])*2.f/v[3]-1-matrix[13])/matrix[5]);
 bottom=fminf(480,((sc[1]-v[1])*2.f/v[3]-1-matrix[13])/matrix[5]);}
 if(l>r||top>bottom)goto done;
 GLint x=(GLint)floorf((m[0]*l+m[12]+1)*nfs_nested.width*.5f),right=(GLint)ceilf((m[0]*r+m[12]+1)*nfs_nested.width*.5f);
 GLint y=(GLint)floorf((m[5]*bottom+m[13]+1)*nfs_nested.height*.5f),end=(GLint)ceilf((m[5]*top+m[13]+1)*nfs_nested.height*.5f);
 NFS_NESTED_CURSOR_GL(glEnable(GL_SCISSOR_TEST));NFS_NESTED_CURSOR_GL(glScissor(x,y,right-x,end-y));}
 NFS_NESTED_CURSOR_GL(glDrawArrays(GL_TRIANGLES,0,6));ok=glGetError()==GL_NO_ERROR;
 done:if(nfs_nested_context()){glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,matrix);nfs_bg_restore(&saved);}else ok=0;
#undef NFS_NESTED_CURSOR_GL
 if(ok&&nfs_nested_current()){nfs_nested.cursorSeen=1;return;}
 refused:nfs_nested_revoke();
}
static GLuint nfs_nested_present(GLuint fbo,GLuint color,int width,int height){
 if(!nfs_nested_current()||nfs_nested.phase!=2||(!nfs_nested.cursorSeen&&nfs_nested.kind!=4)||
 fbo!=nfs_nested.target.framebuffer||color!=nfs_nested.nativeColor||
 width!=(int)nfs_nested.width||height!=(int)nfs_nested.height)return 0;
 return nfs_nested.color;
}

// SPDX-License-Identifier: MIT
/* Main-only responsive presentation. The original game color/depth surfaces,
   original sampling proof and native tile pass are never resized or overwritten. */
#include "MenuResponsiveLayout.h"
#include "MenuResponsiveDebug.h"
#include "MenuResponsiveCached.h"
#include <errno.h>
#include <math.h>
static struct {
 uintptr_t context; uint64_t generation,ownerEpoch,serial,revision;
 GLuint color,fbo,nativeFbo,nativeColor,nativeDepth;
 unsigned width,height,nativeHeight,phase,tailTriangles,cursorSeen;
 float logicalWidth,delta,matrix[16]; NfsMenuResponsiveLayout selected;
 uint32_t ownerForm;NfsMenuCachedSnapshot cached;
} nfs_menu_wide;
#include "MenuOriginalBranding.inc"
/* Original4276F0 emits a padded128-square as TWO full-UV triangles.
   Main opponent slot5 A/B/C share source anchor(510,406); padding below480
   remains clipped by the original viewport/scissor. Never crop/re-fit its ink. */
static int nfs_menu_wide_square128(unsigned count,float left,float top){
 static const unsigned corners[6]={0,1,2,0,2,3};
 static const float dx[4]={0,128,128,0},dy[4]={0,0,128,128};
 static const float us[4]={0,255.f/256.f,255.f/256.f,0},vs[4]={0,0,255.f/256.f,255.f/256.f};
 if(count!=2||!isfinite(left)||!isfinite(top))return 0;
 for(unsigned k=0;k<6;++k){unsigned c=corners[k],i=k/3,j=k%3;
  if(g_vertices[i].vertex[j].x!=left+dx[c]||g_vertices[i].vertex[j].y!=top+dy[c]||g_vertices[i].vertex[j].z!=1||
     g_textureCoord[i].vertex[j].s!=us[c]||g_textureCoord[i].vertex[j].t!=vs[c]||g_textureCoord[i].vertex[j].r!=0||g_textureCoord[i].vertex[j].q!=1)return 0;
 }
 return 1;
}
static int nfs_menu_wide_slot5_square(unsigned count,uint64_t epoch,int originalBranding,int selected){
 NfsMenuResponsiveDebugState source;memset(&source,0,sizeof source);nfs_menu_responsive_debug_state(&source);
 if(source.schema!=3||source.sourceForm!=nfs_menu_wide.ownerForm||!source.cursorValid||source.sourceEpoch!=epoch||source.originalOpponentValid!=1)return 0;
 if(!originalBranding){
  /* A future authored A/B/C mark retains the SAME original source canvas.
     Existing authored digest/lifetime admission is still required by caller. */
  if(selected<0||selected>=NFS_NATIVE_MARK_MAX_BINDINGS)return 0;
  const NfsNativeMarkBinding*b=nfs_native_marks.bindings+selected;
  if(b->edge!=128||b->encoding!=NFS_NATIVE_MARK_SOURCE_RGBA5551_LE||b->address!=source.originalOpponentAddress)return 0;
 }
 return nfs_menu_wide_square128(count,510,406);
}
static int nfs_menu_wide_source(uint64_t*epoch,float*x,float*y){
 if(nfs_menu_wide.cached.schema)return nfs_menu_cached_current(&nfs_menu_wide.cached,epoch,x,y);
 return nfs_menu_responsive_source_main(epoch,x,y);
}
enum {NFS_MENU_WIDE_IDLE=0,NFS_MENU_WIDE_PREPARED=1,NFS_MENU_WIDE_TAIL=2,NFS_MENU_WIDE_SELECTED=3};
/* Stage1 prepare,2 native-finish,3 tail,4 select,5 cancel. First two
   observations per(stage,reason,drawable,sourceEpoch);128keys/256lines max. */
static void nfs_menu_wide_diag(unsigned stage,unsigned reason,int detail,float actual,float expected,float z,float q){
 static struct {unsigned stage,reason,w,h,count;uint64_t epoch;} seen[128];
 static unsigned used,total;int priorErrno=errno,w=0,h=0;unsigned slot=128;
 NfsMenuResponsiveDebugState source;GLint fbo=-1,program=-1,texture=-1;unsigned encoding=0;
 if(!nfs_logo_trace_enabled()||total>=256){errno=priorErrno;return;}
 memset(&source,0,sizeof source);nfs_menu_responsive_debug_state(&source);
 if(sdlWin)SDL_GetWindowSizeInPixels(sdlWin,&w,&h);
 for(unsigned i=0;i<used;++i)if(seen[i].stage==stage&&seen[i].reason==reason&&seen[i].w==(unsigned)w&&seen[i].h==(unsigned)h&&seen[i].epoch==source.sourceEpoch){slot=i;break;}
 if(slot==128&&used<128){slot=used++;seen[slot].stage=stage;seen[slot].reason=reason;seen[slot].w=(unsigned)w;seen[slot].h=(unsigned)h;seen[slot].epoch=source.sourceEpoch;}
 if(slot==128||seen[slot].count>=2){errno=priorErrno;return;}++seen[slot].count;++total;
 if(g_glCtx&&SDL_GL_GetCurrentContext()==g_glCtx&&SDL_IsMainThread()){
  glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_CURRENT_PROGRAM,&program);glGetIntegerv(GL_TEXTURE_BINDING_2D,&texture);
 }
 if(nfs_native_marks.selected>=0&&nfs_native_marks.selected<NFS_NATIVE_MARK_MAX_BINDINGS)encoding=nfs_native_marks.bindings[nfs_native_marks.selected].encoding;
 fprintf(stderr,"MAIN RESPONSIVE DIAG stage=%u reason=%u detail=%d phase=%u serial=%llu sourceForm=%08x epoch=%llu cursorValid=%u cursor=%u,%u shown=%u output=%dx%d prepared=%ux%u nativeFbo=%u actualFbo=%d nativeColor=%u nativeDepth=%u program=%d nativeProgram=%u texture=%d fontTag=%d mark=%d encoding=%u slot5=%u slot5Valid=%u selectedAddress=%u selectedId=%u selectedEdge=%u selectedFormat=%u originalRevision=%llu actual=%g expected=%g z=%g q=%g\n",stage,reason,detail,nfs_menu_wide.phase,(unsigned long long)nfs_menu_wide.serial,source.sourceForm,(unsigned long long)source.sourceEpoch,source.cursorValid,source.sourceX,source.sourceY,source.shownActive,w,h,nfs_menu_wide.width,nfs_menu_wide.height,nfs_menu_wide.nativeFbo,fbo,nfs_menu_wide.nativeColor,nfs_menu_wide.nativeDepth,program,g_shaderProgram,texture,nfs_hd_selected,nfs_native_marks.selected,encoding,source.originalOpponentAddress,source.originalOpponentValid,nfs_menu_original_selection.address,nfs_menu_original_selection.id,nfs_menu_original_selection.edge,nfs_menu_original_selection.format,(unsigned long long)nfs_menu_original_selection.revision,(double)actual,(double)expected,(double)z,(double)q);
 errno=priorErrno;
}
static int nfs_menu_wide_current(void){return nfs_menu_wide.generation&&nfs_menu_wide.context&&g_glCtx&&(uintptr_t)g_glCtx==nfs_menu_wide.context&&SDL_GL_GetCurrentContext()==g_glCtx&&SDL_IsMainThread()&&nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled&&nfs_logo_provenance.context==nfs_menu_wide.generation;}
#define NFS_MENU_WIDE_GL(call) do{if(!nfs_menu_wide_current())goto lost;call;if(!nfs_menu_wide_current())goto lost;}while(0)
/* A pending frame may be canceled while the PREVIOUS successfully presented
   layout remains active. Actual fallback presentation or source owner change
   revokes that layout; preparation itself must not reset a trackpad each frame. */
static void nfs_menu_wide_cancel(void){nfs_menu_wide.phase=0;nfs_menu_wide.serial=0;nfs_menu_wide.tailTriangles=0;nfs_menu_wide.cursorSeen=0;}
static void nfs_menu_wide_cancel_at(unsigned site){if(nfs_menu_wide.phase||nfs_menu_wide.serial)nfs_menu_wide_diag(5,site,-1,0,0,0,0);nfs_menu_wide_cancel();}
static void nfs_menu_wide_abandon(void){nfs_menu_original_forget();nfs_menu_wide_cancel_at(220);nfs_menu_wide.color=nfs_menu_wide.fbo=0;nfs_menu_wide.width=nfs_menu_wide.height=0;nfs_menu_wide.context=0;nfs_menu_wide.generation=0;nfs_menu_responsive_revoke();}
static void nfs_menu_wide_retire(int owner){
 if(owner&&nfs_menu_wide_current()){
  if(nfs_menu_wide.fbo){NFS_MENU_WIDE_GL(glDeleteFramebuffers(1,&nfs_menu_wide.fbo));}
  if(nfs_menu_wide.color){NFS_MENU_WIDE_GL(glDeleteTextures(1,&nfs_menu_wide.color));}
 }
 lost:nfs_menu_wide_abandon();
}
static int nfs_menu_wide_target(unsigned width,unsigned height){
 NfsBgSaved saved;GLuint color=0,fbo=0;GLenum status=0;int ok=0;
 if(!nfs_menu_wide_current()||!nfs_frame_dimensions(width,height))return 0;
 nfs_bg_save(&saved);if(!nfs_menu_wide_current())goto lost;
 if(nfs_menu_wide.color&&nfs_menu_wide.fbo&&nfs_menu_wide.width==width&&nfs_menu_wide.height==height){
  NFS_MENU_WIDE_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_menu_wide.fbo));
  NFS_MENU_WIDE_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));ok=status==GL_FRAMEBUFFER_COMPLETE;
  nfs_bg_restore(&saved);if(!nfs_menu_wide_current())goto lost;return ok;
 }
 NFS_MENU_WIDE_GL(glActiveTexture(GL_TEXTURE0));NFS_MENU_WIDE_GL(glGenTextures(1,&color));if(!color)goto done;
 NFS_MENU_WIDE_GL(glBindTexture(GL_TEXTURE_2D,color));NFS_MENU_WIDE_GL(nfs_frame_texture_params());
 NFS_MENU_WIDE_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL));
 NFS_MENU_WIDE_GL(glGenFramebuffers(1,&fbo));if(!fbo)goto done;
 NFS_MENU_WIDE_GL(glBindFramebuffer(GL_FRAMEBUFFER,fbo));NFS_MENU_WIDE_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,color,0));
 NFS_MENU_WIDE_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));ok=status==GL_FRAMEBUFFER_COMPLETE&&glGetError()==GL_NO_ERROR;
 done:if(!ok){if(fbo){NFS_MENU_WIDE_GL(glDeleteFramebuffers(1,&fbo));}if(color){NFS_MENU_WIDE_GL(glDeleteTextures(1,&color));}}
 nfs_bg_restore(&saved);if(!nfs_menu_wide_current())goto lost;if(!ok)return 0;
 if(nfs_menu_wide.fbo){NFS_MENU_WIDE_GL(glDeleteFramebuffers(1,&nfs_menu_wide.fbo));}if(nfs_menu_wide.color){NFS_MENU_WIDE_GL(glDeleteTextures(1,&nfs_menu_wide.color));}
 nfs_menu_wide.fbo=fbo;nfs_menu_wide.color=color;nfs_menu_wide.width=width;nfs_menu_wide.height=height;return 1;
 lost:nfs_menu_wide_abandon();return 0;
}
static void nfs_menu_wide_prepare_owner(const NfsBackgroundSceneFrameView*v,const NfsNativeLogoTargetProof*target,const NfsMenuCachedSnapshot*cached,int(*captionDraw)(const float*,void*),void*captionUser){
 NfsBgSaved saved;uint64_t epoch=0;float cursorX=0,cursorY=0;int dw=0,dh=0,ok=0;GLint type=0,name=0;NfsLogoVertex logos[18];size_t count=0;float bounds[4],mapMatrix[16],clearColor[4];static const int32_t full[4]={0,0,640,480};
 nfs_menu_wide_cancel();memset(&nfs_menu_wide.cached,0,sizeof nfs_menu_wide.cached);nfs_menu_wide.ownerForm=0x401910u;
 if(cached&&cached->schema){
  if((cached->formTag!=NFS_MENU_CACHED_SETUP&&cached->formTag!=NFS_MENU_CACHED_OPTIONS)||!v||cached->packetSerial!=v->serial||cached->contentRevision!=v->contentRevision||v->tailKind!=(cached->formTag==NFS_MENU_CACHED_SETUP?4u:3u)||!captionDraw)return;
  nfs_menu_wide.cached=*cached;nfs_menu_wide.ownerForm=cached->formTag;
 }
 /* Source Main form, source scene and native target must all agree. No generic
    menu/form enum alone authorizes a responsive image. */
 if(!v||!target){nfs_menu_wide_diag(1,1,-1,0,0,0,0);return;}
 if(!(nfs_menu_wide.cached.schema?nfs_bg_view_common(v):nfs_bg_view(v))){nfs_menu_wide_diag(1,2,-1,0,0,0,0);return;}
 if(!nfs_menu_wide_source(&epoch,&cursorX,&cursorY)){nfs_menu_wide_diag(1,3,-1,0,0,0,0);return;}
 if(!sdlWin){nfs_menu_wide_diag(1,4,-1,0,0,0,0);return;}
 if(!nfs_bg_ready(&nfs_bg_compositor)){nfs_menu_wide_diag(1,5,-1,0,0,0,0);return;}
 if(!SDL_GetWindowSizeInPixels(sdlWin,&dw,&dh)||dw<=0||dh<=0){nfs_menu_wide_diag(1,6,-1,0,0,0,0);return;}
 const float width=480.f*(float)dw/(float)dh;
 if(width<=640.01f||width>854.f){nfs_menu_wide_diag(1,7,-1,width,854,0,0);return;} /* exact accepted 4:3 path remains unchanged */
 if(nfs_menu_wide.context&&((uintptr_t)g_glCtx!=nfs_menu_wide.context||nfs_menu_wide.generation!=nfs_logo_provenance.context))nfs_menu_wide_abandon();
 nfs_menu_wide.context=(uintptr_t)g_glCtx;nfs_menu_wide.generation=nfs_logo_provenance.context;
 if(!nfs_menu_wide_current()){nfs_menu_wide_diag(1,8,-1,0,0,0,0);return;}
 if(!nfs_menu_wide_target((unsigned)dw,(unsigned)dh)){nfs_menu_wide_diag(1,9,-1,0,0,0,0);return;}
 nfs_menu_wide.logicalWidth=width;nfs_menu_wide.delta=width-640;
 memset(nfs_menu_wide.matrix,0,sizeof nfs_menu_wide.matrix);nfs_menu_wide.matrix[0]=2.f/width;nfs_menu_wide.matrix[5]=-2.f/480;nfs_menu_wide.matrix[10]=-2;nfs_menu_wide.matrix[12]=-1;nfs_menu_wide.matrix[13]=1;nfs_menu_wide.matrix[14]=1;nfs_menu_wide.matrix[15]=1;
 memcpy(mapMatrix,nfs_menu_wide.matrix,sizeof mapMatrix);mapMatrix[12]+=nfs_menu_wide.delta*mapMatrix[0];
 if(!nfs_logo_layers(v->logos,3,logos,&count,bounds)){nfs_menu_wide_diag(1,10,-1,0,0,0,0);return;}
 nfs_bg_save(&saved);if(!nfs_menu_wide_current())goto lost;
 NFS_MENU_WIDE_GL(glGetFloatv(GL_COLOR_CLEAR_VALUE,clearColor));
 NFS_MENU_WIDE_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_menu_wide.fbo));NFS_MENU_WIDE_GL(glViewport(0,0,dw,dh));NFS_MENU_WIDE_GL(nfs_frame_draw_state());NFS_MENU_WIDE_GL(glDepthMask(GL_FALSE));
 /* Opaque dark negative space behind the left cutout sheets. Never present
    uninitialized attachment pixels through their transparent teeth/edges. */
 NFS_MENU_WIDE_GL(glClearColor(0,0,0,1));NFS_MENU_WIDE_GL(glClear(GL_COLOR_BUFFER_BIT));
 /* Full-height semantic layers: map right anchored, original gold sheet,
    original animated words and original clipped rotating logo left anchored. */
 NFS_MENU_WIDE_GL(glUseProgram(nfs_bg_compositor.program));NFS_MENU_WIDE_GL(glUniformMatrix4fv(nfs_bg_compositor.matrix,1,GL_FALSE,mapMatrix));
 if(!nfs_bg_quad(&nfs_bg_compositor,v->mapRole,0,0,640,480,0,0,nfs_bg_compositor.art[v->mapRole].width,nfs_bg_compositor.art[v->mapRole].height,full,256,0))goto done;
 NFS_MENU_WIDE_GL(glUniformMatrix4fv(nfs_bg_compositor.matrix,1,GL_FALSE,nfs_menu_wide.matrix));
 if(!nfs_bg_quad(&nfs_bg_compositor,2,0,0,343,480,0,0,1372,1920,v->panelClip,256,1))goto done;
 for(unsigned i=0;i<v->wordCount;++i){const NfsBackgroundSceneWordLayer*w=v->words+i;const NfsBackgroundSceneWord*r=nfs_bg_compositor.regions+w->sourceRole-1;
  if(!nfs_bg_quad(&nfs_bg_compositor,r->textureRole,(float)w->left,(float)w->top,(float)r->logicalWidth,(float)r->logicalHeight,r->x,r->y,r->width,r->height,w->clip,w->scalar256,1))goto done;
 }
 if(count){NFS_MENU_WIDE_GL(glUseProgram(nfs_bg_compositor.logo.program));NFS_MENU_WIDE_GL(glUniformMatrix4fv(nfs_bg_compositor.logo.matrix,1,GL_FALSE,nfs_menu_wide.matrix));NFS_MENU_WIDE_GL(glUniform1i(nfs_bg_compositor.logo.sampler,0));NFS_MENU_WIDE_GL(glBindTexture(GL_TEXTURE_2D,nfs_bg_compositor.logo.texture));NFS_MENU_WIDE_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),logos));NFS_MENU_WIDE_GL(glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),&logos[0].coefficient));NFS_MENU_WIDE_GL(glEnableVertexAttribArray(0));NFS_MENU_WIDE_GL(glEnableVertexAttribArray(1));NFS_MENU_WIDE_GL(glDisableVertexAttribArray(2));NFS_MENU_WIDE_GL(glDisableVertexAttribArray(3));NFS_MENU_WIDE_GL(glEnable(GL_BLEND));NFS_MENU_WIDE_GL(glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA));NFS_MENU_WIDE_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count));}
 if(nfs_menu_wide.cached.schema&&!captionDraw(nfs_menu_wide.matrix,captionUser))goto done;
 ok=glGetError()==GL_NO_ERROR;
 NFS_MENU_WIDE_GL(glBindFramebuffer(GL_FRAMEBUFFER,target->framebuffer));NFS_MENU_WIDE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_MENU_WIDE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||(GLuint)name!=g_framebufferTexture)ok=0;
 done:if(!nfs_menu_wide_current())goto lost;glClearColor(clearColor[0],clearColor[1],clearColor[2],clearColor[3]);nfs_bg_restore(&saved);if(!nfs_menu_wide_current())goto lost;if(!ok){nfs_menu_wide_diag(1,11,-1,0,0,0,0);return;}
 nfs_menu_wide.ownerEpoch=epoch;nfs_menu_wide.serial=v->serial;nfs_menu_wide.nativeFbo=target->framebuffer;nfs_menu_wide.nativeColor=g_framebufferTexture;nfs_menu_wide.nativeDepth=g_framebufferDepth;nfs_menu_wide.nativeHeight=target->height;nfs_menu_wide.phase=NFS_MENU_WIDE_PREPARED;nfs_menu_wide_diag(1,0,-1,0,0,0,0);return;
 lost:nfs_menu_wide_abandon();
}
static void nfs_menu_wide_prepare(const NfsBackgroundSceneFrameView*v,const NfsNativeLogoTargetProof*t){nfs_menu_wide_prepare_owner(v,t,NULL,NULL,NULL);}
static void nfs_menu_wide_prepare_cached(const NfsBackgroundSceneFrameView*v,const NfsNativeLogoTargetProof*t,const NfsMenuCachedSnapshot*p,int(*caption)(const float*,void*),void*user){
 if(!p||!p->schema){nfs_menu_wide_cancel();return;}nfs_menu_wide_prepare_owner(v,t,p,caption,user);
}
static void nfs_menu_wide_native_done(uint64_t serial,int accepted){
 if(!accepted){nfs_menu_wide_diag(2,1,-1,0,0,0,0);nfs_menu_wide_cancel_at(205);return;}
 if(serial!=nfs_menu_wide.serial){nfs_menu_wide_diag(2,2,-1,0,0,0,0);nfs_menu_wide_cancel_at(205);return;}
 if(nfs_menu_wide.phase!=NFS_MENU_WIDE_PREPARED){nfs_menu_wide_diag(2,3,-1,0,0,0,0);nfs_menu_wide_cancel_at(205);return;}
 nfs_menu_wide.phase=NFS_MENU_WIDE_TAIL;nfs_menu_wide_diag(2,0,-1,0,0,0,0);
}
/* Called immediately after original glDrawArrays, BEFORE native-mark uniforms
   are restored. Replays the actual approved foreground, retaining its shader,
   font/cursor texture, tint, UV, alpha, animation position and draw order. */
static void nfs_menu_wide_tail(unsigned count){
 NfsBgSaved saved;Vertices moved[MaxTriangles];float shifts[MaxTriangles];uint64_t epoch=0;float sx=0,sy=0;GLint fbo=0,program=0,clip[4],nativeViewport[4];GLfloat nativeMatrix[16];int ok=0,cursor=0;NfsMenuResponsivePointer pointer;NfsMenuResponsiveLayout shown;
 if(nfs_menu_wide.phase!=NFS_MENU_WIDE_TAIL){nfs_menu_wide_diag(3,1,-1,0,0,0,0);return;}
 if(!nfs_menu_wide_current()){nfs_menu_wide_diag(3,2,-1,0,0,0,0);goto refused;}
 if(!count||count!=g_trianglesCount||count>MaxTriangles){nfs_menu_wide_diag(3,3,-1,(float)count,(float)g_trianglesCount,0,0);goto refused;}
 if(g_fogEnabled||g_textureEnabled!=1){nfs_menu_wide_diag(3,4,-1,g_textureEnabled,1,0,0);goto refused;}
 if(!nfs_menu_wide_source(&epoch,&sx,&sy)||epoch!=nfs_menu_wide.ownerEpoch){nfs_menu_wide_diag(3,5,-1,0,0,0,0);goto refused;}
 const int selected=nfs_native_marks.selected;
 const int mark=selected>=0&&selected<NFS_NATIVE_MARK_MAX_BINDINGS&&nfs_native_marks.bindings[selected].valid&&nfs_native_marks.bindings[selected].generation==nfs_native_marks.generation&&nfs_mark_owner()&&nfs_native_marks.program==g_shaderProgram;
 const int originalBranding=!mark&&nfs_menu_original_branding(epoch);
 const int slot5Canvas=(mark||originalBranding)&&nfs_menu_wide_slot5_square(count,epoch,originalBranding,selected);
 if(originalBranding&&!slot5Canvas){nfs_menu_wide_diag(3,15,5,(float)count,2,0,0);goto refused;}
 if(!mark&&!originalBranding&&nfs_front_font_source_page(nfs_hd_selected)<0){nfs_menu_wide_diag(3,6,-1,0,0,0,0);goto refused;}
 if(originalBranding)nfs_menu_wide_diag(3,14,5,(float)nfs_menu_original_selection.address,(float)nfs_menu_original_selection.id,0,0);
 cursor=mark&&nfs_native_marks.bindings[selected].encoding==NFS_NATIVE_MARK_SOURCE_RGBA4444_LE;
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_CURRENT_PROGRAM,&program);
 if((GLuint)fbo!=nfs_menu_wide.nativeFbo||(GLuint)program!=g_shaderProgram||g_framebufferTexture!=nfs_menu_wide.nativeColor||g_framebufferDepth!=nfs_menu_wide.nativeDepth||g_framebufferHeight!=(int)nfs_menu_wide.nativeHeight){nfs_menu_wide_diag(3,7,-1,0,0,0,0);goto refused;}
 if(glIsEnabled(GL_STENCIL_TEST)||glIsEnabled(GL_CULL_FACE)||glIsEnabled(GL_POLYGON_OFFSET_FILL)){nfs_menu_wide_diag(3,8,-1,0,0,0,0);goto refused;}
 glGetIntegerv(GL_VIEWPORT,nativeViewport);
 if(nativeViewport[2]<=0||nativeViewport[3]<=0){nfs_menu_wide_diag(3,9,-1,0,0,0,0);goto refused;}
 {const float scale=nfs_menu_wide.nativeHeight/480.f;float expected[16]={0};
  expected[0]=2.f*scale/nativeViewport[2];expected[5]=-2.f*scale/nativeViewport[3];expected[10]=-2;
  expected[12]=-1.f-2.f*nativeViewport[0]/nativeViewport[2];expected[13]=1.f+2.f*(nfs_menu_wide.nativeHeight-nativeViewport[1]-nativeViewport[3])/nativeViewport[3];expected[14]=expected[15]=1;
  for(unsigned k=0;k<16;++k)if(!isfinite(g_matrix[k])||fabsf(g_matrix[k]-expected[k])>0.00001f){nfs_menu_wide_diag(3,10,(int)k,g_matrix[k],expected[k],0,0);goto refused;}
 }

 memcpy(moved,g_vertices,count*sizeof(Vertices));
 for(unsigned i=0;i<count;++i){float left=1e9f,right=-1e9f;for(unsigned j=0;j<3;++j){float x=moved[i].vertex[j].x,y=moved[i].vertex[j].y;if(!isfinite(x)||!isfinite(y)||moved[i].vertex[j].z!=1||g_textureCoord[i].vertex[j].q!=1||(!slot5Canvas&&(x<-32||x>672||y<-32||y>512))){nfs_menu_wide_diag(3,11,(int)(3*i+j),x,y,moved[i].vertex[j].z,g_textureCoord[i].vertex[j].q);goto refused;}left=fminf(left,x);right=fmaxf(right,x);}
  float shift=0,dy=0;if(cursor){if(nfs_menu_wide.cached.schema)shift=nfs_menu_wide.delta*.5f;if(nfs_menu_responsive_snapshot(&shown)&&shown.drawableWidth==nfs_menu_wide.width&&shown.drawableHeight==nfs_menu_wide.height&&nfs_menu_responsive_pointer_snapshot(&pointer)&&pointer.revision==shown.revision&&pointer.ownerEpoch==epoch){shift=pointer.x-sx;dy=pointer.y-sy;}}
  /* Main's source-recognized128-square car/track marks are the right
     branding group even when transparent native canvas extends left of its
     visible ink. Never split/crop that canvas using an ink guess. */
  else if(nfs_menu_wide.cached.schema&&!mark&&!originalBranding)shift=nfs_menu_wide.delta*.5f;
  else if(mark||originalBranding||left>=343)shift=nfs_menu_wide.delta;else if(right>343){nfs_menu_wide_diag(3,12,(int)i,left,right,0,0);goto refused;}
  shifts[i]=shift;for(unsigned j=0;j<3;++j){moved[i].vertex[j].x+=shift;moved[i].vertex[j].y+=dy;}
 }
 nfs_bg_save(&saved);if(!nfs_menu_wide_current())goto lost;memcpy(nativeMatrix,g_matrix,sizeof nativeMatrix);
 NFS_MENU_WIDE_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_menu_wide.fbo));NFS_MENU_WIDE_GL(glViewport(0,0,nfs_menu_wide.width,nfs_menu_wide.height));NFS_MENU_WIDE_GL(glDepthMask(GL_FALSE));NFS_MENU_WIDE_GL(glDisable(GL_DEPTH_TEST));
 /* Derive original current clip from actual GL scissor; apply the same anchor
    as the batch, never freeze animation/clip to final-layout coordinates. */
 NFS_MENU_WIDE_GL(glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nfs_menu_wide.matrix));NFS_MENU_WIDE_GL(glVertexAttribPointer(g_aPositionLoc,3,GL_FLOAT,GL_FALSE,0,moved));
 if(saved.common.caps[4]&&!cursor){
  glGetIntegerv(GL_SCISSOR_BOX,clip);const float scale=nfs_menu_wide.nativeHeight/480.f;
  const float l=clip[0]/scale,r=(clip[0]+clip[2])/scale,t=480-(clip[1]+clip[3])/scale,b=480-clip[1]/scale,outputScale=nfs_menu_wide.height/480.f;
  /* One source font batch can contain both anchors. Preserve its order and
     source clip separately for each contiguous semantic group. */
  for(unsigned first=0;first<count;){unsigned end=first+1;while(end<count&&shifts[end]==shifts[first])++end;
   GLint x=(GLint)floorf((l+shifts[first])*outputScale),y=(GLint)floorf((480-b)*outputScale),right=(GLint)ceilf((r+shifts[first])*outputScale),top=(GLint)ceilf((480-t)*outputScale);
   NFS_MENU_WIDE_GL(glScissor(x,y,right-x,top-y));NFS_MENU_WIDE_GL(glDrawArrays(GL_TRIANGLES,(GLint)first*3,(GLsizei)(end-first)*3));first=end;
  }
 }else{NFS_MENU_WIDE_GL(glDisable(GL_SCISSOR_TEST));NFS_MENU_WIDE_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count*3));}
 ok=glGetError()==GL_NO_ERROR;
 done:if(!nfs_menu_wide_current())goto lost;glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nativeMatrix);nfs_bg_restore(&saved);if(!nfs_menu_wide_current())goto lost;if(!ok){nfs_menu_wide_diag(3,13,-1,0,0,0,0);goto refused;}nfs_menu_wide.tailTriangles+=count;if(cursor)nfs_menu_wide.cursorSeen=1;nfs_menu_wide_diag(3,0,-1,0,0,0,0);return;
 refused:nfs_menu_wide_cancel_at(240);return;
 lost:nfs_menu_wide_abandon();
}
static GLuint nfs_menu_wide_present(GLuint frontFbo,GLuint frontTexture,int width,int height){
 uint64_t epoch=0;float x=0,y=0;GLint prior=0,type=0,name=0,depthType=0,depthName=0;GLenum status=0;int ok=0;
 if(nfs_menu_wide.phase!=NFS_MENU_WIDE_TAIL){nfs_menu_wide_diag(4,1,-1,0,0,0,0);goto refused;}
 if(!nfs_menu_wide_current()){nfs_menu_wide_diag(4,2,-1,0,0,0,0);goto refused;}
 if(!nfs_menu_wide.tailTriangles){nfs_menu_wide_diag(4,3,-1,0,0,0,0);goto refused;}
 if(!nfs_menu_wide.cursorSeen){nfs_menu_wide_diag(4,4,-1,0,0,0,0);goto refused;}
 if(width!=(int)nfs_menu_wide.width||height!=(int)nfs_menu_wide.height){nfs_menu_wide_diag(4,5,-1,(float)width,(float)height,0,0);goto refused;}
 if(frontFbo!=nfs_menu_wide.nativeFbo||frontTexture!=nfs_menu_wide.nativeColor){nfs_menu_wide_diag(4,6,-1,(float)frontFbo,(float)frontTexture,0,0);goto refused;}
 if(!nfs_menu_wide_source(&epoch,&x,&y)||epoch!=nfs_menu_wide.ownerEpoch){nfs_menu_wide_diag(4,7,-1,0,0,0,0);goto refused;}
 if(nfs_menu_wide.revision==UINT64_MAX){nfs_menu_wide_diag(4,8,-1,0,0,0,0);goto refused;}
 NFS_MENU_WIDE_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prior));NFS_MENU_WIDE_GL(glBindFramebuffer(GL_FRAMEBUFFER,frontFbo));NFS_MENU_WIDE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_MENU_WIDE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));NFS_MENU_WIDE_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));ok=type==GL_TEXTURE&&(GLuint)name==frontTexture&&status==GL_FRAMEBUFFER_COMPLETE;NFS_MENU_WIDE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&depthType));NFS_MENU_WIDE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&depthName));ok=ok&&depthType==GL_RENDERBUFFER&&(GLuint)depthName==nfs_menu_wide.nativeDepth;
 NFS_MENU_WIDE_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_menu_wide.fbo));NFS_MENU_WIDE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_MENU_WIDE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));NFS_MENU_WIDE_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));ok=ok&&type==GL_TEXTURE&&(GLuint)name==nfs_menu_wide.color&&status==GL_FRAMEBUFFER_COMPLETE;NFS_MENU_WIDE_GL(glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)prior));if(!ok){nfs_menu_wide_diag(4,9,-1,0,0,0,0);goto refused;}
 memset(&nfs_menu_wide.selected,0,sizeof nfs_menu_wide.selected);nfs_menu_wide.selected.schema=1;nfs_menu_wide.selected.active=1;nfs_menu_wide.selected.revision=++nfs_menu_wide.revision;nfs_menu_wide.selected.ownerEpoch=epoch;nfs_menu_wide.selected.drawableWidth=width;nfs_menu_wide.selected.drawableHeight=height;nfs_menu_wide.selected.logicalWidth=nfs_menu_wide.logicalWidth;nfs_menu_wide.selected.delta=nfs_menu_wide.delta;nfs_menu_wide.selected.sourceCursorX=x;nfs_menu_wide.selected.sourceCursorY=y;nfs_menu_wide.selected.cursorValid=1;nfs_menu_wide.phase=NFS_MENU_WIDE_SELECTED;nfs_menu_wide_diag(4,0,-1,0,0,0,0);return nfs_menu_wide.color;
 refused:nfs_menu_wide_cancel_at(230);nfs_menu_responsive_revoke();return 0;
 lost:nfs_menu_wide_abandon();return 0;
}
static void nfs_menu_wide_after_present(int success){
 if(success&&nfs_menu_wide.phase==NFS_MENU_WIDE_SELECTED&&nfs_menu_wide_current()){
  if(nfs_menu_wide.cached.schema)nfs_menu_cached_publish(&nfs_menu_wide.selected,&nfs_menu_wide.cached);
  else nfs_menu_responsive_publish(&nfs_menu_wide.selected);
  NfsMenuResponsiveLayout actual;
  /* Two actual-present witnesses for each new output/owner tuple. Bounded
     memory and64total lines; initial idle frames cannot exhaust later ratios. */
  static struct {unsigned width,height,count;uint64_t epoch;} traces[32];
  static unsigned traceUsed,traceTotal;unsigned traceSlot=32;
  if(nfs_menu_responsive_snapshot(&actual)&&actual.revision==nfs_menu_wide.selected.revision&&actual.ownerEpoch==nfs_menu_wide.ownerEpoch&&nfs_logo_trace_enabled()&&traceTotal<64){
   for(unsigned i=0;i<traceUsed;++i)if(traces[i].width==nfs_menu_wide.width&&traces[i].height==nfs_menu_wide.height&&traces[i].epoch==actual.ownerEpoch){traceSlot=i;break;}
   if(traceSlot==32&&traceUsed<32){traceSlot=traceUsed++;traces[traceSlot].width=nfs_menu_wide.width;traces[traceSlot].height=nfs_menu_wide.height;traces[traceSlot].epoch=actual.ownerEpoch;}
   if(traceSlot<32&&traces[traceSlot].count<2){++traces[traceSlot].count;++traceTotal;if(nfs_menu_wide.cached.schema){fprintf(stderr,"CACHED MENU RESPONSIVE presented owner=%08x epoch=%llu serial=%llu revision=%llu output=%ux%u logical=%.3f tail=%u cursor=%u\n",nfs_menu_wide.ownerForm,(unsigned long long)nfs_menu_wide.ownerEpoch,(unsigned long long)nfs_menu_wide.serial,(unsigned long long)nfs_menu_wide.selected.revision,nfs_menu_wide.width,nfs_menu_wide.height,nfs_menu_wide.logicalWidth,nfs_menu_wide.tailTriangles,nfs_menu_wide.cursorSeen);
    const NfsMenuCachedSnapshot*p=&nfs_menu_wide.cached;
    fprintf(stderr,"CACHED MENU WIDGET SNAPSHOT owner=%08x descriptor=%08x producer=%llu source=%llu widgets=%llu serial=%llu content=%llu count=%u\n",p->formTag,p->descriptor,(unsigned long long)p->producerEpoch,(unsigned long long)p->sourceEpoch,(unsigned long long)p->widgetRevision,(unsigned long long)p->packetSerial,(unsigned long long)p->contentRevision,p->count);
    for(unsigned wi=0;wi<p->count&&wi<NFS_MENU_CACHED_MAX_WIDGETS;++wi){const NfsMenuCachedWidget*w=p->widgets+wi;
     fprintf(stderr,"CACHED MENU WIDGET ROW owner=%08x source=%llu widgets=%llu serial=%llu index=%u item=%u items=%u type=%u flags=%u enabled=%u rect=%d,%d,%d,%d\n",p->formTag,(unsigned long long)p->sourceEpoch,(unsigned long long)p->widgetRevision,(unsigned long long)p->packetSerial,wi,w->itemId,w->itemCount,w->itemType,w->itemFlags,w->enabled,w->left,w->top,w->right,w->bottom);
    }
   }else fprintf(stderr,"MAIN RESPONSIVE presented epoch=%llu serial=%llu revision=%llu output=%ux%u logical=%.3f tail=%u cursor=%u\n",(unsigned long long)nfs_menu_wide.ownerEpoch,(unsigned long long)nfs_menu_wide.serial,(unsigned long long)nfs_menu_wide.selected.revision,nfs_menu_wide.width,nfs_menu_wide.height,nfs_menu_wide.logicalWidth,nfs_menu_wide.tailTriangles,nfs_menu_wide.cursorSeen);}
  }
 }else nfs_menu_responsive_revoke();nfs_menu_wide_cancel_at(210);
}
#undef NFS_MENU_WIDE_GL

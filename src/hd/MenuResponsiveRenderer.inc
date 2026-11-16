// SPDX-License-Identifier: MIT
/* Main-only responsive presentation. The original game color/depth surfaces,
   original sampling proof and native tile pass are never resized or overwritten. */
#include "MenuResponsiveLayout.h"
#include "MenuResponsiveDebug.h"
#include "MenuResponsiveCached.h"
#include <errno.h>
#include <math.h>

/* Two independently owned semantic outputs. Native surfaces are correlation
 * targets only: no native/front pixels are read or copied into these outputs. */
typedef struct NfsMenuIdleResources {
 uint64_t generation,ownerGeneration,logoGeneration,captionGeneration,captionIdentity,photoGeneration,classGeneration,locationGeneration,logoIdentity;
 GLuint program,copyProgram,textures[5],logoTexture,logoProgram,captionTexture,captionProgram,wideTextures[2];uint64_t identities[5];unsigned char artSha256[5][32],captionSha256[32],logoSha256[32];unsigned ratio;
} NfsMenuIdleResources;
typedef struct NfsMenuIdleSlot {
 GLuint fbo,color,nativeFbo,nativeColor,nativeDepth;unsigned width,height,nativeHeight,valid;
 uintptr_t context,mainOwner,mainGame;uint64_t generation,paint,sealedPaint,upload,epoch,locationArtGeneration,mainLife,mainEpoch,mainUpload;
 NfsMenuCachedSnapshot cached;NfsMenuResponsiveLayout layout;NfsMenuIdleResources resources;float x,y;
} NfsMenuIdleSlot;
static NfsMenuIdleSlot nfs_menu_idle_slots[2];static unsigned nfs_menu_idle_slot=2,nfs_menu_idle_selected=2,nfs_menu_idle_next,nfs_menu_idle_mask;static uint64_t nfs_menu_idle_upload_epoch=1;
static void nfs_menu_idle_revoke(unsigned);static void nfs_menu_idle_paint(void);static void nfs_menu_idle_upload(void);
static GLuint nfs_menu_idle_present(GLuint,GLuint,int,int);static void nfs_menu_idle_after_present(int);static void nfs_menu_idle_seal(void);
extern uint64_t nfs_class_picture_art_generation(void);
static struct {
 uintptr_t context; uint64_t generation,ownerEpoch,serial,revision,locationArtGeneration;
 GLuint color,fbo,nativeFbo,nativeColor,nativeDepth;
 GLuint artwork[2]; unsigned artworkRatio;
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
/* Original4276F0 selected slot5 pulses a SIGNED integer half-width about
 *574; the quarter-sine table/1024 yields[-64,64], includingzero/flippedwinding.
 * The original128-high canvas/UV/order remains exact and is viewport-clipped. */
static int nfs_menu_wide_opponent_slot5_pulse(unsigned count){
 static const unsigned corners[6]={0,1,2,0,2,3};static const float us[4]={0,255.f/256.f,255.f/256.f,0},vs[4]={0,0,255.f/256.f,255.f/256.f};
 if(count!=2)return 0;const float left=g_vertices[0].vertex[0].x,half=574.f-left;
 if(!isfinite(left)||!isfinite(half)||half< -64.f||half>64.f||half!=(float)(int)half)return 0;const float right=574.f+half;
 for(unsigned k=0;k<6;++k){const unsigned c=corners[k],i=k/3,j=k%3;if(g_vertices[i].vertex[j].x!=((c==1||c==2)?right:left)||g_vertices[i].vertex[j].y!=(c>=2?534.f:406.f)||g_vertices[i].vertex[j].z!=1||g_textureCoord[i].vertex[j].s!=us[c]||g_textureCoord[i].vertex[j].t!=vs[c]||g_textureCoord[i].vertex[j].r!=0||g_textureCoord[i].vertex[j].q!=1)return 0;}return 1;
}
/* Same original4276F0 signed pulse, independently admitted ONLY by the
   existing three-digest Setup mode upload/select/current-owner ledger. */
static int nfs_menu_wide_setup_mode_pulse(unsigned count){
 static const unsigned corners[6]={0,1,2,0,2,3};static const float us[4]={0,255.f/256.f,255.f/256.f,0},vs[4]={0,0,255.f/256.f,255.f/256.f};
 if(count!=2)return 0;const float left=g_vertices[0].vertex[0].x,half=574.f-left;
 if(!isfinite(left)||!isfinite(half)||half< -64.f||half>64.f||half!=(float)(int)half)return 0;const float right=574.f+half;
 for(unsigned k=0;k<6;++k){const unsigned c=corners[k],i=k/3,j=k%3;if(g_vertices[i].vertex[j].x!=((c==1||c==2)?right:left)||g_vertices[i].vertex[j].y!=(c>=2?148.f:20.f)||g_vertices[i].vertex[j].z!=1||g_textureCoord[i].vertex[j].s!=us[c]||g_textureCoord[i].vertex[j].t!=vs[c]||g_textureCoord[i].vertex[j].r!=0||g_textureCoord[i].vertex[j].q!=1)return 0;}return 1;
}
static int nfs_menu_wide_slot5_square_for(unsigned count,uint64_t epoch,uint32_t ownerForm,int originalBranding,int selected){
 NfsMenuResponsiveDebugState source;memset(&source,0,sizeof source);nfs_menu_responsive_debug_state(&source);
 if(source.schema!=3||source.sourceForm!=ownerForm||!source.cursorValid||source.sourceEpoch!=epoch||source.originalOpponentValid!=1)return 0;
 if(!originalBranding){
  /* A future authored A/B/C mark retains the SAME original source canvas.
     Existing authored digest/lifetime admission is still required by caller. */
  if(selected<0||selected>=NFS_NATIVE_MARK_MAX_BINDINGS)return 0;
  const NfsNativeMarkBinding*b=nfs_native_marks.bindings+selected;
  if(b->edge!=128||b->encoding!=NFS_NATIVE_MARK_SOURCE_RGBA5551_LE||b->address!=source.originalOpponentAddress)return 0;
  /* Shared4276F0 slot5 pulses on Main after returning from Opponents too.
     Keep the exact source epoch/address and authenticated mark checks below;
     this grants no extra geometry to other owners or unrecognized textures. */
  if(ownerForm==0x401910u||ownerForm==NFS_MENU_CACHED_OPPONENT){
   if(!b->valid||b->generation!=nfs_native_marks.generation||!b->revision||b->preparedBytes!=32768||b->texture>=NFS_NATIVE_MARK_MAX_TEXTURES||!nfs_mark_owner()||(b->address&3)||b->address>TextureMem-32768)return 0;
   const TextureInfo*ti=g_textures+(b->address>>2);const NfsNativeMarkTexture*t=nfs_native_marks.textures+b->texture;
   if(!b->nativeId||ti->id!=b->nativeId||ti->size!=128||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->hdDigits||ti->data!=g_textureMem+b->address||!t->texture||t->generation!=nfs_native_marks.generation)return 0;
   return nfs_menu_wide_opponent_slot5_pulse(count);
  }
 }
 return nfs_menu_wide_square128(count,510,406);
}
static int nfs_menu_wide_slot5_square(unsigned count,uint64_t epoch,int branding,int selected){return nfs_menu_wide_slot5_square_for(count,epoch,nfs_menu_wide.ownerForm,branding,selected);}
static int nfs_menu_wide_source(uint64_t*epoch,float*x,float*y){
 if(nfs_menu_wide.cached.schema)return nfs_menu_cached_current(&nfs_menu_wide.cached,epoch,x,y);
 return nfs_menu_responsive_source_main(epoch,x,y);
}
enum {NFS_MENU_WIDE_IDLE=0,NFS_MENU_WIDE_PREPARED=1,NFS_MENU_WIDE_TAIL=2,NFS_MENU_WIDE_SELECTED=3};
/* Stage1 prepare,2 native-finish,3 tail,4 select,5 cancel. First two
   observations per(stage,reason,drawable,sourceEpoch);128keys/256lines max. */
static void nfs_menu_wide_diag(unsigned stage,unsigned reason,int detail,float actual,float expected,float z,float q){
 static struct {unsigned stage,reason,w,h,count;uint64_t epoch,markIdentity;} seen[128];
 static unsigned used,total;int priorErrno=errno,w=0,h=0;unsigned slot=128;
 NfsMenuResponsiveDebugState source;GLint fbo=-1,program=-1,texture=-1;unsigned encoding=0;
 if(!nfs_logo_trace_enabled()||total>=256){errno=priorErrno;return;}
 memset(&source,0,sizeof source);nfs_menu_responsive_debug_state(&source);
 uint64_t markIdentity=0;if(nfs_native_marks.selected>=0&&nfs_native_marks.selected<NFS_NATIVE_MARK_MAX_BINDINGS){const NfsNativeMarkBinding*b=nfs_native_marks.bindings+nfs_native_marks.selected;if(b->texture<NFS_NATIVE_MARK_MAX_TEXTURES)markIdentity=nfs_native_marks.textures[b->texture].art.identity;}
 if(sdlWin)SDL_GetWindowSizeInPixels(sdlWin,&w,&h);
 for(unsigned i=0;i<used;++i)if(seen[i].stage==stage&&seen[i].reason==reason&&seen[i].w==(unsigned)w&&seen[i].h==(unsigned)h&&seen[i].epoch==source.sourceEpoch&&seen[i].markIdentity==markIdentity){slot=i;break;}
 if(slot==128&&used<128){slot=used++;seen[slot].stage=stage;seen[slot].reason=reason;seen[slot].w=(unsigned)w;seen[slot].h=(unsigned)h;seen[slot].epoch=source.sourceEpoch;seen[slot].markIdentity=markIdentity;}
 if(slot==128||seen[slot].count>=2){errno=priorErrno;return;}++seen[slot].count;++total;
 if(g_glCtx&&SDL_GL_GetCurrentContext()==g_glCtx&&SDL_IsMainThread()){
  glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_CURRENT_PROGRAM,&program);glGetIntegerv(GL_TEXTURE_BINDING_2D,&texture);
 }
 if(nfs_native_marks.selected>=0&&nfs_native_marks.selected<NFS_NATIVE_MARK_MAX_BINDINGS)encoding=nfs_native_marks.bindings[nfs_native_marks.selected].encoding;
 fprintf(stderr,"MAIN RESPONSIVE DIAG stage=%u reason=%u detail=%d phase=%u serial=%llu sourceForm=%08x epoch=%llu cursorValid=%u cursor=%u,%u shown=%u output=%dx%d prepared=%ux%u nativeFbo=%u actualFbo=%d nativeColor=%u nativeDepth=%u program=%d nativeProgram=%u texture=%d fontTag=%d mark=%d encoding=%u slot5=%u slot5Valid=%u selectedAddress=%u selectedId=%u selectedEdge=%u selectedFormat=%u originalRevision=%llu markIdentity=%llu actual=%g expected=%g z=%g q=%g\n",stage,reason,detail,nfs_menu_wide.phase,(unsigned long long)nfs_menu_wide.serial,source.sourceForm,(unsigned long long)source.sourceEpoch,source.cursorValid,source.sourceX,source.sourceY,source.shownActive,w,h,nfs_menu_wide.width,nfs_menu_wide.height,nfs_menu_wide.nativeFbo,fbo,nfs_menu_wide.nativeColor,nfs_menu_wide.nativeDepth,program,g_shaderProgram,texture,nfs_hd_selected,nfs_native_marks.selected,encoding,source.originalOpponentAddress,source.originalOpponentValid,nfs_menu_original_selection.address,nfs_menu_original_selection.id,nfs_menu_original_selection.edge,nfs_menu_original_selection.format,(unsigned long long)nfs_menu_original_selection.revision,(unsigned long long)markIdentity,(double)actual,(double)expected,(double)z,(double)q);
 errno=priorErrno;
}
/* Rejection-only actual foreground witness. A frame's cached form is known;
   game-mode names are not renderer authority and are joined to route captures.
   No pixel/guest data and no admission changes.128lines maximum globally. */
static void nfs_menu_wide_tail_witness(unsigned reason,unsigned count){
 static struct {uint64_t epoch,markRevision;uint32_t form,address,id,format,count;unsigned reason;float x,y;} seen[64];
 static unsigned used,lines;int saved=errno;
 if(!nfs_logo_trace_enabled()||lines>=128){errno=saved;return;}
 const int selected=nfs_native_marks.selected;
 const NfsNativeMarkBinding*binding=selected>=0&&selected<NFS_NATIVE_MARK_MAX_BINDINGS?nfs_native_marks.bindings+selected:NULL;
 const uint64_t revision=binding?binding->revision:0;
 const unsigned safe=count==g_trianglesCount&&count<=MaxTriangles?count:0;
 const float x=safe?g_vertices[0].vertex[0].x:0,y=safe?g_vertices[0].vertex[0].y:0;
 unsigned slot=64;
 for(unsigned i=0;i<used;++i)if(seen[i].epoch==nfs_menu_wide.ownerEpoch&&seen[i].form==nfs_menu_wide.ownerForm&&seen[i].reason==reason&&seen[i].address==nfs_menu_original_selection.address&&seen[i].id==nfs_menu_original_selection.id&&seen[i].format==nfs_menu_original_selection.format&&seen[i].count==count&&seen[i].markRevision==revision&&seen[i].x==x&&seen[i].y==y){slot=i;break;}
 if(slot<64){errno=saved;return;}
 if(used>=64){errno=saved;return;}
 slot=used++;seen[slot].epoch=nfs_menu_wide.ownerEpoch;seen[slot].form=nfs_menu_wide.ownerForm;seen[slot].reason=reason;seen[slot].address=nfs_menu_original_selection.address;seen[slot].id=nfs_menu_original_selection.id;seen[slot].format=nfs_menu_original_selection.format;seen[slot].count=count;seen[slot].markRevision=revision;seen[slot].x=x;seen[slot].y=y;
 float left=INFINITY,top=INFINITY,right=-INFINITY,bottom=-INFINITY;unsigned finite=1;
 for(unsigned i=0;i<safe;++i)for(unsigned j=0;j<3;++j){float vx=g_vertices[i].vertex[j].x,vy=g_vertices[i].vertex[j].y;if(!isfinite(vx)||!isfinite(vy)){finite=0;continue;}left=fminf(left,vx);top=fminf(top,vy);right=fmaxf(right,vx);bottom=fmaxf(bottom,vy);}
 ++lines;fprintf(stderr,"MENU WIDE TAIL SOURCE reason=%u owner=%08x epoch=%llu serial=%llu count=%u safe=%u finite=%u rect=%g,%g,%g,%g nativeAddress=%u nativeId=%u nativeFormat=%u nativeEdge=%u font=%d mark=%d markRevision=%llu markEncoding=%u\n",reason,nfs_menu_wide.ownerForm,(unsigned long long)nfs_menu_wide.ownerEpoch,(unsigned long long)nfs_menu_wide.serial,count,safe,finite,(double)left,(double)top,(double)right,(double)bottom,nfs_menu_original_selection.address,nfs_menu_original_selection.id,nfs_menu_original_selection.format,nfs_menu_original_selection.edge,nfs_hd_selected,selected,(unsigned long long)revision,binding?binding->encoding:0);
 for(unsigned i=0;i<safe&&i<2&&lines<128;++i){++lines;fprintf(stderr,"MENU WIDE TAIL VERTICES owner=%08x epoch=%llu serial=%llu triangle=%u",nfs_menu_wide.ownerForm,(unsigned long long)nfs_menu_wide.ownerEpoch,(unsigned long long)nfs_menu_wide.serial,i);for(unsigned j=0;j<3;++j)fprintf(stderr," v%u=%g,%g,%g uv%u=%g,%g,%g,%g",j,(double)g_vertices[i].vertex[j].x,(double)g_vertices[i].vertex[j].y,(double)g_vertices[i].vertex[j].z,j,(double)g_textureCoord[i].vertex[j].s,(double)g_textureCoord[i].vertex[j].t,(double)g_textureCoord[i].vertex[j].r,(double)g_textureCoord[i].vertex[j].q);fputc('\n',stderr);}
 errno=saved;
}
static int nfs_menu_wide_current(void){return nfs_menu_wide.generation&&nfs_menu_wide.context&&g_glCtx&&(uintptr_t)g_glCtx==nfs_menu_wide.context&&SDL_GL_GetCurrentContext()==g_glCtx&&SDL_IsMainThread()&&nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled&&nfs_logo_provenance.context==nfs_menu_wide.generation;}
#define NFS_MENU_WIDE_GL(call) do{if(!nfs_menu_wide_current())goto lost;call;if(!nfs_menu_wide_current())goto lost;}while(0)
/* A pending frame may be canceled while the PREVIOUS successfully presented
   layout remains active. Actual fallback presentation or source owner change
   revokes that layout; preparation itself must not reset a trackpad each frame. */
static void nfs_menu_wide_cancel(void){nfs_menu_wide.phase=0;nfs_menu_wide.serial=0;nfs_menu_wide.tailTriangles=0;nfs_menu_wide.cursorSeen=0;}
static void nfs_menu_wide_cancel_at(unsigned site){if(nfs_menu_wide.phase||nfs_menu_wide.serial)nfs_menu_wide_diag(5,site,-1,0,0,0,0);nfs_menu_wide_cancel();}
static void nfs_menu_wide_abandon(void){memset(nfs_menu_idle_slots,0,sizeof nfs_menu_idle_slots);nfs_menu_idle_mask=0;nfs_menu_idle_slot=nfs_menu_idle_selected=2;nfs_menu_idle_upload_epoch=1;nfs_menu_original_forget();nfs_menu_wide_cancel_at(220);nfs_menu_wide.color=nfs_menu_wide.fbo=0;nfs_menu_wide.artwork[0]=nfs_menu_wide.artwork[1]=0;nfs_menu_wide.width=nfs_menu_wide.height=0;nfs_menu_wide.context=0;nfs_menu_wide.generation=0;nfs_menu_responsive_revoke();}
static void nfs_menu_wide_retire(int owner){
 if(owner&&nfs_menu_wide_current()){
  if(nfs_menu_wide.artwork[0]){NFS_MENU_WIDE_GL(glDeleteTextures(2,nfs_menu_wide.artwork));}
  for(unsigned i=0;i<2;++i){if(nfs_menu_idle_slots[i].fbo){NFS_MENU_WIDE_GL(glDeleteFramebuffers(1,&nfs_menu_idle_slots[i].fbo));}if(nfs_menu_idle_slots[i].color){NFS_MENU_WIDE_GL(glDeleteTextures(1,&nfs_menu_idle_slots[i].color));}}
 }
 lost:nfs_menu_wide_abandon();
}
static int nfs_menu_wide_target(unsigned width,unsigned height){
 NfsBgSaved saved;GLuint color=0,fbo=0;GLenum status=0;int ok=0;
 if(!nfs_menu_wide_current()||nfs_menu_idle_slot>=2||!nfs_frame_dimensions(width,height))return 0;
 NfsMenuIdleSlot*idle=nfs_menu_idle_slots+nfs_menu_idle_slot;nfs_menu_wide.fbo=idle->fbo;nfs_menu_wide.color=idle->color;nfs_menu_wide.width=idle->width;nfs_menu_wide.height=idle->height;
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
 nfs_menu_wide.fbo=fbo;nfs_menu_wide.color=color;nfs_menu_wide.width=width;nfs_menu_wide.height=height;idle->fbo=fbo;idle->color=color;idle->width=width;idle->height=height;return 1;
 lost:nfs_menu_wide_abandon();return 0;
}
/* Cache only the active aspect's two layers for this GL context. */
static int nfs_menu_wide_artwork(unsigned ratio,const NfsWideMenuLayers* layers){
 GLuint next[2]={0,0};
 if(nfs_menu_wide.artwork[0]&&nfs_menu_wide.artworkRatio==ratio)return 1;
 NFS_MENU_WIDE_GL(glActiveTexture(GL_TEXTURE0));
 NFS_MENU_WIDE_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));
 NFS_MENU_WIDE_GL(glGenTextures(2,next));
 for(unsigned i=0;i<2;++i){
  if(!next[i])goto failed;
  NFS_MENU_WIDE_GL(glBindTexture(GL_TEXTURE_2D,next[i]));
  NFS_MENU_WIDE_GL(nfs_frame_texture_params());
  NFS_MENU_WIDE_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,layers->width,layers->height,0,GL_RGBA,GL_UNSIGNED_BYTE,i?layers->panel:layers->map));
  if(glGetError()!=GL_NO_ERROR)goto failed;
 }
 if(nfs_menu_wide.artwork[0]){NFS_MENU_WIDE_GL(glDeleteTextures(2,nfs_menu_wide.artwork));}
 memcpy(nfs_menu_wide.artwork,next,sizeof next);
 nfs_menu_wide.artworkRatio=ratio;
 return 1;
 failed:NFS_MENU_WIDE_GL(glDeleteTextures(2,next));return 0;
 lost:nfs_menu_wide_abandon();return 0;
}
static void nfs_menu_wide_draw_layer(unsigned role,float width){
 const GLfloat quad[]={0,0,0,0,width,0,1,0,0,480,0,1,
                      0,480,0,1,width,0,1,0,width,480,1,1};
 glUniform1i(nfs_bg_compositor.sampler,0);
 glUniform1f(nfs_bg_compositor.opacity,1.f);
 glActiveTexture(GL_TEXTURE0);
 glBindTexture(GL_TEXTURE_2D,nfs_menu_wide.artwork[role]);
 glEnableVertexAttribArray(0);
 glDisableVertexAttribArray(1);glDisableVertexAttribArray(2);glDisableVertexAttribArray(3);
 glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,quad);
 if(role){
  glEnable(GL_BLEND);
  glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD);
  glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
 }else glDisable(GL_BLEND);
 glDrawArrays(GL_TRIANGLES,0,6);
}
/* Location retains its own source/caption route; this guard authorizes only
   this responsive staging attachment, never the native source or another FBO. */
static int nfs_menu_wide_location_owner(void*user,const NfsBackgroundSceneStage*t){
 GLint fbo=0,type=0,name=0;uint64_t epoch=0;float x=0,y=0;
 if(user!=&nfs_menu_wide||!t||!nfs_menu_wide_current()||nfs_menu_wide.ownerForm!=NFS_MENU_CACHED_LOCATION||
    t->generation!=nfs_menu_wide.generation||t->contextIdentity!=nfs_menu_wide.context||
    t->framebuffer!=nfs_menu_wide.fbo||t->width!=nfs_menu_wide.width||t->height!=nfs_menu_wide.height||
    !nfs_menu_wide_source(&epoch,&x,&y))return 0;
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);if((GLuint)fbo!=t->framebuffer)return 0;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);
 return nfs_menu_wide_current()&&type==GL_TEXTURE&&(GLuint)name==nfs_menu_wide.color;
}
static int nfs_menu_wide_opponent_owner(void*user,const NfsBackgroundSceneStage*t){
 GLint fbo=0,type=0,name=0;uint64_t epoch=0;float x=0,y=0;
 if(user!=&nfs_menu_wide||!t||!nfs_menu_wide_current()||nfs_menu_wide.ownerForm!=NFS_MENU_CACHED_OPPONENT||t->generation!=nfs_menu_wide.generation||t->contextIdentity!=nfs_menu_wide.context||t->framebuffer!=nfs_menu_wide.fbo||t->width!=nfs_menu_wide.width||t->height!=nfs_menu_wide.height||!nfs_menu_wide_source(&epoch,&x,&y))return 0;
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);if((GLuint)fbo!=t->framebuffer)return 0;glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);return nfs_menu_wide_current()&&type==GL_TEXTURE&&(GLuint)name==nfs_menu_wide.color;
}
/* Geometry is authenticated by the typed scene; only this owned wide staging
   target may use a non-4:3 projection. The native 4:3 helper stays unchanged. */
static int nfs_menu_wide_soft_caption(const NfsSoftwareTextLayerView*v,const NfsBackgroundSceneStage*stage){
 NfsSoftwareTextStage text;NfsSoftwareTextGpu*gpu=&nfs_bg_compositor.caption;NfsBgSaved saved;float bounds[4];int ok=0;
 if(!v||!stage||!stage->ownerCurrent||!stage->ownerCurrent(stage->ownerUser,stage)||!nfs_soft_gpu_ready(gpu)||gpu->generation!=stage->generation||gpu->texture==nfs_menu_wide.color||gpu->texture==g_framebufferTexture||gpu->fontIdentity!=v->font.identity||memcmp(gpu->fontSha256,v->font.sha256,32))return 0;
 memset(&text,0,sizeof text);text.logicalWidth=v->logicalWidth;text.logicalHeight=v->logicalHeight;text.framebuffer=stage->framebuffer;text.width=stage->width;text.height=stage->height;text.sampling=stage->sampling;
 if(!nfs_soft_gpu_vertices(&text,v->vertices,v->vertexCount,bounds)||!nfs_soft_gpu_clip_contains(&text,bounds))return 0;
 nfs_bg_save(&saved);if(!stage->ownerCurrent(stage->ownerUser,stage))return 0;
 NFS_MENU_WIDE_GL(nfs_frame_draw_state());NFS_MENU_WIDE_GL(glUseProgram(gpu->program));NFS_MENU_WIDE_GL(glUniformMatrix4fv(gpu->matrix,1,GL_FALSE,stage->sampling.projection));NFS_MENU_WIDE_GL(glUniform1i(gpu->sampler,0));NFS_MENU_WIDE_GL(glActiveTexture(GL_TEXTURE0));NFS_MENU_WIDE_GL(glBindTexture(GL_TEXTURE_2D,gpu->texture));NFS_MENU_WIDE_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_MENU_WIDE_GL(glEnableVertexAttribArray(0));NFS_MENU_WIDE_GL(glEnableVertexAttribArray(1));NFS_MENU_WIDE_GL(glDisableVertexAttribArray(2));NFS_MENU_WIDE_GL(glDisableVertexAttribArray(3));NFS_MENU_WIDE_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),v->vertices));NFS_MENU_WIDE_GL(glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),&v->vertices[0].r));NFS_MENU_WIDE_GL(glEnable(GL_BLEND));NFS_MENU_WIDE_GL(glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD));NFS_MENU_WIDE_GL(glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA));NFS_MENU_WIDE_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)v->vertexCount));ok=glGetError()==GL_NO_ERROR&&stage->ownerCurrent(stage->ownerUser,stage);nfs_bg_restore(&saved);return ok&&stage->ownerCurrent(stage->ownerUser,stage);
 lost:nfs_menu_wide_abandon();return 0;
}
static int nfs_menu_wide_opponent_caption(const NfsOpponentSceneFrameView*v,const NfsNativeLogoTargetProof*target){
 NfsBackgroundSceneStage stage;memset(&stage,0,sizeof stage);stage.generation=nfs_menu_wide.generation;stage.contextIdentity=nfs_menu_wide.context;stage.framebuffer=nfs_menu_wide.fbo;stage.width=nfs_menu_wide.width;stage.height=nfs_menu_wide.height;stage.sampling=target->sampling;memcpy(stage.sampling.projection,nfs_menu_wide.matrix,sizeof nfs_menu_wide.matrix);stage.sampling.projection[12]+=nfs_menu_wide.delta*.5f*nfs_menu_wide.matrix[0];stage.sampling.viewport[0]=stage.sampling.viewport[1]=0;stage.sampling.viewport[2]=stage.width;stage.sampling.viewport[3]=stage.height;stage.ownerCurrent=nfs_menu_wide_opponent_owner;stage.ownerUser=&nfs_menu_wide;return nfs_menu_wide_soft_caption(&v->caption,&stage);
}
/* Complete-word captions use the exact typed native resource, but this
   independently owned responsive attachment has its own non-4:3 projection.
   Native caption helpers retain their strict legacy sampling contract. */
static int nfs_menu_wide_word(const NfsBackgroundSceneStage*t,uint64_t generation,GLuint program,GLuint texture,GLint matrix,GLint sampler,float left,float top,float right,float bottom){
 GLint fbo=0,type=0,name=0,vp[4];GLfloat q[24];GLboolean exists=0;GLenum error;
 if(!t||!t->ownerCurrent||!t->ownerCurrent(t->ownerUser,t)||generation!=t->generation||!program||!texture||texture==nfs_menu_wide.color||texture==g_framebufferTexture)return 0;
#define NFS_MENU_WORD_GL(call) do{if(!t->ownerCurrent(t->ownerUser,t))return 0;call;if(!t->ownerCurrent(t->ownerUser,t))return 0;}while(0)
 NFS_MENU_WORD_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_MENU_WORD_GL(glGetIntegerv(GL_VIEWPORT,vp));if((GLuint)fbo!=t->framebuffer||memcmp(vp,t->sampling.viewport,sizeof vp))return 0;
 NFS_MENU_WORD_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_MENU_WORD_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||!name||(GLuint)name==texture)return 0;
 NFS_MENU_WORD_GL(exists=glIsProgram(program));if(!exists)return 0;NFS_MENU_WORD_GL(exists=glIsTexture(texture));if(!exists)return 0;
 {const GLfloat made[]={left,top,0,0,right,top,1,0,left,bottom,0,1,left,bottom,0,1,right,top,1,0,right,bottom,1,1};memcpy(q,made,sizeof q);}
 for(unsigned i=0;i<8;++i){NFS_MENU_WORD_GL(glDisable(nfs_frame_caps[i]));}NFS_MENU_WORD_GL(glColorMask(1,1,1,1));NFS_MENU_WORD_GL(glDepthMask(GL_FALSE));NFS_MENU_WORD_GL(glActiveTexture(GL_TEXTURE0));NFS_MENU_WORD_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_MENU_WORD_GL(glUseProgram(program));NFS_MENU_WORD_GL(glUniformMatrix4fv(matrix,1,GL_FALSE,t->sampling.projection));NFS_MENU_WORD_GL(glUniform1i(sampler,0));NFS_MENU_WORD_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_MENU_WORD_GL(glEnableVertexAttribArray(0));for(unsigned i=1;i<4;++i){NFS_MENU_WORD_GL(glDisableVertexAttribArray(i));}NFS_MENU_WORD_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,q));NFS_MENU_WORD_GL(glEnable(GL_BLEND));NFS_MENU_WORD_GL(glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD));NFS_MENU_WORD_GL(glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA));NFS_MENU_WORD_GL(glDrawArrays(GL_TRIANGLES,0,6));NFS_MENU_WORD_GL(error=glGetError());
#undef NFS_MENU_WORD_GL
 return error==GL_NO_ERROR;
}
static int nfs_menu_wide_location_caption(const NfsLocationSceneFrameView*location,const NfsNativeLogoTargetProof*target){
 NfsBackgroundSceneStage stage;memset(&stage,0,sizeof stage);stage.generation=nfs_menu_wide.generation;stage.contextIdentity=nfs_menu_wide.context;
 stage.framebuffer=nfs_menu_wide.fbo;stage.width=nfs_menu_wide.width;stage.height=nfs_menu_wide.height;stage.sampling=target->sampling;
 memcpy(stage.sampling.projection,nfs_menu_wide.matrix,sizeof nfs_menu_wide.matrix);
 stage.sampling.projection[12]+=nfs_menu_wide.delta*.5f*nfs_menu_wide.matrix[0];
 stage.sampling.viewport[0]=stage.sampling.viewport[1]=0;stage.sampling.viewport[2]=stage.width;stage.sampling.viewport[3]=stage.height;
 stage.ownerCurrent=nfs_menu_wide_location_owner;stage.ownerUser=&nfs_menu_wide;
 if(!nfs_location_caption_empty(&location->captionWord)){
  const NfsCompactCaptionWordView*w=&location->captionWord;const NfsLocationCaptionGpu*g=&nfs_bg_compositor.locationCaption;
  if(!nfs_location_caption_valid(w,g->word))return 0;
  return nfs_menu_wide_word(&stage,g->generation,g->program,g->texture,g->matrix,g->sampler,w->left,w->top,w->right,w->bottom);
 }
 return nfs_menu_wide_soft_caption(&location->caption,&stage);
}
/* Exact Player cached-scene source has its own native admission. These
   callbacks authorize ONLY this independent wide staging attachment. */
static int nfs_menu_wide_player_owner(void*user,const NfsBackgroundSceneStage*t){
 GLint fbo=0,type=0,name=0;uint64_t epoch=0;float x=0,y=0;
 const unsigned kind=nfs_menu_wide.ownerForm==NFS_MENU_CACHED_PLAYER_ONE?NFS_PLAYER_OWNER_ONE:nfs_menu_wide.ownerForm==NFS_MENU_CACHED_PLAYER_TWO?NFS_PLAYER_OWNER_TWO:0;
 if(user!=&nfs_menu_wide||!t||!kind||!nfs_menu_wide_current()||!nfs_player_current()||nfs_player_renderer.activeOwnerKind!=kind||!nfs_player_renderer.serial||nfs_player_renderer.generation!=nfs_menu_wide.generation||nfs_player_renderer.context!=nfs_menu_wide.context||t->generation!=nfs_menu_wide.generation||t->contextIdentity!=nfs_menu_wide.context||t->framebuffer!=nfs_menu_wide.fbo||t->width!=nfs_menu_wide.width||t->height!=nfs_menu_wide.height||!nfs_menu_wide_source(&epoch,&x,&y))return 0;
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);if((GLuint)fbo!=t->framebuffer)return 0;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);
 return nfs_menu_wide_current()&&type==GL_TEXTURE&&(GLuint)name==nfs_menu_wide.color;
}
static NfsBackgroundSceneStage nfs_menu_wide_player_stage(const NfsNativeLogoTargetProof*target,int centered){
 NfsBackgroundSceneStage t;memset(&t,0,sizeof t);t.generation=nfs_menu_wide.generation;t.contextIdentity=nfs_menu_wide.context;t.framebuffer=nfs_menu_wide.fbo;t.width=nfs_menu_wide.width;t.height=nfs_menu_wide.height;t.sampling=target->sampling;memcpy(t.sampling.projection,nfs_menu_wide.matrix,sizeof nfs_menu_wide.matrix);if(centered)t.sampling.projection[12]+=nfs_menu_wide.delta*.5f*t.sampling.projection[0];t.sampling.viewport[0]=t.sampling.viewport[1]=0;t.sampling.viewport[2]=t.width;t.sampling.viewport[3]=t.height;t.ownerCurrent=nfs_menu_wide_player_owner;t.ownerUser=&nfs_menu_wide;return t;
}
static int nfs_menu_wide_player_preview(const NfsPlayerSceneFrameView*v,const NfsNativeLogoTargetProof*target){NfsBackgroundSceneStage t=nfs_menu_wide_player_stage(target,1);return v&&nfs_menu_wide_player_owner(&nfs_menu_wide,&t)&&nfs_player_preview_draw(&t,&v->preview);}
static int nfs_menu_wide_player_caption(const NfsPlayerSceneFrameView*v,const NfsNativeLogoTargetProof*target){
 NfsBackgroundSceneStage t=nfs_menu_wide_player_stage(target,0);
 if(!v||!nfs_menu_wide_player_owner(&nfs_menu_wide,&t))return 0;
 if(nfs_player_caption_empty(&v->captionWord))return nfs_graphics_scene_text(&nfs_player_renderer.caption,&t,&v->caption);
 {const NfsPlayerCaptionWordView*w=&v->captionWord;const NfsPlayerCaptionGpu*g=&nfs_player_renderer.captionWord;
  if(!nfs_player_caption_valid(w,g->word))return 0;
  return nfs_menu_wide_word(&t,g->generation,g->program,g->texture,g->matrix,g->sampler,w->left,w->top,w->right,w->bottom);
 }
}
static void nfs_menu_wide_prepare_owner(const NfsBackgroundSceneFrameView*v,const NfsNativeLogoTargetProof*target,const NfsMenuCachedSnapshot*cached,int(*captionDraw)(const float*,void*),void*captionUser,const NfsLocationSceneFrameView*location,const NfsOpponentSceneFrameView*opponent,const NfsPlayerSceneFrameView*player){
 NfsBgSaved saved;uint64_t epoch=0;float cursorX=0,cursorY=0;int dw=0,dh=0,ok=0;GLint type=0,name=0;NfsLogoVertex logos[18];size_t count=0;float bounds[4],mapMatrix[16],clearColor[4];static const int32_t full[4]={0,0,640,480};
 nfs_menu_wide_cancel();memset(&nfs_menu_wide.cached,0,sizeof nfs_menu_wide.cached);nfs_menu_wide.ownerForm=0x401910u;
 if(cached&&cached->schema){
  const unsigned expected=cached->formTag==NFS_MENU_CACHED_PLAYER_ONE?NFS_BACKGROUND_TAIL_PLAYER1_CAPTION:cached->formTag==NFS_MENU_CACHED_PLAYER_TWO?NFS_BACKGROUND_TAIL_PLAYER2_CAPTION:cached->formTag==NFS_MENU_CACHED_SETUP?4u:cached->formTag==NFS_MENU_CACHED_OPTIONS?3u:cached->formTag==NFS_MENU_CACHED_LOCATION?NFS_BACKGROUND_TAIL_LOCATION_CAPTION:cached->formTag==NFS_MENU_CACHED_OPPONENT?NFS_BACKGROUND_TAIL_OPPONENT_CAPTION:0u;
  if(!expected||!v||!nfs_menu_cached_packet_matches(cached,cached->formTag,v->serial,v->contentRevision)||v->tailKind!=expected)return;
  if(cached->formTag==NFS_MENU_CACHED_LOCATION){if(!location||location->schema!=NFS_LOCATION_SCENE_RUNTIME_SCHEMA||&location->scene!=v)return;}
  else if(cached->formTag==NFS_MENU_CACHED_OPPONENT){if(!opponent||location||opponent->schema!=NFS_OPPONENT_SCENE_RUNTIME_SCHEMA||&opponent->scene!=v)return;}
  else if(cached->formTag==NFS_MENU_CACHED_PLAYER_ONE||cached->formTag==NFS_MENU_CACHED_PLAYER_TWO){if(!player||location||opponent||player->schema!=NFS_PLAYER_SCENE_RUNTIME_SCHEMA||player->form!=cached->formTag||&player->scene!=v)return;}
  else if(location||opponent||player||!captionDraw)return;
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
 nfs_menu_idle_slot=2;
 for(unsigned i=0;i<2;++i)if(nfs_menu_idle_slots[i].nativeFbo==target->framebuffer&&nfs_menu_idle_slots[i].nativeColor==g_framebufferTexture&&nfs_menu_idle_slots[i].nativeDepth==g_framebufferDepth){nfs_menu_idle_slot=i;break;}
 if(nfs_menu_idle_slot==2){nfs_menu_idle_slot=nfs_menu_idle_next;nfs_menu_idle_next^=1;}
 {NfsMenuIdleSlot*slot=nfs_menu_idle_slots+nfs_menu_idle_slot;slot->valid=0;nfs_menu_idle_mask&=~(1u<<nfs_menu_idle_slot);slot->nativeFbo=target->framebuffer;slot->nativeColor=g_framebufferTexture;slot->nativeDepth=g_framebufferDepth;slot->nativeHeight=target->height;}
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
 {
  const unsigned ratio=width>=772.f;
  NfsWideMenuLayers authored={0};
  const int hasAuthored=nfs_wide_menu_layers(ratio,&authored)&&nfs_menu_wide_artwork(ratio,&authored);
  if(!nfs_menu_wide_current())goto lost;
  NFS_MENU_WIDE_GL(glUseProgram(nfs_bg_compositor.program));
  if(hasAuthored){
   NFS_MENU_WIDE_GL(glUniformMatrix4fv(nfs_bg_compositor.matrix,1,GL_FALSE,nfs_menu_wide.matrix));
   NFS_MENU_WIDE_GL(nfs_menu_wide_draw_layer(0,width));

  }else{
   NFS_MENU_WIDE_GL(glUniformMatrix4fv(nfs_bg_compositor.matrix,1,GL_FALSE,mapMatrix));
   if(!nfs_bg_quad(&nfs_bg_compositor,v->mapRole,0,0,640,480,0,0,nfs_bg_compositor.art[v->mapRole].width,nfs_bg_compositor.art[v->mapRole].height,full,256,0))goto done;
  }
  if(player){if(!nfs_menu_wide_player_preview(player,target))goto done;}
  else if(location||opponent){
   /* Entire original preview remains uniformly scaled and center-anchored,
      matching measured child widgets and native foreground/cursor transforms. */
   float previewMatrix[16];memcpy(previewMatrix,nfs_menu_wide.matrix,sizeof previewMatrix);previewMatrix[12]+=nfs_menu_wide.delta*.5f*previewMatrix[0];
   NFS_MENU_WIDE_GL(glUniformMatrix4fv(nfs_bg_compositor.matrix,1,GL_FALSE,previewMatrix));
   if(location?!nfs_bg_preview_draw(&nfs_bg_compositor,&location->preview,target):!nfs_bg_opponent_preview_draw(&nfs_bg_compositor,opponent,target))goto done;
  }
  NFS_MENU_WIDE_GL(glUniformMatrix4fv(nfs_bg_compositor.matrix,1,GL_FALSE,nfs_menu_wide.matrix));
  if(hasAuthored){NFS_MENU_WIDE_GL(nfs_menu_wide_draw_layer(1,width));}
  else if(!nfs_bg_quad(&nfs_bg_compositor,2,0,0,343,480,0,0,1372,1920,v->panelClip,256,1))goto done;
 }
 for(unsigned i=0;i<v->wordCount;++i){const NfsBackgroundSceneWordLayer*w=v->words+i;const NfsBackgroundSceneWord*r=nfs_bg_compositor.regions+w->sourceRole-1;
  if(!nfs_bg_quad(&nfs_bg_compositor,r->textureRole,(float)w->left,(float)w->top,(float)r->logicalWidth,(float)r->logicalHeight,r->x,r->y,r->width,r->height,w->clip,w->scalar256,1))goto done;
 }
 if(count){NFS_MENU_WIDE_GL(glUseProgram(nfs_bg_compositor.logo.program));NFS_MENU_WIDE_GL(glUniformMatrix4fv(nfs_bg_compositor.logo.matrix,1,GL_FALSE,nfs_menu_wide.matrix));NFS_MENU_WIDE_GL(glUniform1i(nfs_bg_compositor.logo.sampler,0));NFS_MENU_WIDE_GL(glBindTexture(GL_TEXTURE_2D,nfs_bg_compositor.logo.texture));NFS_MENU_WIDE_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),logos));NFS_MENU_WIDE_GL(glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,sizeof(NfsLogoVertex),&logos[0].coefficient));NFS_MENU_WIDE_GL(glEnableVertexAttribArray(0));NFS_MENU_WIDE_GL(glEnableVertexAttribArray(1));NFS_MENU_WIDE_GL(glDisableVertexAttribArray(2));NFS_MENU_WIDE_GL(glDisableVertexAttribArray(3));NFS_MENU_WIDE_GL(glEnable(GL_BLEND));NFS_MENU_WIDE_GL(glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA));NFS_MENU_WIDE_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count));}
 if(player){if(!nfs_menu_wide_player_caption(player,target))goto done;}
 else if(location){if(!nfs_menu_wide_location_caption(location,target))goto done;}
 else if(opponent){if(!nfs_menu_wide_opponent_caption(opponent,target))goto done;}
 else if(nfs_menu_wide.cached.schema&&!captionDraw(nfs_menu_wide.matrix,captionUser))goto done;
 ok=glGetError()==GL_NO_ERROR;
 NFS_MENU_WIDE_GL(glBindFramebuffer(GL_FRAMEBUFFER,target->framebuffer));NFS_MENU_WIDE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_MENU_WIDE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(type!=GL_TEXTURE||(GLuint)name!=g_framebufferTexture)ok=0;
 done:if(!nfs_menu_wide_current())goto lost;glClearColor(clearColor[0],clearColor[1],clearColor[2],clearColor[3]);nfs_bg_restore(&saved);if(!nfs_menu_wide_current())goto lost;if(!ok){nfs_menu_wide_diag(1,11,-1,0,0,0,0);return;}
 nfs_menu_wide.locationArtGeneration=location?location->preview.locationArtGeneration:0;nfs_menu_wide.ownerEpoch=epoch;nfs_menu_wide.serial=v->serial;nfs_menu_wide.nativeFbo=target->framebuffer;nfs_menu_wide.nativeColor=g_framebufferTexture;nfs_menu_wide.nativeDepth=g_framebufferDepth;nfs_menu_wide.nativeHeight=target->height;nfs_menu_wide.phase=NFS_MENU_WIDE_PREPARED;nfs_menu_wide_diag(1,0,-1,0,0,0,0);return;
 lost:nfs_menu_wide_abandon();
}
static void nfs_menu_wide_prepare(const NfsBackgroundSceneFrameView*v,const NfsNativeLogoTargetProof*t){nfs_menu_wide_prepare_owner(v,t,NULL,NULL,NULL,NULL,NULL,NULL);}
static void nfs_menu_wide_prepare_cached(const NfsBackgroundSceneFrameView*v,const NfsNativeLogoTargetProof*t,const NfsMenuCachedSnapshot*p,int(*caption)(const float*,void*),void*user){
 if(!p||!p->schema){nfs_menu_wide_cancel();return;}nfs_menu_wide_prepare_owner(v,t,p,caption,user,NULL,NULL,NULL);
}
static void nfs_menu_wide_prepare_location(const NfsLocationSceneFrameView*v,const NfsNativeLogoTargetProof*t){
 if(!v||v->schema!=NFS_LOCATION_SCENE_RUNTIME_SCHEMA||v->responsive.formTag!=NFS_MENU_CACHED_LOCATION||!v->responsive.schema){nfs_menu_wide_cancel();return;}
 nfs_menu_wide_prepare_owner(&v->scene,t,&v->responsive,NULL,NULL,v,NULL,NULL);
}
static void nfs_menu_wide_prepare_opponent(const NfsOpponentSceneFrameView*v,const NfsNativeLogoTargetProof*t){
 if(!v||v->schema!=NFS_OPPONENT_SCENE_RUNTIME_SCHEMA||v->responsive.formTag!=NFS_MENU_CACHED_OPPONENT||!v->responsive.schema){nfs_menu_wide_cancel();return;}nfs_menu_wide_prepare_owner(&v->scene,t,&v->responsive,NULL,NULL,NULL,v,NULL);
}
static void nfs_menu_wide_prepare_player(const NfsPlayerSceneFrameView*v,const NfsNativeLogoTargetProof*t){
 if(!v||v->schema!=NFS_PLAYER_SCENE_RUNTIME_SCHEMA||(v->form!=NFS_MENU_CACHED_PLAYER_ONE&&v->form!=NFS_MENU_CACHED_PLAYER_TWO)||!v->responsive.schema||v->responsive.formTag!=v->form){nfs_menu_wide_cancel();return;}
 nfs_menu_wide_prepare_owner(&v->scene,t,&v->responsive,NULL,NULL,NULL,NULL,v);
}
static void nfs_menu_wide_native_done(uint64_t serial,int accepted){
 if(!accepted||!nfs_bg_location_generation(nfs_menu_wide.locationArtGeneration)){nfs_menu_wide_diag(2,1,-1,0,0,0,0);nfs_menu_wide_cancel_at(205);return;}
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
 if(!nfs_menu_wide_current()){nfs_menu_wide_diag(3,2,-1,0,0,0,0);nfs_menu_wide_tail_witness(2,count);goto refused;}
 if(!count||count!=g_trianglesCount||count>MaxTriangles){nfs_menu_wide_diag(3,3,-1,(float)count,(float)g_trianglesCount,0,0);nfs_menu_wide_tail_witness(3,count);goto refused;}
 if(g_fogEnabled||g_textureEnabled!=1){nfs_menu_wide_diag(3,4,-1,g_textureEnabled,1,0,0);nfs_menu_wide_tail_witness(4,count);goto refused;}
 if(!nfs_menu_wide_source(&epoch,&sx,&sy)||epoch!=nfs_menu_wide.ownerEpoch){nfs_menu_wide_diag(3,5,-1,0,0,0,0);nfs_menu_wide_tail_witness(5,count);goto refused;}
 const int selected=nfs_native_marks.selected;
 const int mark=selected>=0&&selected<NFS_NATIVE_MARK_MAX_BINDINGS&&nfs_native_marks.bindings[selected].valid&&nfs_native_marks.bindings[selected].generation==nfs_native_marks.generation&&nfs_mark_owner()&&nfs_native_marks.program==g_shaderProgram;
 const int originalBranding=!mark&&nfs_menu_original_branding(epoch);
 const int originalMode=!mark&&!originalBranding&&nfs_menu_mode_original_branding(epoch)&&nfs_menu_wide_setup_mode_pulse(count);
 const int slot5Canvas=(mark||originalBranding)&&nfs_menu_wide_slot5_square(count,epoch,originalBranding,selected);
 if(originalBranding&&!slot5Canvas){nfs_menu_wide_diag(3,15,5,(float)count,2,0,0);nfs_menu_wide_tail_witness(15,count);goto refused;}
 if(!mark&&!originalBranding&&!originalMode&&nfs_front_font_source_page(nfs_hd_selected)<0){nfs_menu_wide_diag(3,6,-1,0,0,0,0);nfs_menu_wide_tail_witness(6,count);goto refused;}
 if(originalBranding)nfs_menu_wide_diag(3,14,5,(float)nfs_menu_original_selection.address,(float)nfs_menu_original_selection.id,0,0);
 cursor=mark&&nfs_native_marks.bindings[selected].encoding==NFS_NATIVE_MARK_SOURCE_RGBA4444_LE;
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_CURRENT_PROGRAM,&program);
 if((GLuint)fbo!=nfs_menu_wide.nativeFbo||(GLuint)program!=g_shaderProgram||g_framebufferTexture!=nfs_menu_wide.nativeColor||g_framebufferDepth!=nfs_menu_wide.nativeDepth||g_framebufferHeight!=(int)nfs_menu_wide.nativeHeight){nfs_menu_wide_diag(3,7,-1,0,0,0,0);nfs_menu_wide_tail_witness(7,count);goto refused;}
 if(glIsEnabled(GL_STENCIL_TEST)||glIsEnabled(GL_CULL_FACE)||glIsEnabled(GL_POLYGON_OFFSET_FILL)){nfs_menu_wide_diag(3,8,-1,0,0,0,0);nfs_menu_wide_tail_witness(8,count);goto refused;}
 glGetIntegerv(GL_VIEWPORT,nativeViewport);
 if(nativeViewport[2]<=0||nativeViewport[3]<=0){nfs_menu_wide_diag(3,9,-1,0,0,0,0);nfs_menu_wide_tail_witness(9,count);goto refused;}
 {const float scale=nfs_menu_wide.nativeHeight/480.f;float expected[16]={0};
  expected[0]=2.f*scale/nativeViewport[2];expected[5]=-2.f*scale/nativeViewport[3];expected[10]=-2;
  expected[12]=-1.f-2.f*nativeViewport[0]/nativeViewport[2];expected[13]=1.f+2.f*(nfs_menu_wide.nativeHeight-nativeViewport[1]-nativeViewport[3])/nativeViewport[3];expected[14]=expected[15]=1;
  for(unsigned k=0;k<16;++k)if(!isfinite(g_matrix[k])||fabsf(g_matrix[k]-expected[k])>0.00001f){nfs_menu_wide_diag(3,10,(int)k,g_matrix[k],expected[k],0,0);nfs_menu_wide_tail_witness(10,count);goto refused;}
 }

 memcpy(moved,g_vertices,count*sizeof(Vertices));
 for(unsigned i=0;i<count;++i){float left=1e9f,right=-1e9f;for(unsigned j=0;j<3;++j){float x=moved[i].vertex[j].x,y=moved[i].vertex[j].y;if(!isfinite(x)||!isfinite(y)||moved[i].vertex[j].z!=1||g_textureCoord[i].vertex[j].q!=1||(!slot5Canvas&&(x<-32||x>672||y<-32||y>512))){nfs_menu_wide_diag(3,11,(int)(3*i+j),x,y,moved[i].vertex[j].z,g_textureCoord[i].vertex[j].q);nfs_menu_wide_tail_witness(11,count);goto refused;}left=fminf(left,x);right=fmaxf(right,x);}
  float shift=0,dy=0;if(cursor){if(nfs_menu_wide.cached.schema)shift=nfs_menu_wide.delta*.5f;if(nfs_menu_responsive_snapshot(&shown)&&shown.drawableWidth==nfs_menu_wide.width&&shown.drawableHeight==nfs_menu_wide.height&&nfs_menu_responsive_pointer_snapshot(&pointer)&&pointer.revision==shown.revision&&pointer.ownerEpoch==epoch){shift=pointer.x-sx;dy=pointer.y-sy;}}
  /* Main's source-recognized128-square car/track marks are the right
     branding group even when transparent native canvas extends left of its
     visible ink. Never split/crop that canvas using an ink guess. */
  else if(nfs_menu_wide.cached.schema&&((!mark&&!originalBranding&&!originalMode)||nfs_menu_wide.ownerForm==NFS_MENU_CACHED_LOCATION))shift=nfs_menu_wide.delta*.5f;
  else if(mark||originalBranding||originalMode||left>=343)shift=nfs_menu_wide.delta;else if(right>343){nfs_menu_wide_diag(3,12,(int)i,left,right,0,0);nfs_menu_wide_tail_witness(12,count);goto refused;}
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
 done:if(!nfs_menu_wide_current())goto lost;glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nativeMatrix);nfs_bg_restore(&saved);if(!nfs_menu_wide_current())goto lost;if(!ok){nfs_menu_wide_diag(3,13,-1,0,0,0,0);nfs_menu_wide_tail_witness(13,count);goto refused;}nfs_menu_wide.tailTriangles+=count;if(cursor)nfs_menu_wide.cursorSeen=1;nfs_menu_wide_diag(3,0,-1,0,0,0,0);return;
 refused:nfs_menu_wide_cancel_at(240);return;
 lost:nfs_menu_wide_abandon();
}
static GLuint nfs_menu_wide_present(GLuint frontFbo,GLuint frontTexture,int width,int height){
 uint64_t epoch=0;float x=0,y=0;GLint prior=0,type=0,name=0,depthType=0,depthName=0;GLenum status=0;int ok=0;
 if(nfs_menu_wide.phase!=NFS_MENU_WIDE_TAIL){if(nfs_menu_wide.phase==NFS_MENU_WIDE_IDLE){GLuint idle=nfs_menu_idle_present(frontFbo,frontTexture,width,height);if(idle)return idle;}nfs_menu_wide_diag(4,1,-1,0,0,0,0);goto refused;}
 if(!nfs_menu_wide_current()||!nfs_bg_location_generation(nfs_menu_wide.locationArtGeneration)){nfs_menu_wide_diag(4,2,-1,0,0,0,0);goto refused;}
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
 if(nfs_menu_idle_selected<2){nfs_menu_idle_after_present(success);return;}
 if(success&&nfs_menu_wide.phase==NFS_MENU_WIDE_SELECTED&&nfs_menu_wide_current()&&nfs_bg_location_generation(nfs_menu_wide.locationArtGeneration)){
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
 nfs_menu_idle_seal();
 }else {if(!success)nfs_menu_idle_revoke(8);nfs_menu_responsive_revoke();}nfs_menu_wide_cancel_at(210);
}
#undef NFS_MENU_WIDE_GL

/* Idle receipt predicates are read-only host metadata. A normal producer still
 * must pass the complete original native600triangle/tail/cursor admission. */
/* event1 sealednormal,2 idle-selected,3 successfulidle-present. Deduplicate
 * per owner/output/physicalslot so normalanimation cannot exhaust idle evidence. */
static void nfs_menu_idle_note(unsigned event,unsigned slot){
 static struct{unsigned event,slot,form,width,height,hits;uint64_t epoch;}seen[64];static unsigned used,lines;const int saved=errno;
 if(!nfs_logo_trace_enabled()||slot>=2||lines>=128){errno=saved;return;}const NfsMenuIdleSlot*s=nfs_menu_idle_slots+slot;const unsigned form=s->cached.schema?s->cached.formTag:0x401910u;unsigned i=0;
 for(;i<used;++i)if(seen[i].event==event&&seen[i].slot==slot&&seen[i].form==form&&seen[i].width==s->width&&seen[i].height==s->height&&seen[i].epoch==s->epoch)break;
 if(i==used){if(used==64){errno=saved;return;}seen[i].event=event;seen[i].slot=slot;seen[i].form=form;seen[i].width=s->width;seen[i].height=s->height;seen[i].epoch=s->epoch;++used;}
 if(seen[i].hits>=2){errno=saved;return;}++seen[i].hits;++lines;fprintf(stderr,"MENU IDLE event=%u slot=%u owner=%08x epoch=%llu output=%ux%u nativeFbo=%u paint=%llu upload=%llu valid=%u capped=%u\n",event,slot,form,(unsigned long long)s->epoch,s->width,s->height,s->nativeFbo,(unsigned long long)s->paint,(unsigned long long)nfs_menu_idle_upload_epoch,s->valid,lines==128);errno=saved;
}
static void nfs_menu_idle_revoke(unsigned reason){if(!nfs_menu_idle_mask&&nfs_menu_idle_selected==2)return;for(unsigned i=0;i<2;++i){if(nfs_menu_idle_slots[i].valid)nfs_menu_idle_note(1000+reason,i);nfs_menu_idle_slots[i].valid=0;}nfs_menu_idle_mask=0;nfs_menu_idle_selected=2;}
static void nfs_menu_idle_upload(void){if(!nfs_menu_idle_mask&&nfs_menu_wide.phase==NFS_MENU_WIDE_IDLE)return;nfs_menu_idle_revoke(2);if(nfs_menu_idle_upload_epoch<UINT64_MAX)++nfs_menu_idle_upload_epoch;}
/* Called ONLY after the original native drawTriangles glDrawArrays. Direct
 * semantic-helper GL calls do not pass this anchor and never revoke themselves. */
static void nfs_menu_idle_paint(void){
 GLint actual=0;if(!nfs_menu_idle_mask&&nfs_menu_wide.phase==NFS_MENU_WIDE_IDLE)return;
 if(!nfs_menu_wide_current()){nfs_menu_idle_revoke(3);return;}glGetIntegerv(GL_FRAMEBUFFER_BINDING,&actual);if(!nfs_menu_wide_current()){nfs_menu_idle_revoke(3);return;}const GLenum queryError=glGetError();if(!nfs_menu_wide_current()||queryError!=GL_NO_ERROR){nfs_menu_idle_revoke(3);nfs_menu_wide_cancel();return;}
 unsigned matched=0;for(unsigned i=0;i<2;++i){NfsMenuIdleSlot*s=nfs_menu_idle_slots+i;if(s->nativeFbo==(GLuint)actual){matched=1;if(s->valid)nfs_menu_idle_note(300,i);s->valid=0;nfs_menu_idle_mask&=~(1u<<i);if(s->paint<UINT64_MAX)++s->paint;}}if(!matched||actual<=0)nfs_menu_idle_revoke(5);
}
static void nfs_menu_idle_resources(NfsMenuIdleResources*r){memset(r,0,sizeof*r);r->generation=nfs_bg_compositor.generation;r->ownerGeneration=nfs_bg_compositor.ownerGeneration;r->program=nfs_bg_compositor.program;r->copyProgram=nfs_bg_compositor.copyProgram;memcpy(r->textures,nfs_bg_compositor.textures,sizeof r->textures);for(unsigned i=0;i<5;++i){r->identities[i]=nfs_bg_compositor.art[i].identity;memcpy(r->artSha256[i],nfs_bg_compositor.art[i].sha256,32);}r->logoIdentity=nfs_bg_compositor.logo.artIdentity;r->logoProgram=nfs_bg_compositor.logo.program;memcpy(r->logoSha256,nfs_bg_compositor.logo.artSHA256,32);r->logoGeneration=nfs_bg_compositor.logo.generation;r->logoTexture=nfs_bg_compositor.logo.texture;r->captionProgram=nfs_bg_compositor.caption.program;memcpy(r->captionSha256,nfs_bg_compositor.caption.fontSha256,32);r->captionGeneration=nfs_bg_compositor.caption.generation;r->captionIdentity=nfs_bg_compositor.caption.fontIdentity;r->captionTexture=nfs_bg_compositor.caption.texture;memcpy(r->wideTextures,nfs_menu_wide.artwork,sizeof r->wideTextures);r->ratio=nfs_menu_wide.artworkRatio;r->photoGeneration=nfs_player_photo_art_generation();r->classGeneration=nfs_class_picture_art_generation();r->locationGeneration=nfs_location_photo_art_generation();}
static int nfs_menu_idle_current(const NfsMenuIdleSlot*s){
 uint64_t epoch=0;float x=0,y=0;NfsMenuIdleResources resources;
 if(!s||!s->valid||nfs_menu_idle_upload_epoch==UINT64_MAX||!s->fbo||!s->color||s->paint==UINT64_MAX||s->paint!=s->sealedPaint||s->upload!=nfs_menu_idle_upload_epoch||!nfs_menu_wide_current()||s->context!=nfs_menu_wide.context||s->generation!=nfs_menu_wide.generation||!nfs_bg_location_generation(s->locationArtGeneration))return 0;
 if(s->cached.schema){if(!nfs_menu_cached_current(&s->cached,&epoch,&x,&y))return 0;}
 else{if(!nfs_main_upload.valid||nfs_main_upload.pending||!nfs_main_upload.view.owner||!nfs_main_upload.view.current||(uintptr_t)nfs_main_upload.view.owner!=s->mainOwner||nfs_main_upload.view.domain.game!=s->mainGame||nfs_main_upload.view.domain.life!=s->mainLife||nfs_main_upload.view.domain.ownerEpoch!=s->mainEpoch||nfs_main_upload.view.domain.uploadEvent!=s->mainUpload||!nfs_main_upload.view.current(nfs_main_upload.view.owner)||!nfs_menu_responsive_source_main(&epoch,&x,&y))return 0;}
 if(epoch!=s->epoch||x!=s->x||y!=s->y)return 0;
 nfs_menu_idle_resources(&resources);return !memcmp(&resources,&s->resources,sizeof resources);
}
static int nfs_menu_idle_context(void*user,const NfsBackgroundSceneStage*t){const NfsMenuIdleSlot*s=(const NfsMenuIdleSlot*)user;return s&&t&&nfs_menu_wide_current()&&s->context==t->contextIdentity&&s->generation==t->generation&&t->framebuffer;}
static GLuint nfs_menu_idle_present(GLuint frontFbo,GLuint frontTexture,int width,int height){
 unsigned index=2;GLint kind=0,name=0;GLenum status=0;int ok=0;NfsBgSaved saved;NfsBackgroundSceneStage guard;NfsMenuIdleSlot*s=NULL;
 if(!nfs_menu_idle_mask||nfs_menu_idle_selected<2)return 0;
 for(unsigned i=0;i<2;++i)if(nfs_menu_idle_slots[i].valid&&nfs_menu_idle_slots[i].nativeFbo==frontFbo&&nfs_menu_idle_slots[i].nativeColor==frontTexture){index=i;break;}
 if(index==2)return 0;s=nfs_menu_idle_slots+index;
 if(width!=(int)s->width||height!=(int)s->height||!nfs_menu_idle_current(s)||nfs_menu_wide.revision==UINT64_MAX){nfs_menu_idle_note(301,index);s->valid=0;nfs_menu_idle_mask&=~(1u<<index);return 0;}
 memset(&guard,0,sizeof guard);guard.generation=s->generation;guard.contextIdentity=s->context;guard.framebuffer=frontFbo;guard.ownerCurrent=nfs_menu_idle_context;guard.ownerUser=s;
 if(!nfs_graphics_scene_save(&saved,&guard))return 0;
#define NFS_MENU_IDLE_GL(call) do{if(!nfs_menu_idle_current(s))goto done;call;if(!nfs_menu_idle_current(s))goto done;}while(0)
 NFS_MENU_IDLE_GL(glBindFramebuffer(GL_FRAMEBUFFER,frontFbo));NFS_MENU_IDLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&kind));NFS_MENU_IDLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(kind!=GL_TEXTURE||(GLuint)name!=frontTexture)goto done;NFS_MENU_IDLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&kind));NFS_MENU_IDLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));if(kind!=GL_RENDERBUFFER||(GLuint)name!=s->nativeDepth)goto done;
 NFS_MENU_IDLE_GL(glBindFramebuffer(GL_FRAMEBUFFER,s->fbo));NFS_MENU_IDLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&kind));NFS_MENU_IDLE_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));NFS_MENU_IDLE_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));if(kind!=GL_TEXTURE||(GLuint)name!=s->color||status!=GL_FRAMEBUFFER_COMPLETE||s->color==frontTexture||s->color==g_framebufferTexture||s->fbo==frontFbo||s->fbo==g_framebuffer)goto done;
 NFS_MENU_IDLE_GL(ok=glGetError()==GL_NO_ERROR);
 done:if(nfs_menu_idle_context(s,&guard)&&!nfs_graphics_scene_restore(&saved,&guard))ok=0;
#undef NFS_MENU_IDLE_GL
 if(!ok||!nfs_menu_idle_current(s)){nfs_menu_idle_note(302,index);s->valid=0;nfs_menu_idle_mask&=~(1u<<index);return 0;}
 nfs_menu_idle_selected=index;s->layout.revision=++nfs_menu_wide.revision;nfs_menu_idle_note(2,index);return s->color;
}
static void nfs_menu_idle_after_present(int success){unsigned index=nfs_menu_idle_selected;nfs_menu_idle_selected=2;if(index>=2)return;NfsMenuIdleSlot*s=nfs_menu_idle_slots+index;if(success&&nfs_menu_idle_current(s)){if(s->cached.schema)nfs_menu_cached_publish(&s->layout,&s->cached);else nfs_menu_responsive_publish(&s->layout);nfs_menu_idle_note(3,index);}else{nfs_menu_idle_revoke(4);nfs_menu_responsive_revoke();}}
static void nfs_menu_idle_seal(void){
 if(nfs_menu_idle_slot>=2||nfs_menu_wide.phase!=NFS_MENU_WIDE_SELECTED)return;NfsMenuIdleSlot*s=nfs_menu_idle_slots+nfs_menu_idle_slot;
 s->context=nfs_menu_wide.context;s->generation=nfs_menu_wide.generation;s->epoch=nfs_menu_wide.ownerEpoch;s->locationArtGeneration=nfs_menu_wide.locationArtGeneration;s->cached=nfs_menu_wide.cached;s->layout=nfs_menu_wide.selected;s->x=s->layout.sourceCursorX;s->y=s->layout.sourceCursorY;s->sealedPaint=s->paint;s->upload=nfs_menu_idle_upload_epoch;
 s->mainOwner=(uintptr_t)nfs_main_upload.view.owner;s->mainGame=nfs_main_upload.view.domain.game;s->mainLife=nfs_main_upload.view.domain.life;s->mainEpoch=nfs_main_upload.view.domain.ownerEpoch;s->mainUpload=nfs_main_upload.view.domain.uploadEvent;nfs_menu_idle_resources(&s->resources);s->valid=1;
 if(!nfs_menu_idle_current(s)){nfs_menu_idle_note(303,nfs_menu_idle_slot);s->valid=0;return;}nfs_menu_idle_mask|=1u<<nfs_menu_idle_slot;nfs_menu_idle_note(1,nfs_menu_idle_slot);
}

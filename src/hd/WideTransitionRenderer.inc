// SPDX-License-Identifier: MIT
/* Fresh authored full-width kinds1/3, exact opaque first-full8/9/10,
   plus initial sidebars4/5/6/7 over exact current source-command map/Main receipts.
   No native framebuffer is sampled.
   Begin is called AFTER native source admission; finish AFTER native commit. */
#include "WideTransitionGeometry.h"
#include "MenuSourceCommandModel.h"
static NfsMenuMapSourceStore nfs_mtw_maps;
static uint64_t nfs_mtw_paint=1;
/* Owned immutable image key; never a displayed result or source authority. */
typedef struct NfsMtwStaticKey {
 NfsBackgroundSceneImageKey background;NfsBackgroundSceneWord regions[12];
 uintptr_t context,thread,game,catalog;uint64_t life,epoch,resourceEpoch;
 uint32_t archive,form,kind,ratio;GLuint wideArt[2];GLint captionMatrix,captionSampler;
 float logical;
} NfsMtwStaticKey;
static struct {
 uintptr_t context,thread,sourceGame,sourceCatalog;uint32_t sourceArchive,initialForm;uint64_t initialEpoch;uint64_t sourceLife;uint64_t generation,serial,uploads,clearEvent,locationArtGeneration;
 GLuint image,result,fbo,art[2],nativeFbo,nativeColor,nativeDepth;
 unsigned width,height,ratio,phase,kind,nativeHeight,tailTriangles,cursorSeen,pendingTriangles;
 uint32_t ownerForm,descriptor;uint64_t producerEpoch,ownerEpoch,content;NfsMenuCachedSnapshot cached;NfsMenuResponsiveLayout selected;float logical,matrix[16];
 NfsTransitionMotion motion;NfsMenuMapSource underlay;unsigned underlayValid,locationInfoReturn;
 NfsMtwStaticKey staticKey;unsigned staticReady;uint64_t resourceEpoch,cacheHits,cacheMisses,cacheInvalidations;
} nfs_mtw;
static void nfs_mtw_static_invalidate(void){if(nfs_mtw.staticReady&&nfs_mtw.cacheInvalidations!=UINT64_MAX)++nfs_mtw.cacheInvalidations;nfs_mtw.staticReady=0;}
static int nfs_mtw_current(void){return nfs_mtw.context&&(uintptr_t)g_glCtx==nfs_mtw.context&&g_glCtx&&SDL_GL_GetCurrentContext()==g_glCtx&&nfs_mtw.thread==(uintptr_t)SDL_GetCurrentThreadID()&&nfs_mtw.generation==nfs_logo_provenance.context&&nfs_logo_provenance.alive&&!nfs_logo_provenance.disabled;}
static int nfs_mtw_art_current(void){return nfs_bg_location_generation(nfs_mtw.locationArtGeneration);}
static int nfs_mtw_context_owner(void*user,const NfsBackgroundSceneStage*t){return user==&nfs_mtw&&t&&t->generation==nfs_mtw.generation&&t->contextIdentity==nfs_mtw.context&&nfs_mtw_current();}
static NfsBackgroundSceneStage nfs_mtw_guard(void){NfsBackgroundSceneStage t;memset(&t,0,sizeof t);t.generation=nfs_mtw.generation;t.contextIdentity=nfs_mtw.context;t.framebuffer=g_framebuffer;t.ownerCurrent=nfs_mtw_context_owner;t.ownerUser=&nfs_mtw;return t;}
static void nfs_mtw_branding_reset(void);
static void nfs_mt_wide_cancel(void){nfs_mtw_branding_reset();if(!nfs_mtw.phase&&!nfs_mtw.serial&&!nfs_mtw.underlayValid&&!nfs_mtw.tailTriangles&&!nfs_mtw.cursorSeen&&!nfs_mtw.pendingTriangles&&!nfs_mtw_maps.pendingValid)return;nfs_mtw.phase=0;nfs_mtw.serial=0;nfs_mtw.locationArtGeneration=0;nfs_mtw.underlayValid=0;nfs_mtw.tailTriangles=nfs_mtw.cursorSeen=nfs_mtw.pendingTriangles=0;nfs_menu_map_source_cancel_pending(&nfs_mtw_maps);}
static void nfs_mtw_map_trace(unsigned,int,const NfsMenuSourcePaintKey*,const NfsMenuTransitionView*);
static void nfs_mt_wide_other(void){if(nfs_mtw.phase||nfs_mtw.serial)nfs_mtw_static_invalidate();if(!nfs_mtw.phase&&!nfs_mtw.serial&&!nfs_mtw.underlayValid&&!nfs_mtw_maps.pendingValid&&!nfs_mtw_maps.completedValid[0]&&!nfs_mtw_maps.completedValid[1])return;if(nfs_mtw_maps.pendingValid||nfs_mtw_maps.completedValid[0]||nfs_mtw_maps.completedValid[1])nfs_mtw_map_trace(9,0,NULL,NULL);nfs_mt_wide_cancel();nfs_menu_map_source_invalidate(&nfs_mtw_maps);if(nfs_mtw_paint<UINT64_MAX)++nfs_mtw_paint;}
static NfsMenuSourcePaintKey nfs_mtw_key(const NfsNativeLogoTargetProof*t){NfsMenuSourcePaintKey k;memset(&k,0,sizeof k);k.context=(uintptr_t)g_glCtx;k.generation=nfs_logo_provenance.context;k.paintRevision=nfs_mtw_paint;k.nativeFbo=t->framebuffer;k.nativeColor=g_framebufferTexture;k.nativeDepth=g_framebufferDepth;k.width=t->width;k.height=t->height;return k;}
/* Diagnostic occurrence codes:1finish-entry,2native-proof-refusal,
 *3map-sealed,4seal-refused,5state-save-refused,6wide-ready,7render-refused,
 *8sidebar-missing-map,9actual-invalidation,10map-seal-attempt. Only existing scalar metadata;
 * completed domain equality bits are meaningful when requestDomain=1. */
static void nfs_mtw_map_trace(unsigned event,int accepted,const NfsMenuSourcePaintKey*k,const NfsMenuTransitionView*v){
 struct Key{unsigned event,state,native,pending,completed[2],request,life;};static struct Key seen[16];static unsigned used;struct Key x;unsigned prior;
 const int saved=errno;if(!nfs_logo_trace_enabled()||used>=16){errno=saved;return;}memset(&x,0,sizeof x);x.event=event;x.request=v!=NULL;x.life=(nfs_mtw.sourceLife!=0)|((nfs_mtw_maps.pendingValid&&nfs_mtw_maps.pending.sourceLife==nfs_mtw.sourceLife)<<1);
 x.state=(nfs_mtw.phase&15u)|(!!nfs_mtw_maps.pendingValid<<4)|(!!nfs_mtw_maps.completedValid[0]<<5)|(!!nfs_mtw_maps.completedValid[1]<<6);
 x.native=(!!accepted)|((nfs_mtw.phase==1)<<1)|((nfs_mt.serial==nfs_mtw.serial)<<2)|((nfs_mt.selects==300)<<3)|((nfs_mt.triangles==600)<<4)|((nfs_logo_provenance.uploads==nfs_mtw.uploads)<<5)|((nfs_logo_provenance.clearEvent==nfs_mtw.clearEvent)<<6);
 if(nfs_mtw_maps.pendingValid){const NfsMenuMapSource*m=&nfs_mtw_maps.pending;x.pending=1u|((k&&nfs_menu_source_same_destination(&m->key,k))<<1)|((k&&k->paintRevision>m->key.paintRevision)<<2)|((m->sourceSerial==nfs_mt.serial)<<3)|((m->uploads==nfs_logo_provenance.uploads)<<4)|((m->clearEvent==nfs_logo_provenance.clearEvent)<<5);}
 for(unsigned i=0;i<2;++i){const NfsMenuMapSource*m=nfs_mtw_maps.completed+i;if(!nfs_mtw_maps.completedValid[i])continue;x.completed[i]=1u|((k&&nfs_menu_source_same_destination(&m->key,k))<<1)|((k&&nfs_menu_source_same_paint(&m->key,k))<<2)|((v&&m->sourceGame==v->sourceGame)<<3)|((v&&m->sourceLife==v->sourceLife)<<4)|((v&&m->sourceArchive==v->sourceArchive)<<5)|((v&&m->sourceCatalog==v->sourceCatalog)<<6)|((v&&m->mapRole==v->mapRole)<<7);}
 for(prior=0;prior<used;++prior)if(!memcmp(seen+prior,&x,sizeof x)){errno=saved;return;}seen[used++]=x;
 fprintf(stderr,"MENU TRANSITION MAP diagnostic event=%u phase=%u recordMask=%u wideLifePresent=%u pendingLifeMatches=%u nativeAccepted=%u wideSerialMatch=%u selections300=%u triangles600=%u uploadsSame=%u clearSame=%u pendingPresent=%u pendingDestination=%u pendingAdvance=%u pendingSerial=%u pendingUploads=%u pendingClear=%u requestDomain=%u slot0Present=%u slot0Destination=%u slot0Paint=%u slot0Game=%u slot0Life=%u slot0Archive=%u slot0Catalog=%u slot0Map=%u slot1Present=%u slot1Destination=%u slot1Paint=%u slot1Game=%u slot1Life=%u slot1Archive=%u slot1Catalog=%u slot1Map=%u\n",event,nfs_mtw.phase,(x.state>>4)&7u,!!(x.life&1),!!(x.life&2),!!accepted,!!(x.native&4),!!(x.native&8),!!(x.native&16),!!(x.native&32),!!(x.native&64),!!(x.pending&1),!!(x.pending&2),!!(x.pending&4),!!(x.pending&8),!!(x.pending&16),!!(x.pending&32),x.request,!!(x.completed[0]&1),!!(x.completed[0]&2),!!(x.completed[0]&4),!!(x.completed[0]&8),!!(x.completed[0]&16),!!(x.completed[0]&32),!!(x.completed[0]&64),!!(x.completed[0]&128),!!(x.completed[1]&1),!!(x.completed[1]&2),!!(x.completed[1]&4),!!(x.completed[1]&8),!!(x.completed[1]&16),!!(x.completed[1]&32),!!(x.completed[1]&64),!!(x.completed[1]&128));errno=saved;
}
static void nfs_mt_wide_retired(int owner){nfs_mtw_branding_reset();nfs_menu_map_source_invalidate(&nfs_mtw_maps);if(owner&&nfs_mtw_current()){
 if(nfs_mtw.image)glDeleteTextures(1,&nfs_mtw.image);if(!nfs_mtw_current())goto lost;
 if(nfs_mtw.result)glDeleteTextures(1,&nfs_mtw.result);if(!nfs_mtw_current())goto lost;
 if(nfs_mtw.fbo)glDeleteFramebuffers(1,&nfs_mtw.fbo);if(!nfs_mtw_current())goto lost;
 glDeleteTextures(2,nfs_mtw.art);
 }lost:memset(&nfs_mtw,0,sizeof nfs_mtw);}
static void nfs_mt_wide_created(void){nfs_mtw_branding_reset();nfs_menu_map_source_invalidate(&nfs_mtw_maps);nfs_mtw_paint=1;memset(&nfs_mtw,0,sizeof nfs_mtw);nfs_mtw.context=(uintptr_t)g_glCtx;nfs_mtw.thread=(uintptr_t)SDL_GetCurrentThreadID();nfs_mtw.generation=nfs_logo_provenance.context;}
#define NFS_MTW_GL(call) do{if(!nfs_mtw_current())goto done;call;if(!nfs_mtw_current())goto done;}while(0)
#define NFS_MTW_QUERY(call) do{if(!nfs_mtw_current())return 0;call;if(!nfs_mtw_current())return 0;}while(0)
static int nfs_mtw_draw_state(void){for(unsigned i=0;i<8;++i){NFS_MTW_QUERY(glDisable(nfs_frame_caps[i]));}NFS_MTW_QUERY(glColorMask(1,1,1,1));NFS_MTW_QUERY(glActiveTexture(GL_TEXTURE0));NFS_MTW_QUERY(glBindBuffer(GL_ARRAY_BUFFER,0));return 1;}
static int nfs_mtw_texture_params(void){NFS_MTW_QUERY(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));NFS_MTW_QUERY(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));NFS_MTW_QUERY(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE));NFS_MTW_QUERY(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE));return 1;}
static int nfs_mtw_dimensions(unsigned w,unsigned h){GLint texture=0,viewport[2]={0};if(!w||!h||w>NFS_FRAME_MAX_DIMENSION||h>NFS_FRAME_MAX_DIMENSION||(uint64_t)w*h>NFS_FRAME_MAX_PIXELS)return 0;NFS_MTW_QUERY(glGetIntegerv(GL_MAX_TEXTURE_SIZE,&texture));NFS_MTW_QUERY(glGetIntegerv(GL_MAX_VIEWPORT_DIMS,viewport));return w<=(unsigned)texture&&h<=(unsigned)texture&&w<=(unsigned)viewport[0]&&h<=(unsigned)viewport[1];}
static int nfs_mtw_attachment(GLuint color){GLint fbo=0,type=0,name=0;GLenum status=0;if(!nfs_mtw_current()||!color)return 0;NFS_MTW_QUERY(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));if((GLuint)fbo!=nfs_mtw.fbo)return 0;NFS_MTW_QUERY(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_MTW_QUERY(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));NFS_MTW_QUERY(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));return type==GL_TEXTURE&&(GLuint)name==color&&status==GL_FRAMEBUFFER_COMPLETE;}
static int nfs_mtw_image_owner(void*user,const NfsBackgroundSceneStage*t){return nfs_mtw_context_owner(user,t)&&t->framebuffer==nfs_mtw.fbo&&t->width==nfs_mtw.width&&t->height==nfs_mtw.height&&nfs_mtw_attachment(nfs_mtw.image);}
static int nfs_mtw_resources(unsigned w,unsigned h,unsigned ratio,const NfsWideMenuLayers*l){
 int ok=0,keepArt=0;
 GLuint next[4]={0},fbo=0;
 GLboolean live=0,mapLive=0,panelLive=0;
 GLenum status=0,error=0;
 if(nfs_mtw.ratio==ratio&&nfs_mtw.art[0]&&nfs_mtw.art[1]){
  NFS_MTW_GL(mapLive=glIsTexture(nfs_mtw.art[0]));
  NFS_MTW_GL(panelLive=glIsTexture(nfs_mtw.art[1]));
  keepArt=mapLive&&panelLive;
 }
 if(keepArt&&nfs_mtw.width==w&&nfs_mtw.height==h&&nfs_mtw.image&&nfs_mtw.result&&nfs_mtw.fbo){
  const GLuint ids[2]={nfs_mtw.image,nfs_mtw.result};
  for(unsigned i=0;i<2;++i){NFS_MTW_GL(live=glIsTexture(ids[i]));if(!live)goto done;}
  NFS_MTW_GL(live=glIsFramebuffer(nfs_mtw.fbo));
  if(live)return 1;
  goto done;
 }
 // A drawable resize within one artwork aspect only needs new targets.
 // Keep the old targets and artwork published until the replacement works.
 const unsigned textureCount=keepArt?2:4;
 NFS_MTW_GL(glActiveTexture(GL_TEXTURE0));
 NFS_MTW_GL(glPixelStorei(GL_UNPACK_ALIGNMENT,1));
 NFS_MTW_GL(glGenTextures(textureCount,next));
 NFS_MTW_GL(glGenFramebuffers(1,&fbo));
 if(!fbo)goto done;
 for(unsigned i=0;i<textureCount;++i){
  if(!next[i])goto done;
  NFS_MTW_GL(glBindTexture(GL_TEXTURE_2D,next[i]));
  if(!nfs_mtw_texture_params())goto done;
  NFS_MTW_GL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,i<2?w:l->width,i<2?h:l->height,0,
                         GL_RGBA,GL_UNSIGNED_BYTE,i<2?NULL:i==2?l->map:l->panel));
  if(glGetError()!=GL_NO_ERROR)goto done;
 }
 NFS_MTW_GL(glBindFramebuffer(GL_FRAMEBUFFER,fbo));
 NFS_MTW_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,next[0],0));
 NFS_MTW_GL(status=glCheckFramebufferStatus(GL_FRAMEBUFFER));
 NFS_MTW_GL(error=glGetError());
 if(status!=GL_FRAMEBUFFER_COMPLETE||error!=GL_NO_ERROR)goto done;
 if(nfs_mtw.image){NFS_MTW_GL(glDeleteTextures(1,&nfs_mtw.image));}
 if(nfs_mtw.result){NFS_MTW_GL(glDeleteTextures(1,&nfs_mtw.result));}
 if(!keepArt){NFS_MTW_GL(glDeleteTextures(2,nfs_mtw.art));}
 if(nfs_mtw.fbo){NFS_MTW_GL(glDeleteFramebuffers(1,&nfs_mtw.fbo));}

 nfs_mtw_static_invalidate();
 if(nfs_mtw.resourceEpoch!=UINT64_MAX)++nfs_mtw.resourceEpoch;
 nfs_mtw.image=next[0];nfs_mtw.result=next[1];
 if(!keepArt){nfs_mtw.art[0]=next[2];nfs_mtw.art[1]=next[3];}
 nfs_mtw.fbo=fbo;nfs_mtw.width=w;nfs_mtw.height=h;nfs_mtw.ratio=ratio;ok=1;
 done:
 if(!ok&&nfs_mtw_current()){
  glDeleteTextures(4,next);
  if(nfs_mtw_current()&&fbo)glDeleteFramebuffers(1,&fbo);
 }
 return ok&&nfs_mtw_current();
}
static int nfs_mtw_layer(unsigned role){const GLfloat q[]={0,0,0,0,nfs_mtw.logical,0,1,0,0,480,0,1,0,480,0,1,nfs_mtw.logical,0,1,0,nfs_mtw.logical,480,1,1};NFS_MTW_QUERY(glUniform1i(nfs_bg_compositor.sampler,0));NFS_MTW_QUERY(glUniform1f(nfs_bg_compositor.opacity,1));NFS_MTW_QUERY(glActiveTexture(GL_TEXTURE0));NFS_MTW_QUERY(glBindTexture(GL_TEXTURE_2D,nfs_mtw.art[role]));NFS_MTW_QUERY(glEnableVertexAttribArray(0));for(unsigned i=1;i<4;++i)NFS_MTW_QUERY(glDisableVertexAttribArray(i));NFS_MTW_QUERY(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,q));if(role){NFS_MTW_QUERY(glEnable(GL_BLEND));NFS_MTW_QUERY(glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD));NFS_MTW_QUERY(glBlendFuncSeparate(GL_ONE,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA));}else NFS_MTW_QUERY(glDisable(GL_BLEND));NFS_MTW_QUERY(glDrawArrays(GL_TRIANGLES,0,6));return 1;}
static int nfs_mtw_bands(GLuint image,const NfsTransitionMotion*motion,GLint dstRgb,GLint dstAlpha){
 NfsTransitionBand bands[20];if(!nfs_transition_bands(motion,nfs_mtw.width,nfs_mtw.height,bands)||!nfs_mtw_attachment(nfs_mtw.result)||image==nfs_mtw.result)return 0;
 NFS_MTW_QUERY(glUseProgram(nfs_mt.program));NFS_MTW_QUERY(glUniformMatrix4fv(nfs_mt.matrix,1,GL_FALSE,nfs_mtw.matrix));NFS_MTW_QUERY(glUniform1i(nfs_mt.sampler,0));NFS_MTW_QUERY(glActiveTexture(GL_TEXTURE0));NFS_MTW_QUERY(glBindTexture(GL_TEXTURE_2D,image));NFS_MTW_QUERY(glEnableVertexAttribArray(0));NFS_MTW_QUERY(glEnableVertexAttribArray(1));NFS_MTW_QUERY(glDisableVertexAttribArray(2));NFS_MTW_QUERY(glDisableVertexAttribArray(3));NFS_MTW_QUERY(glEnable(GL_BLEND));NFS_MTW_QUERY(glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD));NFS_MTW_QUERY(glBlendFuncSeparate(GL_ONE,dstRgb,GL_ONE,dstAlpha));
 for(unsigned i=0;i<20;++i){const NfsTransitionBand*b=bands+i;const GLfloat q[]={b->left,b->top,b->u0,1,b->right,b->top,b->u1,1,b->left,b->bottom,b->u0,0,b->left,b->bottom,b->u0,0,b->right,b->top,b->u1,1,b->right,b->bottom,b->u1,0};const GLfloat gray[]={b->grayLeft,b->grayRight,b->grayLeft,b->grayLeft,b->grayRight,b->grayRight};NFS_MTW_QUERY(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,q));NFS_MTW_QUERY(glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,gray));NFS_MTW_QUERY(glDrawArrays(GL_TRIANGLES,0,6));}return 1;
}
static int nfs_mtw_draw_layers(const NfsBackgroundSceneLayers*layers,const NfsBackgroundSceneStage*stage){
 NfsLogoVertex logos[18];float bounds[4];size_t count=0;
 if(!layers||!stage||!nfs_mtw_image_owner(&nfs_mtw,stage)||!nfs_bg_stage_layers(layers)||!nfs_logo_layers(layers->logos,3,logos,&count,bounds))return 0;
 NFS_MTW_QUERY(glUseProgram(nfs_bg_compositor.program));NFS_MTW_QUERY(glUniformMatrix4fv(nfs_bg_compositor.matrix,1,GL_FALSE,nfs_mtw.matrix));
 if(!nfs_mtw_layer(1))return 0;
 return nfs_bg_stage_decoration(&nfs_bg_compositor,stage,layers,logos,count)
     &&nfs_mtw_image_owner(&nfs_mtw,stage);
}
#include "WideTransitionPreviewRenderer.inc"
#include "WideTransitionCaptionRenderer.inc"
#include "WideTransitionBrandingRenderer.inc"
#include "WideTransitionTailRenderer.inc"
/* Bounded source-owned scalar diagnostics; never pixels, paths or guest reads. */
static int nfs_mtw_static_key(const NfsMenuTransitionView*v,const NfsBackgroundSceneStage*t,NfsMtwStaticKey*k){
 NfsBackgroundSceneFrameView scene;NfsNativeLogoTargetProof target;const NfsCompactCaptionWordView*w=&v->childInitial.caption;const NfsCompactCaptionGpu*g=&nfs_mt.childCaption;
 if(v->kind!=8||!nfs_mtw.resourceEpoch||nfs_mtw.resourceEpoch==UINT64_MAX)return 0;
 memset(k,0,sizeof *k);memset(&scene,0,sizeof scene);memset(&target,0,sizeof target);scene.mapRole=v->layers.mapRole;scene.wordCount=v->layers.wordCount;scene.tailKind=8;memcpy(scene.panelClip,v->layers.panelClip,sizeof scene.panelClip);memcpy(scene.words,v->layers.words,sizeof scene.words);memcpy(scene.logos,v->layers.logos,sizeof scene.logos);target.width=t->width;target.height=t->height;target.sampling=t->sampling;nfs_bg_key(&nfs_bg_compositor,0,&scene,&target,&k->background);k->background.color=nfs_mtw.image;k->background.fbo=nfs_mtw.fbo;
 memcpy(k->regions,nfs_bg_compositor.regions,sizeof k->regions);k->context=nfs_mtw.context;k->thread=nfs_mtw.thread;k->game=v->sourceGame;k->catalog=v->sourceCatalog;k->life=v->sourceLife;k->epoch=v->childInitial.ownerEpoch;k->archive=v->sourceArchive;k->form=v->childInitial.form;k->kind=v->kind;k->ratio=nfs_mtw.ratio;k->resourceEpoch=nfs_mtw.resourceEpoch;memcpy(k->wideArt,nfs_mtw.art,sizeof k->wideArt);k->logical=nfs_mtw.logical;k->captionMatrix=g->matrix;k->captionSampler=g->sampler;
 k->background.captionWordProgram=g->program;k->background.captionWordTexture=g->texture;k->background.captionWordSchema=w->schema;k->background.captionWordId=w->word;k->background.captionWordWidth=w->width;k->background.captionWordHeight=w->height;k->background.captionWordBytes=w->bytes;memcpy(k->background.captionWordSha256,w->sha256,32);k->background.captionWordLeft=w->left;k->background.captionWordTop=w->top;k->background.captionWordRight=w->right;k->background.captionWordBottom=w->bottom;return 1;
}
static void nfs_mtw_static_trace(int hit){static unsigned lines=0;const int saved=errno;if(nfs_logo_trace_enabled()&&lines<64){++lines;fprintf(stderr,"MENU TRANSITION IMAGE CACHE kind=8 hit=%u ready=%u resourceEpoch=%llu hits=%llu misses=%llu invalidations=%llu\n",!!hit,nfs_mtw.staticReady,(unsigned long long)nfs_mtw.resourceEpoch,(unsigned long long)nfs_mtw.cacheHits,(unsigned long long)nfs_mtw.cacheMisses,(unsigned long long)nfs_mtw.cacheInvalidations);}errno=saved;}
static void nfs_mtw_begin_trace(const NfsMenuTransitionView*v,unsigned reason){
 if(reason)nfs_mtw_static_invalidate();
 static struct{unsigned kind,reason;GLint rgb,alpha;}seen[32];static unsigned used;
 const int saved=errno;if(!v||(v->kind<1||v->kind>11)||!nfs_logo_trace_enabled()||used>=32){errno=saved;return;}
 for(unsigned i=0;i<used;++i)if(seen[i].kind==v->kind&&seen[i].reason==reason&&seen[i].rgb==nfs_mt.nativeDestRgb&&seen[i].alpha==nfs_mt.nativeDestAlpha){errno=saved;return;}
 seen[used].kind=v->kind;seen[used].reason=reason;seen[used].rgb=nfs_mt.nativeDestRgb;seen[used].alpha=nfs_mt.nativeDestAlpha;++used;
 fprintf(stderr,"MENU TRANSITION WIDE begin kind=%u dstRgb=%d dstAlpha=%d reason=%u\n",v->kind,nfs_mt.nativeDestRgb,nfs_mt.nativeDestAlpha,reason);errno=saved;
}
/* Reasons:0prepared,1source/target,2fullgeometry,3additive-underlay,
 *4aspect,5artsource,6staging,7preview,8caption,9layers. */
static void nfs_mt_wide_begin(const NfsMenuTransitionView*v,const NfsNativeLogoTargetProof*t){
 NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_mtw_guard();GLfloat clear[4];NfsWideMenuLayers l={0};NfsBackgroundSceneStage drawStage;NfsMenuMapSource underlay;unsigned hasUnderlay=0,blackSidebar=0;int w=0,h=0,ok=0;unsigned decline=6;NfsMtwStaticKey staticKey;int staticEligible=0,staticHit=0;
 nfs_mt_wide_cancel();if(!v||v->kind!=8)nfs_mtw_static_invalidate();if(!v||!t||(v->kind!=1&&v->kind!=3&&(v->kind<4||v->kind>11))||!nfs_menu_transition_source_life_current(v->sourceGame,v->sourceLife)||!nfs_mtw_current()||!nfs_mt_current()||!nfs_mt.active||nfs_mt.serial!=v->serial||nfs_mt.sourceKind!=v->kind||nfs_mt.slot!=v->slot||nfs_mt.drawKind!=v->drawKind||memcmp(t,&nfs_mt.target,sizeof *t)||!nfs_bg_ready(&nfs_bg_compositor)||!sdlWin||!SDL_GetWindowSizeInPixels(sdlWin,&w,&h)||w<=0||h<=0||!nfs_mtw_dimensions((unsigned)w,(unsigned)h)){nfs_mtw_begin_trace(v,1);return;}
 const int initialFull=v->kind>=8&&v->kind<=10;const int typedSidebar=(v->kind>=5&&v->kind<=7)||v->kind==11;
 if(v->kind==11&&(v->slot||v->drawKind!=2||v->originX||v->originY||v->stepX!=32||!nfs_mt_player_valid(&v->playerInitial))){nfs_mtw_begin_trace(v,2);return;}
 if(initialFull&&(v->slot!=1||v->drawKind!=2||v->originX||v->originY||v->stepX!=32)){nfs_mtw_begin_trace(v,2);return;}
 if(initialFull&&nfs_mt.nativeDestRgb==GL_ONE){nfs_mtw_begin_trace(v,3);return;}
 if(v->kind==4||typedSidebar){NfsMenuSourcePaintKey key=nfs_mtw_key(t);const NfsMenuMapSource*m=nfs_menu_map_source_acquire_destination(&nfs_mtw_maps,&key,v->sourceGame,v->sourceLife,v->sourceArchive,v->sourceCatalog);
  /* A sidebar contains no map pixels: its placeholder mapRole is not
     authority for the already witnessed destination. Keep the producer
     receipt's own authenticated map role and reject ambiguous ownership. */
  if(m&&m->sourceKind==(typedSidebar&&!v->locationInfoReturn?3u:1u)){underlay=*m;hasUnderlay=1;}
  else if(!typedSidebar&&nfs_logo_black_clear.valid&&nfs_logo_black_clear.context==nfs_mtw.generation&&nfs_logo_black_clear.epoch==nfs_logo_provenance.clearEvent&&nfs_logo_black_clear.fbo==t->framebuffer)blackSidebar=1;
  else{nfs_mtw_map_trace(8,0,&key,v);nfs_mtw_begin_trace(v,11);return;}
 }
 else if(!initialFull&&(!nfs_logo_black_clear.valid||nfs_logo_black_clear.context!=nfs_mtw.generation||nfs_logo_black_clear.epoch!=nfs_logo_provenance.clearEvent||nfs_logo_black_clear.fbo!=t->framebuffer))return;
 const float logical=480.f*w/h;if(logical<=640.01f||logical>854.f){nfs_mtw_begin_trace(v,4);return;}
 if(v->kind!=1&&!nfs_bg_stage_layers(&v->layers)){nfs_mtw_begin_trace(v,9);return;}
 const unsigned ratio=nfs_wide_menu_ratio(logical);if(!nfs_wide_menu_layers(ratio,&l)||!l.map||!l.panel){nfs_mtw_begin_trace(v,5);return;}
 if(!nfs_graphics_scene_save(&saved,&guard)){nfs_mtw_begin_trace(v,10);return;}glGetFloatv(GL_COLOR_CLEAR_VALUE,clear);if(!nfs_mtw_resources((unsigned)w,(unsigned)h,ratio,&l))goto done;
 nfs_mtw.logical=logical;memset(nfs_mtw.matrix,0,sizeof nfs_mtw.matrix);nfs_mtw.matrix[0]=2/logical;nfs_mtw.matrix[5]=-2.f/480;nfs_mtw.matrix[10]=-2;nfs_mtw.matrix[12]=-1;nfs_mtw.matrix[13]=nfs_mtw.matrix[14]=nfs_mtw.matrix[15]=1;
 drawStage=guard;drawStage.framebuffer=nfs_mtw.fbo;drawStage.width=nfs_mtw.width;drawStage.height=nfs_mtw.height;drawStage.sampling=t->sampling;memcpy(drawStage.sampling.projection,nfs_mtw.matrix,sizeof nfs_mtw.matrix);drawStage.sampling.viewport[0]=drawStage.sampling.viewport[1]=0;drawStage.sampling.viewport[2]=w;drawStage.sampling.viewport[3]=h;drawStage.ownerCurrent=nfs_mtw_image_owner;
 NFS_MTW_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_mtw.fbo));NFS_MTW_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,nfs_mtw.image,0));if(!nfs_mtw_attachment(nfs_mtw.image))goto done;
 if(v->kind==8){if(!nfs_mtw_initial_caption_impl(v,&drawStage,0)){decline=8;goto done;}staticEligible=nfs_mtw_static_key(v,&drawStage,&staticKey);staticHit=staticEligible&&nfs_mtw.staticReady&&!memcmp(&staticKey,&nfs_mtw.staticKey,sizeof staticKey);}
 if(staticHit){ok=glGetError()==GL_NO_ERROR;goto done;}
 nfs_mtw_static_invalidate();
 NFS_MTW_GL(glViewport(0,0,w,h));if(!nfs_mtw_draw_state())goto done;NFS_MTW_GL(glDepthMask(GL_FALSE));/* An actually witnessed black destination needs only the transparent
    sidebar source. A source-map receipt instead reconstructs its map below. */
 NFS_MTW_GL(glClearColor(0,0,0,blackSidebar?0:1));NFS_MTW_GL(glClear(GL_COLOR_BUFFER_BIT));NFS_MTW_GL(glUseProgram(nfs_bg_compositor.program));NFS_MTW_GL(glUniformMatrix4fv(nfs_bg_compositor.matrix,1,GL_FALSE,nfs_mtw.matrix));if(!blackSidebar&&!nfs_mtw_layer(0))goto done;
 if(hasUnderlay){
  if(underlay.sourceKind==3&&!nfs_mtw_draw_layers(&underlay.layers,&drawStage))goto done;
  NfsTransitionMotion old={underlay.minimum,underlay.maximum,underlay.direction,underlay.phase,underlay.originX,underlay.originY,underlay.stepX};
  NFS_MTW_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,nfs_mtw.result,0));if(!nfs_mtw_attachment(nfs_mtw.result))goto done;
  NFS_MTW_GL(glClearColor(0,0,0,1));NFS_MTW_GL(glClear(GL_COLOR_BUFFER_BIT));if(!nfs_mtw_bands(nfs_mtw.image,&old,GL_ZERO,GL_ZERO))goto done;
  NFS_MTW_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,nfs_mtw.image,0));if(!nfs_mtw_attachment(nfs_mtw.image))goto done;
  NFS_MTW_GL(glClearColor(0,0,0,0));NFS_MTW_GL(glClear(GL_COLOR_BUFFER_BIT));NFS_MTW_GL(glUseProgram(nfs_bg_compositor.program));NFS_MTW_GL(glUniformMatrix4fv(nfs_bg_compositor.matrix,1,GL_FALSE,nfs_mtw.matrix));
 }
 if(initialFull){if(!nfs_mtw_initial_preview(v,&drawStage)){decline=7;goto done;}NFS_MTW_GL(glUseProgram(nfs_bg_compositor.program));NFS_MTW_GL(glUniformMatrix4fv(nfs_bg_compositor.matrix,1,GL_FALSE,nfs_mtw.matrix));}
 if(v->kind!=1&&!nfs_mtw_draw_layers(&v->layers,&drawStage))goto done;
 if((initialFull||typedSidebar)&&!nfs_mtw_initial_caption(v,&drawStage)){decline=8;goto done;}
 ok=glGetError()==GL_NO_ERROR;
 done:if(!ok)nfs_mtw_static_invalidate();if(ok&&v->kind==8&&staticEligible){nfs_mtw.staticKey=staticKey;nfs_mtw.staticReady=1;if(staticHit){if(nfs_mtw.cacheHits!=UINT64_MAX)++nfs_mtw.cacheHits;}else if(nfs_mtw.cacheMisses!=UINT64_MAX)++nfs_mtw.cacheMisses;nfs_mtw_static_trace(staticHit);}if(!nfs_mtw_current()){memset(&nfs_mtw,0,sizeof nfs_mtw);return;}glClearColor(clear[0],clear[1],clear[2],clear[3]);if(!nfs_mtw_current()){memset(&nfs_mtw,0,sizeof nfs_mtw);return;}if(!nfs_graphics_scene_restore(&saved,&guard)){memset(&nfs_mtw,0,sizeof nfs_mtw);return;}if(!ok||!nfs_mtw_current()){nfs_mtw_begin_trace(v,decline);return;}
 nfs_mtw.sourceGame=v->sourceGame;nfs_mtw.sourceLife=v->sourceLife;nfs_mtw.sourceCatalog=v->sourceCatalog;nfs_mtw.sourceArchive=v->sourceArchive;nfs_mtw.initialForm=v->kind==5?v->childInitial.form:v->kind==6?v->opponentInitial.form:v->kind==7?v->locationInitial.form:v->kind==11?v->playerInitial.form:0;nfs_mtw.initialEpoch=v->kind==5?v->childInitial.ownerEpoch:v->kind==6?v->opponentInitial.ownerEpoch:v->kind==7?v->locationInitial.ownerEpoch:v->kind==11?v->playerInitial.ownerEpoch:0;nfs_mtw.serial=v->serial;nfs_mtw.content=v->content;nfs_mtw.kind=v->kind;nfs_mtw.nativeHeight=t->height;nfs_mtw.ownerForm=v->responsiveForm;nfs_mtw.descriptor=v->responsiveDescriptor;nfs_mtw.producerEpoch=v->responsiveProducerEpoch;nfs_mtw.cached=v->responsive;nfs_mtw.nativeFbo=t->framebuffer;nfs_mtw.nativeColor=g_framebufferTexture;nfs_mtw.nativeDepth=g_framebufferDepth;nfs_mtw.uploads=nfs_logo_provenance.uploads;nfs_mtw.clearEvent=nfs_logo_provenance.clearEvent;nfs_mtw.motion=(NfsTransitionMotion){v->minimum,v->maximum,v->direction,v->phase,v->originX,v->originY,v->stepX};nfs_mtw.underlayValid=hasUnderlay;nfs_mtw.locationInfoReturn=v->locationInfoReturn;if(hasUnderlay)nfs_mtw.underlay=underlay;
 if(v->kind==1||v->kind==3){NfsMenuMapSource m;memset(&m,0,sizeof m);m.key=nfs_mtw_key(t);m.sourceGame=v->sourceGame;m.sourceLife=v->sourceLife;m.sourceCatalog=v->sourceCatalog;m.sourceArchive=v->sourceArchive;m.sourceSerial=v->serial;m.sourceContent=v->content;m.uploads=nfs_mtw.uploads;m.clearEvent=nfs_mtw.clearEvent;m.mapRole=v->mapRole;m.sourceKind=v->kind;if(v->kind==3)m.layers=v->layers;m.drawKind=v->drawKind;m.minimum=v->minimum;m.maximum=v->maximum;m.direction=v->direction;m.phase=v->phase;m.originX=v->originX;m.originY=v->originY;m.stepX=v->stepX;if(!nfs_texture_sha256((const unsigned char*)v->original565,640u*480u*2u,m.originalSha256)||!nfs_menu_map_source_begin(&nfs_mtw_maps,&m)){nfs_mtw_begin_trace(v,12);return;}}
 nfs_mtw.locationArtGeneration=v->kind==9?v->initialFullPreview.locationArtGeneration:0;nfs_mtw.phase=1;nfs_mtw_begin_trace(v,0);
}
static void nfs_mt_wide_finish(uint64_t serial,int accepted){
 NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_mtw_guard();GLfloat clear[4];NfsTransitionBand bands[20];int ok=0;
 if(nfs_mt.sourceKind==1)nfs_mtw_map_trace(1,accepted,NULL,NULL);
 /* Native paint happened even if optional wide preparation declined. */
 if(!accepted||nfs_mtw.phase!=1||serial!=nfs_mtw.serial){if(nfs_mt.sourceKind==1)nfs_mtw_map_trace(2,accepted,NULL,NULL);nfs_mt_wide_other();return;}
 if(!accepted||!nfs_mtw_art_current()||!nfs_menu_transition_source_life_current(nfs_mtw.sourceGame,nfs_mtw.sourceLife)||nfs_mtw.phase!=1||serial!=nfs_mtw.serial||!nfs_mtw_current()||nfs_mt.selects!=300||nfs_mt.triangles!=600||nfs_mt.serial!=serial||nfs_logo_provenance.uploads!=nfs_mtw.uploads||nfs_logo_provenance.clearEvent!=nfs_mtw.clearEvent||!nfs_transition_bands(&nfs_mtw.motion,nfs_mtw.width,nfs_mtw.height,bands)){if(nfs_mt.sourceKind==1)nfs_mtw_map_trace(2,accepted,NULL,NULL);nfs_mt_wide_other();return;}
 if(nfs_mtw_paint==UINT64_MAX){nfs_mt_wide_other();return;}++nfs_mtw_paint;
 if(nfs_mtw.kind==1||nfs_mtw.kind==3){NfsMenuSourcePaintKey key=nfs_mtw_key(&nfs_mt.target);NfsMenuMapNativeWitness witness={serial,nfs_logo_provenance.uploads,nfs_logo_provenance.clearEvent,nfs_mt.selects,nfs_mt.triangles};nfs_mtw_map_trace(10,accepted,&key,NULL);if(!nfs_menu_map_source_finish(&nfs_mtw_maps,&key,&witness)){nfs_mtw_map_trace(4,accepted,&key,NULL);nfs_mt_wide_cancel();return;}nfs_mtw_map_trace(3,accepted,&key,NULL);}
 if(!nfs_graphics_scene_save(&saved,&guard)){if(nfs_mt.sourceKind==1)nfs_mtw_map_trace(5,accepted,NULL,NULL);return;}glGetFloatv(GL_COLOR_CLEAR_VALUE,clear);NFS_MTW_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_mtw.fbo));NFS_MTW_GL(glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,nfs_mtw.result,0));if(!nfs_mtw_attachment(nfs_mtw.result))goto done;
 NFS_MTW_GL(glViewport(0,0,nfs_mtw.width,nfs_mtw.height));if(!nfs_mtw_draw_state())goto done;NFS_MTW_GL(glDepthMask(GL_FALSE));
 if(!nfs_mtw.underlayValid){NFS_MTW_GL(glClearColor(0,0,0,1));NFS_MTW_GL(glClear(GL_COLOR_BUFFER_BIT));}
 if(!nfs_mtw_bands(nfs_mtw.image,&nfs_mtw.motion,nfs_mt.nativeDestRgb,nfs_mt.nativeDestAlpha))goto done;
 ok=glGetError()==GL_NO_ERROR;
 done:if(!nfs_mtw_current()){memset(&nfs_mtw,0,sizeof nfs_mtw);return;}glClearColor(clear[0],clear[1],clear[2],clear[3]);if(!nfs_mtw_current()){memset(&nfs_mtw,0,sizeof nfs_mtw);return;}if(!nfs_graphics_scene_restore(&saved,&guard)){memset(&nfs_mtw,0,sizeof nfs_mtw);return;}if(!ok||!nfs_mtw_current()||!nfs_mtw_art_current()){if(nfs_mt.sourceKind==1)nfs_mtw_map_trace(7,accepted,NULL,NULL);nfs_mt_wide_cancel();return;}nfs_mtw.phase=2;if(nfs_mt.sourceKind==1)nfs_mtw_map_trace(6,accepted,NULL,NULL);
}
static GLuint nfs_mt_wide_present(GLuint frontFbo,GLuint frontTexture,int w,int h){
 GLint prior=0,type=0,name=0,depthType=0,depth=0;int ok=0;
 uint64_t epoch=0;float x=0,y=0;
 if(nfs_mtw.phase!=2){nfs_mt_wide_cancel();return 0;}
 if(!nfs_mtw_art_current()){nfs_mt_wide_cancel();return 0;}
 if(((nfs_mtw.kind>=5&&nfs_mtw.kind<=7)||nfs_mtw.kind==11)&&!nfs_mtw_branding_ready()){nfs_mt_wide_cancel();return 0;}
 if(nfs_mtw.kind>=8&&nfs_mtw.kind<=10&&(!nfs_mtw_tail_source(&epoch,&x,&y)||nfs_mtw.pendingTriangles||!nfs_mtw.tailTriangles||!nfs_mtw.cursorSeen)){nfs_mt_wide_cancel();return 0;}
 if(!nfs_menu_transition_source_life_current(nfs_mtw.sourceGame,nfs_mtw.sourceLife)||nfs_mtw.phase!=2||!nfs_mtw_current()||w!=(int)nfs_mtw.width||h!=(int)nfs_mtw.height||frontFbo!=nfs_mtw.nativeFbo||frontTexture!=nfs_mtw.nativeColor||nfs_logo_provenance.clearEvent!=nfs_mtw.clearEvent){nfs_mt_wide_cancel();return 0;}
 NFS_MTW_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prior));NFS_MTW_GL(glBindFramebuffer(GL_FRAMEBUFFER,frontFbo));NFS_MTW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));NFS_MTW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));NFS_MTW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&depthType));NFS_MTW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&depth));ok=type==GL_TEXTURE&&(GLuint)name==frontTexture&&depthType==GL_RENDERBUFFER&&(GLuint)depth==nfs_mtw.nativeDepth&&glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;NFS_MTW_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_mtw.fbo));ok=ok&&nfs_mtw_attachment(nfs_mtw.result);if(!nfs_mtw_current())goto done;ok=ok&&glIsTexture(nfs_mtw.result);if(!nfs_mtw_current())goto done;NFS_MTW_GL(glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)prior));
 done:if(!ok||!nfs_mtw_current()||!nfs_mtw_art_current()||glGetError()!=GL_NO_ERROR){nfs_mt_wide_cancel();return 0;}nfs_mtw.phase=3;return nfs_mtw.result;
}
static void nfs_mt_wide_after_present(int success){
 if(success&&nfs_mtw_art_current()&&nfs_mtw.phase==3&&nfs_mtw.kind>=8&&nfs_mtw.kind<=10&&nfs_mtw_current()){
  uint64_t epoch=0;float x=0,y=0;if(nfs_mtw_tail_source(&epoch,&x,&y)&&nfs_menu_wide.revision<UINT64_MAX){NfsMenuResponsiveLayout v;memset(&v,0,sizeof v);v.schema=v.active=1;v.revision=++nfs_menu_wide.revision;v.ownerEpoch=epoch;v.drawableWidth=nfs_mtw.width;v.drawableHeight=nfs_mtw.height;v.logicalWidth=nfs_mtw.logical;v.delta=nfs_mtw.logical-640;v.sourceCursorX=x;v.sourceCursorY=y;v.cursorValid=1;nfs_menu_cached_publish(&v,&nfs_mtw.cached);}
 }
 static unsigned lines[12]={0};if(success&&nfs_mtw_art_current()&&nfs_mtw.phase==3&&nfs_mtw_current()&&nfs_mtw.kind<12&&nfs_logo_trace_enabled()&&lines[nfs_mtw.kind]<256){++lines[nfs_mtw.kind];fprintf(stderr,"MENU TRANSITION WIDE presented kind=%u serial=%llu output=%ux%u phase=%d step=%d fresh=1\n",nfs_mtw.kind,(unsigned long long)nfs_mtw.serial,nfs_mtw.width,nfs_mtw.height,nfs_mtw.motion.phase,nfs_mtw.motion.stepX);}nfs_mt_wide_cancel();
}
#undef NFS_MTW_QUERY
#undef NFS_MTW_GL

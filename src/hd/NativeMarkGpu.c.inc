// SPDX-License-Identifier: MIT
#include "NativeMarkGpu.h"
#include "TextureDigest.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
typedef struct {
 GLuint texture;NfsNativeMarkArt art;uint64_t generation,uploads;unsigned heights[8];
} NfsNativeMarkTexture;
typedef struct {
 uint32_t address,nativeId,texture,encoding,edge,preparedBytes;uint64_t generation,revision;int valid;
} NfsNativeMarkBinding;
static struct {
 uintptr_t context;GLuint program;uint64_t generation,revision;
 NfsNativeMarkTexture textures[NFS_NATIVE_MARK_MAX_TEXTURES];
 NfsNativeMarkBinding bindings[NFS_NATIVE_MARK_MAX_BINDINGS];
 GLint sampler,enable,priorUnit2;int selected,sent,restoreUnit2,uniformsReady,disabled;
 unsigned logs;
} nfs_native_marks={.sampler=-1,.enable=-1,.selected=-1};
static int nfs_mark_diagnostics(void){static int checked,enabled;if(!checked){const char*p=getenv("NFS_PRIVATE_BACKGROUND_SCENE_TRACE");enabled=p&&p[0]=='1'&&!p[1];checked=1;}return enabled;}
static void nfs_mark_log(const char*event,NfsNativeMarkTexture*t,const NfsNativeMarkBinding*b,int height){
 // Bound creation, eight supported draw heights and retirement per texture.
 if(!nfs_mark_diagnostics()||nfs_native_marks.logs>=NFS_NATIVE_MARK_MAX_TEXTURES*10u)return;++nfs_native_marks.logs;
 fprintf(stderr,"NATIVE MARK %s context=%" PRIu64 " identity=%" PRIu64 " address=%u nativeId=%u artId=%u height=%d matchingUploads=%" PRIu64 "\n",event,nfs_native_marks.generation,t?t->art.identity:0,b?b->address:0,b?b->nativeId:0,t?t->texture:0,height,t?t->uploads:0);
}
static int nfs_mark_owner(void){return SDL_IsMainThread()&&nfs_native_marks.context&&
 (uintptr_t)SDL_GL_GetCurrentContext()==nfs_native_marks.context;}
static void nfs_mark_clear_bindings(void){memset(nfs_native_marks.bindings,0,sizeof(nfs_native_marks.bindings));nfs_native_marks.selected=-1;}
static void nfs_native_mark_gpu_created(uintptr_t context,uint32_t program){
 nfs_mark_clear_bindings();memset(nfs_native_marks.textures,0,sizeof(nfs_native_marks.textures));
 nfs_native_marks.context=context;nfs_native_marks.program=program;
 nfs_native_marks.sampler=nfs_native_marks.enable=-1;nfs_native_marks.sent=0;
 nfs_native_marks.restoreUnit2=nfs_native_marks.uniformsReady=0;
 if(nfs_native_marks.generation==UINT64_MAX){nfs_native_marks.disabled=1;return;}
 ++nfs_native_marks.generation;
}
static void nfs_native_mark_gpu_retired(int currentOwned){
 if(currentOwned&&nfs_mark_owner())for(unsigned k=0;k<NFS_NATIVE_MARK_MAX_TEXTURES;++k){
  NfsNativeMarkTexture*t=&nfs_native_marks.textures[k];if(t->texture){nfs_mark_log("resource-retire",t,NULL,0);glDeleteTextures(1,&t->texture);}
 }
 memset(nfs_native_marks.textures,0,sizeof(nfs_native_marks.textures));nfs_mark_clear_bindings();
 nfs_native_marks.context=0;nfs_native_marks.program=0;nfs_native_marks.sampler=nfs_native_marks.enable=-1;
 nfs_native_marks.sent=nfs_native_marks.restoreUnit2=nfs_native_marks.uniformsReady=0;
}
static void nfs_native_mark_gpu_invalidate(uint32_t address,uint64_t bytes){
 nfs_native_marks.selected=-1;
 if(bytes>UINT64_MAX-(uint64_t)address)bytes=UINT64_MAX-(uint64_t)address;
 for(unsigned k=0;k<NFS_NATIVE_MARK_MAX_BINDINGS;++k){NfsNativeMarkBinding*b=&nfs_native_marks.bindings[k];
  if(b->valid&&(uint64_t)address<(uint64_t)b->address+b->preparedBytes&&
     (uint64_t)b->address<(uint64_t)address+bytes)b->valid=0;
 }
}
static int nfs_mark_art_shape(const NfsNativeMarkArt*a,const NfsNativeMarkProfile*p){
 if(!a->pixels||a->bytes!=p->artBytes||a->width!=p->artEdge||a->height!=p->artEdge||
    a->logicalWidth!=p->logicalEdge||a->logicalHeight!=p->logicalEdge||a->schema!=NFS_NATIVE_MARK_ART_SCHEMA||
    a->premultiplied!=1||a->opacityBaked||!a->identity)return 0;
 unsigned nonzero=0;for(unsigned k=0;k<32;++k)nonzero|=a->sha256[k];return nonzero!=0;
}
static int nfs_mark_prepare(const NfsNativeMarkArt*a,const NfsNativeMarkProfile*p){
 if(!nfs_mark_art_shape(a,p))return -1;
 int empty=-1;
 for(unsigned k=0;k<NFS_NATIVE_MARK_MAX_TEXTURES;++k){NfsNativeMarkTexture*t=&nfs_native_marks.textures[k];
  if(!t->texture){if(empty<0)empty=(int)k;continue;}
  if(t->art.identity==a->identity)return t->generation==nfs_native_marks.generation&&
   !memcmp(t->art.sha256,a->sha256,32)&&t->art.bytes==a->bytes&&
   t->art.width==a->width&&t->art.height==a->height&&
   t->art.logicalWidth==a->logicalWidth&&t->art.logicalHeight==a->logicalHeight?(int)k:-1;
 }
 if(empty<0)return -1;
 unsigned char actual[32];if(!nfs_texture_sha256(a->pixels,a->bytes,actual)||memcmp(actual,a->sha256,32))return -1;
 for(size_t k=0;k<a->bytes;k+=4)if(a->pixels[k]>a->pixels[k+3]||a->pixels[k+1]>a->pixels[k+3]||a->pixels[k+2]>a->pixels[k+3])return -1;
 /* Fail closed on preexisting/optional GL errors. Error flags cannot be restored. */
 if(glGetError()!=GL_NO_ERROR)return -1;
 GLint size=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&size);
 if(size<(GLint)p->artEdge||glGetError()!=GL_NO_ERROR)return -1;
 if(!nfs_native_marks.uniformsReady){
  GLint units=0;glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&units);
  const GLint sampler=glGetUniformLocation(nfs_native_marks.program,"uMarkTextureSampler");
  const GLint enable=glGetUniformLocation(nfs_native_marks.program,"uMarkActive");
  if(units<3||size<(GLint)p->artEdge||sampler<0||enable<0||glGetError()!=GL_NO_ERROR)return -1;
  nfs_native_marks.sampler=sampler;nfs_native_marks.enable=enable;nfs_native_marks.uniformsReady=1;
 }
 GLint active=0,binding=0,unpack=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glActiveTexture(GL_TEXTURE2);
 glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding);glGetIntegerv(GL_UNPACK_ALIGNMENT,&unpack);glPixelStorei(GL_UNPACK_ALIGNMENT,1);
 GLuint id=0;glGenTextures(1,&id);glBindTexture(GL_TEXTURE_2D,id);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,(GLsizei)a->width,(GLsizei)a->height,0,GL_RGBA,GL_UNSIGNED_BYTE,a->pixels);
 const int okay=id&&glGetError()==GL_NO_ERROR;
 if(!okay&&id)glDeleteTextures(1,&id);
 glPixelStorei(GL_UNPACK_ALIGNMENT,unpack);glBindTexture(GL_TEXTURE_2D,(GLuint)binding);glActiveTexture((GLenum)active);
 if(!okay)return -1;
 NfsNativeMarkTexture*t=&nfs_native_marks.textures[empty];t->texture=id;t->art=*a;t->generation=nfs_native_marks.generation;
 nfs_mark_log("resource-created",t,NULL,0);return empty;
}
static void nfs_native_mark_gpu_uploaded(uint32_t address,uint32_t nativeId,uint32_t edge,
 uint32_t encoding,uint32_t hdTag,const void*prepared){
 NfsNativeMarkProfile profile;
 if(nfs_native_marks.disabled||!nfs_hd_native_mark_provider_available()||!nfs_mark_owner()||
    !nativeId||!prepared||!nfs_native_mark_profile(encoding,edge,&profile)||hdTag||
    (uint64_t)address+profile.preparedBytes>UINT64_C(0x100000000))return;
 unsigned char digest[32];NfsNativeMarkArt art={0};
 if(!nfs_texture_sha256(prepared,profile.preparedBytes,digest)||!nfs_hd_native_mark_art(encoding,edge,digest,&art))return;
 int binding=-1;for(unsigned k=0;k<NFS_NATIVE_MARK_MAX_BINDINGS;++k){NfsNativeMarkBinding*b=&nfs_native_marks.bindings[k];
  if(b->valid&&b->address==address){binding=(int)k;break;}if(!b->valid&&binding<0)binding=(int)k;
 }
 if(binding<0||nfs_native_marks.revision==UINT64_MAX)return;
 const int texture=nfs_mark_prepare(&art,&profile);if(texture<0)return;
 nfs_native_marks.bindings[binding]=(NfsNativeMarkBinding){address,nativeId,(uint32_t)texture,encoding,edge,profile.preparedBytes,
  nfs_native_marks.generation,++nfs_native_marks.revision,1};
 NfsNativeMarkTexture*t=&nfs_native_marks.textures[texture];if(t->uploads!=UINT64_MAX)++t->uploads;
}
static void nfs_native_mark_gpu_selected(uint32_t address,uint32_t nativeId,uint32_t edge,uint32_t encoding,uint32_t hdTag){
 NfsNativeMarkProfile profile;
 nfs_native_marks.selected=-1;if(!nfs_native_mark_profile(encoding,edge,&profile)||hdTag)return;
 for(unsigned k=0;k<NFS_NATIVE_MARK_MAX_BINDINGS;++k){NfsNativeMarkBinding*b=&nfs_native_marks.bindings[k];
  if(b->valid&&b->address==address&&b->nativeId==nativeId&&b->encoding==encoding&&b->edge==edge&&
     b->preparedBytes==profile.preparedBytes&&b->generation==nfs_native_marks.generation){nfs_native_marks.selected=(int)k;return;}
 }
}
static void nfs_native_mark_gpu_sync(int triangles,float textured,int hdSelected,uintptr_t context,
 uint32_t program,int hdLocation,int*hdUniformCache,int targetHeight){
 if(!nfs_native_marks.uniformsReady)return;
 if(context!=nfs_native_marks.context||program!=nfs_native_marks.program||!nfs_mark_owner()){nfs_native_marks.selected=-1;return;}
 GLint current=0;glGetIntegerv(GL_CURRENT_PROGRAM,&current);if((GLuint)current!=nfs_native_marks.program){nfs_native_marks.sent=0;return;}
 int active=0;NfsNativeMarkTexture*t=NULL;NfsNativeMarkBinding*b=NULL;
 if(triangles&&textured==1.0f&&!hdSelected&&nfs_native_marks.selected>=0){
  b=&nfs_native_marks.bindings[nfs_native_marks.selected];t=&nfs_native_marks.textures[b->texture];
  GLint unit=0,id=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&unit);if(unit!=GL_TEXTURE0)glActiveTexture(GL_TEXTURE0);
  glGetIntegerv(GL_TEXTURE_BINDING_2D,&id);if(unit!=GL_TEXTURE0)glActiveTexture((GLenum)unit);
  active=b->valid&&b->generation==nfs_native_marks.generation&&b->revision&&
   (GLuint)id==b->nativeId&&t->texture&&t->generation==nfs_native_marks.generation;
 }
 if(active){
  GLint unit=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&unit);glActiveTexture(GL_TEXTURE2);
  glGetIntegerv(GL_TEXTURE_BINDING_2D,&nfs_native_marks.priorUnit2);nfs_native_marks.restoreUnit2=1;
  glBindTexture(GL_TEXTURE_2D,t->texture);glActiveTexture((GLenum)unit);glUniform1i(nfs_native_marks.sampler,2);
  glUniform1f(hdLocation,0);if(hdUniformCache)*hdUniformCache=0;
  int known=0,empty=-1;for(unsigned k=0;k<8;++k){if(t->heights[k]==(unsigned)targetHeight&&targetHeight>0){known=1;break;}if(!t->heights[k]&&empty<0)empty=(int)k;}
  if(!known&&empty>=0&&targetHeight>0){t->heights[empty]=(unsigned)targetHeight;nfs_mark_log("draw-accepted",t,b,targetHeight);}
 }
 if(nfs_native_marks.sent!=active){glUniform1f(nfs_native_marks.enable,(GLfloat)active);nfs_native_marks.sent=active;}
}
static void nfs_native_mark_gpu_after_draw(void){
 if(!nfs_native_marks.restoreUnit2)return;
 GLint unit=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&unit);glActiveTexture(GL_TEXTURE2);
 glBindTexture(GL_TEXTURE_2D,(GLuint)nfs_native_marks.priorUnit2);glActiveTexture((GLenum)unit);
 nfs_native_marks.restoreUnit2=0;glUniform1f(nfs_native_marks.enable,0);nfs_native_marks.sent=0;
}

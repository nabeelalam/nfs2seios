// SPDX-License-Identifier: MIT
// Exact original animated-logo tail, after a completed authored Victory pass.
#include "VictoryLogoLayerAbi.h"
static struct {
 uint64_t request,content,generation;uintptr_t context;unsigned image,width,height,valid;
 NfsNativeLogoTargetProof target;GLuint color,depth;
}nfs_vl_parent;
static struct {
 NfsVictoryLogoTail view;uint64_t generation,nativeEpoch;uintptr_t context;
 GLuint nativeTexture,hdTexture;unsigned active,hasWide,selects,triangles,batch,quadReady,bound;
 float quad[60];
}nfs_vl;
static void nfs_vl_trace(const char*event,const char*reason){static unsigned lines;if(nfs_logo_trace_enabled()&&lines++<192)fprintf(stderr,"VICTORY LOGO %s reason=%s parent=%llu content=%llu source=%llu selections=%u triangles=%u batch=%u wide=%u\n",event,reason,(unsigned long long)nfs_vl.view.parentRequest,(unsigned long long)nfs_vl.view.content,(unsigned long long)nfs_vl.view.sourceSerial,nfs_vl.selects,nfs_vl.triangles,nfs_vl.batch,nfs_vl.hasWide);}
static void nfs_victory_logo_revoke(void){
 if(nfs_vl.bound&&nfs_victory_context(nfs_vl.generation,nfs_vl.context))glBindTexture(GL_TEXTURE_2D,nfs_vl.nativeTexture);
 memset(&nfs_vl,0,sizeof nfs_vl);memset(&nfs_vl_parent,0,sizeof nfs_vl_parent);
}
static int nfs_vl_fail(const char*why){nfs_vl_trace("refused",why);nfs_victory_logo_revoke();nfs_victory_wide_revoke();return 0;}
static int nfs_vl_attachment(void){
 GLint prior=0,type=0,name=0;int ok=0;
 if(!nfs_vl_parent.valid||!nfs_victory_context(nfs_vl_parent.generation,nfs_vl_parent.context))return 0;
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prior);glBindFramebuffer(GL_FRAMEBUFFER,nfs_vl_parent.target.framebuffer);
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);ok=type==GL_TEXTURE&&(GLuint)name==nfs_vl_parent.color;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);ok=ok&&type==GL_RENDERBUFFER&&(GLuint)name==nfs_vl_parent.depth&&glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE;
 glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)prior);return ok&&nfs_victory_context(nfs_vl_parent.generation,nfs_vl_parent.context)&&glGetError()==GL_NO_ERROR;
}
static int nfs_victory_logo_parent(const NfsVictoryLiveView*v,const NfsNativeLogoTargetProof*t){
 nfs_victory_logo_revoke();if(!v||!t||!v->request||!v->content||!v->artwork.present||!v->artwork.image||!nfs_victory_words_owner_current())return 0;
 GLint prior=0,type=0,name=0;glGetIntegerv(GL_FRAMEBUFFER_BINDING,&prior);glBindFramebuffer(GL_FRAMEBUFFER,t->framebuffer);
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);if(type==GL_TEXTURE)nfs_vl_parent.color=(GLuint)name;
 glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type);glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name);if(type==GL_RENDERBUFFER)nfs_vl_parent.depth=(GLuint)name;
 glBindFramebuffer(GL_FRAMEBUFFER,(GLuint)prior);if(!nfs_vl_parent.color||!nfs_vl_parent.depth||glGetError()!=GL_NO_ERROR)return 0;
 nfs_vl_parent.request=v->request;nfs_vl_parent.content=v->content;nfs_vl_parent.image=v->artwork.image;nfs_vl_parent.target=*t;nfs_vl_parent.generation=nfs_victory_renderer.generation;nfs_vl_parent.context=nfs_victory_renderer.context;nfs_vl_parent.width=winWidth;nfs_vl_parent.height=winHeight;nfs_vl_parent.valid=1;
 if(!nfs_vl_attachment()){nfs_victory_logo_revoke();return 0;}return 1;
}
static int nfs_vl_current(void){
 const NfsVictoryLogoTail*v=&nfs_vl.view;
 if(!nfs_vl.active||!v->request||!nfs_vl_parent.valid||v->parentRequest!=nfs_vl_parent.request||v->content!=nfs_vl_parent.content||v->image!=nfs_vl_parent.image||nfs_vl.generation!=nfs_vl_parent.generation||nfs_vl.context!=nfs_vl_parent.context||winWidth!=(int)nfs_vl_parent.width||winHeight!=(int)nfs_vl_parent.height||!nfs_victory_context(nfs_vl.generation,nfs_vl.context))return 0;
 if(v->sourceSerial!=nfs_tb.serial||v->sourceEpoch!=nfs_tb.epoch||v->artGeneration!=nfs_tb.art||v->artGeneration!=nfs_track_badge_art_generation()||nfs_vl.nativeEpoch!=nfs_tb.nativeEpoch||nfs_vl.hdTexture!=nfs_tb.texture||nfs_vl.nativeTexture!=nfs_tb.native||!nfs_tb_current())return 0;
 return !nfs_vl.hasWide||(nfs_victory_wide.ready&&nfs_victory_wide_current()&&nfs_victory_wide.source.request==v->parentRequest&&nfs_victory_wide.source.content==v->content&&nfs_victory_wide.art.image==v->image);
}
static int nfs_vl_pixels(void){
 const NfsVictoryLogoTail*v=&nfs_vl.view;if(!nfs_vl_current()||!v->original||v->address>TextureMem-131072u||v->address!=nfs_tb.address)return 0;
 const TextureInfo*ti=&g_textures[v->address>>2];return ti->id==nfs_vl.nativeTexture&&ti->data==g_textureMem+v->address&&nfs_tb_pixels(ti,v->original)&&!memcmp(v->original,nfs_tb.words,131072)&&glIsTexture(nfs_vl.nativeTexture)&&glIsTexture(nfs_vl.hdTexture);
}
int nfs_private_victory_logo_begin(const NfsVictoryLogoTail*v){
 drawTriangles();if(nfs_vl.active)return nfs_vl_fail("overlap");
 if(!v||v->schema!=1||!v->request||!v->parentRequest||!v->content||!v->sourceSerial||!v->sourceEpoch||!v->artGeneration||!v->original||v->image<1||v->image>12||(v->address&3)||v->address>TextureMem-131072u)return nfs_vl_fail("input");
 if(!nfs_vl_parent.valid||v->parentRequest!=nfs_vl_parent.request||v->content!=nfs_vl_parent.content||v->image!=nfs_vl_parent.image||!nfs_vl_attachment())return nfs_vl_fail("parent-source");
 nfs_vl.view=*v;nfs_vl.generation=nfs_vl_parent.generation;nfs_vl.context=nfs_vl_parent.context;nfs_vl.nativeTexture=nfs_tb.native;nfs_vl.hdTexture=nfs_tb.texture;nfs_vl.nativeEpoch=nfs_tb.nativeEpoch;nfs_vl.hasWide=nfs_victory_wide.ready;nfs_vl.active=1;
 if(!nfs_vl_pixels()||!nfs_vl.hdTexture||nfs_vl.hdTexture==nfs_vl.nativeTexture||nfs_vl.hdTexture==nfs_vl_parent.color||(nfs_vl.hasWide&&nfs_vl.hdTexture==nfs_victory_wide.color))return nfs_vl_fail("upload-receipt");
 nfs_vl_trace("began","current-parent-and-sealed-upload");return 1;
}
static int nfs_victory_logo_pending(void){return nfs_vl.active!=0;}
static int nfs_victory_logo_select(unsigned tmu,uint32_t address,uint32_t parity,const GrTexInfo*info,TextureInfo*ti){
 if(!nfs_vl.active)return 0;
 if(nfs_vl.selects||nfs_vl.triangles||!nfs_vl_current()||tmu||parity!=3||address!=nfs_vl.view.address||!info||info->smallLod||info->largeLod||info->aspectRatio!=3||info->format!=GR_TEXFMT_ARGB_1555||!ti||ti!=&g_textures[address>>2]||ti->id!=nfs_vl.nativeTexture||!nfs_vl_pixels())return nfs_vl_fail("selection");
 nfs_vl.selects=1;return 1;
}
int nfs_private_victory_logo_quad(uint64_t request,const float*v){
 if(!nfs_vl_current()||request!=nfs_vl.view.request||nfs_vl.selects!=1||nfs_vl.triangles||nfs_vl.quadReady||!v)return nfs_vl_fail("quad-source");
 for(unsigned k=0;k<4;++k){const float*p=v+15*k;for(unsigned j=0;j<15;++j)if((j==0||j==1||j==3||j==4||j==5||j==7||j==8||j==9||j==10||j==11)&&!isfinite(p[j]))return nfs_vl_fail("quad-finite");if(p[0]<VertexSnap-512||p[0]>VertexSnap+1152||p[1]<VertexSnap-512||p[1]>VertexSnap+992||p[8]<=0||1.f/p[8]>65535.f)return nfs_vl_fail("quad-range");}
 memcpy(nfs_vl.quad,v,sizeof nfs_vl.quad);nfs_vl.quadReady=1;return 1;
}
static int nfs_victory_logo_triangle(const GrVertex*a,const GrVertex*b,const GrVertex*c){
 if(!nfs_vl.active)return 0;unsigned half=nfs_vl.triangles;
 if(!nfs_vl_current()||nfs_vl.selects!=1||!nfs_vl.quadReady||half>=2||!a||!b||!c||sizeof(GrVertex)!=60||memcmp(a,nfs_vl.quad,60)||memcmp(b,nfs_vl.quad+15*(half?2:1),60)||memcmp(c,nfs_vl.quad+15*(half?3:2),60))return nfs_vl_fail("triangle-source");
 ++nfs_vl.triangles;return 1;
}
static int nfs_vl_target(GLuint texture){
 GLfloat actualMatrix[16];GLint actualTextureEnabled=0;GLint fbo=0,program=0,unit=0,bound=0,viewport[4];if(!nfs_vl_current()||!nfs_vl_pixels()||!nfs_vl_attachment()||g_renderBuffer!=GR_BUFFER_BACKBUFFER||g_textureEnabled!=1)return 0;
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);glGetIntegerv(GL_CURRENT_PROGRAM,&program);glGetIntegerv(GL_ACTIVE_TEXTURE,&unit);glGetIntegerv(GL_TEXTURE_BINDING_2D,&bound);glGetIntegerv(GL_VIEWPORT,viewport);if((GLuint)program!=g_shaderProgram)return 0;glGetUniformfv(g_shaderProgram,g_uMatrixLoc,actualMatrix);glGetUniformiv(g_shaderProgram,g_uTextureEnabledLoc,&actualTextureEnabled);
 return glGetError()==GL_NO_ERROR&&actualTextureEnabled==1&&!memcmp(actualMatrix,g_matrix,sizeof actualMatrix)&&(GLuint)fbo==nfs_vl_parent.target.framebuffer&&(GLuint)program==g_shaderProgram&&unit==GL_TEXTURE0&&(GLuint)bound==texture&&!memcmp(viewport,nfs_vl_parent.target.sampling.viewport,sizeof viewport)&&!memcmp(g_matrix,nfs_vl_parent.target.sampling.projection,sizeof g_matrix);
}
static int nfs_vl_batch_geometry(unsigned count){
 static const unsigned corners[2][3]={{0,1,2},{0,2,3}};
 if(count!=2||count!=g_trianglesCount||nfs_vl.selects!=1||nfs_vl.triangles!=2||!nfs_vl.quadReady||nfs_vl.batch)return 0;
 for(unsigned i=0;i<2;++i)for(unsigned j=0;j<3;++j){const float*p=nfs_vl.quad+15*corners[i][j];
  if(g_vertices[i].vertex[j].x!=p[0]-VertexSnap||g_vertices[i].vertex[j].y!=p[1]-VertexSnap||g_vertices[i].vertex[j].z!=p[8]||g_textureCoord[i].vertex[j].s!=p[9]/256.f||g_textureCoord[i].vertex[j].t!=p[10]/256.f||g_textureCoord[i].vertex[j].r||g_textureCoord[i].vertex[j].q!=p[8]||g_colorValues[i].vertex[j].r!=p[3]||g_colorValues[i].vertex[j].g!=p[4]||g_colorValues[i].vertex[j].b!=p[5]||g_colorValues[i].vertex[j].a!=p[7]||g_fogCoord[i].vertex[j]!=255-g_fogTable[(uint16_t)(1.f/p[8])])return 0;
 }return 1;
}
// Change the optional texture only at a complete verified native batch, never
// after merely one triangle. Invalid batches retain the original native draw.
static void nfs_victory_logo_batch_begin(unsigned count){
 if(!nfs_vl.active)return;if(!nfs_vl_batch_geometry(count)||!nfs_vl_target(nfs_vl.nativeTexture)){nfs_vl_fail("batch-source");return;}
 nfs_vl.bound=1;glBindTexture(GL_TEXTURE_2D,nfs_vl.hdTexture);if(glGetError()!=GL_NO_ERROR)nfs_vl_fail("HD-binding");
}
static void nfs_victory_logo_batch_end(unsigned count){
 if(!nfs_vl.active||!nfs_vl.bound)return;
 if(!nfs_vl_batch_geometry(count)||!nfs_vl_target(nfs_vl.hdTexture)){nfs_vl_fail("native-draw-state");return;}
 int ok=1;if(nfs_vl.hasWide){
  NfsBgSaved saved;GLfloat nativeMatrix[16],wideMatrix[16];GLint scissor[4];nfs_bg_save(&saved);memcpy(nativeMatrix,g_matrix,sizeof nativeMatrix);memcpy(wideMatrix,nativeMatrix,sizeof wideMatrix);glGetIntegerv(GL_SCISSOR_BOX,scissor);
  const float logical=480.f*nfs_victory_wide.width/nfs_victory_wide.height;wideMatrix[0]=2.f/logical;wideMatrix[12]=-640.f/logical;
#define NFS_VL_GL(call) do{if(!nfs_vl_current()){ok=0;goto logo_done;}call;if(!nfs_vl_current()){ok=0;goto logo_done;}}while(0)
  NFS_VL_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_victory_wide.fbo));if(!nfs_victory_wide_attachment(nfs_victory_wide.fbo,nfs_victory_wide.color,0)){ok=0;goto logo_done;}
  NFS_VL_GL(glViewport(0,0,nfs_victory_wide.width,nfs_victory_wide.height));NFS_VL_GL(glDepthMask(GL_FALSE));NFS_VL_GL(glDisable(GL_DEPTH_TEST));NFS_VL_GL(glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,wideMatrix));NFS_VL_GL(glVertexAttribPointer(g_aPositionLoc,3,GL_FLOAT,GL_FALSE,0,g_vertices));
  if(saved.common.caps[4]){const GLfloat*m=nativeMatrix,*w=wideMatrix;const GLint*vp=nfs_vl_parent.target.sampling.viewport;float l=((scissor[0]-vp[0])*2.f/vp[2]-1-m[12])/m[0],r=((scissor[0]+scissor[2]-vp[0])*2.f/vp[2]-1-m[12])/m[0],top=((scissor[1]+scissor[3]-vp[1])*2.f/vp[3]-1-m[13])/m[5],bottom=((scissor[1]-vp[1])*2.f/vp[3]-1-m[13])/m[5];GLint x=(GLint)floorf((w[0]*l+w[12]+1)*nfs_victory_wide.width*.5f),right=(GLint)ceilf((w[0]*r+w[12]+1)*nfs_victory_wide.width*.5f),y=(GLint)floorf((w[5]*bottom+w[13]+1)*nfs_victory_wide.height*.5f),end=(GLint)ceilf((w[5]*top+w[13]+1)*nfs_victory_wide.height*.5f);if(right<x||end<y){ok=0;goto logo_done;}NFS_VL_GL(glScissor(x,y,right-x,end-y));}else NFS_VL_GL(glDisable(GL_SCISSOR_TEST));
  NFS_VL_GL(glDrawArrays(GL_TRIANGLES,0,6));ok=glGetError()==GL_NO_ERROR;
  logo_done:if(nfs_victory_context(nfs_vl.generation,nfs_vl.context)){glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nativeMatrix);nfs_bg_restore(&saved);}else ok=0;
#undef NFS_VL_GL
 }
 if(!ok||!nfs_vl_current()){nfs_vl_fail("wide-replay");return;}
 glBindTexture(GL_TEXTURE_2D,nfs_vl.nativeTexture);nfs_vl.bound=0;if(glGetError()!=GL_NO_ERROR){nfs_vl_fail("native-binding-restore");return;}nfs_vl.batch=1;
}
int nfs_private_victory_logo_end(uint64_t request,int valid){
 drawTriangles();if(!request||request!=nfs_vl.view.request||!valid||!nfs_vl_current()||nfs_vl.selects!=1||nfs_vl.triangles!=2||nfs_vl.batch!=1||nfs_vl.bound)return nfs_vl_fail("incomplete-tail");
 nfs_vl_trace("published","exact-source-two-triangle-native-and-wide-batch");memset(&nfs_vl,0,sizeof nfs_vl);return 1;
}
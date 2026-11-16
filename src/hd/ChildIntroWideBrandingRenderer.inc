// SPDX-License-Identifier: MIT
/* Normal child4276F0 source channel. Immutable six-slot proof and actual
 * postconversion quads belong to this Prelude owner; no WTR phase/state is read. */
#include "MenuTransitionBrandingAbi.h"
static struct{NfsMenuTransitionBrandingView view;NfsMenuTransitionBrandingQuad quad;unsigned active,completed,nextSlot,selectedSlot,ordinal,armed,triangles,pending,drawn;uint64_t uploads,clearEvent;GLuint nativeId;int mark;}nfs_piw_child_brand;
static void nfs_piw_child_brand_reset(void){memset(&nfs_piw_child_brand,0,sizeof nfs_piw_child_brand);}
static int nfs_piw_child_brand_current(void){const NfsMenuTransitionBrandingView*v=&nfs_piw_child_brand.view;return nfs_piw_current()&&nfs_piw.phase>=6&&nfs_piw.phase<=9&&(nfs_piw.view.family==6||nfs_piw.view.family==7)&&v->serial==nfs_piw.view.frame.requestSerial&&v->content==nfs_piw.view.content&&v->kind==nfs_piw.view.family&&v->form==nfs_piw.view.frame.form&&v->producerEpoch==nfs_piw.view.ownerEpoch&&v->sourceGame==nfs_piw.view.sourceGame&&v->sourceLife==nfs_piw.view.sourceLife&&v->sourceArchive==nfs_piw.view.sourceArchive&&v->sourceCatalog==nfs_piw.view.sourceCatalog&&nfs_logo_provenance.clearEvent==nfs_piw_child_brand.clearEvent&&nfs_logo_provenance.uploads==nfs_piw_child_brand.uploads;}
static unsigned nfs_piw_child_brand_next(unsigned at){while(at<6&&!nfs_piw_child_brand.view.slots[at].enabled)++at;return at;}
/*4281B0 frees its upload scratch canvas after49755C. Descriptor data is
 retired metadata, not a CPU pixel source. Require current native bytes to name
 exactly the immutable artwork retained by the original upload binding. */
static int nfs_piw_child_brand_art_same(const NfsNativeMarkArt*a,const NfsNativeMarkArt*b){
 return a&&b&&a->schema==1&&b->schema==1&&a->premultiplied==1&&b->premultiplied==1&&!a->opacityBaked&&!b->opacityBaked&&a->identity&&a->identity==b->identity&&a->pixels&&a->pixels==b->pixels&&a->width==512&&b->width==512&&a->height==512&&b->height==512&&a->logicalWidth==128&&b->logicalWidth==128&&a->logicalHeight==128&&b->logicalHeight==128&&a->bytes==1048576&&b->bytes==1048576&&!memcmp(a->sha256,b->sha256,32);
}
static int nfs_piw_child_brand_slot(unsigned slot,GLuint*id,int*mark){
 const NfsMenuTransitionBrandingSlot*s;unsigned char sha[32];NfsNativeMarkArt expected={0};int authenticated=0;TextureInfo*ti;
 if(slot>=6)return 0;s=nfs_piw_child_brand.view.slots+slot;
 const uint32_t address=s->descriptorWords[5];
 if(!s->enabled||s->index!=slot||!s->descriptorGuest||s->descriptorWords[0]!=1||s->descriptorWords[1]!=1||s->descriptorWords[2]!=3||s->descriptorWords[3]!=GR_TEXFMT_ARGB_1555||s->descriptorWords[6]!=3||(address&3)||address>TextureMem-32768)return 0;
 ti=g_textures+(address>>2);if(!ti->id||ti->hdDigits||ti->size!=128||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->data!=g_textureMem+address||ti->id==nfs_piw.scratch||ti->id==nfs_piw.slots[nfs_piw.index].color||ti->id==nfs_piw.art[0]||ti->id==nfs_piw.art[1]||!nfs_texture_sha256((const unsigned char*)ti->data,32768,sha))return 0;
 if(!nfs_mark_owner())return 0;
 authenticated=nfs_hd_native_mark_art(NFS_NATIVE_MARK_SOURCE_RGBA5551_LE,128,sha,&expected);
 for(unsigned k=0;k<NFS_NATIVE_MARK_MAX_BINDINGS;++k){const NfsNativeMarkBinding*b=nfs_native_marks.bindings+k;
  if(b->valid&&b->address==address&&b->nativeId==ti->id&&b->encoding==NFS_NATIVE_MARK_SOURCE_RGBA5551_LE&&b->edge==128&&b->preparedBytes==32768&&b->generation==nfs_native_marks.generation&&b->revision&&b->texture<NFS_NATIVE_MARK_MAX_TEXTURES){const NfsNativeMarkTexture*t=nfs_native_marks.textures+b->texture;if(!authenticated||!nfs_piw_child_brand_art_same(&expected,&t->art)||!t->texture||t->generation!=nfs_native_marks.generation||t->texture==nfs_piw.slots[nfs_piw.index].color||t->texture==nfs_piw.scratch||t->texture==nfs_piw.art[0]||t->texture==nfs_piw.art[1])return 0;*id=ti->id;*mark=(int)k;return 1;}
 }return 0;
}
static int nfs_piw_child_brand_ready(void){return nfs_piw_child_brand.completed&&!nfs_piw_child_brand.active&&!nfs_piw_child_brand.armed&&!nfs_piw_child_brand.pending&&nfs_piw_child_brand.nextSlot==6&&nfs_piw_child_brand_current();}
int nfs_private_child_intro_branding_begin(const NfsMenuTransitionBrandingView*v){
 const int saved=errno;drawTriangles();nfs_piw_child_brand_reset();
 if(!v||v->slotCount!=6||!v->serial||!v->content||!v->producerEpoch||!v->sourceGame||!v->sourceLife||!v->sourceArchive||!v->sourceCatalog||v->kind<6||v->kind>7){errno=saved;return 0;}
 nfs_piw_child_brand.view=*v;nfs_piw_child_brand.uploads=nfs_logo_provenance.uploads;nfs_piw_child_brand.clearEvent=nfs_logo_provenance.clearEvent;
 if(!nfs_piw_child_brand_current())goto refused;
 for(unsigned k=0;k<6;++k){GLuint id=0;int mark=0;if(v->slots[k].index!=k||v->slots[k].enabled>1||(v->slots[k].enabled&&!nfs_piw_child_brand_slot(k,&id,&mark)))goto refused;}
 nfs_piw_child_brand.nextSlot=nfs_piw_child_brand_next(0);nfs_piw_child_brand.selectedSlot=6;nfs_piw_child_brand.active=1;errno=saved;return 1;
 refused:nfs_player_intro_wide_other();errno=saved;return 0;
}
static int nfs_piw_child_brand_select(unsigned tmu,uint32_t address,uint32_t parity,const GrTexInfo*info,TextureInfo*ti){
 GLuint id=0;int mark=0;if(!nfs_piw_child_brand.active)return 0;
 if(!nfs_piw_child_brand_current()||nfs_piw_child_brand.armed||nfs_piw_child_brand.pending||nfs_piw_child_brand.selectedSlot!=6||nfs_piw_child_brand.nextSlot>=6||tmu||parity!=3||!info||!ti||info->format!=GR_TEXFMT_ARGB_1555||info->smallLod!=1||info->largeLod!=1||info->aspectRatio!=3||!nfs_piw_child_brand_slot(nfs_piw_child_brand.nextSlot,&id,&mark)||address!=nfs_piw_child_brand.view.slots[nfs_piw_child_brand.nextSlot].descriptorWords[5]||ti->id!=id){nfs_player_intro_wide_other();return 1;}
 nfs_piw_child_brand.selectedSlot=nfs_piw_child_brand.nextSlot;nfs_piw_child_brand.nativeId=id;nfs_piw_child_brand.mark=mark;return 1;
}
static float nfs_piw_child_brand_float(uint32_t word){float f;memcpy(&f,&word,4);return f;}
static int nfs_piw_child_brand_vertex(const GrVertex*v,unsigned index){static const unsigned fields[]={0,1,3,4,5,7,8,9,10};if(!v||sizeof(GrVertex)!=60||index>3)return 0;for(unsigned k=0;k<sizeof fields/sizeof fields[0];++k){uint32_t actual;memcpy(&actual,(const unsigned char*)v+4*fields[k],4);if(actual!=nfs_piw_child_brand.quad.nativeVertices[index][fields[k]])return 0;}return 1;}
int nfs_private_child_intro_branding_quad(const NfsMenuTransitionBrandingQuad*q){
 const int saved=errno;drawTriangles();
 if(!q||!nfs_piw_child_brand.active||!nfs_piw_child_brand_current()||q->serial!=nfs_piw_child_brand.view.serial||q->ordinal!=nfs_piw_child_brand.ordinal||q->slot!=nfs_piw_child_brand.nextSlot||q->slot!=nfs_piw_child_brand.selectedSlot||nfs_piw_child_brand.armed||nfs_piw_child_brand.pending)goto refused;
 for(unsigned i=0;i<4;++i){static const unsigned fields[]={0,1,3,4,5,7,8,9,10};for(unsigned k=0;k<sizeof fields/sizeof fields[0];++k)if(!isfinite(nfs_piw_child_brand_float(q->nativeVertices[i][fields[k]])))goto refused;for(unsigned k=3;k<=7;++k)if(k!=6){const float f=nfs_piw_child_brand_float(q->nativeVertices[i][k]);if(f<0||f>255)goto refused;}if(nfs_piw_child_brand_float(q->nativeVertices[i][8])!=1.f)goto refused;}
 nfs_piw_child_brand.quad=*q;nfs_piw_child_brand.triangles=nfs_piw_child_brand.drawn=0;nfs_piw_child_brand.armed=1;errno=saved;return 1;
 refused:nfs_player_intro_wide_other();errno=saved;return 0;
}
static int nfs_piw_child_brand_triangle(const GrVertex*a,const GrVertex*b,const GrVertex*c){
 if(!nfs_piw_child_brand.active)return 0;const unsigned half=nfs_piw_child_brand.triangles;
 if(!nfs_piw_child_brand_current()||!nfs_piw_child_brand.armed||half>=2||!g_textureEnabled||g_fogEnabled||nfs_native_marks.selected!=nfs_piw_child_brand.mark||!nfs_piw_child_brand_vertex(a,0)||!nfs_piw_child_brand_vertex(b,half?2:1)||!nfs_piw_child_brand_vertex(c,half?3:2)){nfs_player_intro_wide_other();return 1;}
 ++nfs_piw_child_brand.triangles;++nfs_piw_child_brand.pending;return 1;
}
#define NFS_CHILD_BRAND_GL(call) do{if(!nfs_piw_child_brand_current())goto lost;call;if(!nfs_piw_child_brand_current())goto lost;}while(0)
static int nfs_piw_child_brand_array(unsigned i,GLint size,GLenum type,GLboolean normalized,const void*pointer){
 GLint enabled=0,n=0,kind=0,normal=0,stride=0,buffer=0;void*p=NULL;if(i>3)return 0;
 NFS_CHILD_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_ENABLED,&enabled));NFS_CHILD_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_SIZE,&n));NFS_CHILD_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_TYPE,&kind));NFS_CHILD_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,&normal));NFS_CHILD_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_STRIDE,&stride));NFS_CHILD_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING,&buffer));NFS_CHILD_BRAND_GL(glGetVertexAttribPointerv(i,GL_VERTEX_ATTRIB_ARRAY_POINTER,&p));return enabled&&n==size&&(GLenum)kind==type&&normal==normalized&&!stride&&!buffer&&p==pointer;
 lost:return 0;
}
static int nfs_piw_child_brand_batch(unsigned count){
 NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_piw_guard();NfsFrameLegacySampling sampling;GLfloat realMatrix[16],flag=0;GLint fbo=0,program=0,active=0,binding=0,vp[4],clip[4],unit=0,samplerLoc=-1;GLboolean enabled=0,liveTexture=0;GLuint id=0;int mark=0,savedReady=0,ok=0;const unsigned corners[2][3]={{0,1,2},{0,2,3}};
 if(!nfs_piw_child_brand.active||!nfs_piw_child_brand.armed||!nfs_piw_child_brand_current()||!count||count>2||count!=g_trianglesCount||count!=nfs_piw_child_brand.pending||nfs_piw_child_brand.drawn+count!=nfs_piw_child_brand.triangles||g_fogEnabled||g_textureEnabled!=1||!nfs_piw_child_brand_slot(nfs_piw_child_brand.selectedSlot,&id,&mark)||id!=nfs_piw_child_brand.nativeId||mark!=nfs_piw_child_brand.mark)goto refused;
 for(unsigned i=0;i<count;++i)for(unsigned j=0;j<3;++j){const uint32_t*w=nfs_piw_child_brand.quad.nativeVertices[corners[nfs_piw_child_brand.drawn+i][j]];if(g_vertices[i].vertex[j].x!=nfs_piw_child_brand_float(w[0])-VertexSnap||g_vertices[i].vertex[j].y!=nfs_piw_child_brand_float(w[1])-VertexSnap||g_vertices[i].vertex[j].z!=1.f||g_textureCoord[i].vertex[j].s!=nfs_piw_child_brand_float(w[9])/256.f||g_textureCoord[i].vertex[j].t!=nfs_piw_child_brand_float(w[10])/256.f||g_textureCoord[i].vertex[j].q!=1.f||g_textureCoord[i].vertex[j].r!=0.f||g_colorValues[i].vertex[j].r!=(uint8_t)nfs_piw_child_brand_float(w[3])||g_colorValues[i].vertex[j].g!=(uint8_t)nfs_piw_child_brand_float(w[4])||g_colorValues[i].vertex[j].b!=(uint8_t)nfs_piw_child_brand_float(w[5])||g_colorValues[i].vertex[j].a!=(uint8_t)nfs_piw_child_brand_float(w[7]))goto refused;}
 if(!nfs_frame_gpu_legacy_sampling((unsigned)g_framebufferHeight*4u/3u,(unsigned)g_framebufferHeight,linearFiltering?GL_LINEAR:GL_NEAREST,&sampling)||!nfs_graphics_scene_save(&saved,&guard))goto refused;savedReady=1;
 if(!nfs_piw_child_brand_array((unsigned)g_aPositionLoc,3,GL_FLOAT,GL_FALSE,g_vertices)||!nfs_piw_child_brand_array((unsigned)g_aTexCoordLoc,4,GL_FLOAT,GL_FALSE,g_textureCoord)||!nfs_piw_child_brand_array((unsigned)g_aColorLoc,4,GL_UNSIGNED_BYTE,GL_TRUE,g_colorValues))goto refused;
 NFS_CHILD_BRAND_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_CHILD_BRAND_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&program));NFS_CHILD_BRAND_GL(glGetIntegerv(GL_ACTIVE_TEXTURE,&active));NFS_CHILD_BRAND_GL(glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding));NFS_CHILD_BRAND_GL(glGetIntegerv(GL_VIEWPORT,vp));
 if((GLuint)fbo!=nfs_piw.slots[nfs_piw.index].nativeFbo||(GLuint)program!=g_shaderProgram||active!=GL_TEXTURE0||(GLuint)binding!=id||g_framebufferTexture!=nfs_piw.slots[nfs_piw.index].nativeColor||g_framebufferDepth!=nfs_piw.slots[nfs_piw.index].nativeDepth||memcmp(vp,sampling.viewport,sizeof vp)||memcmp(g_matrix,sampling.projection,sizeof g_matrix)||nfs_native_marks.selected!=mark||(mark<0||!nfs_native_marks.sent))goto refused;
 NFS_CHILD_BRAND_GL(glGetUniformfv(g_shaderProgram,g_uMatrixLoc,realMatrix));if(memcmp(realMatrix,g_matrix,sizeof realMatrix))goto refused;
 NFS_CHILD_BRAND_GL(glGetUniformfv(g_shaderProgram,g_uTextureEnabledLoc,&flag));if(flag!=1.f)goto refused;NFS_CHILD_BRAND_GL(glGetUniformfv(g_shaderProgram,g_uFogEnabledLoc,&flag));if(flag!=0.f)goto refused;NFS_CHILD_BRAND_GL(samplerLoc=glGetUniformLocation(g_shaderProgram,"uTextureSampler"));if(samplerLoc<0)goto refused;NFS_CHILD_BRAND_GL(glGetUniformiv(g_shaderProgram,samplerLoc,&unit));if(unit!=0)goto refused;
 NFS_CHILD_BRAND_GL(glGetUniformfv(g_shaderProgram,nfs_native_marks.enable,&flag));if(flag!=(mark>=0?1.f:0.f))goto refused;
 if(mark>=0){NFS_CHILD_BRAND_GL(glGetUniformiv(g_shaderProgram,nfs_native_marks.sampler,&unit));if(unit!=2)goto refused;
 NFS_CHILD_BRAND_GL(glActiveTexture(GL_TEXTURE2));NFS_CHILD_BRAND_GL(glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding));NFS_CHILD_BRAND_GL(glActiveTexture(GL_TEXTURE0));if((GLuint)binding!=nfs_native_marks.textures[nfs_native_marks.bindings[mark].texture].texture)goto refused;NFS_CHILD_BRAND_GL(liveTexture=glIsTexture((GLuint)binding));if(!liveTexture)goto refused;}
 NFS_CHILD_BRAND_GL(enabled=glIsEnabled(GL_STENCIL_TEST));if(enabled)goto refused;NFS_CHILD_BRAND_GL(enabled=glIsEnabled(GL_CULL_FACE));if(enabled)goto refused;NFS_CHILD_BRAND_GL(enabled=glIsEnabled(GL_POLYGON_OFFSET_FILL));if(enabled)goto refused;
 {GLint sr=0,sa=0,dr=0,da=0,er=0,ea=0;NFS_CHILD_BRAND_GL(enabled=glIsEnabled(GL_BLEND));if(!enabled)goto refused;NFS_CHILD_BRAND_GL(glGetIntegerv(GL_BLEND_SRC_RGB,&sr));NFS_CHILD_BRAND_GL(glGetIntegerv(GL_BLEND_SRC_ALPHA,&sa));NFS_CHILD_BRAND_GL(glGetIntegerv(GL_BLEND_DST_RGB,&dr));NFS_CHILD_BRAND_GL(glGetIntegerv(GL_BLEND_DST_ALPHA,&da));NFS_CHILD_BRAND_GL(glGetIntegerv(GL_BLEND_EQUATION_RGB,&er));NFS_CHILD_BRAND_GL(glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&ea));if(sr!=GL_SRC_ALPHA||sa!=GL_SRC_ALPHA||er!=GL_FUNC_ADD||ea!=GL_FUNC_ADD||(dr!=GL_ZERO&&dr!=GL_ONE&&dr!=GL_ONE_MINUS_SRC_ALPHA)||(da!=GL_ZERO&&da!=GL_ONE&&da!=GL_ONE_MINUS_SRC_ALPHA))goto refused;}
 NFS_CHILD_BRAND_GL(glGetIntegerv(GL_SCISSOR_BOX,clip));for(unsigned i=0;i<4;++i)if(clip[i]<-16384||clip[i]>16384)goto refused;if(clip[2]<0||clip[3]<0)goto refused;
 ok=1;
 refused:if(savedReady&&nfs_piw_context())ok=nfs_graphics_scene_restore(&saved,&guard)&&ok;return ok&&nfs_piw_child_brand_current();
 lost:if(savedReady&&nfs_piw_context())(void)nfs_graphics_scene_restore(&saved,&guard);return 0;
}
#undef NFS_CHILD_BRAND_GL
static void nfs_piw_child_brand_drawn(unsigned count){if(!nfs_piw_child_brand_current()||!count||count!=nfs_piw_child_brand.pending||nfs_piw_child_brand.drawn+count!=nfs_piw_child_brand.triangles){nfs_player_intro_wide_other();return;}nfs_piw_child_brand.drawn+=count;nfs_piw_child_brand.pending=0;if(nfs_piw_child_brand.drawn==2){nfs_piw_child_brand.armed=0;++nfs_piw_child_brand.ordinal;nfs_piw_child_brand.nextSlot=nfs_piw_child_brand_next(nfs_piw_child_brand.selectedSlot+1);nfs_piw_child_brand.selectedSlot=6;}}
int nfs_private_child_intro_branding_end(uint64_t serial){const int saved=errno;drawTriangles();if(!nfs_piw_child_brand.active||serial!=nfs_piw_child_brand.view.serial||!nfs_piw_child_brand_current()||nfs_piw_child_brand.armed||nfs_piw_child_brand.pending||nfs_piw_child_brand.nextSlot!=6){nfs_player_intro_wide_other();errno=saved;return 0;}nfs_piw_child_brand.active=0;nfs_piw_child_brand.completed=1;errno=saved;return 1;}
void nfs_private_child_intro_branding_cancel(uint64_t serial){const int saved=errno;if(serial&&serial==nfs_piw_child_brand.view.serial)nfs_player_intro_wide_other();errno=saved;}

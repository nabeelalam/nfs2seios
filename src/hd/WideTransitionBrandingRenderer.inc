// SPDX-License-Identifier: MIT
/* Noninteractive initial5/6/7 branding only. Source ownership is the immutable
   original4276F0 whole-pass proof, never widget/cursor/font snapshots. */
#include "MenuTransitionBrandingAbi.h"
static struct{NfsMenuTransitionBrandingView view;NfsMenuTransitionBrandingQuad quad;unsigned active,completed,nextSlot,selectedSlot,ordinal,armed,triangles,pending,drawn;uint64_t uploads,clearEvent;GLuint nativeId;int mark;}nfs_mtw_brand;
static void nfs_mtw_branding_note(unsigned event){static unsigned seen[11][4];const int saved=errno;const unsigned kind=nfs_mtw.kind,index=kind==11?3:kind-5;if(event<11&&((kind>=5&&kind<=7)||kind==11)&&nfs_logo_trace_enabled()&&seen[event][index]<2){++seen[event][index];fprintf(stderr,"MENU BRANDING WIDE event=%u kind=%u serial=%llu ordinal=%u slot=%u pending=%u active=%u completed=%u\n",event,kind,(unsigned long long)nfs_mtw_brand.view.serial,nfs_mtw_brand.ordinal,nfs_mtw_brand.nextSlot,nfs_mtw_brand.pending,nfs_mtw_brand.active,nfs_mtw_brand.completed);}errno=saved;}
static void nfs_mtw_branding_reset(void){memset(&nfs_mtw_brand,0,sizeof nfs_mtw_brand);}
static int nfs_mtw_branding_current(void){const NfsMenuTransitionBrandingView*v=&nfs_mtw_brand.view;return nfs_mtw_current()&&nfs_mtw.phase==2&&((nfs_mtw.kind>=5&&nfs_mtw.kind<=7)||nfs_mtw.kind==11)&&v->serial&&v->serial==nfs_mtw.serial&&v->content==nfs_mtw.content&&v->kind==nfs_mtw.kind&&v->form==nfs_mtw.initialForm&&v->producerEpoch==nfs_mtw.initialEpoch&&v->sourceGame==nfs_mtw.sourceGame&&v->sourceLife==nfs_mtw.sourceLife&&v->sourceArchive==nfs_mtw.sourceArchive&&v->sourceCatalog==nfs_mtw.sourceCatalog&&nfs_menu_transition_source_life_current(v->sourceGame,v->sourceLife)&&nfs_logo_provenance.clearEvent==nfs_mtw_brand.clearEvent&&nfs_logo_provenance.uploads==nfs_mtw_brand.uploads;}
static unsigned nfs_mtw_branding_next(unsigned at){while(at<6&&!nfs_mtw_brand.view.slots[at].enabled)++at;return at;}
/* Original rare Setup mode is authorized by its independent fixed digest and
   actual upload ledger, plus this typed source slotâ€”not widget debug state. */
static int nfs_mtw_branding_mode(unsigned slot,GLuint id,int selected){
 unsigned char nativeSha[32];if(slot>=6)return 0;const uint32_t address=nfs_mtw_brand.view.slots[slot].descriptorWords[5];if((address&3)||address>TextureMem-32768||!nfs_texture_sha256((const unsigned char*)g_textureMem+address,32768,nativeSha))return 0;
 if(nfs_main_original_knockout_digest(nativeSha)&&nfs_menu_original_knockout_binding(slot,address,id,selected))return 1;
 if(nfs_mtw_brand.view.kind!=5||nfs_mtw_brand.view.form!=0x4d2790u||!nfs_setup_original_mode_digest(nativeSha)||!nfs_menu_original_owner()||!nfs_menu_mode_original.valid||!nfs_menu_mode_original.revision||nfs_menu_mode_original.context!=(uintptr_t)g_glCtx||nfs_menu_mode_original.generation!=nfs_logo_provenance.context||nfs_menu_mode_original.address!=nfs_mtw_brand.view.slots[slot].descriptorWords[5]||nfs_menu_mode_original.id!=id)return 0;
 if(selected&&(!nfs_menu_mode_original_selection.valid||nfs_menu_mode_original_selection.context!=nfs_menu_mode_original.context||nfs_menu_mode_original_selection.generation!=nfs_menu_mode_original.generation||nfs_menu_mode_original_selection.revision!=nfs_menu_mode_original.revision||nfs_menu_mode_original_selection.address!=nfs_menu_mode_original.address||nfs_menu_mode_original_selection.id!=id||nfs_menu_mode_original_selection.format!=GR_TEXFMT_ARGB_1555||nfs_menu_mode_original_selection.edge!=128))return 0;return 1;
}
/*4281B0 frees its upload scratch canvas after49755C. Descriptor data is
 retired metadata, not a CPU pixel source. Require current native bytes to name
 exactly the immutable artwork retained by the original upload binding. */
static int nfs_mtw_branding_art_same(const NfsNativeMarkArt*a,const NfsNativeMarkArt*b){
 return a&&b&&a->schema==1&&b->schema==1&&a->premultiplied==1&&b->premultiplied==1&&!a->opacityBaked&&!b->opacityBaked&&a->identity&&a->identity==b->identity&&a->pixels&&a->pixels==b->pixels&&a->width==512&&b->width==512&&a->height==512&&b->height==512&&a->logicalWidth==128&&b->logicalWidth==128&&a->logicalHeight==128&&b->logicalHeight==128&&a->bytes==1048576&&b->bytes==1048576&&!memcmp(a->sha256,b->sha256,32);
}
static int nfs_mtw_branding_slot(unsigned slot,GLuint*id,int*mark){
 const NfsMenuTransitionBrandingSlot*s;unsigned char sha[32];NfsNativeMarkArt expected={0};int authenticated=0;TextureInfo*ti;
 if(slot>=6)return 0;s=nfs_mtw_brand.view.slots+slot;
 const uint32_t address=s->descriptorWords[5];
 if(!s->enabled||s->index!=slot||!s->descriptorGuest||s->descriptorWords[0]!=1||s->descriptorWords[1]!=1||s->descriptorWords[2]!=3||s->descriptorWords[3]!=GR_TEXFMT_ARGB_1555||s->descriptorWords[6]!=3||(address&3)||address>TextureMem-32768)return 0;
 ti=g_textures+(address>>2);if(!ti->id||ti->hdDigits||ti->size!=128||ti->fmt!=GR_TEXFMT_ARGB_1555||ti->data!=g_textureMem+address||ti->id==nfs_mtw.image||ti->id==nfs_mtw.result||ti->id==nfs_mtw.art[0]||ti->id==nfs_mtw.art[1]||!nfs_texture_sha256((const unsigned char*)ti->data,32768,sha))return 0;
 if(!nfs_mark_owner())return 0;
 authenticated=nfs_hd_native_mark_art(NFS_NATIVE_MARK_SOURCE_RGBA5551_LE,128,sha,&expected);
 for(unsigned k=0;k<NFS_NATIVE_MARK_MAX_BINDINGS;++k){const NfsNativeMarkBinding*b=nfs_native_marks.bindings+k;
  if(b->valid&&b->address==address&&b->nativeId==ti->id&&b->encoding==NFS_NATIVE_MARK_SOURCE_RGBA5551_LE&&b->edge==128&&b->preparedBytes==32768&&b->generation==nfs_native_marks.generation&&b->revision&&b->texture<NFS_NATIVE_MARK_MAX_TEXTURES){const NfsNativeMarkTexture*t=nfs_native_marks.textures+b->texture;if(!authenticated||!nfs_mtw_branding_art_same(&expected,&t->art)||!t->texture||t->generation!=nfs_native_marks.generation||t->texture==nfs_mtw.result||t->texture==nfs_mtw.image||t->texture==nfs_mtw.art[0]||t->texture==nfs_mtw.art[1])return 0;*id=ti->id;*mark=(int)k;return 1;}
 }if(nfs_mtw_branding_mode(slot,ti->id,0)){*id=ti->id;*mark=-1;return 1;}return 0;
}
static int nfs_mtw_branding_ready(void){return nfs_mtw_brand.completed&&!nfs_mtw_brand.active&&!nfs_mtw_brand.armed&&!nfs_mtw_brand.pending&&nfs_mtw_brand.nextSlot==6&&nfs_mtw_branding_current();}
/* Trace-only scalar admission evidence; never prints identities or source bytes. */
static void nfs_mtw_branding_begin_diagnostic(unsigned reason,unsigned slot){
 const int saved=errno;static unsigned lines=0;uint32_t domain=0,info=0;unsigned char sha[32];
 if(!nfs_logo_trace_enabled()||lines>=16){errno=saved;return;}++lines;
 const NfsMenuTransitionBrandingView*v=&nfs_mtw_brand.view;
 if(nfs_mtw_current())domain|=1u<<0;if(nfs_mtw.phase==2)domain|=1u<<1;
 if(v->serial&&v->serial==nfs_mtw.serial)domain|=1u<<2;if(v->content==nfs_mtw.content)domain|=1u<<3;
 if(v->kind==nfs_mtw.kind)domain|=1u<<4;if(v->form==nfs_mtw.initialForm)domain|=1u<<5;
 if(v->producerEpoch==nfs_mtw.initialEpoch)domain|=1u<<6;if(v->sourceGame==nfs_mtw.sourceGame)domain|=1u<<7;
 if(v->sourceLife==nfs_mtw.sourceLife)domain|=1u<<8;if(v->sourceArchive==nfs_mtw.sourceArchive)domain|=1u<<9;
 if(v->sourceCatalog==nfs_mtw.sourceCatalog)domain|=1u<<10;if(nfs_menu_transition_source_life_current(v->sourceGame,v->sourceLife))domain|=1u<<11;
 if(nfs_logo_provenance.clearEvent==nfs_mtw_brand.clearEvent)domain|=1u<<12;if(nfs_logo_provenance.uploads==nfs_mtw_brand.uploads)domain|=1u<<13;
 if(slot<6){const NfsMenuTransitionBrandingSlot*x=v->slots+slot;const uint32_t address=x->descriptorWords[5];
  if(x->enabled)info|=1u<<0;if(x->index==slot)info|=1u<<1;if(x->descriptorGuest)info|=1u<<2;
  if(x->descriptorWords[0]==1&&x->descriptorWords[1]==1&&x->descriptorWords[2]==3&&x->descriptorWords[3]==GR_TEXFMT_ARGB_1555&&x->descriptorWords[6]==3)info|=1u<<3;
  if(!(address&3)&&address<=TextureMem-32768){TextureInfo*ti=g_textures+(address>>2);info|=1u<<4;
   if(ti->id)info|=1u<<5;if(!ti->hdDigits&&ti->size==128&&ti->fmt==GR_TEXFMT_ARGB_1555)info|=1u<<6;
   if(ti->data==g_textureMem+address)info|=1u<<7;
   if(ti->data==g_textureMem+address&&nfs_texture_sha256((const unsigned char*)ti->data,32768,sha)){NfsNativeMarkArt actual={0};if(nfs_hd_native_mark_art(NFS_NATIVE_MARK_SOURCE_RGBA5551_LE,128,sha,&actual)||nfs_setup_original_mode_digest(sha))info|=1u<<8;}
   if(ti->id!=nfs_mtw.image&&ti->id!=nfs_mtw.result&&ti->id!=nfs_mtw.art[0]&&ti->id!=nfs_mtw.art[1])info|=1u<<9;
   if(nfs_mark_owner())info|=1u<<10;
   for(unsigned k=0;k<NFS_NATIVE_MARK_MAX_BINDINGS;++k){const NfsNativeMarkBinding*b=nfs_native_marks.bindings+k;
    if(b->valid&&b->address==address)info|=1u<<11;
    if(b->valid&&b->address==address&&b->nativeId==ti->id&&b->encoding==NFS_NATIVE_MARK_SOURCE_RGBA5551_LE&&b->edge==128&&b->preparedBytes==32768&&b->generation==nfs_native_marks.generation&&b->revision&&b->texture<NFS_NATIVE_MARK_MAX_TEXTURES){const NfsNativeMarkTexture*t=nfs_native_marks.textures+b->texture;info|=1u<<12;if(t->texture&&t->generation==nfs_native_marks.generation&&t->texture!=nfs_mtw.result&&t->texture!=nfs_mtw.image&&t->texture!=nfs_mtw.art[0]&&t->texture!=nfs_mtw.art[1])info|=1u<<13;}
   }
   if(nfs_mtw_branding_mode(slot,ti->id,0))info|=1u<<14;
  }
 }
 fprintf(stderr,"MENU BRANDING BEGIN reason=%u kind=%u slot=%u domainMask=%u infoFlags=%u phase=%u\n",reason,v->kind,slot,domain,info,nfs_mtw.phase);errno=saved;
}
int nfs_private_menu_branding_begin(const NfsMenuTransitionBrandingView*v){
 const int saved=errno;drawTriangles();nfs_mtw_branding_reset();
 if(!v||v->slotCount!=6||!v->serial||!v->content||!v->producerEpoch||!v->sourceGame||!v->sourceLife||!v->sourceArchive||!v->sourceCatalog||!((v->kind>=5&&v->kind<=7)||v->kind==11)){errno=saved;return 0;}
 nfs_mtw_brand.view=*v;nfs_mtw_brand.uploads=nfs_logo_provenance.uploads;nfs_mtw_brand.clearEvent=nfs_logo_provenance.clearEvent;
 if(!nfs_mtw_branding_current()){nfs_mtw_branding_begin_diagnostic(1,6);goto refused;}
 for(unsigned k=0;k<6;++k){GLuint id=0;int mark=0;if(v->slots[k].index!=k||v->slots[k].enabled>1||(v->slots[k].enabled&&!nfs_mtw_branding_slot(k,&id,&mark))){nfs_mtw_branding_begin_diagnostic(2,k);goto refused;}}
 nfs_mtw_brand.nextSlot=nfs_mtw_branding_next(0);nfs_mtw_brand.selectedSlot=6;nfs_mtw_brand.active=1;nfs_mtw_branding_note(1);errno=saved;return 1;
 refused:nfs_mtw_branding_note(2);nfs_mt_wide_other();errno=saved;return 0;
}
static int nfs_mtw_branding_select(unsigned tmu,uint32_t address,uint32_t parity,const GrTexInfo*info,TextureInfo*ti){
 GLuint id=0;int mark=0;if(!nfs_mtw_brand.active)return 0;
 if(!nfs_mtw_branding_current()||nfs_mtw_brand.armed||nfs_mtw_brand.pending||nfs_mtw_brand.selectedSlot!=6||nfs_mtw_brand.nextSlot>=6||tmu||parity!=3||!info||!ti||info->format!=GR_TEXFMT_ARGB_1555||info->smallLod!=1||info->largeLod!=1||info->aspectRatio!=3||!nfs_mtw_branding_slot(nfs_mtw_brand.nextSlot,&id,&mark)||address!=nfs_mtw_brand.view.slots[nfs_mtw_brand.nextSlot].descriptorWords[5]||ti->id!=id){nfs_mt_wide_other();return 1;}
 nfs_mtw_brand.selectedSlot=nfs_mtw_brand.nextSlot;nfs_mtw_brand.nativeId=id;nfs_mtw_brand.mark=mark;return 1;
}
static float nfs_mtw_branding_float(uint32_t word){float f;memcpy(&f,&word,4);return f;}
static int nfs_mtw_branding_vertex(const GrVertex*v,unsigned index){static const unsigned fields[]={0,1,3,4,5,7,8,9,10};if(!v||sizeof(GrVertex)!=60||index>3)return 0;for(unsigned k=0;k<sizeof fields/sizeof fields[0];++k){uint32_t actual;memcpy(&actual,(const unsigned char*)v+4*fields[k],4);if(actual!=nfs_mtw_brand.quad.nativeVertices[index][fields[k]])return 0;}return 1;}
int nfs_private_menu_branding_quad(const NfsMenuTransitionBrandingQuad*q){
 const int saved=errno;drawTriangles();
 if(!q||!nfs_mtw_brand.active||!nfs_mtw_branding_current()||q->serial!=nfs_mtw_brand.view.serial||q->ordinal!=nfs_mtw_brand.ordinal||q->slot!=nfs_mtw_brand.nextSlot||q->slot!=nfs_mtw_brand.selectedSlot||nfs_mtw_brand.armed||nfs_mtw_brand.pending)goto refused;
 for(unsigned i=0;i<4;++i){static const unsigned fields[]={0,1,3,4,5,7,8,9,10};for(unsigned k=0;k<sizeof fields/sizeof fields[0];++k)if(!isfinite(nfs_mtw_branding_float(q->nativeVertices[i][fields[k]])))goto refused;for(unsigned k=3;k<=7;++k)if(k!=6){const float f=nfs_mtw_branding_float(q->nativeVertices[i][k]);if(f<0||f>255)goto refused;}if(nfs_mtw_branding_float(q->nativeVertices[i][8])!=1.f)goto refused;}
 nfs_mtw_brand.quad=*q;nfs_mtw_brand.triangles=nfs_mtw_brand.drawn=0;nfs_mtw_brand.armed=1;nfs_mtw_branding_note(3);errno=saved;return 1;
 refused:nfs_mt_wide_other();errno=saved;return 0;
}
static int nfs_mtw_branding_triangle(const GrVertex*a,const GrVertex*b,const GrVertex*c){
 if(!nfs_mtw_brand.active)return 0;const unsigned half=nfs_mtw_brand.triangles;
 if(!nfs_mtw_branding_current()||!nfs_mtw_brand.armed||half>=2||!g_textureEnabled||g_fogEnabled||nfs_native_marks.selected!=nfs_mtw_brand.mark||!nfs_mtw_branding_vertex(a,0)||!nfs_mtw_branding_vertex(b,half?2:1)||!nfs_mtw_branding_vertex(c,half?3:2)){nfs_mt_wide_other();return 1;}
 ++nfs_mtw_brand.triangles;++nfs_mtw_brand.pending;return 1;
}
#define NFS_MTW_BRAND_GL(call) do{if(!nfs_mtw_branding_current())goto lost;call;if(!nfs_mtw_branding_current())goto lost;}while(0)
static int nfs_mtw_branding_array(unsigned i,GLint size,GLenum type,GLboolean normalized,const void*pointer){
 GLint enabled=0,n=0,kind=0,normal=0,stride=0,buffer=0;void*p=NULL;if(i>3)return 0;
 NFS_MTW_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_ENABLED,&enabled));NFS_MTW_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_SIZE,&n));NFS_MTW_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_TYPE,&kind));NFS_MTW_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,&normal));NFS_MTW_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_STRIDE,&stride));NFS_MTW_BRAND_GL(glGetVertexAttribiv(i,GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING,&buffer));NFS_MTW_BRAND_GL(glGetVertexAttribPointerv(i,GL_VERTEX_ATTRIB_ARRAY_POINTER,&p));return enabled&&n==size&&(GLenum)kind==type&&normal==normalized&&!stride&&!buffer&&p==pointer;
 lost:return 0;
}
static void nfs_mtw_branding_batch(unsigned count){
 NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_mtw_guard();Vertices moved[2];GLfloat nativeMatrix[16],realMatrix[16],flag=0;GLint fbo=0,program=0,active=0,binding=0,vp[4],clip[4],unit=0,samplerLoc=-1;GLboolean scissor=0,enabled=0,liveTexture=0;GLenum error=0;GLuint id=0;int mark=0,ok=0,switched=0,savedReady=0;const unsigned corners[2][3]={{0,1,2},{0,2,3}};
 if(!nfs_mtw_brand.active||!nfs_mtw_brand.armed||!nfs_mtw_branding_current()||!count||count>2||count!=g_trianglesCount||count!=nfs_mtw_brand.pending||nfs_mtw_brand.drawn+count!=nfs_mtw_brand.triangles||g_fogEnabled||g_textureEnabled!=1||!nfs_mtw_branding_slot(nfs_mtw_brand.selectedSlot,&id,&mark)||id!=nfs_mtw_brand.nativeId||mark!=nfs_mtw_brand.mark)goto refused;
 for(unsigned i=0;i<count;++i)for(unsigned j=0;j<3;++j){const uint32_t*w=nfs_mtw_brand.quad.nativeVertices[corners[nfs_mtw_brand.drawn+i][j]];if(g_vertices[i].vertex[j].x!=nfs_mtw_branding_float(w[0])-VertexSnap||g_vertices[i].vertex[j].y!=nfs_mtw_branding_float(w[1])-VertexSnap||g_vertices[i].vertex[j].z!=1.f||g_textureCoord[i].vertex[j].s!=nfs_mtw_branding_float(w[9])/256.f||g_textureCoord[i].vertex[j].t!=nfs_mtw_branding_float(w[10])/256.f||g_textureCoord[i].vertex[j].q!=1.f||g_textureCoord[i].vertex[j].r!=0.f||g_colorValues[i].vertex[j].r!=(uint8_t)nfs_mtw_branding_float(w[3])||g_colorValues[i].vertex[j].g!=(uint8_t)nfs_mtw_branding_float(w[4])||g_colorValues[i].vertex[j].b!=(uint8_t)nfs_mtw_branding_float(w[5])||g_colorValues[i].vertex[j].a!=(uint8_t)nfs_mtw_branding_float(w[7]))goto refused;}
 if(!nfs_mtw_branding_array((unsigned)g_aPositionLoc,3,GL_FLOAT,GL_FALSE,g_vertices)||!nfs_mtw_branding_array((unsigned)g_aTexCoordLoc,4,GL_FLOAT,GL_FALSE,g_textureCoord)||!nfs_mtw_branding_array((unsigned)g_aColorLoc,4,GL_UNSIGNED_BYTE,GL_TRUE,g_colorValues))goto refused;
 NFS_MTW_BRAND_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_MTW_BRAND_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&program));NFS_MTW_BRAND_GL(glGetIntegerv(GL_ACTIVE_TEXTURE,&active));NFS_MTW_BRAND_GL(glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding));NFS_MTW_BRAND_GL(glGetIntegerv(GL_VIEWPORT,vp));
 if((GLuint)fbo!=nfs_mtw.nativeFbo||(GLuint)program!=g_shaderProgram||active!=GL_TEXTURE0||(GLuint)binding!=id||g_framebufferTexture!=nfs_mtw.nativeColor||g_framebufferDepth!=nfs_mtw.nativeDepth||memcmp(vp,nfs_mt.target.sampling.viewport,sizeof vp)||memcmp(g_matrix,nfs_mt.target.sampling.projection,sizeof g_matrix)||nfs_native_marks.selected!=mark||(mark>=0?!nfs_native_marks.sent:!nfs_mtw_branding_mode(nfs_mtw_brand.selectedSlot,id,1)))goto refused;
 NFS_MTW_BRAND_GL(glGetUniformfv(g_shaderProgram,g_uMatrixLoc,realMatrix));if(memcmp(realMatrix,g_matrix,sizeof realMatrix))goto refused;
 NFS_MTW_BRAND_GL(glGetUniformfv(g_shaderProgram,g_uTextureEnabledLoc,&flag));if(flag!=1.f)goto refused;NFS_MTW_BRAND_GL(glGetUniformfv(g_shaderProgram,g_uFogEnabledLoc,&flag));if(flag!=0.f)goto refused;NFS_MTW_BRAND_GL(samplerLoc=glGetUniformLocation(g_shaderProgram,"uTextureSampler"));if(samplerLoc<0)goto refused;NFS_MTW_BRAND_GL(glGetUniformiv(g_shaderProgram,samplerLoc,&unit));if(unit!=0)goto refused;
 NFS_MTW_BRAND_GL(glGetUniformfv(g_shaderProgram,nfs_native_marks.enable,&flag));if(flag!=(mark>=0?1.f:0.f))goto refused;
 if(mark>=0){NFS_MTW_BRAND_GL(glGetUniformiv(g_shaderProgram,nfs_native_marks.sampler,&unit));if(unit!=2)goto refused;
 NFS_MTW_BRAND_GL(glActiveTexture(GL_TEXTURE2));NFS_MTW_BRAND_GL(glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding));NFS_MTW_BRAND_GL(glActiveTexture(GL_TEXTURE0));if((GLuint)binding!=nfs_native_marks.textures[nfs_native_marks.bindings[mark].texture].texture)goto refused;NFS_MTW_BRAND_GL(liveTexture=glIsTexture((GLuint)binding));if(!liveTexture)goto refused;}
 NFS_MTW_BRAND_GL(enabled=glIsEnabled(GL_STENCIL_TEST));if(enabled)goto refused;NFS_MTW_BRAND_GL(enabled=glIsEnabled(GL_CULL_FACE));if(enabled)goto refused;NFS_MTW_BRAND_GL(enabled=glIsEnabled(GL_POLYGON_OFFSET_FILL));if(enabled)goto refused;
 {GLint sr=0,sa=0,dr=0,da=0,er=0,ea=0;NFS_MTW_BRAND_GL(enabled=glIsEnabled(GL_BLEND));if(!enabled)goto refused;NFS_MTW_BRAND_GL(glGetIntegerv(GL_BLEND_SRC_RGB,&sr));NFS_MTW_BRAND_GL(glGetIntegerv(GL_BLEND_SRC_ALPHA,&sa));NFS_MTW_BRAND_GL(glGetIntegerv(GL_BLEND_DST_RGB,&dr));NFS_MTW_BRAND_GL(glGetIntegerv(GL_BLEND_DST_ALPHA,&da));NFS_MTW_BRAND_GL(glGetIntegerv(GL_BLEND_EQUATION_RGB,&er));NFS_MTW_BRAND_GL(glGetIntegerv(GL_BLEND_EQUATION_ALPHA,&ea));if(sr!=GL_SRC_ALPHA||sa!=GL_SRC_ALPHA||er!=GL_FUNC_ADD||ea!=GL_FUNC_ADD||(dr!=GL_ZERO&&dr!=GL_ONE&&dr!=GL_ONE_MINUS_SRC_ALPHA)||(da!=GL_ZERO&&da!=GL_ONE&&da!=GL_ONE_MINUS_SRC_ALPHA))goto refused;}
 NFS_MTW_BRAND_GL(scissor=glIsEnabled(GL_SCISSOR_TEST));NFS_MTW_BRAND_GL(glGetIntegerv(GL_SCISSOR_BOX,clip));for(unsigned i=0;i<4;++i)if(clip[i]<-16384||clip[i]>16384)goto refused;if(clip[2]<0||clip[3]<0)goto refused;
 const float shift=(nfs_mtw.logical-640.f)*(nfs_mtw.locationInfoReturn?.5f:1.f);
 memcpy(moved,g_vertices,count*sizeof(Vertices));for(unsigned i=0;i<count;++i)for(unsigned j=0;j<3;++j)moved[i].vertex[j].x+=shift;
 memcpy(nativeMatrix,g_matrix,sizeof nativeMatrix);if(!nfs_graphics_scene_save(&saved,&guard))goto lost;savedReady=1;
 NFS_MTW_BRAND_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_mtw.fbo));switched=1;if(!nfs_mtw_attachment(nfs_mtw.result))goto done;NFS_MTW_BRAND_GL(glViewport(0,0,nfs_mtw.width,nfs_mtw.height));NFS_MTW_BRAND_GL(glDepthMask(GL_FALSE));NFS_MTW_BRAND_GL(glDisable(GL_DEPTH_TEST));NFS_MTW_BRAND_GL(glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nfs_mtw.matrix));NFS_MTW_BRAND_GL(glVertexAttribPointer(g_aPositionLoc,3,GL_FLOAT,GL_FALSE,0,moved));
 {const float scale=nfs_mtw.nativeHeight/480.f,out=nfs_mtw.height/480.f;float left=fmaxf(0.f,(float)vp[0]),right=fminf((float)nfs_mt.target.width,(float)(vp[0]+vp[2])),bottom=fmaxf(0.f,(float)vp[1]),upper=fminf((float)nfs_mt.target.height,(float)(vp[1]+vp[3]));
  if(scissor){left=fmaxf(left,(float)clip[0]);right=fminf(right,(float)(clip[0]+clip[2]));bottom=fmaxf(bottom,(float)clip[1]);upper=fminf(upper,(float)(clip[1]+clip[3]));}
  const GLint l=(GLint)floorf((left/scale+shift)*out),r=(GLint)ceilf((right/scale+shift)*out),b=(GLint)floorf(bottom/scale*out),top=(GLint)ceilf(upper/scale*out);NFS_MTW_BRAND_GL(glEnable(GL_SCISSOR_TEST));NFS_MTW_BRAND_GL(glScissor(l,b,r>l?r-l:0,top>b?top-b:0));}
 NFS_MTW_BRAND_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count*3));NFS_MTW_BRAND_GL(error=glGetError());ok=error==GL_NO_ERROR;
 done:if(!nfs_mtw_current())goto lost;NFS_MTW_BRAND_GL(glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nativeMatrix));if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;switched=0;savedReady=0;if(!ok)goto refused;
 if(nfs_mtw_paint==UINT64_MAX)goto refused;++nfs_mtw_paint;nfs_menu_map_source_invalidate(&nfs_mtw_maps);
 nfs_mtw_brand.drawn+=count;nfs_mtw_brand.pending=0;if(nfs_mtw_brand.drawn==2){nfs_mtw_brand.armed=0;++nfs_mtw_brand.ordinal;nfs_mtw_brand.nextSlot=nfs_mtw_branding_next(nfs_mtw_brand.selectedSlot+1);nfs_mtw_brand.selectedSlot=6;}return;
 refused:nfs_mtw_branding_note(8);nfs_mt_wide_other();return;
 lost:if(savedReady&&nfs_mtw_current()){glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nativeMatrix);if(nfs_mtw_current())(void)nfs_graphics_scene_restore(&saved,&guard);}(void)switched;nfs_mt_wide_retired(0);
}
#undef NFS_MTW_BRAND_GL
int nfs_private_menu_branding_end(uint64_t serial){const int saved=errno;drawTriangles();if(!nfs_mtw_brand.active||serial!=nfs_mtw_brand.view.serial||!nfs_mtw_branding_current()||nfs_mtw_brand.armed||nfs_mtw_brand.pending||nfs_mtw_brand.nextSlot!=6){nfs_mt_wide_other();errno=saved;return 0;}nfs_mtw_brand.active=0;nfs_mtw_brand.completed=1;nfs_mtw_branding_note(9);errno=saved;return 1;}
void nfs_private_menu_branding_cancel(uint64_t serial){const int saved=errno;if(serial&&serial==nfs_mtw_brand.view.serial)nfs_mt_wide_other();errno=saved;}

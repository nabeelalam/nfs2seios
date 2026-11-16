// SPDX-License-Identifier: MIT
// Repeat only the original cursor batch on the current full-width Results target.
static int nfs_rsw_cursor_current(void){
 return nfs_rsw_current()&&nfs_rsw.cursor.request&&!nfs_rsw.cursorComplete;
}

// The original native batch supplies blend and fog. Check the source texture
// and actual render target used by that batch, before replaying its geometry.
static const char*nfs_rsw_tail_target_refusal(GLuint expectedTexture){
 GLint unit=0,texture=0,fbo=0,program=0,viewport[4];
 glGetIntegerv(GL_ACTIVE_TEXTURE,&unit);
 if(unit!=GL_TEXTURE0)glActiveTexture(GL_TEXTURE0);
 glGetIntegerv(GL_TEXTURE_BINDING_2D,&texture);
 if(unit!=GL_TEXTURE0)glActiveTexture((GLenum)unit);
 glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);
 glGetIntegerv(GL_CURRENT_PROGRAM,&program);
 glGetIntegerv(GL_VIEWPORT,viewport);
 if((GLuint)fbo!=nfs_rsw.nativeTarget.framebuffer||
    memcmp(viewport,nfs_rsw.nativeTarget.sampling.viewport,sizeof viewport)||
    memcmp(g_matrix,nfs_rsw.nativeTarget.sampling.projection,sizeof g_matrix))
  return "native-target";
 if((GLuint)program!=g_shaderProgram||g_textureEnabled!=1)return "native-program";
 if((GLuint)texture!=expectedTexture)return "native-texture";
 return NULL;
}

static int nfs_rsw_cursor_mark(int pin){
 if(!nfs_rsw_cursor_current())return 0;
 const int selected=nfs_native_marks.selected;
 if(selected<0||selected>=NFS_NATIVE_MARK_MAX_BINDINGS||!nfs_mark_owner()||
    nfs_native_marks.program!=g_shaderProgram||nfs_hd_selected){
  nfs_rsw_trace("cursor-mark","mark-selection",nfs_rsw.width,nfs_rsw.height);
  return 0;
 }
 const NfsNativeMarkBinding*b=nfs_native_marks.bindings+selected;
 const uint32_t at=nfs_rsw.cursor.address;
 if(!b->valid||b->encoding!=NFS_NATIVE_MARK_SOURCE_RGBA4444_LE||b->edge!=32||
    b->preparedBytes!=2048||b->address!=at||b->generation!=nfs_native_marks.generation||
    !b->revision||b->texture>=NFS_NATIVE_MARK_MAX_TEXTURES||at>TextureMem-2048){
  nfs_rsw_trace("cursor-mark","mark-binding",nfs_rsw.width,nfs_rsw.height);
  return 0;
 }
 const TextureInfo*ti=g_textures+(at>>2);
 const NfsNativeMarkTexture*t=nfs_native_marks.textures+b->texture;
 if(ti->id!=b->nativeId||ti->fmt!=GR_TEXFMT_ARGB_4444||ti->size!=32||
    ti->data!=g_textureMem+at||ti->hdDigits||!t->texture||
    t->generation!=nfs_native_marks.generation||!t->art.identity||
    t->texture==nfs_rsw.color||t->texture==nfs_rsw.nativeColor){
  nfs_rsw_trace("cursor-mark","mark-texture",nfs_rsw.width,nfs_rsw.height);
  return 0;
 }
 const char*reason=nfs_rsw_tail_target_refusal(b->nativeId);
 if(reason){
  nfs_rsw_trace("cursor-mark",reason,nfs_rsw.width,nfs_rsw.height);
  return 0;
 }
 if(pin)nfs_rsw.cursorRevision=b->revision;
 return b->revision==nfs_rsw.cursorRevision;
}

static void nfs_results_wide_branding_select(unsigned tmu,uint32_t address,uint32_t parity,
 const GrTexInfo*info,const TextureInfo*texture){
 const NfsResultsCursor*v=&nfs_rsw_cursor_pending.view;
 if(!v->request||!v->branding||!nfs_rsw.ready)return;
 if(nfs_rsw_cursor_pending.brandingTexture||tmu||parity!=3||address!=v->address||
    !info||info->smallLod||info->largeLod||info->aspectRatio!=3||
    info->format!=GR_TEXFMT_ARGB_1555||!texture||texture->size!=256||
    texture->fmt!=info->format||texture->data!=g_textureMem+address||
    !texture->id||texture->id==nfs_rsw.color||texture->id==nfs_rsw.nativeColor||
    texture->hdDigits||!glIsTexture(texture->id)){
  nfs_results_wide_invalidate("branding-select");
  return;
 }
 nfs_rsw_cursor_pending.brandingTexture=texture->id;
}

static int nfs_rsw_branding_texture(void){
 if(!nfs_rsw_cursor_current()||!nfs_rsw_cursor_pending.brandingTexture)return 0;
 const char*reason=nfs_rsw_tail_target_refusal(nfs_rsw_cursor_pending.brandingTexture);
 if(reason)nfs_rsw_trace("branding-draw",reason,nfs_rsw.width,nfs_rsw.height);
 return !reason;
}

int nfs_private_results_cursor_begin(const NfsResultsCursor*v){
 drawTriangles();
 if(!v||!v->request||!v->game||!v->epoch||!v->content||v->branding>1||
    (v->branding?v->edge!=256:(v->edge<1||v->edge>64))||
    (v->address&3)||v->address>TextureMem-(v->branding?131072u:2048u)||
    !nfs_results_context(nfs_results_renderer.generation,nfs_results_renderer.context)){
  nfs_rsw_trace("cursor-begin","source-or-context",nfs_rsw.width,nfs_rsw.height);
  return 0;
 }
 if(nfs_rsw_cursor_pending.view.request){
  memset(&nfs_rsw_cursor_pending,0,sizeof nfs_rsw_cursor_pending);
  nfs_results_wide_invalidate("cursor-overlap");
  return 0;
 }
 nfs_rsw_cursor_pending.view=*v;
 nfs_rsw_cursor_pending.generation=nfs_results_renderer.generation;
 nfs_rsw_cursor_pending.context=nfs_results_renderer.context;
 return 1;
}

// Bind only at the actual first triangle, once the frame and its upload stamp
// are current. An unmatched triangle still invalidates the wide frame.
static int nfs_rsw_cursor_bind(void){
 const NfsResultsCursor*v=&nfs_rsw_cursor_pending.view;
 if(!v->request||!nfs_rsw_current()||nfs_rsw.cursor.request||
    nfs_rsw_cursor_pending.generation!=nfs_rsw.generation||
    nfs_rsw_cursor_pending.context!=nfs_rsw.context||
    v->game!=nfs_rsw.view.sourceGame||v->epoch!=nfs_rsw.view.sourceEpoch||
    v->content!=nfs_rsw.view.content||(!v->branding&&(v->x<-64||v->x>704||v->y<-64||v->y>544)))
  return 0;
 nfs_rsw.cursor=*v;
 return 1;
}

static const unsigned nfs_rsw_cursor_x[4]={0,1,1,0};
static const unsigned nfs_rsw_cursor_y[4]={0,0,1,1};
static const unsigned nfs_rsw_cursor_corners[2][3]={{0,1,2},{0,2,3}};

static int nfs_results_wide_cursor_triangle(const GrVertex*a,const GrVertex*b,const GrVertex*c){
 const GrVertex*in[3]={a,b,c};
 if(!nfs_rsw.ready)return 0;
 if(!nfs_rsw.cursor.request&&!nfs_rsw_cursor_bind()){
  nfs_rsw_trace("cursor-triangle",nfs_rsw_cursor_pending.view.request?"receipt-mismatch":"receipt-missing",nfs_rsw.width,nfs_rsw.height);
  return 0;
 }
 if(!nfs_rsw_cursor_current()||nfs_rsw.cursorTriangles>=2){
  nfs_rsw_trace("cursor-triangle",nfs_rsw.cursor.request?"scope-mismatch":"scope-unarmed",nfs_rsw.width,nfs_rsw.height);
  return 0;
 }
 if(nfs_rsw.cursor.branding){
  if(!nfs_rsw_cursor_pending.brandingTexture){
   nfs_rsw_trace("branding-draw","selection-missing",nfs_rsw.width,nfs_rsw.height);
   return 0;
  }
  if(!nfs_rsw_branding_texture())return 0;
  for(unsigned i=0;i<3;++i){
   if(!in[i])return 0;
   float words[15];
   memcpy(words,in[i],sizeof words);
   if(!isfinite(words[0])||!isfinite(words[1])||
      words[0]<VertexSnap-512||words[0]>VertexSnap+1152||
      words[1]<VertexSnap-512||words[1]>VertexSnap+992||
      !isfinite(words[8])||!isfinite(words[9])||!isfinite(words[10])){
    nfs_rsw_trace("branding-draw","vertex-range",nfs_rsw.width,nfs_rsw.height);
    return 0;
   }
  }
  ++nfs_rsw.cursorTriangles;
  return 1;
 }
 if(!nfs_rsw_cursor_mark(!nfs_rsw.cursorTriangles)){
  nfs_rsw_trace("cursor-triangle","mark-mismatch",nfs_rsw.width,nfs_rsw.height);
  return 0;
 }
 for(unsigned i=0;i<3;++i){
  if(!in[i])return 0;
  float words[15];
  memcpy(words,in[i],sizeof words);
  const unsigned k=nfs_rsw_cursor_corners[nfs_rsw.cursorTriangles][i];
  const unsigned x=nfs_rsw_cursor_x[k],y=nfs_rsw_cursor_y[k];
  if(words[0]!=nfs_rsw.cursor.x+(int)x*nfs_rsw.cursor.edge+VertexSnap||
     words[1]!=nfs_rsw.cursor.y+(int)y*nfs_rsw.cursor.edge+VertexSnap||
     words[3]!=255||words[4]!=255||words[5]!=255||words[7]!=255||
     words[8]!=1||words[9]!=255.f*x||words[10]!=255.f*y){
   nfs_rsw_trace("cursor-triangle","vertex-mismatch",nfs_rsw.width,nfs_rsw.height);
   return 0;
  }
 }
 ++nfs_rsw.cursorTriangles;
 return 1;
}

static void nfs_results_wide_cursor_batch(unsigned count){
 if(!nfs_rsw.ready)return;
 if(!nfs_rsw_cursor_current()||nfs_rsw.cursorTriangles!=2||nfs_rsw.cursorBatch||
    count!=2||count!=g_trianglesCount||
    (nfs_rsw.cursor.branding?!nfs_rsw_branding_texture():(!nfs_rsw_cursor_mark(0)||!nfs_native_marks.sent))){
  nfs_results_wide_invalidate("cursor-batch");
  return;
 }
 if(!nfs_rsw.cursor.branding)for(unsigned i=0;i<2;++i)for(unsigned j=0;j<3;++j){
  const unsigned corner=nfs_rsw_cursor_corners[i][j];
  const unsigned x=nfs_rsw_cursor_x[corner],y=nfs_rsw_cursor_y[corner];
  if(g_vertices[i].vertex[j].x!=nfs_rsw.cursor.x+(int)x*nfs_rsw.cursor.edge||
     g_vertices[i].vertex[j].y!=nfs_rsw.cursor.y+(int)y*nfs_rsw.cursor.edge||
     g_vertices[i].vertex[j].z!=1||
     g_textureCoord[i].vertex[j].s!=255.f*x/256.f||g_textureCoord[i].vertex[j].t!=255.f*y/256.f||
     g_textureCoord[i].vertex[j].r!=0||g_textureCoord[i].vertex[j].q!=1){
   nfs_results_wide_invalidate("cursor-geometry");
   return;
  }
 }
 NfsBgSaved saved;
 GLfloat nativeMatrix[16];
 GLint scissor[4];
 int ok=0;
 nfs_bg_save(&saved);memcpy(nativeMatrix,g_matrix,sizeof nativeMatrix);glGetIntegerv(GL_SCISSOR_BOX,scissor);
#define NFS_RSW_CURSOR_GL(call) do{if(!nfs_rsw_cursor_current())goto cursor_done;call;if(!nfs_rsw_cursor_current())goto cursor_done;}while(0)
 NFS_RSW_CURSOR_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_rsw.fbo));if(!nfs_rsw_own())goto cursor_done;NFS_RSW_CURSOR_GL(glViewport(0,0,nfs_rsw.width,nfs_rsw.height));NFS_RSW_CURSOR_GL(glDepthMask(GL_FALSE));NFS_RSW_CURSOR_GL(glDisable(GL_DEPTH_TEST));NFS_RSW_CURSOR_GL(glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nfs_rsw.sampling.projection));NFS_RSW_CURSOR_GL(glVertexAttribPointer(g_aPositionLoc,3,GL_FLOAT,GL_FALSE,0,g_vertices));
 if(saved.common.caps[4]){const float*old=nativeMatrix,*m=nfs_rsw.sampling.projection;const GLint*vp=nfs_rsw.nativeTarget.sampling.viewport;float l=((scissor[0]-vp[0])*2.f/vp[2]-1-old[12])/old[0],r=((scissor[0]+scissor[2]-vp[0])*2.f/vp[2]-1-old[12])/old[0],top=((scissor[1]+scissor[3]-vp[1])*2.f/vp[3]-1-old[13])/old[5],bottom=((scissor[1]-vp[1])*2.f/vp[3]-1-old[13])/old[5];GLint x=(GLint)floorf((m[0]*l+m[12]+1)*nfs_rsw.width*.5f),right=(GLint)ceilf((m[0]*r+m[12]+1)*nfs_rsw.width*.5f),y=(GLint)floorf((m[5]*bottom+m[13]+1)*nfs_rsw.height*.5f),end=(GLint)ceilf((m[5]*top+m[13]+1)*nfs_rsw.height*.5f);if(right<x||end<y)goto cursor_done;NFS_RSW_CURSOR_GL(glScissor(x,y,right-x,end-y));}else NFS_RSW_CURSOR_GL(glDisable(GL_SCISSOR_TEST));
 NFS_RSW_CURSOR_GL(glDrawArrays(GL_TRIANGLES,0,6));ok=glGetError()==GL_NO_ERROR;
 cursor_done:if(nfs_results_context(nfs_rsw.generation,nfs_rsw.context)){glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nativeMatrix);nfs_bg_restore(&saved);}else ok=0;
#undef NFS_RSW_CURSOR_GL

 if(!ok||!nfs_rsw_cursor_current()){
  nfs_results_wide_invalidate("cursor-draw");
  return;
 }
 nfs_rsw.cursorBatch=1;
}

int nfs_private_results_cursor_end(uint64_t request,int sourceValid){
 drawTriangles();
 const int receiptMatches=request&&request==nfs_rsw_cursor_pending.view.request;
 memset(&nfs_rsw_cursor_pending,0,sizeof nfs_rsw_cursor_pending);
 if(!receiptMatches||!sourceValid||!nfs_rsw_cursor_current()||request!=nfs_rsw.cursor.request||
    nfs_rsw.cursorTriangles!=2||!nfs_rsw.cursorBatch){
  nfs_results_wide_invalidate("cursor-incomplete");
  return 0;
 }
 if(nfs_rsw.cursor.branding){
  nfs_rsw.cursor.request=0;
  nfs_rsw.cursorTriangles=nfs_rsw.cursorBatch=0;
 }else nfs_rsw.cursorComplete=1;
 return 1;
}

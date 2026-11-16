// SPDX-License-Identifier: MIT
/* Synchronous typed6/7/9/10 caption and9/10 preview replay only, after native admission and before its
   strong packet borrow ends. No native framebuffer/depth pixels are sampled. */
#define NFS_MTW_PREVIEW_GL(call) do{if(!nfs_graphics_scene_current(t)||!nfs_mt_current())return 0;call;if(!nfs_graphics_scene_current(t)||!nfs_mt_current())return 0;}while(0)
static int nfs_mtw_initial_owned(const NfsMenuTransitionView*v,const NfsBackgroundSceneStage*t,float matrix[16]){
 float logical;
 if(!v||!t||(v->kind!=6&&v->kind!=7&&v->kind!=9&&v->kind!=10)||!nfs_mt_current()||!nfs_mt.active||nfs_mt.serial!=v->serial||nfs_mt.sourceKind!=v->kind||!nfs_menu_transition_source_life_current(v->sourceGame,v->sourceLife)||!nfs_graphics_scene_current(t)||t->generation!=nfs_mt.context||t->contextIdentity!=nfs_mt.identity||t->framebuffer==nfs_mt.target.framebuffer||!t->sampling.projection[0])return 0;
 if(v->slot!=((v->kind==6||v->kind==7)?0u:1u)||v->drawKind!=2||v->originX||v->originY||v->stepX!=32)return 0;
 if((v->kind==7||v->kind==9)?!nfs_mt_location_valid(&v->locationInitial):!nfs_mt_opponent_valid(&v->opponentInitial))return 0;
 logical=2.f/t->sampling.projection[0];if(!isfinite(logical)||logical<=640.f||logical>854.f)return 0;
 memcpy(matrix,t->sampling.projection,16*sizeof(float));if(v->kind==9||v->kind==10||v->locationInfoReturn)matrix[12]+=(logical-640.f)/logical;return 1;
}
static int nfs_mtw_initial_texture(const NfsBackgroundSceneStage*t,GLuint texture){
 GLboolean exists=0;GLint type=0,name=0;
 if(!texture)return 0;
 NFS_MTW_PREVIEW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,&type));
 NFS_MTW_PREVIEW_GL(glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,&name));
 if(type!=GL_TEXTURE||!name||(GLuint)name==texture)return 0;
 NFS_MTW_PREVIEW_GL(exists=glIsTexture(texture));return exists!=0;
}
static int nfs_mtw_initial_preview(const NfsMenuTransitionView*v,const NfsBackgroundSceneStage*t){
 float matrix[16];NfsBackgroundSceneStage centered;unsigned char sha[32];const NfsOriginalPreviewLayerView*p;
 if(v&&(v->kind==5||v->kind==6||v->kind==7||v->kind==8))return nfs_graphics_scene_current(t)&&nfs_mt_current();
 if(!v||(v->kind!=9&&v->kind!=10)||!nfs_mtw_initial_owned(v,t,matrix))return 0;p=&v->initialFullPreview;
 if(!nfs_bg_preview_view(p)||nfs_mt.previewLocationArtGeneration!=p->locationArtGeneration||!nfs_mt.previewReady||nfs_mt_preview_alias(nfs_mt.preview)||!nfs_mtw_initial_texture(t,nfs_mt.preview))return 0;
 if(!nfs_texture_sha256(p->coverage,p->coverageCount,sha)||memcmp(sha,nfs_mt.previewCoverageSha,32))return 0;
 if(p->schema==NFS_AUTHORED_PREVIEW_LAYER_SCHEMA){if(nfs_mt.previewWidth!=p->authoredWidth||nfs_mt.previewHeight!=p->authoredHeight||memcmp(p->authoredSha256,nfs_mt.previewSha,32))return 0;}
 else if(nfs_mt.previewWidth!=p->width||nfs_mt.previewHeight!=p->height||memcmp(p->sha256,nfs_mt.previewSha,32))return 0;
 centered=*t;memcpy(centered.sampling.projection,matrix,sizeof matrix);
 NFS_MTW_PREVIEW_GL(glActiveTexture(GL_TEXTURE0));
 return nfs_mt_preview_draw(&centered,p)&&nfs_graphics_scene_current(t)&&nfs_mt_current()&&nfs_bg_preview_generation_current(p);
}
static int nfs_mtw_initial_preview_caption(const NfsMenuTransitionView*v,const NfsBackgroundSceneStage*t){
 float matrix[16];GLuint program=0,texture=0;GLint uniformMatrix=0,sampler=0;GLenum error=0;GLboolean exists=0;GLfloat quad[24];const NfsSoftwareTextVertex*vertices=NULL;size_t count=0;
 if(!nfs_mtw_initial_owned(v,t,matrix))return 0;
 if(v->kind==7||v->kind==9){const NfsCompactCaptionWordView*w=&v->locationInitial.caption;NfsLocationCaptionGpu*s=&nfs_mt.locationCaption;
  if(s->generation!=t->generation||s->word!=2||!nfs_location_caption_valid(w,2)||nfs_location_caption_empty(w)||nfs_mt_location_alias())return 0;
  {const GLfloat q[]={w->left,w->top,0,0,w->right,w->top,1,0,w->left,w->bottom,0,1,w->left,w->bottom,0,1,w->right,w->top,1,0,w->right,w->bottom,1,1};memcpy(quad,q,sizeof quad);}
  program=s->program;texture=s->texture;uniformMatrix=s->matrix;sampler=s->sampler;count=6;
 }else{NfsSoftwareTextGpu*s=&nfs_mt.opponentCaption;const NfsSoftwareTextLayerView*w=&v->opponentInitial.caption;
  if(!nfs_soft_gpu_ready(s)||s->generation!=t->generation||s->fontIdentity!=w->font.identity||memcmp(s->fontSha256,w->font.sha256,32)||nfs_mt_initial_font_alias(s)||!w->vertices||!w->vertexCount||w->vertexCount>4096)return 0;
  program=s->program;texture=s->texture;uniformMatrix=s->matrix;sampler=s->sampler;vertices=w->vertices;count=w->vertexCount;
 }
 if(!program||!nfs_mtw_initial_texture(t,texture))return 0;NFS_MTW_PREVIEW_GL(exists=glIsProgram(program));if(!exists)return 0;
 for(unsigned i=0;i<8;++i){NFS_MTW_PREVIEW_GL(glDisable(nfs_frame_caps[i]));}
 NFS_MTW_PREVIEW_GL(glColorMask(1,1,1,1));NFS_MTW_PREVIEW_GL(glDepthMask(GL_FALSE));NFS_MTW_PREVIEW_GL(glActiveTexture(GL_TEXTURE0));NFS_MTW_PREVIEW_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_MTW_PREVIEW_GL(glUseProgram(program));NFS_MTW_PREVIEW_GL(glUniformMatrix4fv(uniformMatrix,1,GL_FALSE,matrix));NFS_MTW_PREVIEW_GL(glUniform1i(sampler,0));NFS_MTW_PREVIEW_GL(glBindTexture(GL_TEXTURE_2D,texture));NFS_MTW_PREVIEW_GL(glEnableVertexAttribArray(0));
 if(vertices){NFS_MTW_PREVIEW_GL(glEnableVertexAttribArray(1));NFS_MTW_PREVIEW_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),vertices));NFS_MTW_PREVIEW_GL(glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(NfsSoftwareTextVertex),&vertices[0].r));}
 else{NFS_MTW_PREVIEW_GL(glDisableVertexAttribArray(1));NFS_MTW_PREVIEW_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,quad));}
 NFS_MTW_PREVIEW_GL(glDisableVertexAttribArray(2));NFS_MTW_PREVIEW_GL(glDisableVertexAttribArray(3));if(!nfs_graphics_scene_blend(t,1))return 0;
 NFS_MTW_PREVIEW_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count));NFS_MTW_PREVIEW_GL(error=glGetError());return error==GL_NO_ERROR;
}
#undef NFS_MTW_PREVIEW_GL

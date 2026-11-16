// SPDX-License-Identifier: MIT
/* Actual known native foreground only. Deferred triangles are never authority:
   each actual submitted batch must pass and consume the exact pending count. */
static int nfs_mtw_full_form(void){return nfs_mtw.kind==8?(nfs_mtw.ownerForm==NFS_MENU_CACHED_SETUP||nfs_mtw.ownerForm==NFS_MENU_CACHED_OPTIONS):nfs_mtw.kind==9?nfs_mtw.ownerForm==NFS_MENU_CACHED_LOCATION:nfs_mtw.kind==10?nfs_mtw.ownerForm==NFS_MENU_CACHED_OPPONENT:0;}
static int nfs_mtw_tail_source(uint64_t*epoch,float*x,float*y){
 if(!nfs_mtw_current()||!nfs_mtw_full_form()||!nfs_mtw.producerEpoch||!nfs_mtw.descriptor||!nfs_menu_transition_source_life_current(nfs_mtw.sourceGame,nfs_mtw.sourceLife))return 0;
 if(!nfs_mtw.cached.schema){if(nfs_mtw.tailTriangles||!nfs_menu_cached_snapshot(nfs_mtw.ownerForm,nfs_mtw.descriptor,nfs_mtw.producerEpoch,nfs_mtw.sourceGame,nfs_mtw.serial,nfs_mtw.content,&nfs_mtw.cached))return 0;}
 if(!nfs_menu_cached_packet_matches(&nfs_mtw.cached,nfs_mtw.ownerForm,nfs_mtw.serial,nfs_mtw.content)||nfs_mtw.cached.descriptor!=nfs_mtw.descriptor||nfs_mtw.cached.producerEpoch!=nfs_mtw.producerEpoch||nfs_mtw.cached.game!=nfs_mtw.sourceGame)return 0;
 if(!nfs_menu_cached_current(&nfs_mtw.cached,epoch,x,y))return 0;nfs_mtw.ownerEpoch=*epoch;return 1;
}
static void nfs_mt_wide_nonpaint(void){
 if(nfs_mtw.phase==2&&nfs_mtw_current()&&nfs_menu_transition_source_life_current(nfs_mtw.sourceGame,nfs_mtw.sourceLife)&&((nfs_mtw_full_form()&&nfs_mtw.producerEpoch&&nfs_mtw.descriptor)||nfs_mtw.kind==1||nfs_mtw.kind==3||(nfs_mtw.kind>=4&&nfs_mtw.kind<=7)||nfs_mtw.kind==11))return;
 nfs_mt_wide_cancel();
}
static int nfs_mt_wide_defer_triangle(void){
 uint64_t epoch=0;float x=0,y=0;if(nfs_mtw.phase!=2||!nfs_mtw_tail_source(&epoch,&x,&y)||nfs_mtw.pendingTriangles>=MaxTriangles)return 0;++nfs_mtw.pendingTriangles;return 1;
}
static void nfs_mtw_tail_reject(unsigned reason,unsigned count){static unsigned lines=0;const int saved=errno;if(nfs_logo_trace_enabled()&&lines<64){++lines;fprintf(stderr,"MENU TRANSITION WIDE tail kind=%u reason=%u count=%u pending=%u form=%08x serial=%llu\n",nfs_mtw.kind,reason,count,nfs_mtw.pendingTriangles,nfs_mtw.ownerForm,(unsigned long long)nfs_mtw.serial);}errno=saved;}
static void nfs_mtw_tail_diag(unsigned stage,unsigned reason,int detail,float a,float b,float z,float q){(void)stage;(void)detail;(void)a;(void)b;(void)z;(void)q;if(reason)nfs_mtw_tail_reject(reason,0);}
/* Failure-only actual geometry evidence; classification is diagnostic only. */
static void nfs_mtw_tail_vertex_note(unsigned index,float x,float y,float z,float q,int mark,int original,int slot5,int selected){
 static unsigned lines;const int saved=errno;if(lines>=24||!nfs_logo_trace_enabled()){errno=saved;return;}++lines;NfsMenuResponsiveDebugState source;memset(&source,0,sizeof source);nfs_menu_responsive_debug_state(&source);uint32_t address=0,encoding=0;unsigned valid=0,sourceSlot5=0;
 if(selected>=0&&selected<NFS_NATIVE_MARK_MAX_BINDINGS){const NfsNativeMarkBinding*b=nfs_native_marks.bindings+selected;valid=b->valid;address=b->address;encoding=b->encoding;sourceSlot5=valid&&source.originalOpponentValid&&address==source.originalOpponentAddress;}
 fprintf(stderr,"MENU TRANSITION WIDE rejected-vertex kind=%u form=%x serial=%llu index=%u x=%.9g y=%.9g z=%.9g q=%.9g mark=%d selected=%d markValid=%u encoding=%u fontTag=%d fontPage=%d slot5Canvas=%d originalBranding=%d selectedSourceSlot5=%u selectedAddress=%u sourceSlot5Address=%u sourceSlot5Valid=%u sourceForm=%x sourceEpoch=%llu currentEpoch=%llu\n",nfs_mtw.kind,nfs_mtw.ownerForm,(unsigned long long)nfs_mtw.serial,index,(double)x,(double)y,(double)z,(double)q,mark,selected,valid,encoding,nfs_hd_selected,nfs_front_font_source_page(nfs_hd_selected),slot5,original,sourceSlot5,address,source.originalOpponentAddress,source.originalOpponentValid,source.sourceForm,(unsigned long long)source.sourceEpoch,(unsigned long long)nfs_mtw.ownerEpoch);errno=saved;
}
#define NFS_MTW_TAIL_GL(call) do{if(!nfs_mtw_current())goto lost;call;if(!nfs_mtw_current())goto lost;}while(0)
#if defined(__GNUC__) || defined(__clang__)
#define NFS_MTW_NOINLINE __attribute__((noinline))
#elif defined(_MSC_VER)
#define NFS_MTW_NOINLINE __declspec(noinline)
#else
#define NFS_MTW_NOINLINE
#endif
static NFS_MTW_NOINLINE void nfs_mtw_tail_replay(unsigned count){
 NfsBgSaved saved;NfsBackgroundSceneStage guard=nfs_mtw_guard();Vertices moved[MaxTriangles];float shifts[MaxTriangles];uint64_t epoch=0;float sx=0,sy=0;GLint fbo=0,program=0,clip[4],nativeViewport[4];GLfloat nativeMatrix[16];int ok=0,cursor=0;NfsMenuResponsivePointer pointer;NfsMenuResponsiveLayout shown;
 if(count!=nfs_mtw.pendingTriangles){nfs_mtw_tail_reject(16,count);goto refused;}
 if(!nfs_mtw_current()){nfs_mtw_tail_diag(3,2,-1,0,0,0,0);nfs_mtw_tail_reject(2,count);goto refused;}
 if(!count||count!=g_trianglesCount||count>MaxTriangles){nfs_mtw_tail_diag(3,3,-1,(float)count,(float)g_trianglesCount,0,0);nfs_mtw_tail_reject(3,count);goto refused;}
 if(g_fogEnabled||g_textureEnabled!=1){nfs_mtw_tail_diag(3,4,-1,g_textureEnabled,1,0,0);nfs_mtw_tail_reject(4,count);goto refused;}
 if(!nfs_mtw_tail_source(&epoch,&sx,&sy)||epoch!=nfs_mtw.ownerEpoch){nfs_mtw_tail_diag(3,5,-1,0,0,0,0);nfs_mtw_tail_reject(5,count);goto refused;}
 const int selected=nfs_native_marks.selected;
 const int mark=selected>=0&&selected<NFS_NATIVE_MARK_MAX_BINDINGS&&nfs_native_marks.bindings[selected].valid&&nfs_native_marks.bindings[selected].generation==nfs_native_marks.generation&&nfs_mark_owner()&&nfs_native_marks.program==g_shaderProgram;
 const int originalBranding=!mark&&nfs_menu_original_branding_for(nfs_mtw.ownerForm,epoch);
 const int originalMode=!mark&&!originalBranding&&nfs_menu_mode_original_branding_for(nfs_mtw.ownerForm,epoch)&&nfs_menu_wide_setup_mode_pulse(count);
 const int slot5Canvas=(mark||originalBranding)&&nfs_menu_wide_slot5_square_for(count,epoch,nfs_mtw.ownerForm,originalBranding,selected);
 if(originalBranding&&!slot5Canvas){nfs_mtw_tail_diag(3,15,5,(float)count,2,0,0);nfs_mtw_tail_reject(15,count);goto refused;}
 if(!mark&&!originalBranding&&!originalMode&&nfs_front_font_source_page(nfs_hd_selected)<0){nfs_mtw_tail_diag(3,6,-1,0,0,0,0);nfs_mtw_tail_reject(6,count);goto refused;}
 if(originalBranding)nfs_mtw_tail_diag(3,14,5,(float)nfs_menu_original_selection.address,(float)nfs_menu_original_selection.id,0,0);
 cursor=mark&&nfs_native_marks.bindings[selected].encoding==NFS_NATIVE_MARK_SOURCE_RGBA4444_LE;
 GLint active=0,binding=0;GLboolean liveTexture=0;NFS_MTW_TAIL_GL(glGetIntegerv(GL_ACTIVE_TEXTURE,&active));if(active!=GL_TEXTURE0)goto refused;NFS_MTW_TAIL_GL(glGetIntegerv(GL_TEXTURE_BINDING_2D,&binding));
 if(!binding||(GLuint)binding==nfs_mtw.result||(GLuint)binding==nfs_mtw.image||(GLuint)binding==nfs_mtw.art[0]||(GLuint)binding==nfs_mtw.art[1])goto refused;NFS_MTW_TAIL_GL(liveTexture=glIsTexture((GLuint)binding));if(!liveTexture)goto refused;
 if(!mark&&!originalBranding&&!originalMode){const int page=nfs_front_font_source_page(nfs_hd_selected);TextureInfo*owner=page>=0?nfs_front_source_owner[page]:NULL;if(page<0||!nfs_front_font_valid[page]||!nfs_front_source_matched[page]||!owner||owner->id!=(GLuint)binding||owner->hdDigits!=nfs_hd_selected||owner->size!=256||owner->fmt!=GR_TEXFMT_ARGB_4444)goto refused;}

 NFS_MTW_TAIL_GL(glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo));NFS_MTW_TAIL_GL(glGetIntegerv(GL_CURRENT_PROGRAM,&program));
 if((GLuint)fbo!=nfs_mtw.nativeFbo||(GLuint)program!=g_shaderProgram||g_framebufferTexture!=nfs_mtw.nativeColor||g_framebufferDepth!=nfs_mtw.nativeDepth||g_framebufferHeight!=(int)nfs_mtw.nativeHeight){nfs_mtw_tail_diag(3,7,-1,0,0,0,0);nfs_mtw_tail_reject(7,count);goto refused;}
 GLboolean stencil=0,cull=0,polygon=0;NFS_MTW_TAIL_GL(stencil=glIsEnabled(GL_STENCIL_TEST));NFS_MTW_TAIL_GL(cull=glIsEnabled(GL_CULL_FACE));NFS_MTW_TAIL_GL(polygon=glIsEnabled(GL_POLYGON_OFFSET_FILL));
 if(stencil||cull||polygon){nfs_mtw_tail_diag(3,8,-1,0,0,0,0);nfs_mtw_tail_reject(8,count);goto refused;}
 NFS_MTW_TAIL_GL(glGetIntegerv(GL_VIEWPORT,nativeViewport));
 if(nativeViewport[2]<=0||nativeViewport[3]<=0){nfs_mtw_tail_diag(3,9,-1,0,0,0,0);nfs_mtw_tail_reject(9,count);goto refused;}
 {const float scale=nfs_mtw.nativeHeight/480.f;float expected[16]={0};
  expected[0]=2.f*scale/nativeViewport[2];expected[5]=-2.f*scale/nativeViewport[3];expected[10]=-2;
  expected[12]=-1.f-2.f*nativeViewport[0]/nativeViewport[2];expected[13]=1.f+2.f*(nfs_mtw.nativeHeight-nativeViewport[1]-nativeViewport[3])/nativeViewport[3];expected[14]=expected[15]=1;
  for(unsigned k=0;k<16;++k)if(!isfinite(g_matrix[k])||fabsf(g_matrix[k]-expected[k])>0.00001f){nfs_mtw_tail_diag(3,10,(int)k,g_matrix[k],expected[k],0,0);nfs_mtw_tail_reject(10,count);goto refused;}
 }

 memcpy(moved,g_vertices,count*sizeof(Vertices));
 for(unsigned i=0;i<count;++i){float left=1e9f,right=-1e9f;for(unsigned j=0;j<3;++j){float x=moved[i].vertex[j].x,y=moved[i].vertex[j].y;if(!isfinite(x)||!isfinite(y)||moved[i].vertex[j].z!=1||g_textureCoord[i].vertex[j].q!=1||(!slot5Canvas&&(x<-32||x>672||y<-32||y>512))){nfs_mtw_tail_vertex_note(3*i+j,x,y,moved[i].vertex[j].z,g_textureCoord[i].vertex[j].q,mark,originalBranding,slot5Canvas,selected);nfs_mtw_tail_diag(3,11,(int)(3*i+j),x,y,moved[i].vertex[j].z,g_textureCoord[i].vertex[j].q);nfs_mtw_tail_reject(11,count);goto refused;}left=fminf(left,x);right=fmaxf(right,x);}
  float shift=0,dy=0;if(cursor){if(nfs_mtw.cached.schema)shift=(nfs_mtw.logical-640.f)*.5f;if(nfs_menu_responsive_snapshot(&shown)&&shown.drawableWidth==nfs_mtw.width&&shown.drawableHeight==nfs_mtw.height&&nfs_menu_responsive_pointer_snapshot(&pointer)&&pointer.revision==shown.revision&&pointer.ownerEpoch==epoch){shift=pointer.x-sx;dy=pointer.y-sy;}}
  /* Main's source-recognized128-square car/track marks are the right
     branding group even when transparent native canvas extends left of its
     visible ink. Never split/crop that canvas using an ink guess. */
  else if(nfs_mtw.cached.schema&&((!mark&&!originalBranding&&!originalMode)||nfs_mtw.ownerForm==NFS_MENU_CACHED_LOCATION))shift=(nfs_mtw.logical-640.f)*.5f;
  else if(mark||originalBranding||originalMode||left>=343)shift=(nfs_mtw.logical-640.f);else if(right>343){nfs_mtw_tail_diag(3,12,(int)i,left,right,0,0);nfs_mtw_tail_reject(12,count);goto refused;}
  shifts[i]=shift;for(unsigned j=0;j<3;++j){moved[i].vertex[j].x+=shift;moved[i].vertex[j].y+=dy;}
 }
 if(!nfs_graphics_scene_save(&saved,&guard))goto lost;memcpy(nativeMatrix,g_matrix,sizeof nativeMatrix);
 NFS_MTW_TAIL_GL(glBindFramebuffer(GL_FRAMEBUFFER,nfs_mtw.fbo));if(!nfs_mtw_attachment(nfs_mtw.result))goto done;NFS_MTW_TAIL_GL(glViewport(0,0,nfs_mtw.width,nfs_mtw.height));NFS_MTW_TAIL_GL(glDepthMask(GL_FALSE));NFS_MTW_TAIL_GL(glDisable(GL_DEPTH_TEST));
 /* Derive original current clip from actual GL scissor; apply the same anchor
    as the batch, never freeze animation/clip to final-layout coordinates. */
 NFS_MTW_TAIL_GL(glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nfs_mtw.matrix));NFS_MTW_TAIL_GL(glVertexAttribPointer(g_aPositionLoc,3,GL_FLOAT,GL_FALSE,0,moved));
 if(saved.common.caps[4]&&!cursor){
  NFS_MTW_TAIL_GL(glGetIntegerv(GL_SCISSOR_BOX,clip));const float scale=nfs_mtw.nativeHeight/480.f;
  const float l=clip[0]/scale,r=(clip[0]+clip[2])/scale,t=480-(clip[1]+clip[3])/scale,b=480-clip[1]/scale,outputScale=nfs_mtw.height/480.f;
  /* One source font batch can contain both anchors. Preserve its order and
     source clip separately for each contiguous semantic group. */
  for(unsigned first=0;first<count;){unsigned end=first+1;while(end<count&&shifts[end]==shifts[first])++end;
   GLint x=(GLint)floorf((l+shifts[first])*outputScale),y=(GLint)floorf((480-b)*outputScale),right=(GLint)ceilf((r+shifts[first])*outputScale),top=(GLint)ceilf((480-t)*outputScale);
   NFS_MTW_TAIL_GL(glScissor(x,y,right-x,top-y));NFS_MTW_TAIL_GL(glDrawArrays(GL_TRIANGLES,(GLint)first*3,(GLsizei)(end-first)*3));first=end;
  }
 }else{NFS_MTW_TAIL_GL(glDisable(GL_SCISSOR_TEST));NFS_MTW_TAIL_GL(glDrawArrays(GL_TRIANGLES,0,(GLsizei)count*3));}
 ok=glGetError()==GL_NO_ERROR;
 done:if(!nfs_mtw_current())goto lost;NFS_MTW_TAIL_GL(glUniformMatrix4fv(g_uMatrixLoc,1,GL_FALSE,nativeMatrix));if(!nfs_graphics_scene_restore(&saved,&guard))goto lost;if(!nfs_mtw_current())goto lost;if(!ok){nfs_mtw_tail_diag(3,13,-1,0,0,0,0);nfs_mtw_tail_reject(13,count);goto refused;}nfs_mtw.pendingTriangles=0;if(nfs_mtw_paint==UINT64_MAX)goto refused;++nfs_mtw_paint;nfs_menu_map_source_invalidate(&nfs_mtw_maps);nfs_mtw.tailTriangles+=count;if(cursor)nfs_mtw.cursorSeen=1;nfs_mtw_tail_diag(3,0,-1,0,0,0,0);return;
 refused:nfs_mt_wide_other();return;
 lost:nfs_mt_wide_retired(0);
}

#undef NFS_MTW_TAIL_GL

/* The inactive race path never enters the large bounded replay stack frame. */
static void nfs_mt_wide_tail(unsigned count){
 if(nfs_mtw.phase==2&&((nfs_mtw.kind>=5&&nfs_mtw.kind<=7)||nfs_mtw.kind==11)){nfs_mtw_branding_batch(count);return;}
 if(nfs_mtw.phase!=2||nfs_mtw.kind<8||nfs_mtw.kind>10){if(!nfs_mt.active)nfs_mt_wide_other();return;}
 nfs_mtw_tail_replay(count);
}
#undef NFS_MTW_NOINLINE

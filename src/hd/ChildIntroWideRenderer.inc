// SPDX-License-Identifier: MIT
/* Typed child admission and independently owned captions. No Player source,
 * Player native pass, MT pending GPU packet or widget state grants authority. */
#include "ChildIntroWidePreludeAbi.h"
static int nfs_piw_child_caption_alias(GLuint texture){if(!texture||texture==g_framebufferTexture||texture==g_frontTexture||texture==nfs_piw.scratch||texture==nfs_piw.art[0]||texture==nfs_piw.art[1])return 1;for(unsigned i=0;i<2;++i)if(texture==nfs_piw.slots[i].color)return 1;for(unsigned i=0;i<300;++i)if(texture==nfs_piw.panelPass.ids[i]||texture==nfs_piw.mainPass.ids[i])return 1;return 0;}
#define NFS_CHILD_WIDE_GL(call) do{if(!nfs_piw_current())return 0;call;if(!nfs_piw_current())return 0;}while(0)
static int nfs_piw_child_word(const NfsBackgroundSceneStage*t,const NfsLocationCaptionGpu*g,const NfsCompactCaptionWordView*w){
 if(!t||!g||!w||!nfs_piw_stage_owner(&nfs_piw,t)||g->generation!=nfs_piw.generation||g->word!=2||!nfs_location_caption_valid(w,2)||nfs_location_caption_empty(w)||nfs_piw_child_caption_alias(g->texture)||g->matrix<0||g->sampler<0)return 0;
 GLboolean live=0;NFS_CHILD_WIDE_GL(live=glIsTexture(g->texture));if(!live)return 0;NFS_CHILD_WIDE_GL(live=glIsProgram(g->program));if(!live)return 0;
 // Center the Location UI on the same 640-wide logical area as its full cache.
 const float shift=nfs_piw.view.locationInfoReturn?(nfs_piw.logical-640.f)*.5f:0.f;
 const float left=w->left+shift,right=w->right+shift;
 const GLfloat q[]={left,w->top,0,0,right,w->top,1,0,left,w->bottom,0,1,left,w->bottom,0,1,right,w->top,1,0,right,w->bottom,1,1};
 for(unsigned i=0;i<8;++i)NFS_CHILD_WIDE_GL(glDisable(nfs_frame_caps[i]));NFS_CHILD_WIDE_GL(glColorMask(1,1,1,1));NFS_CHILD_WIDE_GL(glDepthMask(GL_FALSE));NFS_CHILD_WIDE_GL(glActiveTexture(GL_TEXTURE0));NFS_CHILD_WIDE_GL(glBindBuffer(GL_ARRAY_BUFFER,0));NFS_CHILD_WIDE_GL(glUseProgram(g->program));NFS_CHILD_WIDE_GL(glUniformMatrix4fv(g->matrix,1,GL_FALSE,nfs_piw.matrix));NFS_CHILD_WIDE_GL(glUniform1i(g->sampler,0));NFS_CHILD_WIDE_GL(glBindTexture(GL_TEXTURE_2D,g->texture));NFS_CHILD_WIDE_GL(glEnableVertexAttribArray(0));for(unsigned i=1;i<4;++i)NFS_CHILD_WIDE_GL(glDisableVertexAttribArray(i));NFS_CHILD_WIDE_GL(glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,q));if(!nfs_graphics_scene_blend(t,1))return 0;NFS_CHILD_WIDE_GL(glDrawArrays(GL_TRIANGLES,0,6));return nfs_piw_error()==GL_NO_ERROR&&nfs_piw_stage_owner(&nfs_piw,t);
}
static int nfs_piw_child_caption(const NfsBackgroundSceneStage*t){
 NfsBackgroundSceneStage guard=nfs_piw_guard();if(!t||!nfs_piw_stage_owner(&nfs_piw,t))return 0;
 if(nfs_piw.view.family==6){const NfsOpponentInitialTransitionView*v=&nfs_piw.view.opponentInitial;NfsSoftwareTextStage text;float bounds[4];memset(&text,0,sizeof text);text.logicalWidth=343;text.logicalHeight=480;
  if(!nfs_mt_opponent_valid(v)||v->ownerEpoch!=nfs_piw.view.ownerEpoch||!nfs_soft_gpu_vertices(&text,v->caption.vertices,v->caption.vertexCount,bounds)||!nfs_player_intro_font_prepare(&nfs_piw.opponentCaption,&v->caption.font)||!nfs_piw_current()||nfs_piw_child_caption_alias(nfs_piw.opponentCaption.texture))return 0;
  return nfs_graphics_scene_text(&nfs_piw.opponentCaption,t,&v->caption)&&nfs_piw_stage_owner(&nfs_piw,t);
 }
 if(nfs_piw.view.family==7){const NfsLocationInitialTransitionView*v=&nfs_piw.view.locationInitial;if(!nfs_mt_location_valid(v)||v->ownerEpoch!=nfs_piw.view.ownerEpoch||!nfs_location_caption_prepare(&nfs_piw.locationCaption,&v->caption,2,&guard)||!nfs_piw_current()||nfs_piw_child_caption_alias(nfs_piw.locationCaption.texture))return 0;return nfs_piw_child_word(t,&nfs_piw.locationCaption,&v->caption);}
 return 0;
}
static int nfs_piw_child_begin_refused(const NfsChildIntroWidePreludeView*v,unsigned reason){NfsIntroWideGpuBorrow b;memset(&b,0,sizeof b);if(v){b.family=v->family;b.countdown=v->countdown;b.frame.form=v->form;b.frame.requestSerial=v->serial;}nfs_piw_note(1,&b,0,reason);return 0;}
int nfs_private_child_intro_wide_begin(const NfsChildIntroWidePreludeView*v){
 NfsIntroWideGpuBorrow b;if(!v||v->schema!=1||(v->family!=6&&v->family!=7)||v->photoFamily!=v->family||!v->content||!v->uploadEvent||!v->ownerEpoch||!nfs_mt_child_empty(&v->childInitial))return nfs_piw_child_begin_refused(v,5);
 /* These shared helpers validate immutable payload shape only. They do not
  * inspect nfs_mt, admit a transition packet or borrow transition authority. */
 if(v->family==6){if(v->form!=0x4d3204||!nfs_mt_opponent_valid(&v->opponentInitial)||v->opponentInitial.ownerEpoch!=v->ownerEpoch||!nfs_mt_location_empty(&v->locationInitial))return nfs_piw_child_begin_refused(v,6);}
 else if(v->form!=0x4d2a20||!nfs_mt_location_valid(&v->locationInitial)||v->locationInitial.ownerEpoch!=v->ownerEpoch||!nfs_mt_opponent_empty(&v->opponentInitial))return nfs_piw_child_begin_refused(v,7);
 memset(&b,0,sizeof b);b.family=v->family;b.countdown=v->countdown;b.photoFamily=v->photoFamily;b.locationInfoReturn=v->locationInfoReturn;b.frame.schema=1;b.frame.form=v->form;b.frame.sourceOwner=v->sourceOwner;b.frame.sourceBitmap=v->sourceBitmap;b.frame.requestSerial=v->serial;b.frame.uploadEvent=v->uploadEvent;b.frame.inheritedMain=v->inheritedMain;b.frame.paddedOriginal565=v->paddedOriginal565;b.frame.wordsCount=v->wordsCount;memcpy(b.frame.keys,v->keys,sizeof b.frame.keys);b.opponentInitial=v->opponentInitial;b.locationInitial=v->locationInitial;b.content=v->content;b.photoSerial=v->photoSerial;b.ownerEpoch=v->ownerEpoch;b.sourceGame=v->sourceGame;b.sourceCatalog=v->sourceCatalog;b.sourceLife=v->sourceLife;b.sourceArchive=v->sourceArchive;b.owner=v->owner;b.retain=v->retain;b.release=v->release;b.current=v->current;return nfs_intro_wide_begin_geometry(&b);
}
int nfs_private_child_intro_wide_stage(uint64_t serial,unsigned stage,int32_t x,int32_t y){return nfs_piw.view.family!=1&&(nfs_piw.view.family==6||nfs_piw.view.family==7)?nfs_intro_wide_stage_geometry(serial,stage,x,y):0;}
int nfs_private_child_intro_wide_cancel(uint64_t serial){if((nfs_piw.view.family!=6&&nfs_piw.view.family!=7)||!serial||serial!=nfs_piw.view.frame.requestSerial)return 0;nfs_player_intro_wide_other();return 1;}
#undef NFS_CHILD_WIDE_GL

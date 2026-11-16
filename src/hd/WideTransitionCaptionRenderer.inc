// SPDX-License-Identifier: MIT
/* Typed initial-sidebar/full Setup/Options caption. This synchronous draw owns only WTR's
 * fresh wide staging image; native caption sampling/eligibility is unchanged. */
static int nfs_mtw_initial_caption_impl(const NfsMenuTransitionView*v,const NfsBackgroundSceneStage*t,int draw){
 const NfsCompactCaptionWordView*w;const NfsCompactCaptionGpu*g;unsigned word;
 if(!v||!t||(!draw&&v->kind!=8))return 0;
 if(v->kind==11){const NfsPlayerInitialTransitionView*p=&v->playerInitial;const NfsPlayerCaptionGpu*g=&nfs_mt.playerCaption;const NfsPlayerCaptionWordView*w=&p->caption;const unsigned player=p->form==0x4d3050u?1u:2u;
  if(!nfs_mtw_image_owner(&nfs_mtw,t)||v->serial!=nfs_mt.serial||nfs_mt.sourceKind!=11||!nfs_mt.active||v->slot||v->drawKind!=2||v->originX||v->originY||v->stepX!=32||!nfs_menu_transition_source_life_current(v->sourceGame,v->sourceLife)||!nfs_mt_player_valid(p)||g->generation!=t->generation||g->word!=player||!g->program||!g->texture||g->matrix<0||g->sampler<0||nfs_mt_player_alias()||g->texture==nfs_mtw.image||g->texture==nfs_mtw.result||g->texture==nfs_mtw.art[0]||g->texture==nfs_mtw.art[1])return 0;
  return nfs_menu_wide_word(t,g->generation,g->program,g->texture,g->matrix,g->sampler,w->left,w->top,w->right,w->bottom)&&nfs_mtw_image_owner(&nfs_mtw,t);
 }
 if(v->kind==6||v->kind==7||v->kind==9||v->kind==10)return nfs_mtw_initial_preview_caption(v,t);
 if(v->kind!=5&&v->kind!=8)return 1;
 if(!nfs_mtw_image_owner(&nfs_mtw,t)||v->serial!=nfs_mt.serial||nfs_mt.sourceKind!=v->kind||!nfs_mt.active||
    v->slot!=(v->kind==5?0u:1u)||v->drawKind!=2||v->originX||v->originY||v->stepX!=32||
    !nfs_menu_transition_source_life_current(v->sourceGame,v->sourceLife)||!nfs_mt_child_valid(&v->childInitial))return 0;
 word=v->childInitial.form==0x4d2790u?1u:3u;w=&v->childInitial.caption;g=&nfs_mt.childCaption;
 if(g->generation!=t->generation||g->word!=word||!g->program||!g->texture||g->matrix<0||g->sampler<0||
    nfs_compact_caption_empty(w)||!nfs_compact_caption_valid(w,word)||
    g->texture==nfs_mtw.image||g->texture==nfs_mtw.result||g->texture==nfs_mtw.art[0]||g->texture==nfs_mtw.art[1]||
    g->texture==nfs_mt.image||g->texture==nfs_mt.backup||g->texture==nfs_mt.result||
    nfs_compact_caption_alias(g->texture,0,nfs_mt.preview,nfs_mtw.image,nfs_mt.ids))return 0;
 if(!draw){GLboolean program=0,texture=0;if(!nfs_mtw_image_owner(&nfs_mtw,t))return 0;program=glIsProgram(g->program);if(!nfs_mtw_image_owner(&nfs_mtw,t)||!program)return 0;texture=glIsTexture(g->texture);return texture&&nfs_mtw_image_owner(&nfs_mtw,t);}
 /* Same left anchor and uniform height scale as steady Setup/Options; only
    the separately owned image's projection differs from the native canvas. */
 return nfs_menu_wide_word(t,g->generation,g->program,g->texture,g->matrix,g->sampler,
                           w->left,w->top,w->right,w->bottom)&&nfs_mtw_image_owner(&nfs_mtw,t);
}

static int nfs_mtw_initial_caption(const NfsMenuTransitionView*v,const NfsBackgroundSceneStage*t){return nfs_mtw_initial_caption_impl(v,t,1);}

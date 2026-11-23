// SPDX-License-Identifier: MIT
#pragma once
#include "VictoryArtProvider.h"

#include "VictoryCanvasLayout.h"
static int nfs_victory_art_identity_valid(const NfsVictoryArtworkIdentity*identity){
 if(!identity||identity->present>1||identity->image>=16)return 0;
 if(identity->present)return nfs_victory_art_source_identity_valid(identity->image,identity->sourceSha256);
 if(identity->image)return 0;for(unsigned k=0;k<32;++k)if(identity->sourceSha256[k])return 0;return 1;
}
static int nfs_victory_art_prepare(unsigned*aspect,int*authored,uint64_t*generation){
 NfsVictoryArtView view;NfsBgSaved saved;GLint limit=0;int ok=0;
 *authored=0;*aspect=0;*generation=nfs_victory_art_generation();if(!*generation)return 1;
 if(!nfs_victory_renderer.view.artwork.present||!nfs_victory_art_identity_valid(&nfs_victory_renderer.view.artwork))return 0;
 NfsVictoryCanvasLayout layout;if(!nfs_victory_canvas_layout(nfs_victory_renderer.target.sampling.viewport,&layout))return 0;*aspect=layout.aspect;
 memset(&view,0,sizeof view);
 if(!nfs_victory_art_acquire(*aspect,nfs_victory_renderer.view.artwork.image,nfs_victory_renderer.view.artwork.sourceSha256,&view))return 0;
 if(view.schema!=1||view.generation!=*generation||view.aspect!=*aspect||view.image!=nfs_victory_renderer.view.artwork.image||!view.owner||!view.release||!view.pixels||!view.width||!view.height||view.width>4096||view.height>4096||view.bytes!=(size_t)view.width*view.height*4||memcmp(view.sourceSha256,nfs_victory_renderer.view.artwork.sourceSha256,32))goto done;
 if(nfs_victory_renderer.artContext==nfs_victory_renderer.generation&&nfs_victory_renderer.artGeneration==view.generation&&nfs_victory_renderer.artIdentity==view.identity&&nfs_victory_renderer.artWidth==view.width&&nfs_victory_renderer.artHeight==view.height&&!memcmp(nfs_victory_renderer.artSha,view.rgbaSha256,32)&&glIsTexture(nfs_victory_renderer.clean)){ok=1;goto done;}
 {unsigned char sha[32];if(!nfs_texture_sha256(view.pixels,view.bytes,sha)||memcmp(sha,view.rgbaSha256,32))goto done;}
 if(!nfs_victory_words_owner_current())goto done;
 nfs_bg_save(&saved);glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);
 if(limit>0&&view.width<=(unsigned)limit&&view.height<=(unsigned)limit){
  glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,nfs_victory_renderer.clean);glPixelStorei(GL_UNPACK_ALIGNMENT,1);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,view.width,view.height,0,GL_RGBA,GL_UNSIGNED_BYTE,view.pixels);ok=glGetError()==GL_NO_ERROR;
 }
 if(!nfs_victory_words_owner_current()){nfs_victory_abandon();ok=0;goto done;}nfs_bg_restore(&saved);
 if(ok){nfs_victory_renderer.artContext=nfs_victory_renderer.generation;nfs_victory_renderer.artGeneration=view.generation;nfs_victory_renderer.artIdentity=view.identity;nfs_victory_renderer.artWidth=view.width;nfs_victory_renderer.artHeight=view.height;memcpy(nfs_victory_renderer.artSha,view.rgbaSha256,32);}
 done:if(view.owner&&view.release)view.release(view.owner);
 if(!nfs_victory_words_owner_current()||nfs_victory_art_generation()!=*generation)ok=0;
 if(ok)*authored=1;return ok;
}

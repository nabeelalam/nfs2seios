// SPDX-License-Identifier: MIT
#include "FrontendFontPixels.inc"
#include "FrontendFontLayout.inc"
static GLuint nfs_front_font_texture;
static unsigned char nfs_front_font_original[2][131072];
static int nfs_front_font_valid[2];
static TextureInfo *nfs_front_source_owner[2];
static int nfs_front_source_matched[2];
static int nfs_front_extended_ready;
static int nfs_front_font_source_page(unsigned tag){return tag==41?0:tag==42?1:-1;}
static void nfs_front_source_refresh(void){
 if(!nfs_front_source_matched[0] || !nfs_front_source_matched[1])nfs_front_extended_ready=0;
 nfs_front_hit_refresh();
}
static void nfs_front_frame_committed(void){nfs_front_extended_ready=nfs_front_source_matched[0] && nfs_front_source_matched[1];}
static void *nfs_front_font_restore_source(TextureInfo *ti){
 int page=nfs_front_font_source_page(ti->hdDigits);
 return page>=0 && nfs_front_font_valid[page] && ti->size==256 && ti->fmt==GR_TEXFMT_ARGB_4444?nfs_front_font_original[page]:ti->data;
}
#include "FrontendFontGeometry.inc"
static int nfs_front_font_upload(TextureInfo *ti){
 for(unsigned p=0;p<2;++p)if(ti==nfs_front_source_owner[p])nfs_front_source_matched[p]=0;
 if(!nfs_hd_builtin_enabled || !nfs_hd_cell_layout_available || ti->fmt!=GR_TEXFMT_ARGB_4444 || ti->size!=256 || !ti->data){nfs_front_source_refresh();return 0;}
 unsigned char digest[32];int page=-1;
 if(nfs_texture_sha256(ti->data,131072,digest)){
  if(!memcmp(digest,nfs_front_original_digest,32))page=0;
  else if(!memcmp(digest,nfs_front_accent_digest,32))page=1;
 }
 if(page<0){nfs_front_source_refresh();return 0;}
 GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);
 if(limit<NFS_FRONT_FONT_WIDTH || limit<NFS_FRONT_FONT_HEIGHT){nfs_front_source_refresh();return 0;}
 if(!nfs_front_font_texture){
  unsigned char *pixels=(unsigned char *)malloc(NFS_FRONT_FONT_RGBA_BYTES);if(!pixels){nfs_front_source_refresh();return 0;}
  if(!nfs_front_font_expand(pixels,NFS_FRONT_FONT_RGBA_BYTES)){free(pixels);nfs_front_source_refresh();return 0;}
  glActiveTexture(GL_TEXTURE1);glGenTextures(1,&nfs_front_font_texture);
  if(!nfs_front_font_texture){glActiveTexture(GL_TEXTURE0);free(pixels);nfs_front_source_refresh();return 0;}
  glBindTexture(GL_TEXTURE_2D,nfs_front_font_texture);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,NFS_FRONT_FONT_WIDTH,NFS_FRONT_FONT_HEIGHT,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);free(pixels);
  nfs_hd_bound_aux_texture=nfs_front_font_texture;glActiveTexture(GL_TEXTURE0);++nfs_hd_uploads;
 }
 if(ti->data!=nfs_front_font_original[page])memcpy(nfs_front_font_original[page],ti->data,131072);
 nfs_front_font_valid[page]=1;ti->hdDigits=41+page;nfs_front_source_owner[page]=ti;nfs_front_source_matched[page]=1;nfs_front_source_refresh();return 1;
}

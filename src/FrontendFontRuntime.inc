// SPDX-License-Identifier: MIT
#include "FrontendFontPixels.inc"
#include "FrontendFontLayout.inc"
static GLuint nfs_front_font_texture;
static unsigned char nfs_front_font_original[131072];
static int nfs_front_font_valid;
static TextureInfo *nfs_front_source_owner;
static int nfs_front_source_matched;
#include "FrontendFontGeometry.inc"
static int nfs_front_font_upload(TextureInfo *ti){
 if(ti==nfs_front_source_owner){nfs_front_source_matched=0;nfs_front_hit_refresh();}
 if(!nfs_hd_builtin_enabled || !nfs_hd_cell_layout_available || ti->fmt!=GR_TEXFMT_ARGB_4444 || ti->size!=256 || !ti->data)return 0;
 unsigned char digest[32];if(!nfs_texture_sha256(ti->data,131072,digest) || memcmp(digest,nfs_front_original_digest,32))return 0;
 GLint limit=0;glGetIntegerv(GL_MAX_TEXTURE_SIZE,&limit);if(limit<NFS_FRONT_FONT_WIDTH || limit<NFS_FRONT_FONT_HEIGHT)return 0;
 if(!nfs_front_font_texture){
  unsigned char *pixels=(unsigned char *)malloc(NFS_FRONT_FONT_RGBA_BYTES);if(!pixels)return 0;
  if(!nfs_front_font_expand(pixels,NFS_FRONT_FONT_RGBA_BYTES)){free(pixels);return 0;}
  glActiveTexture(GL_TEXTURE1);glGenTextures(1,&nfs_front_font_texture);
  if(!nfs_front_font_texture){glActiveTexture(GL_TEXTURE0);free(pixels);return 0;}
  glBindTexture(GL_TEXTURE_2D,nfs_front_font_texture);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,NFS_FRONT_FONT_WIDTH,NFS_FRONT_FONT_HEIGHT,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);free(pixels);
  nfs_hd_bound_aux_texture=nfs_front_font_texture;glActiveTexture(GL_TEXTURE0);++nfs_hd_uploads;
 }
 if(ti->data!=nfs_front_font_original)memcpy(nfs_front_font_original,ti->data,131072);nfs_front_font_valid=1;ti->hdDigits=41;nfs_front_source_owner=ti;nfs_front_source_matched=1;nfs_front_hit_refresh();return 1;
}

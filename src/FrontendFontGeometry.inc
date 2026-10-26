// SPDX-License-Identifier: MIT
#include "FrontendTextContext.h"
#include "FrontendFontCapacity.h"
static int nfs_front_legacy_geometry(const GrVertex *a,const GrVertex *b,const GrVertex *c,GrVertex output[3]){
 if(!nfs_hd_builtin_enabled || !nfs_hd_visible || (nfs_hd_selected!=41 && nfs_hd_selected!=42) || !nfs_front_font_texture || !nfs_hd_cell_layout_available)return 0;
 const GrVertex *v[3]={a,b,c};float uv[6],u0=256,u1=0,v0=256,v1=0;
 for(unsigned i=0;i<3;++i){if(v[i]->oow!=1)return 0;float u=v[i]->tmuvtx[0].sow,t=v[i]->tmuvtx[0].tow;
  if(!(u>=0 && u<=256 && t>=0 && t<=256))return 0;uv[i*2]=u;uv[i*2+1]=t;
  u0=fminf(u0,u);u1=fmaxf(u1,u);v0=fminf(v0,t);v1=fmaxf(v1,t);
 }
 unsigned x=(unsigned)floorf(u0*256/255+.02f),row=(unsigned)floorf((v0*256/255+.02f)/51);if(x>=256 || row>=5)return 0;
 unsigned index=nfs_hd_selected==41?nfs_front_cell_lookup[row*256+x]:nfs_front_accent_lookup[row*256+x];if(!index || index>sizeof(nfs_front_cells)/sizeof(nfs_front_cells[0]))return 0;
 const NfsFrontCell *cell=nfs_front_cells+index-1;float offsets[3][2]={{1,3},{1,0},{0,0}};unsigned modes=cell->large?1:3;
 if(!cell->large){offsets[0][0]=cell->code==69?.5f:1;offsets[0][1]=cell->code>=65 && cell->code<=90?-.5f:0;}
 for(unsigned mode=0;mode<modes;++mode){float bx=offsets[mode][0],by=offsets[mode][1];
  float left=(cell->x+bx+.5f)*255/256,right=(cell->x+cell->width+bx)*255/256;
  float top=(cell->y+by+.5f)*255/256,bottom=(cell->y+51+by)*255/256;
  if(!(fabsf(u0-left)<.01f && fabsf(u1-right)<.01f && fabsf(v0-top)<.01f && fabsf(v1-bottom)<.01f))continue;
  unsigned corners=0,indices[3];int valid=1;
  for(unsigned i=0;i<3;++i){unsigned rx=fabsf(uv[2*i]-left)<.01f?0:fabsf(uv[2*i]-right)<.01f?1:2;
   unsigned ry=fabsf(uv[2*i+1]-top)<.01f?0:fabsf(uv[2*i+1]-bottom)<.01f?1:2;
   if(rx>1 || ry>1){valid=0;break;}indices[i]=rx+2*ry;corners|=1u<<indices[i];}
  if(!valid || !(corners==7 || corners==11 || corners==13 || corners==14))continue;
  for(unsigned i=0;i<3;++i){output[i]=*v[i];output[i].tmuvtx[0].sow=cell->x+((indices[i]&1)?cell->width-.125f:.125f);output[i].tmuvtx[0].tow=cell->y+by+((indices[i]&2)?50.875f:.125f);}
  return (int)index;
 }
 return 0;
}

#include "FrontendWordLayout.inc"
static int nfs_front_font_geometry(const GrVertex *a,const GrVertex *b,const GrVertex *c,GrVertex output[3]){
 GrVertex legacy[3];int cell=nfs_front_legacy_geometry(a,b,c,legacy);if(!cell)return 0;
 const GrVertex *v[3]={a,b,c};float x0=1e9f,y0=1e9f,x1=-1e9f,y1=-1e9f,u0=1e9f,v0=1e9f,u1=-1e9f,v1=-1e9f;
 for(unsigned i=0;i<3;++i){float x=v[i]->x-786432,y=v[i]->y-786432;if(!(x>=0 && x<=640 && y>=0 && y<=480))return 0;x0=fminf(x0,x);x1=fmaxf(x1,x);y0=fminf(y0,y);y1=fmaxf(y1,y);u0=fminf(u0,legacy[i].tmuvtx[0].sow);u1=fmaxf(u1,legacy[i].tmuvtx[0].sow);v0=fminf(v0,legacy[i].tmuvtx[0].tow);v1=fmaxf(v1,legacy[i].tmuvtx[0].tow);}
 for(unsigned n=0;n<sizeof(nfs_front_word_glyphs)/sizeof(nfs_front_word_glyphs[0]);++n){const NfsFrontWordGlyph *g=nfs_front_word_glyphs+n;
  if(g->cell!=cell || fabsf(g->x-x0)>.01f || fabsf(g->y-y0)>.01f || fabsf(g->w-(x1-x0))>.01f || fabsf(g->h-(y1-y0))>.01f)continue;
  if(g->word && g->word!=nfs_front_text_context())continue;
  if(!nfs_front_atlas_row_fits(g->atlasX,g->atlasY,g->width,g->inkHeight,nfs_front_font_uploaded_height,NFS_FRONT_FONT_HEIGHT))continue;
  if(g->word==UINT64_C(0x385db5487d40488d) && !nfs_front_extended_ready)continue;
  unsigned corner[3],bits=0;
  for(unsigned i=0;i<3;++i){float x=v[i]->x-786432,y=v[i]->y-786432;int rx=fabsf(x-x1)<.01f,ry=fabsf(y-y1)<.01f;
   if((!rx && fabsf(x-x0)>.01f) || (!ry && fabsf(y-y0)>.01f) || fabsf(legacy[i].tmuvtx[0].sow-(rx?u1:u0))>.01f || fabsf(legacy[i].tmuvtx[0].tow-(ry?v1:v0))>.01f)return 0;
   corner[i]=rx+2*ry;bits|=1u<<corner[i];}
  if(bits!=7 && bits!=11 && bits!=13 && bits!=14)return 0;
  for(unsigned i=0;i<3;++i){unsigned rx=corner[i]&1,ry=corner[i]&2;float edge=rx?g->right:g->left;output[i]=*v[i];float sx=g->outputScaleX>0?g->outputScaleX:1,sy=g->outputScaleY>0?g->outputScaleY:1;output[i].x=786432+g->outputX+edge*g->width*sx;output[i].y=786432+g->y+g->inkTop*sy+(ry?g->inkHeight*sy:0);output[i].tmuvtx[0].sow=g->atlasX+edge*g->width;output[i].tmuvtx[0].tow=nfs_front_atlas_tow(g->atlasY+(ry?g->inkHeight*nfs_front_font_v_scale:0),nfs_front_font_uploaded_height);}
  return cell;
 }
 return 0;
}

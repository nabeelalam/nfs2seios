// SPDX-License-Identifier: MIT
#ifndef NFS_WARNING_WORD_SELECT_H
#define NFS_WARNING_WORD_SELECT_H
#define NFS_WARNING_WORD_SELECTOR 76
#define NFS_WARNING_SINGLE_SELECTOR 77
/* The maintained caller supplies exact atlas authentication and highp availability. */
typedef struct {float x,y,oow,r,g,b,a;} NfsWarningWordVertex;
static int nfs_warning_word_triangle(const NfsWarningWordVertex *v,const float uv[6]){
 if(!v||!uv)return 0;unsigned corners=0;float row=-1;int single=0;
 for(unsigned i=0;i<3;++i)if(uv[2*i]==255.f)single=1;
 for(unsigned i=0;i<3;++i){
  if(v[i].oow!=1||v[i].r!=255||v[i].g!=255||v[i].b!=255||v[i].a!=255)return 0;
  const int right=uv[2*i]==(single?255.f:222.12890625f),bottom=uv[2*i+1]==20.91796875f;
  if((!right&&uv[2*i]!=.498046875f)||(!bottom&&uv[2*i+1]!=.498046875f))return 0;
  if(v[i].x!=(single?(right?539.f:283.f):(right?522.f:299.f)))return 0;
  const float top=v[i].y-(bottom?21.f:0.f);if(top!=7.f&&(single||top!=248.f))return 0;
  if(i&&top!=row)return 0;row=top;corners|=1u<<(right+2*bottom);
 }
 return corners==7||corners==11||corners==13||corners==14?(single?NFS_WARNING_SINGLE_SELECTOR:NFS_WARNING_WORD_SELECTOR):0;
}
#endif

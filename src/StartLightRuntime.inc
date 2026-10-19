// SPDX-License-Identifier: MIT
// Complete sphere UVs in the exact original timing atlas; preserve the game depth.
static int nfs_start_light_triangle(const GrVertex *a,const GrVertex *b,const GrVertex *c,const float uv[6]){
 const GrVertex *v[3]={a,b,c};unsigned corners=0;
 for(unsigned i=0;i<3;++i){
  if(v[i]->oow!=65535.0f/65536)return 0;
  unsigned right=2,bottom=2;
  if(fabsf(uv[2*i]-159.0f*255/256)<.01f)right=0;else if(fabsf(uv[2*i]-209.0f*255/256)<.01f)right=1;
  if(fabsf(uv[2*i+1]-130.0f*255/256)<.01f)bottom=0;else if(fabsf(uv[2*i+1]-180.0f*255/256)<.01f)bottom=1;
  if(right>1 || bottom>1)return 0;corners|=1u<<(right+2*bottom);
 }
 return corners==7 || corners==11 || corners==13 || corners==14;
}

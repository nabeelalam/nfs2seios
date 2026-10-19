#include <math.h>
// Private exact-match English pause source and captured complete-quad identities.
static const unsigned char nfs_pause_original_digest[32]={11,73,58,57,216,202,210,31,136,147,112,140,25,244,205,186,85,183,90,127,251,5,109,15,77,70,155,153,118,183,23,184};
static const float nfs_pause_uv[6][4]={{0.498046875f,0.498046875f,154.39453125f,23.90625f},{0.498046875f,25.400390625f,154.39453125f,48.80859375f},{0.498046875f,73.212890625f,154.39453125f,96.62109375f},{0.498046875f,97.119140625f,254.00390625f,120.52734375f},{0.498046875f,49.306640625f,154.39453125f,72.71484375f},{0.498046875f,198.720703125f,254.00390625f,226.11328125f}};
static unsigned char nfs_pause_original[131072];
static int nfs_pause_valid;
static int nfs_pause_match(TextureInfo *ti){
 if(ti->fmt!=GR_TEXFMT_ARGB_1555 || ti->size!=256)return 0;
 unsigned char digest[32];if(!nfs_texture_sha256(ti->data,131072,digest) || memcmp(digest,nfs_pause_original_digest,32))return 0;
 if(ti->data!=nfs_pause_original)memcpy(nfs_pause_original,ti->data,131072);
 nfs_pause_valid=1;ti->hdDigits=40;return 1;
}
static int nfs_pause_triangle(const float uv[6]){
 for(unsigned d=0;d<6;++d){unsigned corners=0;
  for(unsigned i=0;i<3;++i){unsigned x=2,y=2;float u=uv[2*i],v=uv[2*i+1];
   if(fabsf(u-nfs_pause_uv[d][0])<=.01f)x=0;else if(fabsf(u-nfs_pause_uv[d][2])<=.01f)x=1;
   if(fabsf(v-nfs_pause_uv[d][1])<=.01f)y=0;else if(fabsf(v-nfs_pause_uv[d][3])<=.01f)y=1;
   if(x>1 || y>1){corners=0;break;}corners|=1u<<(x+y*2);
  }
  if(corners==7 || corners==11 || corners==13 || corners==14)return 50+d;
 }
 return 0;
}

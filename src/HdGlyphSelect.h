// SPDX-License-Identifier: MIT
#ifndef NFS_HD_GLYPH_SELECT_H
#define NFS_HD_GLYPH_SELECT_H
/* The game shrinks texel coordinates by 255/256. Undo that only for identifying
 * complete timing cells; leave actual sampling coordinates untouched. In raw
 * coordinates digits 7-9 begin slightly before their integer cell boundary.
 * Reject constant UV, clipped and cross-cell primitives. */
static int nfs_hd_digit_triangle(const float uv[6]){
    const float scale=256.0f/255.0f,tolerance=.01f;
    float low_u=uv[0]*scale,high_u=low_u,low_v=uv[1]*scale,high_v=low_v;
    for(unsigned i=0;i<3;++i){
        float u=uv[2*i]*scale,v=uv[2*i+1]*scale;
        if(!(u>=48-tolerance && u<=168+tolerance && v>=21-tolerance && v<=47+tolerance))return 0;
        if(u<low_u)low_u=u;if(u>high_u)high_u=u;
        if(v<low_v)low_v=v;if(v>high_v)high_v=v;
    }
    unsigned digit=(unsigned)(((low_u+high_u)*.5f-48)/12);
    if(digit>=10)return 0;
    float left=48+12*digit;
    if(!(low_u>=left+.5f-tolerance && low_u<=left+.5f+tolerance &&
           high_u>=left+12-tolerance && high_u<=left+12+tolerance &&
           low_v>=21.5f-tolerance && low_v<=21.5f+tolerance &&
           high_v>=47-tolerance && high_v<=47+tolerance))return 0;
    unsigned corners=0;
    for(unsigned i=0;i<3;++i){
        float u=uv[2*i]*scale,v=uv[2*i+1]*scale;
        int right=u>=high_u-tolerance,bottom=v>=high_v-tolerance;
        if(!right && u>low_u+tolerance)return 0;
        if(!bottom && v>low_v+tolerance)return 0;
        corners|=1u<<(right+2*bottom);
    }
    return corners==7 || corners==11 || corners==13 || corners==14;
}
#endif

// SPDX-License-Identifier: MIT
#ifndef NFS_HD_GLYPH_SELECT_H
#define NFS_HD_GLYPH_SELECT_H
/* Full, same-cell timing numeral triangles only. Constant-UV color primitives
 * and clipped/unknown atlas regions retain original sampling. */
static int nfs_hd_digit_triangle(const float uv[6]){
    float low_u=uv[0],high_u=uv[0],low_v=uv[1],high_v=uv[1];
    for(unsigned i=0;i<3;++i){
        float u=uv[2*i],v=uv[2*i+1];
        if(!(u>=48 && u<168 && v>=21 && v<47))return 0;
        if(u<low_u)low_u=u;if(u>high_u)high_u=u;
        if(v<low_v)low_v=v;if(v>high_v)high_v=v;
    }
    unsigned digit=(unsigned)((low_u-48)/12);
    return high_u<48+12*(digit+1) && high_u-low_u>8 && high_v-low_v>20;
}
#endif

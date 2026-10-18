// SPDX-License-Identifier: MIT
#ifndef NFS_HD_GLYPH_SELECT_H
#define NFS_HD_GLYPH_SELECT_H
/* The game shrinks texel coordinates by 255/256. Undo that only for identifying
 * complete timing cells; leave actual sampling coordinates untouched. Gold speed
 * cells use their complete observed corner bounds, including original trims. In raw
 * coordinates digits 7-9 begin slightly before their integer cell boundary.
 * Reject constant UV, clipped and cross-cell primitives. */
// Small letters occupy separate exact-content textures. Only complete
// observed corners qualify; clipped, constant and nonfinite UVs stay original.
static int nfs_hd_small_font_triangle(const float uv[6],unsigned edge){
    if(!uv || (edge!=4 && edge!=8))return 0;
    const float low=127.5f/edge;unsigned corners=0;
    for(unsigned i=0;i<3;++i){float u=uv[i*2],v=uv[i*2+1];
        if(!(u>=low-.01f && u<=255.01f && v>=low-.01f && v<=255.01f))return 0;
        int right=u>=254.99f,bottom=v>=254.99f;
        if((!right && u>low+.01f)||(!bottom && v>low+.01f))return 0;
        corners|=1u<<(right+2*bottom);
    }
    return corners==7||corners==11||corners==13||corners==14;
}
static int nfs_hd_digit_triangle(const float uv[6]){

    static const float bounds[10][4]={{43.330078125f,47.314453125f,72.71484375f,70.72265625f},{72.216796875f,47.314453125f,100.60546875f,70.72265625f},{101.103515625f,47.314453125f,130.48828125f,70.72265625f},{129.990234375f,47.314453125f,159.375f,70.72265625f},{159.873046875f,47.314453125f,189.2578125f,70.72265625f},{188.759765625f,47.314453125f,218.14453125f,70.72265625f},{217.646484375f,47.314453125f,246.03515625f,70.72265625f},{73.212890625f,71.220703125f,101.6015625f,94.62890625f},{102.099609375f,71.220703125f,131.484375f,94.62890625f},{130.986328125f,71.220703125f,160.37109375f,94.62890625f}};
    for(unsigned d=0;d<10;++d){
        const float *b=bounds[d];unsigned corners=0;int inside=1;
        for(unsigned i=0;i<3;++i){float u=uv[2*i],v=uv[2*i+1];
            if(!(u>=b[0]-.01f && u<=b[2]+.01f && v>=b[1]-.01f && v<=b[3]+.01f)){inside=0;break;}
            int right=u>=b[2]-.01f,bottom=v>=b[3]-.01f;
            if((!right && u>b[0]+.01f)||(!bottom && v>b[1]+.01f)){inside=0;break;}
            corners|=1u<<(right+2*bottom);
        }
        if(inside && (corners==7||corners==11||corners==13||corners==14))return 2+d;
    }

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

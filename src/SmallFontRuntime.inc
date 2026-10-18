// SPDX-License-Identifier: MIT
#include "SmallFontIdentity.h"
// Own converted source pixels only after an exact runtime content match. The
// guest texture RAM can overlap, so context restoration must not borrow it.
static unsigned char nfs_small_font_original[26][128];
static uint32_t nfs_small_font_valid;
static int nfs_small_font_match(TextureInfo *ti){
    if(ti->fmt!=GR_TEXFMT_ARGB_1555 || (ti->size!=4 && ti->size!=8))return 0;
    unsigned char digest[32];unsigned bytes=ti->size*ti->size*2;
    if(!nfs_texture_sha256(ti->data,bytes,digest))return 0;
    for(unsigned d=0;d<26;++d)if(ti->size==nfs_small_font_edge[d] && !memcmp(digest,nfs_small_font_digest[d],32)){
        if(ti->data!=nfs_small_font_original[d])memcpy(nfs_small_font_original[d],ti->data,bytes);
        nfs_small_font_valid|=1u<<d;ti->hdDigits=2+d;return 1;
    }
    return 0;
}

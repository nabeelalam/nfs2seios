// SPDX-License-Identifier: MIT
#include "RecordFontIdentity.h"
static unsigned char nfs_record_font_original[11][128];
static uint16_t nfs_record_font_valid;
// Reuse the upload digest already computed by the uppercase matcher.
static int nfs_record_font_match(TextureInfo *ti,const unsigned char digest[32],unsigned bytes){
    for(unsigned d=0;d<11;++d)if(ti->size==nfs_record_font_edge[d] && !memcmp(digest,nfs_record_font_digest[d],32)){
        if(ti->data!=nfs_record_font_original[d])memcpy(nfs_record_font_original[d],ti->data,bytes);
        nfs_record_font_valid|=(uint16_t)(1u<<d);ti->hdDigits=28+d;return 1;
    }
    return 0;
}

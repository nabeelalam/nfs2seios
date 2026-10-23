// SPDX-License-Identifier: MIT
// Font artwork: OFL Bebas Neue/Oxanium plus complete Main word graphics; see assets/hud/frontend/MAIN_WORD_ART.md.
enum {NFS_FRONT_FONT_WIDTH=1024,NFS_FRONT_FONT_HEIGHT=2048,NFS_FRONT_FONT_PIXELS=2097152,NFS_FRONT_FONT_RGBA_BYTES=8388608};
static const float nfs_front_font_v_scale=.5f;
static const unsigned char nfs_front_font_rle[]={
#include "FrontendFontPixelsPart0.inc"
#include "FrontendFontPixelsPart1.inc"
#include "FrontendFontPixelsPart2.inc"
#include "FrontendFontPixelsPart3.inc"
#include "FrontendFontPixelsPart4.inc"
#include "FrontendFontPixelsPart5.inc"
#include "FrontendFontPixelsPart6.inc"
#include "FrontendFontPixelsPart7.inc"
#include "FrontendFontPixelsPart8.inc"
#include "FrontendFontPixelsPart9.inc"
#include "FrontendFontPixelsPart10.inc"
#include "FrontendFontPixelsPart11.inc"
#include "FrontendFontPixelsPart12.inc"
#include "FrontendFontPixelsPart13.inc"
#include "FrontendFontPixelsPart14.inc"
#include "FrontendFontPixelsPart15.inc"
#include "FrontendFontPixelsPart16.inc"
#include "FrontendFontPixelsPart17.inc"
#include "FrontendFontPixelsPart18.inc"
#include "FrontendFontPixelsPart19.inc"
#include "FrontendFontPixelsPart20.inc"
#include "FrontendFontPixelsPart21.inc"
#include "FrontendFontPixelsPart22.inc"
#include "FrontendFontPixelsPart23.inc"
#include "FrontendFontPixelsPart24.inc"
#include "FrontendFontPixelsPart25.inc"
#include "FrontendFontPixelsPart26.inc"
#include "FrontendFontPixelsPart27.inc"
};

static int nfs_front_font_expand(unsigned char *rgba,size_t capacity){
 if(!rgba || capacity<NFS_FRONT_FONT_RGBA_BYTES || sizeof(nfs_front_font_rle)%6)return 0;unsigned count=0;
 for(size_t i=0;i<sizeof(nfs_front_font_rle);i+=6){unsigned n=nfs_front_font_rle[i]|((unsigned)nfs_front_font_rle[i+1]<<8);if(!n || n>NFS_FRONT_FONT_PIXELS-count)return 0;count+=n;}if(count!=NFS_FRONT_FONT_PIXELS)return 0;
 unsigned pos=0;for(size_t i=0;i<sizeof(nfs_front_font_rle);i+=6){unsigned n=nfs_front_font_rle[i]|((unsigned)nfs_front_font_rle[i+1]<<8);for(unsigned j=0;j<n;++j,++pos)memcpy(rgba+pos*4,nfs_front_font_rle+i+2,4);}return 1;
}

// Renderer code: MIT. Six fixed authored word graphics: see assets/hud/pause/classic-word-art-*.json.
static const unsigned char nfs_pause_pixels_rle[]={
#include "PauseFontPixelsPart0.inc"
#include "PauseFontPixelsPart1.inc"
#include "PauseFontPixelsPart2.inc"
#include "PauseFontPixelsPart3.inc"
#include "PauseFontPixelsPart4.inc"
#include "PauseFontPixelsPart5.inc"
#include "PauseFontPixelsPart6.inc"
#include "PauseFontPixelsPart7.inc"
#include "PauseFontPixelsPart8.inc"
#include "PauseFontPixelsPart9.inc"
};
static int nfs_pause_expand(unsigned char *rgba,size_t capacity){
 if(!rgba || capacity<4194304 || sizeof(nfs_pause_pixels_rle)%6)return 0;
 unsigned count=0;
 for(size_t i=0;i<sizeof(nfs_pause_pixels_rle);i+=6){unsigned n=nfs_pause_pixels_rle[i]|((unsigned)nfs_pause_pixels_rle[i+1]<<8);if(!n || n>270336-count)return 0;count+=n;}
 if(count!=270336)return 0;unsigned position=0;
 for(size_t i=0;i<sizeof(nfs_pause_pixels_rle);i+=6){unsigned n=nfs_pause_pixels_rle[i]|((unsigned)nfs_pause_pixels_rle[i+1]<<8);for(unsigned j=0;j<n;++j,++position)memcpy(rgba+((size_t)(640+position/704)*1024+320+position%704)*4,nfs_pause_pixels_rle+i+2,4);}
 return 1;
}

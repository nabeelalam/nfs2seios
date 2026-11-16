#pragma once
#include <math.h>
#include <stdint.h>
/* CPU screen projection stays in the original logical viewport. These are
 * contracts for a future native-aspect world pass, not a presentation toggle.
 * All three capabilities are mandatory: camera-only Hor+ stretches the HUD. */
enum { NFS_ASPECT_NATIVE_WORLD=1, NFS_ASPECT_SEPARATE_HUD=2,
       NFS_ASPECT_MATCHED_CULLING=4, NFS_ASPECT_REQUIRED=7 };
typedef struct NfsAdaptiveAspectInput {
    int32_t drawableWidth,drawableHeight;
    int32_t viewLeft,viewTop,viewWidth,viewHeight;
    uint32_t capabilities;
    int32_t requested,raceWorldOwner;
} NfsAdaptiveAspectInput;
typedef struct NfsAdaptiveAspectProjection {
    double horizontalFactor;
    float focalX;
    int32_t integerFocalX,horizontalHalfAngle1024;
    double physicalLeft,physicalTop,physicalWidth,physicalHeight;
} NfsAdaptiveAspectProjection;
typedef struct NfsAdaptiveAspectPixelRect {
    int32_t left,top,width,height;
} NfsAdaptiveAspectPixelRect;
typedef struct NfsAdaptiveAspectRasterLayout {
    NfsAdaptiveAspectPixelRect world,hud;
} NfsAdaptiveAspectRasterLayout;

/* Resolve the real drawable aspect at the selected render height. Returning
 * zero leaves the existing contain target in use. The renderer and the world
 * camera must both use the same accepted target dimensions. */
static inline int32_t nfs_adaptive_aspect_target_width(
    int32_t drawableWidth,int32_t drawableHeight,int32_t renderHeight,int32_t maximumExtent)
{
    if(drawableWidth<=0 || drawableHeight<=0 || renderHeight<=0 ||
       maximumExtent<renderHeight)return 0;
    const int64_t scaledWidth=(int64_t)drawableWidth*3;
    const int64_t scaledHeight=(int64_t)drawableHeight*4;
    if(scaledWidth<=scaledHeight || scaledWidth>scaledHeight*4)return 0;
    const int64_t width=((int64_t)drawableWidth*renderHeight+drawableHeight/2)/drawableHeight;
    const int64_t containWidth=((int64_t)renderHeight*4+1)/3;
    return width>containWidth && width<=maximumExtent?(int32_t)width:0;
}

/* Use the renderer's logical clip here. In stacked split-screen, the original
 * projection is 640x480 with a y offset, while the raster clip is 640x240.
 * Snap shared edges once so adjacent views meet on odd framebuffer sizes. */
static inline int nfs_adaptive_aspect_raster_layout(
    int32_t targetWidth,int32_t targetHeight,NfsAdaptiveAspectPixelRect clip,
    NfsAdaptiveAspectRasterLayout *out)
{
    if(!out || targetWidth<=0 || targetHeight<=0 ||
       (int64_t)targetWidth*3 < (int64_t)targetHeight*4 ||
       clip.left<0 || clip.top<0 || clip.width<=0 || clip.height<=0 ||
       clip.width>640-clip.left || clip.height>480-clip.top)return 0;

    const int32_t hudWidth=(int32_t)(((int64_t)targetHeight*4+1)/3);
    const int32_t hudOrigin=(targetWidth-hudWidth)/2;
    const int32_t bottom=clip.top+clip.height,right=clip.left+clip.width;
    const int32_t topPx=(int32_t)(((int64_t)targetHeight*clip.top+240)/480);
    const int32_t bottomPx=(int32_t)(((int64_t)targetHeight*bottom+240)/480);
    const int32_t worldLeft=(int32_t)(((int64_t)targetWidth*clip.left+320)/640);
    const int32_t worldRight=(int32_t)(((int64_t)targetWidth*right+320)/640);
    const int32_t hudLeft=hudOrigin+(int32_t)(((int64_t)hudWidth*clip.left+320)/640);
    const int32_t hudRight=hudOrigin+(int32_t)(((int64_t)hudWidth*right+320)/640);
    if(bottomPx==topPx || worldRight==worldLeft || hudRight==hudLeft)return 0;
    const NfsAdaptiveAspectRasterLayout result={
        {worldLeft,topPx,worldRight-worldLeft,bottomPx-topPx},
        {hudLeft,topPx,hudRight-hudLeft,bottomPx-topPx}};
    *out=result;
    return 1;
}

/* 4808D0 -> 433910 writes float focal 4D5294. The same setup calls
 * 4A0704 -> 4A05C8, writing integer focal 4DAC24 and half-angle 4DAC30.
 * Preserve vertical focal/angle, logical centers and 433A40 clip rectangles.
 * Screen clipping still covers the full logical viewport after Hor+.
 * The renderer must use the physical viewport below for WORLD ONLY. HUD uses
 * an independent contain/anchored transform. Deferred camera command snapshots
 * must capture both projections before any subsequent camera/mirror setup. */
static inline int nfs_adaptive_aspect_projection(
    const NfsAdaptiveAspectInput *in,float originalFocalX,
    int32_t originalIntegerFocalX,int32_t originalHalfAngle1024,
    NfsAdaptiveAspectProjection *out)
{
    const double pi=3.14159265358979323846;
    if(!in||!out||!in->requested||!in->raceWorldOwner||
       (in->capabilities&NFS_ASPECT_REQUIRED)!=NFS_ASPECT_REQUIRED||
       in->drawableWidth<=0||in->drawableHeight<=0||
       in->viewLeft<0||in->viewTop<0||in->viewWidth<=0||in->viewHeight<=0||
       in->viewWidth>640-in->viewLeft||in->viewHeight>480-in->viewTop||
       !isfinite(originalFocalX)||originalFocalX==0||
       originalIntegerFocalX==0||originalHalfAngle1024<=0||
       originalHalfAngle1024>=256)return 0;
    /* The per-view target/original aspect quotient cancels view dimensions.
     * Thus stacked split-screen gets the SAME factor, not twice the factor. */
    const int64_t scaledWidth=(int64_t)in->drawableWidth*3;
    const int64_t scaledHeight=(int64_t)in->drawableHeight*4;
    const double factor=(double)scaledWidth/scaledHeight;
    /* Hor+ never removes visible subjects on a narrower portrait surface.
     * Such surfaces retain contain until an explicit Vert+ path is supplied. */
    if(!isfinite(factor)||factor<1.0||factor>4.0)return 0;
    /* Exact 4:3 is an identity, including the integer angle. Evaluating
       atan(tan(angle)) and rounding up can otherwise add an angle unit. */
    const double angle=scaledWidth==scaledHeight?originalHalfAngle1024:
        atan(tan(originalHalfAngle1024*pi/512.0)*factor)*512.0/pi;
    const double focal=originalIntegerFocalX/factor;
    if(!isfinite(angle)||angle>=256||!isfinite(focal)||fabs(focal)<1)return 0;
    NfsAdaptiveAspectProjection result;
    result.horizontalFactor=factor;
    result.focalX=(float)(originalFocalX/factor);
    result.integerFocalX=(int32_t)focal;
    /* Round OUTWARD for conservative visibility; never cull a newly exposed
     * side object due to angle-unit truncation. Original focal keeps precision. */
    result.horizontalHalfAngle1024=(int32_t)ceil(angle);
    result.physicalLeft=(double)in->viewLeft*in->drawableWidth/640;
    result.physicalTop=(double)in->viewTop*in->drawableHeight/480;
    result.physicalWidth=(double)in->viewWidth*in->drawableWidth/640;
    result.physicalHeight=(double)in->viewHeight*in->drawableHeight/480;
    *out=result;return 1;
}

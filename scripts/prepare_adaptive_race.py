#!/usr/bin/env python3
"""Prepare an experimental adaptive split-screen pair from prepared sources.

The paired output is explicit and separate from default engine and renderer
preparation. Use the engine and renderer directories from the same output root.
"""
from pathlib import Path
import argparse
import shutil


PATCHES = {
    'engine': {
        'Methods_03.cpp': [
            (r"""#include "hd/MenuTransitionGameHooks.inc"
// NFS_HD_BACKGROUND_END:include
Fn(void) Game::_sub_4225C0()
{
""", r"""#include "hd/MenuTransitionGameHooks.inc"
// NFS_HD_BACKGROUND_END:include
#include "hd/AdaptiveRaceTarget.h"

Fn(void) Game::_sub_4225C0()
{
"""),
            (r"""	edx = to32i(ebx+4); //mov
	eax = ebx; //mov
	call(to32i(dword_4EC500+edx*4));
	ebx = to32i(ebx); //mov
	test(ebx, ebx);
""", r"""	edx = to32i(ebx+4); //mov
	eax = ebx; //mov
	{
	    nfs::AdaptiveRaceDrawScope scope(to32i(dword_5222DC),to32i(dword_5222E0));
	    call(to32i(dword_4EC500+edx*4));
	}
	ebx = to32i(ebx); //mov
	test(ebx, ebx);
"""),
        ],
        'Methods_07.cpp': [
            (r"""#include "DATA.h"
#include "FunctionIds.h"
Fn(void) Game::_sub_45F980()
{
""", r"""#include "DATA.h"
#include "FunctionIds.h"
#include "MainThreadBridge.h"
extern "C" void nfs_adaptive_race_end_wide_race(void);
Fn(void) Game::_sub_45F980()
{
"""),
            (r"""	goto loc_4634C3;
loc_46350A:
	push32(eax);
	push32(4);
""", r"""	goto loc_4634C3;
loc_46350A:
	nfs::renderer_command([]()->int32_t{nfs_adaptive_race_end_wide_race();return 0;});
	push32(eax);
	push32(4);
"""),
        ],
        'Methods_08.cpp': [
            (r"""#include "DATA.h"
#include "FunctionIds.h"
Fn(void) Game::_sub_470AF0()
{
""", r"""#include "DATA.h"
#include "FunctionIds.h"
extern "C" void nfs_adaptive_race_begin_race_frame();
Fn(void) Game::_sub_470AF0()
{
"""),
            (r"""		goto loc_471CCD;
	eax = to32i(dword_540F28); //mov
	esp -= 4; _sub_46F7A0(); esp += 4; //call
loc_471CCD:
""", r"""		goto loc_471CCD;
	eax = to32i(dword_540F28); //mov
	nfs_adaptive_race_begin_race_frame();
	esp -= 4; _sub_46F7A0(); esp += 4; //call
loc_471CCD:
"""),
        ],
        'Methods_09.cpp': [
            (r"""// NFS_LOCATION_CAPTION_END:include

Fn(void) Game::_sub_477C40()
{
""", r"""// NFS_LOCATION_CAPTION_END:include

#include "hd/AdaptiveAspect.h"
extern "C" int nfs_adaptive_race_admit_target(int *,int *);
Fn(void) Game::_sub_477C40()
{
"""),
            (r"""	edx = eax; //mov
	eax = to32i(esp+0x10); //mov
	esp -= 4; _sub_4808D0(); esp += 4; //call
	goto loc_480757;
loc_4807CC:
""", r"""	edx = eax; //mov
	eax = to32i(esp+0x10); //mov
	{
	const bool splitRaceViewport=int32_t(eax)==0 && int32_t(ebx)==640 &&
	    int32_t(ecx)==480 && (int32_t(edx)==-120 || int32_t(edx)==120);
	esp -= 4; _sub_4808D0(); esp += 4; //call
	int targetWidth=0,targetHeight=0;
	if(splitRaceViewport && nfs_adaptive_race_admit_target(&targetWidth,&targetHeight)){
	    nfs::GuestCopyTransaction transaction;
	    NfsAdaptiveAspectProjection projection;
	    const NfsAdaptiveAspectInput view={targetWidth,targetHeight,0,0,640,240,NFS_ASPECT_REQUIRED,1,1};
	    if(nfs_adaptive_aspect_projection(&view,transaction.load<float>(dword_4D5294),
	        transaction.load<int32_t>(dword_4DAC24),transaction.load<int32_t>(dword_4DAC30),&projection)){
	        transaction.store<float>(dword_4D5294,projection.focalX);
	        transaction.store<int32_t>(dword_4DAC24,projection.integerFocalX);
	        transaction.store<int32_t>(dword_4DAC30,projection.horizontalHalfAngle1024);

	    }
	}
	}

	goto loc_480757;
loc_4807CC:
"""),
        ],
    },
    'renderer': {
        'GlideBridge.cpp': [
            (r"""#include "GuestAccess.h"
#include "NativeGlide.h"
""", r"""#include "hd/AdaptiveRaceTarget.h"
#include "MainThreadBridge.h"
namespace {
nfs::AdaptiveRaceTarget raceTarget;
int readExtent(nfs::AdaptiveRaceExtent size,int *width,int *height){
    if(!size)return 0;
    *width=size.width;*height=size.height;return 1;
}
}
extern "C" void nfs_adaptive_race_publish_target(int width,int height){raceTarget.offer({width,height});}
extern "C" int nfs_adaptive_race_admit_target(int *width,int *height){return readExtent(raceTarget.active(),width,height);}
extern "C" int nfs_adaptive_race_admitted_target(int *width,int *height){return readExtent(raceTarget.active(),width,height);}
extern "C" void nfs_adaptive_race_release_target(void){raceTarget.commit({});}
extern "C" void nfs_adaptive_race_replace_wide_target(int width,int height);
extern "C" void nfs_adaptive_race_begin_race_frame(){
    const auto desired=raceTarget.offered();
    if(desired==raceTarget.active())return;
    nfs::renderer_operation([desired]{
        nfs_adaptive_race_replace_wide_target(desired.width,desired.height);
        raceTarget.commit(desired);
        return 0;
    });
}
extern "C" void nfs_adaptive_race_apply_race_raster(unsigned);
#include "GuestAccess.h"
#include "NativeGlide.h"
"""),
            (r""" auto value0 = guest_load<GrVertex>(uint32_t(arg0));
 auto value1 = guest_load<GrVertex>(uint32_t(arg1));
 nfs::renderer_command([=]() mutable -> int32_t { native_grDrawLine(&value0, &value1); return 0; });
 return 0;
}
""", r""" auto value0 = guest_load<GrVertex>(uint32_t(arg0));
 auto value1 = guest_load<GrVertex>(uint32_t(arg1));
 const unsigned raceScope=nfs::AdaptiveRaceDrawScope::current();
 nfs::renderer_command([=]() mutable -> int32_t { nfs_adaptive_race_apply_race_raster(raceScope); native_grDrawLine(&value0, &value1); return 0; });
 return 0;
}
"""),
            (r""" auto value2 = guest_load<GrVertex>(uint32_t(arg2));
 const uint64_t text_context = nfs_front_text_context();
 nfs::renderer_command([=]() mutable -> int32_t { nfs::FrontendTextScope text_scope(text_context); native_grDrawTriangle(&value0, &value1, &value2); return 0; });
 return 0;
}
""", r""" auto value2 = guest_load<GrVertex>(uint32_t(arg2));
 const uint64_t text_context = nfs_front_text_context();
 const unsigned raceScope=nfs::AdaptiveRaceDrawScope::current();
 nfs::renderer_command([=]() mutable -> int32_t { nfs::FrontendTextScope text_scope(text_context); nfs_adaptive_race_apply_race_raster(raceScope); native_grDrawTriangle(&value0, &value1, &value2); return 0; });
 return 0;
}
"""),
        ],
        'Glide2x/OpenGL2.c': [
            (r"""//	fprintf(stderr, "grDrawLine: [%d]\n", g_trianglesCount);
	drawTriangles();
""", r"""    // The lower record plaque's left bevel has endpoints displaced by
    // one split viewport in opposite directions. Scope the repair to its
    // two observed lines, preserving their colors and the other race lines.
    GrVertex bevelStart,bevelEnd;
    if(a->x==b->x && a->a==127
       && ((a->x==VertexSnap+2 && a->y==VertexSnap+719 && b->y==VertexSnap+205
            && a->r==140 && a->g==138 && a->b==140)
           ||(a->x==VertexSnap+3 && a->y==VertexSnap+718 && b->y==VertexSnap+206
              && a->r==107 && a->g==109 && a->b==107))){
        bevelStart=*a;bevelEnd=*b;
        bevelStart.y-=240;bevelEnd.y+=240;
        a=&bevelStart;b=&bevelEnd;
    }
	drawTriangles();
"""),
            (r"""static uint32_t g_framebufferDepth;
static int32_t  g_framebufferHeight;

/* GLSL config */
""", r"""static uint32_t g_framebufferDepth;
static int32_t  g_framebufferHeight;
#include "hd/AdaptiveAspect.h"
static int wideRace=0,raceFramebufferWidth=0,raceWorldClip=-1;
static int wideRaceWidth=0,wideRaceHeight=0;
extern void nfs_adaptive_race_publish_target(int width,int height);
extern int nfs_adaptive_race_admitted_target(int *width,int *height);
extern void nfs_adaptive_race_release_target(void);
static NfsAdaptiveAspectPixelRect raceLogicalClip={0,0,640,480};

/* GLSL config */
"""),
            (r"""/* NFS_HD_SCENE_END:createFrameBuffer-head */

	int32_t shorterEdge = fixedFramebufferSize ? SDL_min(initialWinWidth, initialWinHeight) : SDL_min(winWidth, winHeight);
	if (g_framebufferHeight == shorterEdge)
		return;

	int32_t w = shorterEdge * 4 / 3;
	int32_t h = shorterEdge;

""", r"""/* NFS_HD_SCENE_END:createFrameBuffer-head */

	int32_t shorterEdge = wideRace ? wideRaceHeight :
	    fixedFramebufferSize ? SDL_min(initialWinWidth, initialWinHeight) : SDL_min(winWidth, winHeight);
	int32_t w = wideRace ? wideRaceWidth : shorterEdge * 4 / 3;
	if (g_framebufferHeight == shorterEdge && raceFramebufferWidth==w)
		return;
	int32_t h = shorterEdge;

"""),
            (r"""    glBindFramebuffer(GL_FRAMEBUFFER,g_framebuffer);
    g_framebufferHeight = shorterEdge;
    nfs_render_target_changed(w,h);
}
""", r"""    glBindFramebuffer(GL_FRAMEBUFFER,g_framebuffer);
    g_framebufferHeight = shorterEdge;
    raceFramebufferWidth=w;
    nfs_render_target_changed(w,h);
}
"""),
            (r"""// 	fprintf(stderr, "grAlphaTestReferenceValue: %f\n", value / 255.0f);
}
REALIGN STDCALL void native_grClipWindow(uint32_t minX, uint32_t minY, uint32_t maxX, uint32_t maxY)
{
""", r"""// 	fprintf(stderr, "grAlphaTestReferenceValue: %f\n", value / 255.0f);
}
static int applyRaceClip(int world)
{
    NfsAdaptiveAspectRasterLayout layout;
    if(!nfs_adaptive_aspect_raster_layout(raceFramebufferWidth,g_framebufferHeight,raceLogicalClip,&layout))return 0;
    const NfsAdaptiveAspectPixelRect r=world?layout.world:layout.hud;
    glViewport(r.left,g_framebufferHeight-r.top-r.height,r.width,r.height);
    glScissor(r.left,g_framebufferHeight-r.top-r.height,r.width,r.height);
    raceWorldClip=world;
    return 1;
}
void nfs_adaptive_race_apply_race_raster(unsigned scope)
{
    const int world=scope==1 || scope==2;
    if(!wideRace){
        if(!world || raceLogicalClip.width!=640 || raceLogicalClip.height!=240)return;
        if(!nfs_adaptive_race_admitted_target(&wideRaceWidth,&wideRaceHeight))return;
        drawTriangles();
        GLint bound=0;glGetIntegerv(GL_TEXTURE_BINDING_2D,&bound);
        wideRace=1;
        createFrameBuffer();
        useGameProgram(true);
        glScissor(0,0,raceFramebufferWidth,g_framebufferHeight);
        glDepthMask(GL_TRUE);
        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthMask(g_depthMask);
        glBindTexture(GL_TEXTURE_2D,(GLuint)bound);

    }
    if(raceWorldClip!=world){
        drawTriangles();
        applyRaceClip(world);
    }
}
REALIGN STDCALL void native_grClipWindow(uint32_t minX, uint32_t minY, uint32_t maxX, uint32_t maxY)
{
"""),
            (r"""	glViewport(scaledMinX, g_framebufferHeight - scaledMaxY, scaledMaxX - scaledMinX, scaledMaxY - scaledMinY);
	glScissor (scaledMinX, g_framebufferHeight - scaledMaxY, scaledMaxX - scaledMinX, scaledMaxY - scaledMinY);

	glLineWidth(2.0f * ratio);

""", r"""    raceLogicalClip=(NfsAdaptiveAspectPixelRect){(int32_t)minX,(int32_t)minY,(int32_t)(maxX-minX),(int32_t)(maxY-minY)};
    raceWorldClip=-1;
    if(!wideRace || !applyRaceClip(0)){
        glViewport(scaledMinX,g_framebufferHeight-scaledMaxY,scaledMaxX-scaledMinX,scaledMaxY-scaledMinY);
        glScissor(scaledMinX,g_framebufferHeight-scaledMaxY,scaledMaxX-scaledMinX,scaledMaxY-scaledMinY);
    }
	glLineWidth(2.0f * ratio);

"""),
            (r"""	matrixScale2(ratio, ratio);
	glUniformMatrix4fv(g_uMatrixLoc, 1, GL_FALSE, g_matrix);
}
REALIGN STDCALL void native_grBufferClear(GrColor_t color, GrAlpha_t alpha, uint16_t depth)
""", r"""	matrixScale2(ratio, ratio);
	glUniformMatrix4fv(g_uMatrixLoc, 1, GL_FALSE, g_matrix);
}
void nfs_adaptive_race_replace_wide_target(int width,int height)
{
    // Admission from contain still waits for a world draw. Handle an existing
    // wide target here before the next frame's camera or geometry is computed.
    if(!wideRace)return;
    drawTriangles();
    GLint bound=0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D,&bound);
    const NfsAdaptiveAspectPixelRect clip=raceLogicalClip;
    wideRace=width>0 && height>0;
    wideRaceWidth=width;wideRaceHeight=height;raceWorldClip=-1;
    createFrameBuffer();
    useGameProgram(true);
    glScissor(0,0,raceFramebufferWidth,g_framebufferHeight);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthMask(g_depthMask);
    native_grClipWindow(clip.left,clip.top,clip.left+clip.width,clip.top+clip.height);
    glBindTexture(GL_TEXTURE_2D,(GLuint)bound);

}
void nfs_adaptive_race_end_wide_race(void)
{
    nfs_adaptive_race_release_target();
    if(!wideRace)return;
    drawTriangles();
    GLint bound=0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D,&bound);
    wideRace=0;
    raceWorldClip=-1;
    createFrameBuffer();
    useGameProgram(true);
    native_grClipWindow(0,0,640,480);
    glBindTexture(GL_TEXTURE_2D,(GLuint)bound);

}
REALIGN STDCALL void native_grBufferClear(GrColor_t color, GrAlpha_t alpha, uint16_t depth)
"""),
            (r"""#endif

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

/* NFS_HD_SCENE_BEGIN:native_grBufferClear-tail */
""", r"""#endif

    NfsAdaptiveAspectRasterLayout wideClear;
    const int adjustClear=wideRace && nfs_adaptive_aspect_raster_layout(
        raceFramebufferWidth,g_framebufferHeight,raceLogicalClip,&wideClear);
    if(adjustClear){
        const NfsAdaptiveAspectPixelRect r=wideClear.world;
        glScissor(r.left,g_framebufferHeight-r.top-r.height,r.width,r.height);
    }
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    if(adjustClear){
        const NfsAdaptiveAspectPixelRect r=raceWorldClip==1?wideClear.world:wideClear.hud;
        glScissor(r.left,g_framebufferHeight-r.top-r.height,r.width,r.height);
    }

/* NFS_HD_SCENE_BEGIN:native_grBufferClear-tail */
"""),
            (r"""/* NFS_MENU_RESPONSIVE_RENDERER_END:presentation-viewport */

    clearUnusedArea(viewport.left,viewport.top,viewport.width,viewport.height);
    glViewport(viewport.left,winHeight-viewport.height-viewport.top,viewport.width,viewport.height);
""", r"""/* NFS_MENU_RESPONSIVE_RENDERER_END:presentation-viewport */

    const int height=fixedFramebufferSize ? SDL_min(initialWinWidth, initialWinHeight) : SDL_min(winWidth,winHeight);
    static int publishedWidth=0,publishedHeight=0,publishedRenderHeight=0;
    if(winWidth!=publishedWidth || winHeight!=publishedHeight || height!=publishedRenderHeight){
        GLint maxTexture=0;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE,&maxTexture);
        const int width=nfs_adaptive_aspect_target_width(winWidth,winHeight,height,maxTexture);
        nfs_adaptive_race_publish_target(width,height);
        publishedWidth=winWidth;publishedHeight=winHeight;publishedRenderHeight=height;
    }
    if(wideRace && !nfs_aspect_viewport(raceFramebufferWidth,g_framebufferHeight,winWidth,winHeight,true,&viewport))return false;
    clearUnusedArea(viewport.left,viewport.top,viewport.width,viewport.height);
    glViewport(viewport.left,winHeight-viewport.height-viewport.top,viewport.width,viewport.height);
"""),
        ],
    },
}



def prepare(engine, renderer, output):
    build = (Path(__file__).resolve().parents[1] / ".build").resolve()
    roots = {"engine": engine.resolve(), "renderer": renderer.resolve()}
    output = output.resolve()
    if output.exists() or not output.parent.is_dir() or build not in output.parents:
        raise ValueError("Use a new output directory within .build")
    if any(build not in path.parents or not path.is_dir() or path in output.parents
           for path in roots.values()) or roots["engine"] == roots["renderer"]:
        raise ValueError("Expected two separate prepared source directories within .build")

    staged = {}
    for group, files in PATCHES.items():
        for name, changes in files.items():
            source = roots[group] / name
            if source.is_symlink() or roots[group] not in source.resolve().parents:
                raise ValueError(f"Refusing to stage an external source: {name}")
            text = source.read_text(encoding="utf-8")
            for index, (old, new) in enumerate(changes):
                if text.count(old) != 1:
                    raise ValueError(f"Adaptive race source anchor changed: {name} #{index + 1}")
                text = text.replace(old, new)
            staged[group, name] = text

    output.mkdir(exist_ok=False)
    for group, source in roots.items():
        shutil.copytree(source, output / group, symlinks=True)
    for (group, name), text in staged.items():
        path = output / group / name
        if path.is_symlink() or output not in path.resolve().parents:
            raise ValueError(f"Refusing to write a source symlink: {name}")
        path.write_text(text, encoding="utf-8")
    return output


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--engine", required=True, type=Path)
    parser.add_argument("--renderer", required=True, type=Path)
    parser.add_argument("--output-dir", required=True, type=Path)
    args = parser.parse_args()
    prepare(args.engine, args.renderer, args.output_dir)
    print("Prepared experimental adaptive split-screen engine and renderer sources")

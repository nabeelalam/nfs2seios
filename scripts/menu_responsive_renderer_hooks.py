"""Main-only actual semantic presentation hooks; original operations preserved."""
import re
MARKER="NFS_MENU_RESPONSIVE_RENDERER"
BLOCK=re.compile(r"\n/\* NFS_MENU_RESPONSIVE_RENDERER_BEGIN:([a-z0-9-]+) \*/\n.*?\n/\* NFS_MENU_RESPONSIVE_RENDERER_END:\1 \*/\n",re.S)
def add_menu_responsive_renderer_hooks(text):
 original=text
 if MARKER in text:raise ValueError("Responsive renderer already inserted")
 def block(tag,code):return "\n/* "+MARKER+"_BEGIN:"+tag+" */\n"+code+"\n/* "+MARKER+"_END:"+tag+" */\n"
 def after(needle,code,tag):
  nonlocal text
  if text.count(needle)!=1:raise ValueError("Responsive renderer anchor changed: "+tag)
  text=text.replace(needle,needle+block(tag,code))
 def before(needle,code,tag):
  nonlocal text
  if text.count(needle)!=1:raise ValueError("Responsive renderer anchor changed: "+tag)
  text=text.replace(needle,block(tag,code)+needle)
 after("} TextureInfo;", "static void nfs_menu_wide_cancel(void);\nstatic void nfs_menu_wide_cancel_at(unsigned);\nstatic void nfs_menu_wide_retire(int);\nstatic void nfs_menu_wide_tail(unsigned);\nstatic GLuint nfs_menu_wide_present(GLuint,GLuint,int,int);\nstatic void nfs_menu_wide_after_present(int);\nstatic void nfs_menu_original_invalidate(uint32_t,uint64_t);\nstatic void nfs_menu_original_upload_begin(TextureInfo*);\nstatic void nfs_menu_original_upload_done(TextureInfo*);\nstatic void nfs_menu_original_select(unsigned,uint32_t,uint32_t,const GrTexInfo*,TextureInfo*);", "forward")
 # Runs while all actual native HD-font/mark uniforms and texture units remain
 # live. Existing intro/mark callbacks and the original draw stay byte-exact.
 after("\tglDrawArrays(GL_TRIANGLES, 0, g_trianglesCount * 3);"," nfs_menu_wide_tail(g_trianglesCount);","tail")
 before(" nfs_player_intro_renderer_retired(g_glCtx && SDL_GL_GetCurrentContext()==g_glCtx);"," nfs_menu_wide_retire(g_glCtx && SDL_GL_GetCurrentContext()==g_glCtx);","retire")
 before(" nfs_player_intro_renderer_retired(0);"," nfs_menu_wide_retire(0);","abandon")
 # Unsupported lines/LFB writes/clear/resize cannot leave an old pending frame.
 for site,name in enumerate(("destroyFrameBuffer","createFrameBuffer","native_grDrawLine","native_grLfbLock","native_grLfbUnlock","native_grBufferClear"),101):
  needle=" nfs_player_intro_renderer_other();"
  pattern=re.compile(r"(\n/\* NFS_PLAYER_INTRO_RENDERER_BEGIN:entry-"+name.lower().replace("_","-")+r" \*/\n)(.*?)(\n/\* NFS_PLAYER_INTRO_RENDERER_END:entry-"+name.lower().replace("_","-")+r" \*/\n)",re.S)
  hits=list(pattern.finditer(text))
  if len(hits)!=1:raise ValueError("Responsive invalidation hook changed: "+name)
  hit=hits[0];point=hit.start();text=text[:point]+block("invalidate-"+name.lower().replace("_","-")," nfs_menu_wide_cancel_at("+str(site)+");")+text[point:]
 # Invalidate before source bytes are overwritten; bind source identity only
 # after actual upload and on actual native selection, never by GL name alone.
 needle=" nfs_logo_native_upload_begin(ti);"
 if text.count(needle)!=2:raise ValueError("Original branding upload inventory changed")
 pieces=text.split(needle);text=pieces[0]+needle+block("original-upload-begin-0"," nfs_menu_original_upload_begin(ti);")+pieces[1]+needle+block("original-upload-begin-1"," nfs_menu_original_upload_begin(ti);")+pieces[2]
 after(" nfs_logo_native_upload_done(ti);"," nfs_menu_original_upload_done(ti);","original-upload-done")
 after(" nfs_native_mark_gpu_invalidate(startAddress,(uint64_t)ti->size*ti->size*(ti->fmt==GR_TEXFMT_P_8?1u:2u));"," nfs_menu_original_invalidate(startAddress,(uint64_t)ti->size*ti->size*(ti->fmt==GR_TEXFMT_P_8?1u:2u));","original-download-invalidate")
 after(" nfs_player_intro_renderer_select(tmu,startAddress,evenOdd,info,ti);"," nfs_menu_original_select(tmu,startAddress,evenOdd,info,ti);","original-select")
 # Choose output BEFORE viewport selection; wide texture alone would still be
 # pillarboxed by the old640x480 projection. Other presentation owners retain
 # their old arbitration and exact fallback.
 before("    NfsAspectRect viewport;","    const GLuint nfs_menu_wide_texture=nfs_menu_wide_present(g_frontFramebuffer,g_frontTexture,winWidth,winHeight);","presentation-select")
 after("    if(!nfs_aspect_viewport(640,480,winWidth,winHeight,keepAspectRatio,&viewport))return false;","    if(nfs_menu_wide_texture){viewport.left=viewport.top=0;viewport.width=winWidth;viewport.height=winHeight;}","presentation-viewport")
 after(" glBindTexture(GL_TEXTURE_2D,nfs_player_intro_renderer_presentation_texture(g_frontFramebuffer,g_frontTexture));"," if(nfs_menu_wide_texture)glBindTexture(GL_TEXTURE_2D,nfs_menu_wide_texture);","presentation-texture")
 after("const BOOL presented=SDL_GL_SwapWindow(sdlWin);"," nfs_menu_wide_after_present(presented);","presentation-complete")
 text+=block("implementation",'#include "hd/MenuResponsiveRenderer.inc"')
 if BLOCK.sub("",text)!=original:raise ValueError("Responsive renderer insertion reversal failed")
 return text

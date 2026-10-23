"""Insertion-only hooks for display-only normal Player intro rendering.

Apply after background-scene and native-mark hooks. Original front/back color
surfaces remain the game's history; the intro texture is selected only during
window presentation. No blanket callback on normal BufferSwap.
"""
import re

MARKER = "NFS_PLAYER_INTRO_RENDERER"
_BLOCK = re.compile(r"\n/\* NFS_PLAYER_INTRO_RENDERER_BEGIN:([a-z0-9-]+) \*/\n.*?\n/\* NFS_PLAYER_INTRO_RENDERER_END:\1 \*/\n", re.S)
HELPERS = ("PlayerIntroRuntimeAbi.h", "PlayerIntroNativePass.h", "PlayerIntroRenderer.inc")


def reverse_player_intro_renderer_hooks(text):
    tags = _BLOCK.findall(text)
    if len(tags) != len(set(tags)):
        raise ValueError("Duplicate Player intro renderer tag")
    result = _BLOCK.sub("", text)
    if MARKER in result:
        raise ValueError("Malformed Player intro renderer hook")
    return result


def add_player_intro_renderer_hooks(text):
    original = text
    if MARKER in text:
        raise ValueError("Player intro renderer hooks already present")
    tags = set()
    def block(tag, code):
        if tag in tags:
            raise ValueError("Duplicate Player intro renderer tag")
        tags.add(tag)
        return "\n/* " + MARKER + "_BEGIN:" + tag + " */\n" + code + "\n/* " + MARKER + "_END:" + tag + " */\n"
    def after(anchor, code, tag):
        nonlocal text
        if text.count(anchor) != 1:
            raise ValueError("Player intro renderer anchor changed: " + tag)
        text = text.replace(anchor, anchor + block(tag, code))
    def before(anchor, code, tag):
        nonlocal text
        if text.count(anchor) != 1:
            raise ValueError("Player intro renderer anchor changed: " + tag)
        text = text.replace(anchor, block(tag, code) + anchor)
    def bounds(name):
        pattern = r"(?:native_)?" + re.escape(name[7:]) if name.startswith("native_") else re.escape(name)
        matches = list(re.finditer(r"^[^\n;]*\b" + pattern + r"\([^;\n]*\)\s*\n\{", text, re.M))
        if len(matches) != 1:
            raise ValueError("Player intro renderer function changed: " + name)
        start = text.index("{", matches[0].start()); end = start + 1; depth = 1
        while depth:
            if end >= len(text):
                raise ValueError("Unbalanced Player intro renderer function")
            depth += (text[end] == "{") - (text[end] == "}"); end += 1
        return start, end
    def entry(name, code):
        nonlocal text
        start, _ = bounds(name)
        text = text[:start+1] + block("entry-" + name.lower().replace("_", "-"), code) + text[start+1:]
    def in_function(name, anchor, code, tag):
        nonlocal text
        start, end = bounds(name); part = text[start:end]
        if part.count(anchor) != 1:
            raise ValueError("Player intro renderer lexical anchor changed: " + tag)
        part = part.replace(anchor, anchor + block(tag, code))
        text = text[:start] + part + text[end:]
    after("} TextureInfo;", """static void nfs_player_intro_renderer_created(void);
static void nfs_player_intro_renderer_retired(int);
static void nfs_player_intro_renderer_other(void);
static void nfs_player_intro_renderer_state(void);
static void nfs_player_intro_renderer_upload_begin(TextureInfo*);
static void nfs_player_intro_renderer_select(unsigned,uint32_t,uint32_t,const GrTexInfo*,TextureInfo*);
static void nfs_player_intro_renderer_triangle(const GrVertex*,const GrVertex*,const GrVertex*);
static void nfs_player_intro_renderer_tail_batch(unsigned);
static uint32_t nfs_player_intro_renderer_presentation_texture(uint32_t,uint32_t);
static void nfs_player_intro_renderer_after_present(void);""", "forward")
    after(" nfs_logo_native_created();", " nfs_player_intro_renderer_created();", "created")
    before(" nfs_logo_native_retired(g_glCtx && SDL_GL_GetCurrentContext()==g_glCtx);", " nfs_player_intro_renderer_retired(g_glCtx && SDL_GL_GetCurrentContext()==g_glCtx);", "retired")
    before(" nfs_logo_native_retired(0);", " nfs_player_intro_renderer_retired(0);", "abandoned")
    for name in ("destroyFrameBuffer", "createFrameBuffer", "native_grDrawLine", "native_grLfbLock", "native_grLfbUnlock", "native_grBufferClear"):
        entry(name, " nfs_player_intro_renderer_other();")
    # BufferSwap intentionally preserves Ready until physical FBO/color swap
    # has happened and presentation_texture independently matches that pair.
    for name in ("native_grAlphaBlendFunction", "native_grClipWindow", "native_grDepthBufferFunction", "native_grDepthMask", "native_grFogColorValue", "native_grFogMode", "native_grRenderBuffer", "native_grTexDownloadTable"):
        entry(name, " nfs_player_intro_renderer_state();")
    entry("native_grAlphaCombine", " if(other!=GR_COMBINE_OTHER_TEXTURE || !g_textureEnabled)nfs_player_intro_renderer_state();")
    # Texture upload plus palette re-expansion: both revoke a pending display.
    anchor = " nfs_logo_native_upload_begin(ti);"
    if text.count(anchor) != 2:
        raise ValueError("Player intro renderer upload inventory changed")
    pieces = text.split(anchor)
    text = pieces[0] + anchor + block("upload-0", " nfs_player_intro_renderer_upload_begin(ti);") + pieces[1] + anchor + block("upload-1", " nfs_player_intro_renderer_upload_begin(ti);") + pieces[2]
    after(" nfs_logo_native_select(tmu,startAddress,evenOdd,info,ti);", " nfs_player_intro_renderer_select(tmu,startAddress,evenOdd,info,ti);", "select")
    after(" nfs_logo_native_triangle(a,b,c);", " nfs_player_intro_renderer_triangle(a,b,c);", "triangle")
    in_function("drawTriangles", "\tglDrawArrays(GL_TRIANGLES, 0, g_trianglesCount * 3);", " nfs_player_intro_renderer_tail_batch(g_trianglesCount);", "tail-batch")
    # Insert a second bind, preserving the original bind byte-for-byte for
    # reversal and leaving both original game surfaces untouched.
    in_function("presentFrontSurface", "\tglBindTexture(GL_TEXTURE_2D, g_frontTexture);", " glBindTexture(GL_TEXTURE_2D,nfs_player_intro_renderer_presentation_texture(g_frontFramebuffer,g_frontTexture));", "display-texture")
    in_function("presentFrontSurface", "const BOOL presented=SDL_GL_SwapWindow(sdlWin);", " nfs_player_intro_renderer_after_present();", "display-finished")
    text += block("implementation", '#include "hd/PlayerIntroRenderer.inc"')
    if reverse_player_intro_renderer_hooks(text) != original:
        raise ValueError("Player intro renderer insertion reversal failed")
    return text

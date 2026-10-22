"""Fetch pinned SDL3 and prepare the existing Glide GLES2 renderer for iOS.

Includes typed guest adapters and tested framebuffer transfers; device/game
scene integration remains separate.
"""
import shutil
import subprocess
import re
from prepare_engine import ROOT, prepare

SDL_PIN = "829a65d769d935c4852f8159e964312c0957260a"
SDL_TAG = "release-3.4.18"


def frontend_pixel_parts(text):
    parts=re.findall(r'^#include "(FrontendFontPixelsPart[0-9]+\.inc)"$',text,re.M)
    includes=re.findall(r'^\s*#include\b[^\n]*$',text,re.M)
    if not 1<=len(parts)<=64 or len(parts)!=len(includes) or parts!=['FrontendFontPixelsPart'+str(i)+'.inc' for i in range(len(parts))]:
        raise ValueError('Unexpected authored frontend pixel header inventory')
    return parts


def prepare_renderer():
    vendor = ROOT / ".build/vendor/SDL"
    if not vendor.exists():
        subprocess.run(["git", "clone", "--depth", "1", "--branch", SDL_TAG,
                        "https://github.com/libsdl-org/SDL.git", str(vendor)], check=True)
    actual = subprocess.check_output(["git", "-C", str(vendor), "rev-parse", "HEAD"], text=True).strip()
    if actual != SDL_PIN: raise ValueError(f"SDL source mismatch: {actual}")
    source = prepare()
    dest = ROOT / ".build/renderer"
    dest.mkdir(exist_ok=True)
    for name in ["Glide2x.c", "Glide2x.h", "Wrapper.h"]:
        shutil.copyfile(source / name, dest / name)
    wrapper = dest / "Wrapper.h"
    wrapper_text=wrapper.read_text().replace('void *this', 'void *game_context')
    allocation='void *malloc32(size_t size);\nvoid free32(void *p);'
    if wrapper_text.count(allocation)!=1:raise ValueError('Unexpected native allocation declarations')
    wrapper_text=wrapper_text.replace(allocation,'#ifdef __cplusplus\nextern "C" {\n#endif\n'+allocation+'\n#ifdef __cplusplus\n}\n#endif')
    wrapper.write_text(wrapper_text)
    shutil.copytree(source / "Glide2x", dest / "Glide2x", dirs_exist_ok=True)
    path = dest / "Glide2x/OpenGL2.c"
    text = path.read_text()
    if text.count("glBindFramebuffer(GL_FRAMEBUFFER, 0);") != 1:
        raise ValueError("Unexpected default framebuffer binding count")
    text = text.replace('#include <stdlib.h>', '#include <stdlib.h>\n#include <signal.h>')
    text = text.replace('static SDL_GLContext g_glCtx;', 'static SDL_GLContext g_glCtx;\nstatic GLint g_drawableFramebuffer;\nstatic GLint g_drawableRenderbuffer;')
    needle = '\n#ifndef GLES2\n\tglGetShaderiv = SDL_GL_GetProcAddress'
    if text.count(needle) != 1: raise ValueError("Cannot identify post-context initialization")
    text = text.replace(needle, '\n#ifdef GLES2\n\tglGetIntegerv(GL_FRAMEBUFFER_BINDING, &g_drawableFramebuffer);\n\tglGetIntegerv(GL_RENDERBUFFER_BINDING, &g_drawableRenderbuffer);\n#endif\n' + needle)
    text = text.replace('glBindFramebuffer(GL_FRAMEBUFFER, 0);', 'glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)g_drawableFramebuffer);')
    # SDL UIKit presents the currently bound drawable renderbuffer. The game's
    # offscreen depth target leaves renderbuffer 0 bound; restore before swap.
    if text.count('SDL_GL_SwapWindow(sdlWin);') != 1: raise ValueError("Unexpected presentation call count")
    text = text.replace('SDL_GL_SwapWindow(sdlWin);', '#ifdef GLES2\n\tglBindRenderbuffer(GL_RENDERBUFFER, (GLuint)g_drawableRenderbuffer);\n#endif\n\tSDL_GL_SwapWindow(sdlWin);')
    text = text.replace('#include <signal.h>', '#include <signal.h>\n#include "GuestRuntimeC.h"')
    pointer = 'info->lfbPtr = (uint32_t)(uintptr_t)g_lfb;'
    if text.count(pointer) != 1: raise ValueError("Cannot identify linear framebuffer pointer export")
    text = text.replace(pointer, 'info->lfbPtr = nfs_guest_address(g_lfb);')
    pointer = 'uint16_t *dataIn  = (uint16_t *)(uintptr_t)info->data;'
    if text.count(pointer) != 1: raise ValueError("Cannot identify texture pointer import")
    text = text.replace(pointer, 'uint16_t *dataIn = (uint16_t *)nfs_host_address(info->data, ti->size * ti->size * (ti->fmt == GR_TEXFMT_P_8 ? 1 : 2));')
    text = text.replace('uint16_t value = dataIn[i];', 'uint16_t value; memcpy(&value, (const uint8_t *)dataIn + i * 2, sizeof(value));')
    text = text.replace('uint32_t value = g_palette[data[i]];', 'uint32_t value; memcpy(&value, (const uint8_t *)g_palette + data[i] * 4, sizeof(value));')
    # largeLod=0 is a supported 256x256 texture. Palette expansion writes one
    # RGBA value per texel, including the last row; the upstream 32x32 scratch
    # buffer cannot hold that valid download.
    scratch = 'g_tmpTexture[0x400]'
    if text.count(scratch) != 1: raise ValueError("Cannot identify palette conversion scratch buffer")
    text = text.replace(scratch, 'g_tmpTexture[256 * 256]')
    text = prepare_palettes(text, dest)
    text = text.replace('g_lfb = (uint8_t *)malloc32(640*480*2);', 'g_lfb = (uint8_t *)malloc32(640*480*2);\n\t\tif (!g_lfb) return false;')
    text = prepare_surfaces(text, dest)
    text = prepare_hd_atlas(text, dest)
    opening='\tcreateContext();\n\tcreateFrameBuffer();\n\tuseGameProgram(true);\n\n\thandleDpr();'
    if text.count(opening)!=1: raise ValueError('Unexpected drawable initialization order')
    text=text.replace(opening,'\thandleDpr();\n\tcreateContext();\n\tcreateFrameBuffer();\n\tuseGameProgram(true);')
    # Permanent suspended shutdown releases an already-unbound context.
    # Do not run the ordinary explicit GL deletion path in the background.
    marker = "static void setTextureFiltering()"
    if text.count(marker) != 1: raise ValueError("Cannot identify context abandonment insertion")
    text = text.replace(marker, 'BOOL nfs_glide_abandon_unbound(void){\n    if(SDL_GL_GetCurrentContext()!=NULL)return false;\n    nfs_glide_lfb_reset();\n    if(g_glCtx && !SDL_GL_DestroyContext(g_glCtx))return false;\n    nfs_hd_context_retired(0);g_glCtx=NULL;g_trianglesCount=0;g_palette=NULL;\n    return true;\n}\n' + marker)
    text = prepare_background_scene(text, dest)
    text = apply_native_mark_hooks(text)
    copy_native_mark_helpers(dest)
    path.write_text('#include "FramePresentation.h"\n' + text)
    prepare_glide_exports(source, dest)
    print(f"Prepared Glide GLES2 renderer and SDL {SDL_TAG} at {SDL_PIN}")




def copy_native_mark_helpers(dest):
    target = dest / 'hd'
    target.mkdir(exist_ok=True)
    for name in ('NativeMarkGpu.h', 'NativeMarkGpu.c.inc', 'NativeMarkArt.h', 'NativeMarkProfile.h', 'NativeMarkProvider.h'):
        shutil.copyfile(ROOT / 'src/hd' / name, target / name)


def apply_native_mark_hooks(text):
    """Add optional native-mark sampling without altering original renderer bytes."""
    original = text
    def block(tag, body):
        return '\n/* NFS_HD_MARK_BEGIN:' + tag + ' */\n' + body + '\n/* NFS_HD_MARK_END:' + tag + ' */\n'
    def bounds(name):
        pattern = r'(?:native_)?' + re.escape(name[7:]) if name.startswith('native_') else re.escape(name)
        matches = list(re.finditer(r'^[^\n;]*\b' + pattern + r'\([^;\n]*\)\s*\n?\{', text, re.M))
        if len(matches) != 1: raise ValueError('Missing or repeated native mark hook: ' + name)
        a = text.index('{', matches[0].start()); b = a + 1; depth = 1
        while depth:
            if b >= len(text): raise ValueError('Unbalanced native mark function')
            depth += (text[b] == '{') - (text[b] == '}'); b += 1
        return a,b
    def insert(name, body, tail=False):
        nonlocal text
        a,b = bounds(name); at = b-1 if tail else a+1
        text = text[:at] + block(name + ('-tail' if tail else '-head'), body) + text[at:]
    def after(needle, body, tag, name=None):
        nonlocal text
        a,b = bounds(name) if name else (0,len(text)); part = text[a:b]
        if part.count(needle) != 1: raise ValueError('Unexpected native mark anchor: ' + tag)
        text = text[:a] + part.replace(needle, needle + block(tag,body)) + text[b:]
    after('} TextureInfo;', '#include "hd/NativeMarkGpu.h"', 'declaration')
    a = text.index('const char g_fShaderSrc[] ='); b = text.index('const char g_vShaderDispSrc[] =',a)
    part = text[a:b]
    for needle,body,tag in [
      ('"uniform sampler2D uTextureSampler;"', '"uniform sampler2D uMarkTextureSampler;uniform float uMarkActive;"', 'shader-uniform'),
      ('"vec4 texture = uTextureEnabled * sampled;"', '"if(uMarkActive>0.5){vec4 mark=texture2D(uMarkTextureSampler,vTexCoord.st/vTexCoord.q);sampled=vec4(mark.a>0.0?mark.rgb/mark.a:vec3(0.0),mark.a);}"', 'shader-sample')]:
        if part.count(needle)!=1: raise ValueError('Unexpected native mark shader: '+tag)
        part = part.replace(needle, block(tag,body)+needle)
    text=text[:a]+part+text[b:]
    insert('createContext',' nfs_native_mark_gpu_created((uintptr_t)g_glCtx,g_shaderProgram);',True)
    insert('destroyContext',' nfs_native_mark_gpu_retired(g_glCtx && SDL_GL_GetCurrentContext()==g_glCtx);')
    after('if(SDL_GL_GetCurrentContext()!=NULL)return false;', ' nfs_native_mark_gpu_retired(0);', 'unbound')
    insert('uploadTexture', ' if((uintptr_t)ti->data>=(uintptr_t)g_textureMem && (uintptr_t)ti->data-(uintptr_t)g_textureMem<sizeof(g_textureMem))nfs_native_mark_gpu_invalidate((uint32_t)((uintptr_t)ti->data-(uintptr_t)g_textureMem),(uint64_t)ti->size*ti->size*(ti->fmt==GR_TEXFMT_P_8?1u:2u));')
    insert('uploadTexture', ' NfsNativeMarkProfile nfs_mark_profile; uint32_t nfs_mark_encoding=ti->fmt==GR_TEXFMT_ARGB_1555?NFS_NATIVE_MARK_SOURCE_RGBA5551_LE:(ti->fmt==GR_TEXFMT_ARGB_4444?NFS_NATIVE_MARK_SOURCE_RGBA4444_LE:0u); if(nfs_native_mark_profile(nfs_mark_encoding,ti->size,&nfs_mark_profile) && (uintptr_t)ti->data>=(uintptr_t)g_textureMem && (uintptr_t)ti->data-(uintptr_t)g_textureMem<=sizeof(g_textureMem) && sizeof(g_textureMem)-((uintptr_t)ti->data-(uintptr_t)g_textureMem)>=nfs_mark_profile.preparedBytes)nfs_native_mark_gpu_uploaded((uint32_t)((uintptr_t)ti->data-(uintptr_t)g_textureMem),ti->id,ti->size,nfs_mark_encoding,ti->hdDigits,ti->data);',True)
    after('\tdrawTriangles();', ' nfs_native_mark_gpu_invalidate(startAddress,(uint64_t)ti->size*ti->size*(ti->fmt==GR_TEXFMT_P_8?1u:2u));', 'download', 'native_grTexDownloadMipMap')
    after('\tglBindTexture(GL_TEXTURE_2D, ti->id);', ' NfsNativeMarkProfile nfs_mark_profile; uint32_t nfs_mark_encoding=ti->fmt==GR_TEXFMT_ARGB_1555?NFS_NATIVE_MARK_SOURCE_RGBA5551_LE:(ti->fmt==GR_TEXFMT_ARGB_4444?NFS_NATIVE_MARK_SOURCE_RGBA4444_LE:0u); if(!nfs_native_mark_profile(nfs_mark_encoding,ti->size,&nfs_mark_profile)||info->format!=ti->fmt||info->largeLod!=nfs_mark_profile.sourceLod||info->smallLod!=nfs_mark_profile.sourceLod||(nfs_mark_encoding==NFS_NATIVE_MARK_SOURCE_RGBA4444_LE&&(tmu!=0||info->aspectRatio!=GR_ASPECT_1x1)))nfs_mark_encoding=0; nfs_native_mark_gpu_selected(startAddress,ti->id,ti->size,nfs_mark_encoding,ti->hdDigits);', 'selected','native_grTexSource')
    common='g_textureEnabled,ti->hdDigits,(uintptr_t)g_glCtx,g_shaderProgram,nfs_hd_active_location,&nfs_hd_uniform_sent,g_framebufferHeight'
    # drawTriangles uses its current texture record, while lines always disable marks.
    after('nfs_hd_sync_uniform();', ' nfs_native_mark_gpu_sync(1,g_textureEnabled,nfs_hd_selected,(uintptr_t)g_glCtx,g_shaderProgram,nfs_hd_active_location,&nfs_hd_uniform_sent,g_framebufferHeight);','triangles-sync','drawTriangles')
    after('\tglDrawArrays(GL_TRIANGLES, 0, g_trianglesCount * 3);', ' nfs_native_mark_gpu_after_draw();','triangles-reset','drawTriangles')
    after('nfs_hd_sync_uniform();',' nfs_native_mark_gpu_sync(0,0,0,(uintptr_t)g_glCtx,g_shaderProgram,nfs_hd_active_location,&nfs_hd_uniform_sent,g_framebufferHeight);','line-sync','native_grDrawLine')
    insert('useGameProgram',' nfs_native_mark_gpu_sync(0,0,0,(uintptr_t)g_glCtx,g_shaderProgram,nfs_hd_active_location,&nfs_hd_uniform_sent,g_framebufferHeight);')
    text += block('implementation','#include "hd/NativeMarkGpu.c.inc"')
    restored = re.sub(r'\n/\* NFS_HD_MARK_BEGIN:([^*]+) \*/\n.*?\n/\* NFS_HD_MARK_END:\1 \*/\n', '', text, flags=re.S)
    if restored != original: raise ValueError('Native mark source reverse audit failed')
    return text

def prepare_background_scene(text, dest):
    # Add native pass witnesses; stripping tagged inserts restores every input byte.
    original = text
    def block(tag, body):
        return '\n/* NFS_HD_SCENE_BEGIN:' + tag + ' */\n' + body + '\n/* NFS_HD_SCENE_END:' + tag + ' */\n'
    def bounds(name):
        pattern = (r'(?:native_)?' + re.escape(name[7:])) if name.startswith('native_') else re.escape(name)
        matches = list(re.finditer(r'^[^\n;]*\b' + pattern + r'\([^;\n]*\)\s*\n\{', text, re.M))
        if len(matches) != 1: raise ValueError('Missing or repeated native scene hook: ' + name)
        m = matches[0]
        start = text.index('{', m.start()); depth = 1; end = start + 1
        while depth:
            if end >= len(text): raise ValueError('Unbalanced renderer function')
            depth += (text[end] == '{') - (text[end] == '}')
            end += 1
        return start, end
    def insert(name, body, tail=False):
        nonlocal text
        a,b = bounds(name); at = b-1 if tail else a+1
        text = text[:at] + block(name + ('-tail' if tail else '-head'), body) + text[at:]
    def replace_once(needle, body, tag):
        nonlocal text
        if text.count(needle) != 1: raise ValueError('Unexpected scene source anchor: ' + tag)
        text = text.replace(needle, needle + block(tag, body))
    text = block('early-forward', 'static void nfs_logo_native_other(void);') + text
    replace_once('} TextureInfo;', '''static void nfs_logo_native_created(void);
static void nfs_logo_native_retired(int);
static void nfs_logo_native_upload_begin(TextureInfo*);
static void nfs_logo_native_upload_done(TextureInfo*);
static void nfs_logo_native_state(void);
static void nfs_logo_native_clear(uint16_t,float,float,float);
static void nfs_logo_native_select(unsigned,uint32_t,uint32_t,const GrTexInfo*,TextureInfo*);
static void nfs_logo_native_triangle(const GrVertex*,const GrVertex*,const GrVertex*);''', 'forward')
    for name in ['destroyFrameBuffer','createFrameBuffer']:
        insert(name, ' nfs_logo_native_other();')
    insert('createContext',' nfs_logo_native_created();',True)
    insert('destroyContext',' nfs_logo_native_retired(g_glCtx && SDL_GL_GetCurrentContext()==g_glCtx);')
    replace_once('if(SDL_GL_GetCurrentContext()!=NULL)return false;', ' nfs_logo_native_retired(0);', 'unbound')
    insert('uploadTexture',' nfs_logo_native_upload_begin(ti);')
    insert('uploadTexture',' nfs_logo_native_upload_done(ti);',True)
    for name in ['native_grAlphaBlendFunction','native_grClipWindow','native_grDepthBufferFunction',
                 'native_grDepthMask','native_grFogColorValue','native_grFogMode',
                 'native_grRenderBuffer','native_grTexDownloadTable']:
        insert(name,' nfs_logo_native_state();')
    insert('native_grAlphaCombine',' if(other!=GR_COMBINE_OTHER_TEXTURE || !g_textureEnabled)nfs_logo_native_state();')
    insert('native_grBufferClear',' nfs_logo_native_clear(depth,r,g,b);',True)
    for name in ['native_grBufferSwap','native_grDrawLine','native_grLfbLock','native_grLfbUnlock']:
        insert(name,' nfs_logo_native_other();')
    insert('native_grDrawTriangle',' nfs_logo_native_triangle(a,b,c);')
    a,b = bounds('native_grTexSource'); part=text[a:b]
    needle='\tglBindTexture(GL_TEXTURE_2D, ti->id);'
    if part.count(needle)!=1: raise ValueError('Unexpected scene tile selection')
    part=part.replace(needle,needle+block('selected',' nfs_logo_native_select(tmu,startAddress,evenOdd,info,ti);'))
    needle='\t\tglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, g_tmpTexture);'
    if part.count(needle)!=1: raise ValueError('Unexpected scene palette upload')
    part=part.replace(needle,block('palette',' nfs_logo_native_upload_begin(ti);')+needle)
    text=text[:a]+part+text[b:]
    text += block('implementation','#include "hd/BackgroundSceneRenderer.inc"')
    restored = re.sub(r'\n/\* NFS_HD_SCENE_BEGIN:([^*]+) \*/\n.*?\n/\* NFS_HD_SCENE_END:\1 \*/\n', '', text, flags=re.S)
    if restored != original: raise ValueError('Scene source reverse audit failed')
    target=dest/'hd';target.mkdir(exist_ok=True)
    names=['BackgroundSceneRenderer.inc','NativeSceneProvenance.h','BackgroundSceneCompositor.h',
           'BackgroundSceneCompositor.c.inc','NativeSceneTarget.h','SceneGpuSupport.h',
           'SceneGpuSupport.inc','SmallLogoGpu.h','SmallLogoGpu.c.inc','HdArtProvider.h',
           'BackgroundSceneArt.h','SmallLogoArtAbi.h','BackgroundSceneRuntimeAbi.h','SmallLogoLayerAbi.h',
           'SoftwareTextVertex.h','SoftwareTextLayerAbi.h','SoftwareTextGpu.h',
           'SoftwareTextGpu.c.inc','LocationSceneRuntimeAbi.h','OriginalPreviewLayerAbi.h',
           'BackgroundSceneLayersAbi.h','AudioArtProvider.h','AudioLayerAbi.h','AudioBarGpu.h','AudioBarGpu.c.inc',
           'AudioSceneLayerAbi.h','AudioSceneLayers.h','AudioSceneLayers.c.inc',
           'AudioSceneRuntimeAbi.h','AudioNativePass.h','AudioSceneRenderer.inc',
           'BackgroundSceneResourcePrepare.h','BackgroundSceneResourcePrepare.c.inc',
           'GraphicsSceneRuntimeAbi.h','GraphicsSceneLayerAbi.h','GraphicsSceneLayers.h',
           'GraphicsSceneLayers.c.inc','GraphicsSceneRenderer.inc',
           'OptionsSceneRuntimeAbi.h','SetupSceneRuntimeAbi.h','OptionsSceneLayerAbi.h','OptionsSceneLayers.h',
           'OptionsSceneLayers.c.inc','OptionsSceneRenderer.inc']
    for name in names:shutil.copyfile(ROOT/'src/hd'/name,target/name)
    return text


def prepare_hd_atlas(text, dest):
    def checked(old, new, count=1):
        nonlocal text
        if text.count(old) != count:
            raise ValueError(f'Unexpected isolated HD renderer pattern: {old[:80]}')
        text = text.replace(old, new)

    marker = '#include "RendererPalette.inc"'
    if text.count(marker) != 1: raise ValueError("Cannot identify HD atlas helper insertion")
    text = text.replace(marker, marker + '\n#include "RendererHdAtlas.inc"\n#include "PanelMaterialRuntime.inc"')
    checked('\tuint32_t id;\n} TextureInfo;', '\tuint32_t id;\n\tuint32_t hdDigits;\n} TextureInfo;')
    checked('"uniform float uTextureEnabled;"',
            '"uniform sampler2D uHdTextureSampler;"\n\t"uniform float uHdActive;"\n\t"uniform float uHdCellLayout;"\n\t"uniform float uTextureEnabled;"')
    checked('"vec4 texture = uTextureEnabled * texture2D(uTextureSampler, vTexCoord.st / vTexCoord.q);"',
            '"vec4 sampled = texture2D(uTextureSampler, vTexCoord.st / vTexCoord.q);"\n'
            '\t\t"if (uHdActive > 0.5) {"\n'
            '#include "SpeedShader.inc"\n'
            '\t\t"vec4 texture = uTextureEnabled * sampled;"')
    checked('"vec4 ret = vColor * ((1.0 - uTextureEnabled) + texture);"', '"vec4 ret = vColor * ((1.0 - uTextureEnabled) + texture);"\n\t\t"if(uHdActive>.5 && vTexCoord.z>69.5 && vTexCoord.z<70.5)ret=sampled;"')
    checked('glUniform1i(glGetUniformLocation(g_shaderProgram, "uTextureSampler"), 0);',
            'glUniform1i(glGetUniformLocation(g_shaderProgram, "uTextureSampler"), 0);\n\t\tnfs_hd_program_created(g_shaderProgram);')
    checked('\tif(!g_glCtx)return;', '\tif(!g_glCtx)return;\n\tnfs_hd_context_retired(1);')
    # In full preparation abandonment is inserted below; a previously prepared
    # renderer used by focused fixtures already contains this function.
    if 'BOOL nfs_glide_abandon_unbound(void)' in text:
        checked('g_glCtx=NULL;g_trianglesCount=0;g_palette=NULL;',
                'nfs_hd_context_retired(0);g_glCtx=NULL;g_trianglesCount=0;g_palette=NULL;')
    checked('\tglDrawArrays(GL_TRIANGLES, 0, g_trianglesCount * 3);',
            '\tnfs_hd_sync_uniform();\n\tglDrawArrays(GL_TRIANGLES, 0, g_trianglesCount * 3);')
    checked('\tconst GrVertex *grVertices[3] = {a, b, c};',
            '\tGrVertex slashVertices[3];int slash=!g_textureEnabled?nfs_hd_silver_slash(a,b,c,slashVertices):0;\n\tif(slash==2)return;\n\tif(slash==1){a=slashVertices;b=slashVertices+1;c=slashVertices+2;}\n\tGrVertex panelVertices[3];\n\tif(!slash && !g_textureEnabled && nfs_hd_builtin_enabled && nfs_hd_visible && nfs_panel_material(a,b,c,panelVertices)){a=panelVertices;b=panelVertices+1;c=panelVertices+2;}\n\tGrVertex frontendVertices[3];int frontend=g_textureEnabled?nfs_front_font_geometry(a,b,c,frontendVertices):0;\n\tif(frontend){a=frontendVertices;b=frontendVertices+1;c=frontendVertices+2;}\n\tint hdDigit=frontend?72:slash==1?70:g_textureEnabled ? nfs_hd_triangle_selected(a,b,c) : 0;\n\tGrVertex labelVertices[3];int labelLayout=nfs_hd_label_geometry(hdDigit,a,b,c,labelVertices);\n\tif(labelLayout>0){a=labelVertices;b=labelVertices+1;c=labelVertices+2;}else if(labelLayout<0)hdDigit=0;\n\tconst GrVertex *grVertices[3] = {a, b, c};')
    checked('\t\tg_textureCoord[g_trianglesCount].vertex[i].q = grVertex->oow;',
            '\t\tg_textureCoord[g_trianglesCount].vertex[i].r = (float)hdDigit;\n\t\tg_textureCoord[g_trianglesCount].vertex[i].q = grVertex->oow;')
    checked('\t\tg_vertices->vertex[i].x = grVertices[i]->x - VertexSnap;',
            '\t\tg_textureCoord->vertex[i].r = 0;\n\t\tg_vertices->vertex[i].x = grVertices[i]->x - VertexSnap;')
    checked('\tglDrawArrays(GL_LINES, 0, 2);', '\tnfs_hd_sync_uniform();\n\tglDrawArrays(GL_LINES, 0, 2);')
    checked('\tti->data = &g_textureMem[startAddress];',
            '\tti->hdDigits=0;\n\tti->data = &g_textureMem[startAddress];')
    checked('\tglBindTexture(GL_TEXTURE_2D, ti->id);\n\tif (info->format',
            '\tglBindTexture(GL_TEXTURE_2D, ti->id);\n\tnfs_hd_selected=ti->hdDigits;\n\tif (info->format')
    # Desktop GL loads extension entry points explicitly; GLES2 exports them.
    checked('static PFNGLUNIFORM1IPROC glUniform1i;',
            'static PFNGLUNIFORM1IPROC glUniform1i;\nstatic PFNGLACTIVETEXTUREPROC glActiveTexture;')
    checked('\tglUniform1i = SDL_GL_GetProcAddress("glUniform1i");',
            '\tglUniform1i = SDL_GL_GetProcAddress("glUniform1i");\n\tglActiveTexture = SDL_GL_GetProcAddress("glActiveTexture");')
    checked('\tok &= !!glUniform1i;', '\tok &= !!glUniform1i;\n\tok &= !!glActiveTexture;')
    marker = '\tswitch (ti->fmt)'
    start = text.index('static void uploadTexture(TextureInfo *ti)')
    at = text.index(marker, start)
    text = text[:at] + '''\tvoid *originalData=ti->data;
    ti->data=nfs_front_font_restore_source(ti);
    if(ti->hdDigits==40 && nfs_hd_builtin_enabled && nfs_pause_valid && ti->size==256 && ti->fmt==GR_TEXFMT_ARGB_1555)ti->data=nfs_pause_original;
    if(ti->hdDigits==39 && nfs_hd_builtin_enabled && nfs_logo_valid && ti->size==64 && ti->fmt==GR_TEXFMT_ARGB_4444)ti->data=nfs_logo_original;
    if(ti->hdDigits>=28 && ti->hdDigits<=38 && nfs_hd_builtin_enabled &&
       (nfs_record_font_valid&(1u<<(ti->hdDigits-28))) &&
       ti->size==nfs_record_font_edge[ti->hdDigits-28] && ti->fmt==GR_TEXFMT_ARGB_1555)
        ti->data=nfs_record_font_original[ti->hdDigits-28];
    if(ti->hdDigits>=2 && ti->hdDigits<=27 && nfs_hd_builtin_enabled &&
       (nfs_small_font_valid&(1u<<(ti->hdDigits-2))) &&
       ti->size==nfs_small_font_edge[ti->hdDigits-2] && ti->fmt==GR_TEXFMT_ARGB_1555)
        ti->data=nfs_small_font_original[ti->hdDigits-2];
    if(ti->hdDigits==1 && nfs_hd_builtin_enabled && nfs_hd_atlas.original &&
       ti->size==nfs_hd_atlas.original_size && ti->fmt==GR_TEXFMT_ARGB_1555)
        ti->data=nfs_hd_atlas.original;
    if(nfs_hd_upload_texture(ti)){ti->data=originalData;return;}
    if(newTexture || ti->fmt!=GR_TEXFMT_P_8)nfs_hd_selected=ti->hdDigits;

''' + text[at:]
    end = text.index('\nstatic void recreateContext()', start)
    section = text[start:end]
    assert section.endswith('\n}\n')
    text = text[:start] + section[:-3] + '\n\tti->data=originalData;\n}\n' + text[end:]
    # A word spanning two source pages becomes eligible after one full frame.
    frames=list(re.finditer(r'\b(?:native_)?grBufferSwap\([^)]*\)\s*\{',text))
    if len(frames)!=1:raise ValueError('Unexpected original frame-commit entry')
    close=frames[0].end();depth=1
    while depth and close<len(text):
        depth+=(text[close]=='{')-(text[close]=='}');close+=1
    if depth:raise ValueError('Unterminated original frame-commit entry')
    text=text[:close-1]+' nfs_front_frame_committed();\n'+text[close-1:]
    font_parts=frontend_pixel_parts((ROOT/'src/FrontendFontPixels.inc').read_text())
    for name in ('HdAtlas.h', 'RendererHdAtlas.inc', 'BuiltinTiming.h', 'TextureDigest.h', 'TimingFontPixels.inc', 'HdGlyphSelect.h', 'BuiltinSpeed.h', 'SpeedFontPixels.inc', 'SpeedShader.inc', 'BuiltinSmallFont.h', 'SmallFontPixels.inc', 'SmallFontIdentity.h', 'SmallFontRuntime.inc', 'BuiltinRecordFont.h', 'RecordFontPixels.inc', 'RecordFontIdentity.h', 'RecordFontRuntime.inc', 'SmallGlyphShader.inc', 'LogoPixels.inc', 'PauseFontPixels.inc', 'PauseFontRuntime.inc', 'PanelMaterialRuntime.inc', 'GaugeMaskPixels.inc', 'GearPixels.inc', 'WideLabelPixels.inc', 'LabelCellLayout.inc', 'ClockPunctuationPixels.inc', 'SilverSlashPixels.inc', 'SilverSlashRuntime.inc', 'StartLightPixels.inc', 'StartLightRuntime.inc', 'FrontendWordMode.h', 'FrontendTextContext.h', 'FrontendFontPixels.inc', 'FrontendFontLayout.inc', 'FrontendFontGeometry.inc', 'FrontendFontRuntime.inc', 'FrontendWordLayout.inc',*font_parts):
        shutil.copyfile(ROOT/'src'/name, dest/'Glide2x'/name)
    return text


def prepare_palettes(text, dest):
    def checked(old, new, count=1):
        nonlocal text
        if text.count(old) != count: raise ValueError(f"Unexpected palette pattern: {old[:90]}")
        text = text.replace(old, new)
    checked('void *palette;', 'uint64_t paletteGeneration;')
    checked('static uint32_t *g_palette, g_tmpTexture[256 * 256];',
            'static uint32_t *g_palette, g_tmpTexture[256 * 256];\n#include "RendererPalette.inc"')
    checked('ti->palette = NULL;', 'ti->paletteGeneration = 0;', 2)
    checked('if (type == GR_TEXTABLE_PALETTE)\n\t\tg_palette = (uint32_t *)data;',
            'if (type == GR_TEXTABLE_PALETTE)\n\t\tnfs_glide_palette_table_owned(tmu,type,(uint64_t)(uintptr_t)data,(const GuTexPalette *)data);')
    checked('ti->palette != g_palette', 'ti->paletteGeneration != g_paletteGeneration')
    checked('ti->palette = g_palette;', 'ti->paletteGeneration = g_paletteGeneration;\n\t\t++nfs_palette_uploads;')
    text = text.replace('// Update only when palette or texture changes (let\'s assume every palette has different pointer)',
                        '// Update when exact submitted palette contents or texture data changes.')
    text = text.replace('// When texture changes, palette is NULL', '// Texture uploads/context recreation invalidate the stored generation.')
    shutil.copyfile(ROOT/'src/RendererPalette.inc', dest/'Glide2x/RendererPalette.inc')
    return text


def prepare_surfaces(text, dest):
    def checked(old, new):
        nonlocal text
        if text.count(old) != 1: raise ValueError(f"Unexpected renderer surface pattern: {old[:90]}")
        text = text.replace(old,new)
    checked('static uint32_t g_framebuffer;', 'static uint32_t g_framebuffer;\nstatic GLuint g_frontFramebuffer, g_frontTexture;\nstatic GrBuffer_t g_renderBuffer = GR_BUFFER_BACKBUFFER;')
    checked('static void destroyFrameBuffer()\n{', 'static void destroyFrameBuffer()\n{\n\tif(g_frontFramebuffer)glDeleteFramebuffers(1,&g_frontFramebuffer);\n\tif(g_frontTexture)glDeleteTextures(1,&g_frontTexture);\n\tg_frontFramebuffer=g_frontTexture=0;')
    checked('\tg_framebufferHeight = shorterEdge;', '''
    glGenTextures(1,&g_frontTexture);glBindTexture(GL_TEXTURE_2D,g_frontTexture);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,w,h,0,GL_RGB,GL_UNSIGNED_BYTE,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,(!fixedFramebufferSize || !framebufferLinearFiltering)?GL_NEAREST:GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glGenFramebuffers(1,&g_frontFramebuffer);glBindFramebuffer(GL_FRAMEBUFFER,g_frontFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,g_frontTexture,0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,g_framebufferDepth);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){framebufferError=true;raise(SIGABRT);}
    glBindTexture(GL_TEXTURE_2D,0);
    glBindFramebuffer(GL_FRAMEBUFFER,g_framebuffer);
    g_framebufferHeight = shorterEdge;
    nfs_render_target_changed(w,h);''')
    # Other framebuffer bindings during construction must stay unconditional.
    checked('\t\tglBindFramebuffer(GL_FRAMEBUFFER, g_framebuffer);', '\t\tglBindFramebuffer(GL_FRAMEBUFFER, g_renderBuffer==GR_BUFFER_FRONTBUFFER?g_frontFramebuffer:g_framebuffer);')
    begin=text.index('REALIGN STDCALL BOOL grLfbLock(')
    end=text.index('REALIGN STDCALL void grRenderBuffer(',begin)
    text=text[:begin]+'''#include "RendererSurface.h"
#include "RendererLfbGL.inc"
REALIGN STDCALL BOOL grLfbLock(GrLock_t type, GrBuffer_t buffer, GrLfbWriteMode_t writeMode, GrOriginLocation_t origin, BOOL pixelPipeline, GrLfbInfo_t *info)
{
    const NfsSurfaceOperations operations={lfbRead,lfbWrite};
    return nfs_glide_lfb_lock(type,buffer,writeMode,origin,pixelPipeline,info,operations);
}
REALIGN STDCALL BOOL grLfbUnlock(GrLock_t type, GrBuffer_t buffer)
{
    return nfs_glide_lfb_unlock(type,buffer);
}
'''+text[end:]
    # The manager is needed by teardown, which is above its transfer helpers.
    checked('#include <stdlib.h>', '#include <stdlib.h>\n#include "RendererSurface.h"')
    checked('static void destroyContext()\n{', 'static void destroyContext()\n{\n\tnfs_glide_lfb_reset();\n\tif(!g_glCtx)return;')
    checked('REALIGN STDCALL BOOL grSstQueryHardware(GrHwConfiguration *hwconfig)\n{\n\treturn true;\n}', 'REALIGN STDCALL BOOL grSstQueryHardware(GrHwConfiguration *hwconfig)\n{\n\treturn nfs_glide_query_hardware(hwconfig);\n}')
    checked('// \tfprintf(stderr, "grRenderBuffer\\n");', '''// Rendering and LFB access select the same independent front/back surfaces.
    if(buffer!=GR_BUFFER_FRONTBUFFER && buffer!=GR_BUFFER_BACKBUFFER)return;
    drawTriangles();g_renderBuffer=buffer;useGameProgram(true);''')
    swap=text.index('REALIGN STDCALL void grBufferSwap(')
    end=text.index('REALIGN STDCALL void grColorCombine(',swap)
    body=text[swap:end]
    if body.count('\tdrawTriangles();')!=1:raise ValueError('Unexpected buffer swap flush')
    body=body.replace('\tdrawTriangles();','''\tdrawTriangles();
    const GLuint oldTexture=g_framebufferTexture,oldFramebuffer=g_framebuffer;
    g_framebufferTexture=g_frontTexture;g_framebuffer=g_frontFramebuffer;
    g_frontTexture=oldTexture;g_frontFramebuffer=oldFramebuffer;''')
    body=body.replace('glBindTexture(GL_TEXTURE_2D, g_framebufferTexture);','glBindTexture(GL_TEXTURE_2D, g_frontTexture);')
    begin_presentation=body.index('\tint32_t xOffset = 0')
    end_presentation=body.index('\tSDL_GL_SwapWindow(sdlWin);')+len('\tSDL_GL_SwapWindow(sdlWin);')
    presentation=body[begin_presentation:end_presentation].replace('\tSDL_GL_SwapWindow(sdlWin);','\tconst BOOL presented=SDL_GL_SwapWindow(sdlWin);\n\tif(presented)nfs_frame_presented(sdlWin);\n\treturn presented;')
    helper='#include "FramePresentation.h"\nstatic BOOL presentFrontSurface(void)\n{\n\tuseGameProgram(false);\n\tglUniform1f(g_uGammaLocDisp,g_gammaValue);\n'+presentation+'\n}\n'
    body=body[:begin_presentation]+'\tpresentFrontSurface();'+body[end_presentation:]
    text=text[:swap]+helper+body+text[end:]
    checked('static float g_gammaValue;', 'static float g_gammaValue = 1.0f;')
    shutil.copyfile(ROOT/'scripts/renderer_lfb_gl.inc',dest/'Glide2x/RendererLfbGL.inc')
    return text


def prepare_glide_exports(source, dest):
    declarations = dict(re.findall(r'extern "C" int32_t ((?:gr|gu)\w+)\(([^;]*)\);', (source / "Cpp/DATA.h").read_text()))
    implementations = {}
    for path in [dest / "Glide2x.c", dest / "Glide2x/OpenGL2.c"]:
        for result, name, args in re.findall(r"REALIGN STDCALL (void|BOOL|uint32_t) (\w+)\(([^)]*)\)", path.read_text()):
            if name in declarations: implementations[name] = (result, args)
    if set(implementations) != set(declarations):
        raise ValueError(f"Missing Glide implementations: {set(declarations)-set(implementations)}")
    header = '#pragma once\n#include "Glide2x.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    header += 'void nfs_glide_palette_table_owned(GrChipID_t,GrTexTable_t,uint64_t,const GuTexPalette *);\nuint64_t nfs_glide_palette_upload_count(void);\n'
    bridge = '#include "GuestAccess.h"\n#include "NativeGlide.h"\n#undef true\n#undef false\n#include "MainThreadBridge.h"\n#include "FramePresentation.h"\n#include "FrontendTextContext.h"\n'
    pointer_bridge = bridge
    pointer_exports = {'grDrawTriangle','grTexDownloadMipMap','grTexSource','grLfbLock','grFogTable','guFogGenerateExp','grTexDownloadTable'}
    for name, (result, native_args) in sorted(implementations.items()):
        begin_bridge = len(bridge)
        header += f'{result} native_{name}({native_args});\n'
        bridge += f'extern "C" int32_t {name}({declarations[name]}) {{\n'
        arguments = []
        parameters = [] if native_args == 'void' else native_args.split(',')
        for index, parameter in enumerate(parameters):
            parameter = parameter.strip()
            if '*' in parameter or '[' in parameter:
                typename = re.sub(r"\w+(?:\[[^]]+\])?$", "", parameter).replace('*', '').strip()
                if typename in ('const GrVertex', 'GrTexInfo'):
                    type_name = typename.replace('const ', '')
                    bridge += f' auto value{index} = guest_load<{type_name}>(uint32_t(arg{index}));\n'
                    if type_name == 'GrTexInfo':
                        bridge += f' if (value{index}.largeLod < 0 || value{index}.largeLod > 8) __builtin_trap();\n'
                        bridge += ' if (uint32_t(arg1) >= 0x200000u || (uint32_t(arg1) & 3)) __builtin_trap();\n'
                        if name == 'grTexDownloadMipMap':
                            bridge += f' const size_t size = size_t(256 >> value{index}.largeLod);\n'
                            bridge += f' const size_t bytes = size * size * (value{index}.format == GR_TEXFMT_P_8 ? 1 : 2);\n'
                            bridge += ' if (bytes > 0x200000u - uint32_t(arg1)) __builtin_trap();\n'
                            bridge += f' nfs_host_address(value{index}.data,bytes);\n'
                    arguments.append(f'&value{index}')
                elif typename in ('GrLfbInfo_t', 'GrHwConfiguration'):
                    bridge += f' nfs_host_address(uint32_t(arg{index}),sizeof({typename}));\n {typename} value{index}{{}};\n'
                    if typename == 'GrLfbInfo_t': bridge += f' value{index} = guest_load<GrLfbInfo_t>(uint32_t(arg{index}));\n'
                    arguments.append(f'&value{index}')
                elif typename in ('const GrFog_t', 'GrFog_t'):
                    arguments.append(f'({typename} *)nfs_host_address(uint32_t(arg{index}),GR_FOG_TABLE_SIZE)')
                elif typename == 'void' and name == 'grTexDownloadTable':
                    arguments.append(f'nfs_host_address(uint32_t(arg{index}),arg1 == GR_TEXTABLE_PALETTE ? sizeof(GuTexPalette) : sizeof(GuNccTable))')
                else:
                    raise ValueError(f"Unreviewed Glide pointer parameter: {name}: {parameter}: {typename}")
            else:
                arguments.append(f'arg{index}')
        if name == 'grBufferSwap':
            bridge += ' nfs_wait_frame_slot();\n'
        invocation = f'native_{name}({", ".join(arguments)})'
        operation = f'{invocation}; return 0;' if result == 'void' else f'return static_cast<int32_t>({invocation});'
        if name == 'grDrawTriangle':
            # Text may return/change on the worker before this owned command
            # executes. Snapshot its identity beside the copied vertices.
            bridge += ' const uint64_t text_context = nfs_front_text_context();\n'
            operation = 'nfs::FrontendTextScope text_scope(text_context); ' + operation
        if name == 'grTexDownloadTable':
            # Glide downloads table contents, so the caller may mutate/free its
            # palette after return. Admit the owned snapshot in global FIFO.
            bridge += ' if(arg1==GR_TEXTABLE_PALETTE){auto palette=guest_load<GuTexPalette>(uint32_t(arg2));nfs::renderer_command([=]() mutable -> int32_t {nfs_glide_palette_table_owned(arg0,arg1,uint32_t(arg2),&palette);return 0;});return 0;}\n'
        # Copy scalar state and vertex arguments into owned, bounded commands.
        # Palette storage and texture memory are owner-held. Source selection
        # copies metadata; uploads/readbacks/context operations retain barriers.
        owned = result == 'void' and name not in {
            'grBufferSwap','grGlideInit','grGlideShutdown','grSstWinClose',
            'grSstIdle','grFogTable','grTexDownloadMipMap','grTexDownloadTable',
            'guFogGenerateExp'}
        if owned:
            bridge += f' nfs::renderer_command([=]() mutable -> int32_t {{ {operation} }});\n'
        else:
            bridge += f' const int32_t result = nfs::renderer_operation([&]() -> int32_t {{ {operation} }});\n'
        for index, parameter in enumerate(parameters):
            if 'GrLfbInfo_t *' in parameter or 'GrHwConfiguration *' in parameter:
                bridge += f' guest_store(uint32_t(arg{index}),value{index});\n'
        bridge += ' return 0;\n}\n' if result == 'void' else ' return result;\n}\n'
        if name in pointer_exports: pointer_bridge += bridge[begin_bridge:]
    header += '#ifdef __cplusplus\n}\n#endif\n'
    (dest / 'NativeGlide.h').write_text(header)
    (dest / 'GuestGlide.h').write_text('#pragma once\n#include <stdint.h>\n' + ''.join(
        f'extern "C" int32_t {name}({args});\n' for name, args in sorted(declarations.items())))
    (dest / 'GlideBridge.cpp').write_text(bridge)
    (dest / 'PointerGlideBridge.cpp').write_text(pointer_bridge)
    for path in [dest / "Glide2x.c", dest / "Glide2x/OpenGL2.c"]:
        text = path.read_text()
        for name in declarations: text = re.sub(r'\b' + name + r'\(', 'native_' + name + '(', text)
        path.write_text(text)
    print(f"Prepared {len(implementations)} typed Glide exports")


if __name__ == "__main__":
    prepare_renderer()

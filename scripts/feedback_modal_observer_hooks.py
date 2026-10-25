"""Original-preserving Feedback modal observer renderer hooks for the existing prepared GLES file.

Callbacks observe original native operations. Indexed palette re-expansion
invalidates a Feedback modal observer pass; it is not an authenticated RGB565 tile upload.
"""
# SPDX-License-Identifier: MIT
import re

MARKER = 'NFS_FEEDBACK_MODAL_OBSERVER'
HELPERS = ('FeedbackModalNativeObserver.inc', 'FeedbackModalNativeObserverAbi.h',
           'FeedbackModalNativePass.h', 'AudioNativePass.h')
OTHER_SITES = (
    ('destroyFrameBuffer', ' nfs_logo_native_other();'),
    ('createFrameBuffer', ' nfs_logo_native_other();'),
    ('native_grAlphaBlendFunction', ' nfs_logo_native_state();'),
    ('native_grClipWindow', ' nfs_logo_native_state();'),
    ('native_grBufferClear', ' nfs_logo_native_clear(depth,r,g,b);'),
    ('native_grDepthBufferFunction', ' nfs_logo_native_state();'),
    ('native_grDepthMask', ' nfs_logo_native_state();'),
    ('native_grDrawLine', ' nfs_logo_native_other();'),
    ('native_grFogColorValue', ' nfs_logo_native_state();'),
    ('native_grFogMode', ' nfs_logo_native_state();'),
    ('native_grLfbLock', ' nfs_logo_native_other();'),
    ('native_grLfbUnlock', ' nfs_logo_native_other();'),
    ('native_grRenderBuffer', ' nfs_logo_native_state();'),
    ('native_grTexDownloadTable', ' nfs_logo_native_state();'),
)
_BLOCK = re.compile(r'\r?\n/\* NFS_FEEDBACK_MODAL_OBSERVER_BEGIN:([a-z0-9-]+) \*/\r?\n.*?\r?\n/\* NFS_FEEDBACK_MODAL_OBSERVER_END:\1 \*/\r?\n', re.S)


def reverse_feedback_modal_observer_hooks(text):
    tags = _BLOCK.findall(text)
    if len(tags) != len(set(tags)):
        raise ValueError('Duplicate Feedback modal observer renderer tag')
    result = _BLOCK.sub('', text)
    if MARKER in result:
        raise ValueError('Malformed Feedback modal observer renderer hook')
    return result


remove_feedback_modal_observer_hooks = reverse_feedback_modal_observer_hooks


def _code(text):
    """Mask C comments/literals without changing offsets or line boundaries."""
    pattern = r'/\*.*?\*/|//[^\r\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
    return re.sub(pattern, lambda m: re.sub(r'[^\r\n]', ' ', m.group()), text, flags=re.S)


def _bounds(text, name):
    masked = _code(text)
    # The maintained pipeline exports gr* names after hooks; private prepared
    # files already carry native_gr*. Admit either exact stage, never both.
    symbol = r'(?:native_)?' + re.escape(name[7:]) if name.startswith('native_') else re.escape(name)
    found = list(re.finditer(r'^[^\r\n;{}]*\b' + symbol + r'\([^;{}\r\n]*\)\s*\{', masked, re.M))
    if len(found) != 1:
        raise ValueError('Missing/duplicate Feedback modal observer renderer function: ' + name)
    start = masked.index('{', found[0].start())
    end, depth = start + 1, 1
    while depth and end < len(masked):
        depth += (masked[end] == '{') - (masked[end] == '}')
        end += 1
    if depth:
        raise ValueError('Unbalanced Feedback modal observer renderer function: ' + name)
    return start, end


def add_feedback_modal_observer_hooks(text):
    original = text
    if MARKER in text:
        raise ValueError('Feedback modal observer renderer hooks already present')
    if text.count(' nfs_logo_native_upload_begin(ti);') != 2:
        raise ValueError('Feedback modal observer original upload inventory changed')
    nl = '\r\n' if '\r\n' in text else '\n'
    tags = set()

    def block(tag, code):
        if tag in tags:
            raise ValueError('Duplicate Feedback modal observer renderer insertion tag')
        tags.add(tag)
        return nl + '/* ' + MARKER + '_BEGIN:' + tag + ' */' + nl + code + nl + '/* ' + MARKER + '_END:' + tag + ' */' + nl

    def outside(at):
        before = text[:at]
        if before.count('// checked copy transaction begin') != before.count('// checked copy transaction end'):
            raise ValueError('Feedback modal observer renderer hook inside guest-copy lease')

    def insert(anchor, code, tag, function=None, after=False):
        nonlocal text
        start, end = _bounds(text, function) if function else (0, len(text))
        part = text[start:end]
        if part.count(anchor) != 1:
            raise ValueError('Missing/duplicate Feedback modal observer renderer anchor: ' + tag)
        at = start + part.index(anchor)
        outside(at)
        if after:
            at += len(anchor)
        text = text[:at] + block(tag, code) + text[at:]

    # Pin both upload sites by lexical owner and original operations, never by
    # first global string occurrence or a guessed palette witness completion.
    start, end = _bounds(text, 'uploadTexture')
    upload = text[start:end]
    early = 'if(nfs_hd_upload_texture(ti)){ti->data=originalData;return;}'
    for anchor in ('BOOL newTexture = (ti->id == 0);', early,
                   ' nfs_logo_native_upload_begin(ti);', ' nfs_logo_native_upload_done(ti);'):
        if upload.count(anchor) != 1:
            raise ValueError('Feedback modal observer real uploadTexture body changed')
    final_restore = list(re.finditer(r'^[ \t]*ti->data=originalData;[ \t]*\r?$', upload, re.M))
    if upload.count('ti->data=originalData;') != 2 or len(final_restore) != 1:
        raise ValueError('Feedback modal observer uploadTexture restoration inventory changed')
    if not upload.index(' nfs_logo_native_upload_begin(ti);') < upload.index('BOOL newTexture = (ti->id == 0);') < upload.index(early) < final_restore[0].start() < upload.index(' nfs_logo_native_upload_done(ti);'):
        raise ValueError('Feedback modal observer real uploadTexture order changed')
    start, end = _bounds(text, 'native_grTexSource')
    source = text[start:end]
    palette = 'if (info->format == GR_TEXFMT_P_8 && g_palette && ti->paletteGeneration != g_paletteGeneration)'
    palette_upload = 'glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, g_tmpTexture);'
    for anchor in (palette, palette_upload, 'ti->paletteGeneration = g_paletteGeneration;', ' nfs_logo_native_upload_begin(ti);'):
        if source.count(anchor) != 1:
            raise ValueError('Feedback modal observer indexed palette path changed')
    if not source.index(palette) < source.index(' nfs_logo_native_upload_begin(ti);') < source.index(palette_upload) < source.index('ti->paletteGeneration = g_paletteGeneration;'):
        raise ValueError('Feedback modal observer indexed palette order changed')

    start, end = _bounds(text, 'drawTriangles')
    batch = text[start:end]
    sync = 'nfs_native_mark_gpu_sync(1,g_textureEnabled,nfs_hd_selected,(uintptr_t)g_glCtx,g_shaderProgram,nfs_hd_active_location,&nfs_hd_uniform_sent,g_framebufferHeight);'
    draw = 'glDrawArrays(GL_TRIANGLES, 0, g_trianglesCount * 3);'
    for anchor in ('nfs_hd_sync_uniform();', sync, draw, 'g_trianglesCount = 0;'):
        if batch.count(anchor) != 1:
            raise ValueError('Modal actual batch inventory changed')
    if not batch.index('nfs_hd_sync_uniform();') < batch.index(sync) < batch.index(draw) < batch.index('g_trianglesCount = 0;'):
        raise ValueError('Modal actual batch sync/draw order changed')

    insert('} TextureInfo;', '''static void nfs_feedback_observer_created(void);
static void nfs_feedback_observer_retired(int);
static void nfs_feedback_observer_actual_batch(void);
static void nfs_feedback_observer_before_swap(void);
static void nfs_feedback_observer_other(void);
static void nfs_feedback_observer_upload_begin(TextureInfo*);
static void nfs_feedback_observer_upload_done(TextureInfo*);
static void nfs_feedback_observer_select(unsigned,uint32_t,uint32_t,const GrTexInfo*,TextureInfo*);
static void nfs_feedback_observer_triangle(const GrVertex*,const GrVertex*,const GrVertex*);'''.replace('\n', nl), 'forward', after=True)
    insert(' nfs_logo_native_created();', ' nfs_feedback_observer_created();', 'created', 'createContext', True)
    insert(' nfs_logo_native_retired(g_glCtx && SDL_GL_GetCurrentContext()==g_glCtx);', ' nfs_feedback_observer_retired(g_glCtx && SDL_GL_GetCurrentContext()==g_glCtx);', 'retired', 'destroyContext')
    insert(' nfs_logo_native_retired(0);', ' nfs_feedback_observer_retired(0);', 'abandoned', 'nfs_glide_abandon_unbound')
    for function, anchor in OTHER_SITES:
        insert(anchor, ' nfs_feedback_observer_other();', 'other-' + function.lower().replace('_', '-'), function)
    insert(' if(other!=GR_COMBINE_OTHER_TEXTURE || !g_textureEnabled)nfs_logo_native_state();', ' if(other!=GR_COMBINE_OTHER_TEXTURE || !g_textureEnabled)nfs_feedback_observer_other();', 'conditional-alpha-combine', 'native_grAlphaCombine')
    insert(' nfs_logo_native_upload_begin(ti);', ' nfs_feedback_observer_upload_begin(ti);', 'upload-begin', 'uploadTexture')
    insert(' nfs_logo_native_upload_done(ti);', ' nfs_feedback_observer_upload_done(ti);', 'upload-done', 'uploadTexture')
    insert(' nfs_logo_native_upload_begin(ti);', ' nfs_feedback_observer_other();', 'indexed-palette-invalidates', 'native_grTexSource')
    insert(' nfs_logo_native_select(tmu,startAddress,evenOdd,info,ti);', ' nfs_feedback_observer_select(tmu,startAddress,evenOdd,info,ti);', 'select', 'native_grTexSource')
    insert(' nfs_logo_native_triangle(a,b,c);', ' nfs_feedback_observer_triangle(a,b,c);', 'triangle', 'native_grDrawTriangle')
    insert(draw, ' nfs_feedback_observer_actual_batch();', 'actual-batch-after-sync', 'drawTriangles')
    insert(' nfs_logo_native_other();', ' nfs_feedback_observer_before_swap();', 'finish-before-original-swap', 'native_grBufferSwap')
    insert('#include "hd/BackgroundSceneRenderer.inc"', '#include "hd/FeedbackModalNativeObserver.inc"', 'implementation', after=True)
    if reverse_feedback_modal_observer_hooks(text) != original:
        raise ValueError('Feedback modal observer renderer insertion reversal failed')
    return text
"""Controls renderer insertion/reversal tests; no GL or original game execution."""
# SPDX-License-Identifier: MIT
import importlib.util
from pathlib import Path
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / 'scripts/controls_renderer_hooks.py'
spec = importlib.util.spec_from_file_location('controls_renderer_hooks', SCRIPT)
hooks = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hooks)


def fixture(nl='\n'):
    def fn(name, body):
        return 'void ' + name + '()\n{\n' + body + '\n}\n'
    text = 'typedef struct { unsigned id; } TextureInfo;\n'
    text += '/* Existing Audio/Graphics/Options and native provenance remain. */\n'
    text += fn('createContext', ' nfs_logo_native_created();\n existing_graphics_created();')
    text += fn('destroyContext', ' existing_options_retired();\n nfs_logo_native_retired(g_glCtx && SDL_GL_GetCurrentContext()==g_glCtx);\n SDL_GL_DestroyContext(g_glCtx);')
    text += fn('nfs_glide_abandon_unbound', ' nfs_logo_native_retired(0);\n original_abandon();')
    text += fn('uploadTexture', ' nfs_logo_native_upload_begin(ti);\n BOOL newTexture = (ti->id == 0);\n if(nfs_hd_upload_texture(ti)){ti->data=originalData;return;}\n original_gl_upload();\n ti->data=originalData;\n nfs_logo_native_upload_done(ti);')
    text += fn('native_grTexSource', ''' nfs_logo_native_select(tmu,startAddress,evenOdd,info,ti);
 if (info->format == GR_TEXFMT_P_8 && g_palette && ti->paletteGeneration != g_paletteGeneration)
 {
  original_palette_loop();
 nfs_logo_native_upload_begin(ti);
 glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, g_tmpTexture);
 ti->paletteGeneration = g_paletteGeneration;
 }''')
    text += fn('native_grDrawTriangle', ' nfs_logo_native_triangle(a,b,c);\n original_triangle_conversion();')
    for name, anchor in hooks.OTHER_SITES:
        text += fn(name, anchor + '\n original_operation();')
    text += fn('native_grAlphaCombine', ' if(other!=GR_COMBINE_OTHER_TEXTURE || !g_textureEnabled)nfs_logo_native_state();\n original_combine();')
    text += '#include "hd/BackgroundSceneRenderer.inc"\n'
    return text.replace('\n', nl)


class ControlsRendererHooksTests(unittest.TestCase):
    def test_pre_export_names_preserve_exact_original_calls(self):
        for nl in ('\n', '\r\n'):
            original = fixture(nl).replace('native_gr', 'gr')
            result = hooks.add_controls_renderer_hooks(original)
            self.assertEqual(hooks.reverse_controls_renderer_hooks(result), original)
            self.assertIn('nfs_controls_renderer_upload_done(ti);', result)
            duplicate = original + '\nvoid native_grTexSource() { original(); }\n'
            with self.assertRaises(ValueError):
                hooks.add_controls_renderer_hooks(duplicate)

    def test_exact_lf_and_crlf_reversal_and_existing_calls(self):
        for nl in ('\n', '\r\n'):
            original = fixture(nl)
            result = hooks.add_controls_renderer_hooks(original)
            self.assertEqual(hooks.reverse_controls_renderer_hooks(result), original)
            self.assertEqual(hooks.remove_controls_renderer_hooks(result), original)
            for name in ('existing_graphics_created();', 'existing_options_retired();', 'original_gl_upload();', 'original_palette_loop();', 'original_triangle_conversion();', 'original_combine();', 'SDL_GL_DestroyContext(g_glCtx);'):
                self.assertEqual(result.count(name), original.count(name))
            with self.assertRaises(ValueError):
                hooks.add_controls_renderer_hooks(result)

    def test_real_upload_is_paired_before_original_error_consumers(self):
        result = hooks.add_controls_renderer_hooks(fixture())
        start, end = hooks._bounds(result, 'uploadTexture')
        real = result[start:end]
        self.assertEqual(real.count('nfs_controls_renderer_upload_begin(ti);'), 1)
        self.assertEqual(real.count('nfs_controls_renderer_upload_done(ti);'), 1)
        self.assertLess(real.index('nfs_controls_renderer_upload_begin(ti);'), real.index('nfs_logo_native_upload_begin(ti);'))
        self.assertEqual(real.count('ti->data=originalData;'), 2)
        self.assertIn('if(nfs_hd_upload_texture(ti)){ti->data=originalData;return;}', real)
        self.assertLess(real.rindex('ti->data=originalData;'), real.index('nfs_controls_renderer_upload_done(ti);'))
        self.assertLess(real.index('nfs_controls_renderer_upload_done(ti);'), real.index('nfs_logo_native_upload_done(ti);'))
        start, end = hooks._bounds(result, 'native_grTexSource')
        palette = result[start:end]
        self.assertNotIn('nfs_controls_renderer_upload_begin', palette)
        self.assertNotIn('nfs_controls_renderer_upload_done', palette)
        self.assertLess(palette.index('NFS_CONTROLS_RENDERER_BEGIN:indexed-palette-invalidates'), palette.index('glTexImage2D'))
        self.assertLess(palette.index('nfs_controls_renderer_select('), palette.index('nfs_logo_native_select('))

    def test_context_lifetime_other_operations_and_conditional_guard(self):
        result = hooks.add_controls_renderer_hooks(fixture())
        for function, first, second in (
            ('createContext', 'nfs_logo_native_created();', 'nfs_controls_renderer_created();'),
            ('destroyContext', 'nfs_controls_renderer_retired(g_glCtx', 'nfs_logo_native_retired(g_glCtx'),
            ('nfs_glide_abandon_unbound', 'nfs_controls_renderer_retired(0);', 'nfs_logo_native_retired(0);'),
            ('native_grDrawTriangle', 'nfs_controls_renderer_triangle(', 'nfs_logo_native_triangle(')):
            start, end = hooks._bounds(result, function)
            self.assertLess(result[start:end].index(first), result[start:end].index(second))
        for function, original in hooks.OTHER_SITES:
            start, end = hooks._bounds(result, function)
            self.assertLess(result[start:end].index('nfs_controls_renderer_other();'), result[start:end].index(original))
        self.assertIn('if(other!=GR_COMBINE_OTHER_TEXTURE || !g_textureEnabled)nfs_controls_renderer_other();', result)
        self.assertEqual(result.count('#include "hd/ControlsSceneRenderer.inc"'), 1)

    def test_missing_duplicate_misplaced_and_changed_anchors_refuse(self):
        before = fixture()
        mutations = (
            ('} TextureInfo;', '} ChangedInfo;'),
            (' nfs_logo_native_created();', ''),
            (' nfs_logo_native_triangle(a,b,c);', ' nfs_logo_native_triangle(a,b,c);' * 2),
            (' nfs_logo_native_upload_begin(ti);', ''),
            ('BOOL newTexture = (ti->id == 0);', 'BOOL newTexture = true;'),
            ('ti->data=originalData;', ''),
            ('if(nfs_hd_upload_texture(ti)){ti->data=originalData;return;}', 'if(nfs_hd_upload_texture(ti)){return;}'),
            (' original_gl_upload();', ' original_gl_upload(); ti->data=originalData;'),
            (' ti->data=originalData;\n nfs_logo_native_upload_done(ti);', ' nfs_logo_native_upload_done(ti);'),
            ('ti->paletteGeneration = g_paletteGeneration;', ''),
            ('if (info->format == GR_TEXFMT_P_8', 'if (info->format == GR_TEXFMT_RGB_565'),
            ('void native_grTexDownloadTable()', 'void changed()'),
            ('#include "hd/BackgroundSceneRenderer.inc"', ''),
        )
        for old, new in mutations:
            with self.subTest(anchor=old), self.assertRaises(ValueError):
                hooks.add_controls_renderer_hooks(before.replace(old, new, 1))
        with self.assertRaises(ValueError):
            hooks.add_controls_renderer_hooks(before + ' nfs_logo_native_upload_begin(ti);')
        # Swapping ownership of the two text-identical upload begins is refused.
        misowned = before.replace(' nfs_logo_native_upload_begin(ti);', '', 1)
        misowned = misowned.replace('original_palette_loop();', 'original_palette_loop();\n nfs_logo_native_upload_begin(ti);')
        with self.assertRaises(ValueError):
            hooks.add_controls_renderer_hooks(misowned)

    def test_comments_strings_braces_leases_and_malformed_blocks(self):
        before = fixture().replace('original_gl_upload();', 'original_gl_upload(); /* } { */\n const char*label="} {";')
        self.assertEqual(hooks.reverse_controls_renderer_hooks(hooks.add_controls_renderer_hooks(before)), before)
        leased = fixture().replace(' nfs_logo_native_created();', '// checked copy transaction begin\n nfs_logo_native_created();\n// checked copy transaction end')
        with self.assertRaises(ValueError):
            hooks.add_controls_renderer_hooks(leased)
        after = hooks.add_controls_renderer_hooks(fixture())
        for bad in (after + after, after.replace('_END:forward', '_END:broken', 1)):
            with self.assertRaises(ValueError):
                hooks.reverse_controls_renderer_hooks(bad)


if __name__ == '__main__':
    unittest.main()
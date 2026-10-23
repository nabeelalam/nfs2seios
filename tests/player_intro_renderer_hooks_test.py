"""Original-preserving normal Player intro renderer hook tests."""
import importlib.util
from pathlib import Path
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / "scripts/player_intro_renderer_hooks.py"
spec = importlib.util.spec_from_file_location("player_intro_renderer_hooks", SCRIPT)
hooks = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hooks)


def fixture(prefix="native_"):
    def fn(name, code=""):
        return "void " + name + "()\n{\n" + code + "\n}\n"
    s = "typedef struct { unsigned id; } TextureInfo;\n"
    for name, body in (("createContext", " nfs_logo_native_created();"), ("destroyContext", " nfs_logo_native_retired(g_glCtx && SDL_GL_GetCurrentContext()==g_glCtx);"), ("abandon", " nfs_logo_native_retired(0);"), ("uploadTexture", " nfs_logo_native_upload_begin(ti);")):
        s += fn(name, body)
    for name in ("destroyFrameBuffer", "createFrameBuffer", "drawTriangles", "presentFrontSurface"):
        body = ""
        if name == "drawTriangles":
            body = "\tglDrawArrays(GL_TRIANGLES, 0, g_trianglesCount * 3);\n nfs_native_mark_gpu_after_draw();"
        if name == "presentFrontSurface":
            body = "\tglBindTexture(GL_TEXTURE_2D, g_frontTexture);\nconst BOOL presented=SDL_GL_SwapWindow(sdlWin);"
        s += fn(name, body)
    for name in ("grDrawLine", "grLfbLock", "grLfbUnlock", "grBufferClear", "grAlphaBlendFunction", "grClipWindow", "grDepthBufferFunction", "grDepthMask", "grFogColorValue", "grFogMode", "grRenderBuffer", "grTexDownloadTable", "grAlphaCombine", "grBufferSwap", "grTexSource", "grDrawTriangle"):
        body = ""
        if name == "grTexSource":
            body = " nfs_logo_native_select(tmu,startAddress,evenOdd,info,ti);\n nfs_logo_native_upload_begin(ti);"
        if name == "grDrawTriangle":
            body = " nfs_logo_native_triangle(a,b,c);"
        s += fn(prefix + name, body)
    return s


class IntroRendererHooksTests(unittest.TestCase):
    def test_exact_reversal_before_and_after_export_renaming(self):
        for prefix in ("", "native_"):
            with self.subTest(prefix=prefix):
                before = fixture(prefix)
                after = hooks.add_player_intro_renderer_hooks(before)
                self.assertEqual(hooks.reverse_player_intro_renderer_hooks(after), before)
                with self.assertRaises(ValueError):
                    hooks.add_player_intro_renderer_hooks(after)

    def test_tail_mirror_before_uniform_reset_and_display_only_selection(self):
        after = hooks.add_player_intro_renderer_hooks(fixture())
        self.assertLess(after.index("glDrawArrays(GL_TRIANGLES"), after.index("nfs_player_intro_renderer_tail_batch(g_trianglesCount)"))
        self.assertLess(after.index("nfs_player_intro_renderer_tail_batch(g_trianglesCount)"), after.index("nfs_native_mark_gpu_after_draw"))
        self.assertEqual(after.count("nfs_player_intro_renderer_presentation_texture(g_frontFramebuffer,g_frontTexture)"), 1)
        self.assertNotIn("glCopyTexSubImage", after)
        swap = after[after.index("void native_grBufferSwap()"):after.index("void native_grTexSource()")]
        self.assertNotIn("nfs_player_intro_renderer_other", swap)
        self.assertNotIn("nfs_player_intro_renderer_state", swap)
        self.assertEqual(after.count("nfs_player_intro_renderer_upload_begin(ti);"), 2)
        self.assertLess(after.index("nfs_player_intro_renderer_retired(g_glCtx"), after.index("nfs_logo_native_retired(g_glCtx"))
        self.assertLess(after.index("nfs_player_intro_renderer_retired(0);"), after.index("nfs_logo_native_retired(0);"))
        self.assertLess(after.index("SDL_GL_SwapWindow(sdlWin)"), after.index("nfs_player_intro_renderer_after_present();"))

    def test_missing_repeated_or_changed_anchors_rejected(self):
        before = fixture()
        for old, new in (("} TextureInfo;", "} Changed;"), (" nfs_logo_native_created();", ""), (" nfs_logo_native_upload_begin(ti);", ""), ("void native_grBufferClear()", "void other()"), ("glBindTexture(GL_TEXTURE_2D, g_frontTexture)", "glBindTexture(GL_TEXTURE_2D, g_framebufferTexture)"), ("const BOOL presented=SDL_GL_SwapWindow(sdlWin);", "SDL_GL_SwapWindow(sdlWin);"), (" nfs_logo_native_triangle(a,b,c);", " nfs_logo_native_triangle(a,b,c);" * 2)):
            with self.subTest(anchor=old):
                with self.assertRaises(ValueError):
                    hooks.add_player_intro_renderer_hooks(before.replace(old,new,1))

    def test_malformed_blocks_rejected(self):
        after = hooks.add_player_intro_renderer_hooks(fixture())
        with self.assertRaises(ValueError):
            hooks.reverse_player_intro_renderer_hooks(after.replace("_END:forward", "_END:broken",1))
        with self.assertRaises(ValueError):
            hooks.reverse_player_intro_renderer_hooks(after+after)


if __name__ == "__main__":
    unittest.main()

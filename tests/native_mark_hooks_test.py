# SPDX-License-Identifier: MIT
"""Synthetic renderer scaffolds verify additive hook placement and drift rejection."""
import re
import sys
import tempfile
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"scripts"))
from prepare_renderer import apply_native_mark_hooks,copy_native_mark_helpers


def scaffold():
    return """typedef struct { unsigned id; } TextureInfo;
const char g_fShaderSrc[] =
"uniform sampler2D uTextureSampler;"
"vec4 texture = uTextureEnabled * sampled;";
const char g_vShaderDispSrc[] = "display";
static void createContext()
{
 originalCreate();
}
static void destroyContext()
{
 originalDestroy();
}
static int recreateContext()
{
 if(SDL_GL_GetCurrentContext()!=NULL)return false;
 originalRestore();
}
static void uploadTexture(TextureInfo *ti)
{
 originalUpload(ti);
}
void native_grTexDownloadMipMap()
{
\tdrawTriangles();
 originalConvertAndUpload();
}
void native_grTexSource()
{
\tdrawTriangles();
\tglBindTexture(GL_TEXTURE_2D, ti->id);
 originalSelection();
}
static void drawTriangles()
{
 originalVerticesAndState();
 nfs_hd_sync_uniform();
\tglDrawArrays(GL_TRIANGLES, 0, g_trianglesCount * 3);
 originalAfterDraw();
}
void native_grDrawLine()
{
 nfs_hd_sync_uniform();
 originalLine();
}
static void useGameProgram()
{
 originalProgram();
}
"""


def reverse(text):
    return re.sub(r"\n/\* NFS_HD_MARK_BEGIN:([^*]+) \*/\n.*?\n/\* NFS_HD_MARK_END:\1 \*/\n","",text,flags=re.S)


class NativeMarkHooks(unittest.TestCase):
    def test_all_original_draw_upload_and_state_bytes_unchanged(self):
        original=scaffold();prepared=apply_native_mark_hooks(original)
        self.assertEqual(reverse(prepared),original)
        self.assertEqual(prepared.count("originalVerticesAndState();"),1)
        self.assertEqual(prepared.count("glDrawArrays(GL_TRIANGLES"),1)
        self.assertEqual(prepared.count("NFS_HD_MARK_BEGIN:"),15)
        self.assertLess(prepared.index("originalUpload(ti);"),prepared.index("nfs_native_mark_gpu_uploaded("))
        self.assertLess(prepared.index("nfs_native_mark_gpu_invalidate("),prepared.index("originalUpload(ti);"))
    def test_second_preparation_and_anchor_drift_rejected(self):
        with self.assertRaises(ValueError):apply_native_mark_hooks(apply_native_mark_hooks(scaffold()))
        for old,new in (("originalUpload(ti);\n}","originalUpload(ti);\n"),("void native_grTexSource()","void unknownTextureSource()"),("nfs_hd_sync_uniform();","unknownUniform();")):
            with self.subTest(old=old),self.assertRaises(ValueError):apply_native_mark_hooks(scaffold().replace(old,new))
    def test_helpers_include_shared_profile_and_are_byte_exact(self):
        root=Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory() as tmp:
            dest=Path(tmp);copy_native_mark_helpers(dest)
            for name in ("NativeMarkGpu.h","NativeMarkGpu.c.inc","NativeMarkArt.h","NativeMarkProfile.h","NativeMarkProvider.h"):
                self.assertEqual((dest/"hd"/name).read_bytes(),(root/"src/hd"/name).read_bytes())


if __name__=="__main__":unittest.main()

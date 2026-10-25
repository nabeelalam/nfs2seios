"""Source-only modal observer hook tests; no GL/game execution or compilation."""
# SPDX-License-Identifier: MIT
import importlib.util
from pathlib import Path
import unittest
ROOT = Path(__file__).resolve().parents[1]
def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module
hooks = load('feedback_modal_observer_hooks', ROOT/'scripts/feedback_modal_observer_hooks.py')
existing = load('existing_controls_fixture', ROOT/'tests/controls_renderer_hooks_test.py')

def fixture(nl='\n'):
    base = existing.fixture()
    base += """
static inline void drawTriangles()
{
 if (g_trianglesCount == 0) return;
 nfs_hd_sync_uniform();
 nfs_native_mark_gpu_sync(1,g_textureEnabled,nfs_hd_selected,(uintptr_t)g_glCtx,g_shaderProgram,nfs_hd_active_location,&nfs_hd_uniform_sent,g_framebufferHeight);
 glDrawArrays(GL_TRIANGLES, 0, g_trianglesCount * 3);
 nfs_native_mark_gpu_after_draw();
 g_trianglesCount = 0;
}
"""
    return base.replace('\n', nl)

class FeedbackModalObserverHooksTests(unittest.TestCase):
    def test_exact_reverse_and_no_original_suppression(self):
        for nl in ('\n', '\r\n'):
            original = fixture(nl)
            transformed = hooks.add_feedback_modal_observer_hooks(original)
            self.assertEqual(hooks.reverse_feedback_modal_observer_hooks(transformed), original)
            self.assertIn('#include "hd/FeedbackModalNativeObserver.inc"', transformed)
            self.assertEqual(transformed.count('glDrawArrays(GL_TRIANGLES, 0, g_trianglesCount * 3);'), 1)
            with self.assertRaises(ValueError):
                hooks.add_feedback_modal_observer_hooks(transformed)
    def test_actual_batch_after_sync_and_before_original_draw(self):
        text = hooks.add_feedback_modal_observer_hooks(fixture())
        a,b = hooks._bounds(text, 'drawTriangles')
        batch = text[a:b]
        self.assertLess(batch.index('nfs_hd_sync_uniform();'), batch.index('nfs_feedback_observer_actual_batch();'))
        self.assertLess(batch.index('nfs_native_mark_gpu_sync('), batch.index('nfs_feedback_observer_actual_batch();'))
        self.assertLess(batch.index('nfs_feedback_observer_actual_batch();'), batch.index('glDrawArrays('))
        a,b = hooks._bounds(text, 'native_grBufferSwap')
        self.assertLess(text[a:b].index('nfs_feedback_observer_before_swap();'), text[a:b].index('nfs_logo_native_other();'))
    def test_preexport_names_and_unknown_batch_refused(self):
        original = fixture().replace('native_gr', 'gr')
        self.assertEqual(hooks.reverse_feedback_modal_observer_hooks(hooks.add_feedback_modal_observer_hooks(original)), original)
        for old,new in [('nfs_hd_sync_uniform();',''),('g_trianglesCount * 3','g_trianglesCount * 6'),('nfs_native_mark_gpu_sync(1,','nfs_native_mark_gpu_sync(0,')]:
            with self.assertRaises(ValueError):
                hooks.add_feedback_modal_observer_hooks(fixture().replace(old,new))
    def test_malformed_duplicate_and_missing_anchor_refused(self):
        text = hooks.add_feedback_modal_observer_hooks(fixture())
        with self.assertRaises(ValueError):
            hooks.reverse_feedback_modal_observer_hooks(text.replace('NFS_FEEDBACK_MODAL_OBSERVER_END:forward','NFS_FEEDBACK_MODAL_OBSERVER_END:wrong'))
        with self.assertRaises(ValueError):
            hooks.add_feedback_modal_observer_hooks(fixture().replace(' nfs_logo_native_upload_done(ti);',''))
    def test_adapter_owns_source_and_reads_actual_draw_state(self):
        source = (ROOT/'src/hd/FeedbackModalNativeObserver.inc').read_text()
        for required in ('glGetUniformfv(', 'glGetUniformiv', 'glGetVertexAttribPointerv(', 'g_vertices[t].vertex[v].x', 'g_textureCoord[t].vertex[v].s', 'g_colorValues[t].vertex[v].a', 'nfs_feedback_native_actual_batch(', 'releaseOwner', 'nfs_feedback_observer_read_target'):
            self.assertIn(required, source)
        self.assertNotIn('nfs_controls_',source)
        self.assertNotIn('glDrawArrays(',source)
        self.assertNotIn('glUniform',source)

if __name__ == '__main__':
    unittest.main()

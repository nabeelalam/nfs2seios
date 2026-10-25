"""Structural renderer transform checks; no actual GL or Game execution."""
import importlib.util
from pathlib import Path
import sys,unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
import feedback_modal_scene_renderer_hooks as scene
import feedback_modal_observer_hooks as observer
import controls_renderer_hooks as controls
def load(path):
 spec=importlib.util.spec_from_file_location('observer_fixture',path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
fixture=load(ROOT/'tests/feedback_modal_observer_hooks_test.py').fixture
def prepared(nl='\n'):
 return observer.add_feedback_modal_observer_hooks(controls.add_controls_renderer_hooks(fixture(nl)))
class SceneHooksTests(unittest.TestCase):
 def test_exact_reversal_and_order(self):
  for nl in ('\n','\r\n'):
   original=prepared(nl);out=scene.add_feedback_modal_scene_renderer_hooks(original)
   self.assertEqual(scene.reverse_feedback_modal_scene_renderer_hooks(out),original)
   self.assertLess(out.index('#include "hd/FeedbackModalNativeObserver.inc"'),out.index('#include "hd/ControlsSceneRenderer.inc"'))
   self.assertLess(out.index('#include "hd/ControlsSceneRenderer.inc"'),out.index('#include "hd/FeedbackModalSceneRenderer.inc"'))
   self.assertLess(out.index(' nfs_feedback_scene_retired();'),out.index(' nfs_feedback_observer_retired(g_glCtx'))
   self.assertLess(out.index(' nfs_feedback_scene_forget();'),out.index(' nfs_feedback_observer_retired(0);'))
 def test_reject_duplicate_and_missing(self):
  p=prepared();out=scene.add_feedback_modal_scene_renderer_hooks(p)
  with self.assertRaises(ValueError):scene.add_feedback_modal_scene_renderer_hooks(out)
  for anchor in (' nfs_feedback_observer_retired(0);','#include "hd/ControlsSceneRenderer.inc"','#include "hd/FeedbackModalNativeObserver.inc"'):
   with self.assertRaises(ValueError):scene.add_feedback_modal_scene_renderer_hooks(p.replace(anchor,''))
 def test_reject_lease_and_malformed(self):
  p=prepared().replace(' nfs_feedback_observer_retired(0);','// checked copy transaction begin\n nfs_feedback_observer_retired(0);\n// checked copy transaction end')
  with self.assertRaises(ValueError):scene.add_feedback_modal_scene_renderer_hooks(p)
  with self.assertRaises(ValueError):scene.reverse_feedback_modal_scene_renderer_hooks('/* NFS_FEEDBACK_MODAL_SCENE_RENDERER_BEGIN:broken */')
if __name__=='__main__':unittest.main()

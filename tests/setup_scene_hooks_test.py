# SPDX-License-Identifier: MIT
"""Setup hooks remain additive over the already maintained Options source."""
import re,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from options_scene_hooks_test import original as original_before_options
from options_scene_hooks import add_options_scene_hooks
from setup_scene_hooks import add_setup_scene_hooks,NAMES

def original(name):return add_options_scene_hooks(original_before_options(name),name)
def reverse(t):return re.sub(r'\n// NFS_SETUP_SCENE_BEGIN:([^\n]+)\n.*?// NFS_SETUP_SCENE_END:\1\n','',t,flags=re.S)
class SetupHooks(unittest.TestCase):
 def test_all_original_bytes_reverse_and_double_apply_rejected(self):
  for name in NAMES:
   a=original(name);b=add_setup_scene_hooks(a,name);self.assertEqual(reverse(b),a)
   with self.assertRaises(ValueError):add_setup_scene_hooks(b,name)
 def test_scope_order_and_single_original_route_arm(self):
  a=original('Methods_01.cpp');b=add_setup_scene_hooks(a,'Methods_01.cpp');self.assertEqual(b.count('NativePresentationScope'),a.count('NativePresentationScope'))
  for before,after in [('nfs_logo_publisher::SetupMenuOwnerScope','nfs_setup_caption::OwnerScope'),('nfs_setup_caption::OwnerScope','nfs_setup_scene_live::SetupScope'),('nfs_setup_scene_live::TailScope','nfs_setup_caption::TailScope')]:self.assertLess(b.index(before),b.index(after))
  part=b[b.index('Fn(void) Game::_sub_40C160()'):];part=part[part.index('nfs_logo_publisher::NativePresentationScope'):];self.assertLess(part.index('nfs_setup_scene_live::begin'),part.index('_sub_427B30'));self.assertLess(part.index('_sub_427B30'),part.index('nfs_setup_scene_live::finish'))
 def test_exact_lazy_gate_measurement_and_source_store_observers(self):
  b=add_setup_scene_hooks(original('Methods_01.cpp'),'Methods_01.cpp');self.assertLess(b.index('loc_40BC20:'),b.index('nfs_setup_iteration.start()'));self.assertEqual(b.count('nfs_setup_caption::measured'),2)
  for name,count in [('Methods_13.cpp',4),('Methods_14.cpp',1)]:self.assertEqual(add_setup_scene_hooks(original(name),name).count('nfs_setup_caption::WordCapture'),count)
 def test_missing_source_anchors_fail_closed(self):
  for anchor in ['loc_40C3CC:','loc_40C5C6:','guest_address(aBlk4)','nfs_audio_sidebar_text::SourceMeasureAlignment::right','nfs_hd_scene_iteration.start();']:
   with self.assertRaises(ValueError):add_setup_scene_hooks(original('Methods_01.cpp').replace(anchor,'changed',1),'Methods_01.cpp')
 def test_native_cache_scope_order_and_unrelated_units(self):
  b=add_setup_scene_hooks(original('Methods_03.cpp'),'Methods_03.cpp');self.assertLess(b.index('nfs_setup_scene_live::UploadEndScope'),b.index('nfs_logo_publisher::UploadScope'));self.assertEqual(add_setup_scene_hooks('unrelated','Methods_15.cpp'),'unrelated')
 def test_fixed_independent_policy_bindings(self):
  root=Path(__file__).resolve().parents[1];live=(root/'src/hd/SetupSceneLiveGameHooks.inc').read_text();prepare=(root/'src/hd/SetupScenePrepare.h').read_text();queue=(root/'src/hd/SetupSceneQueueAdapter.inc').read_text();cap=(root/'src/hd/SetupCaptionGameHooks.inc').read_text()
  self.assertIn('NFS_CACHED_MENU_DESCRIPTOR off_4D2790',live);self.assertIn('NFS_CACHED_MENU_FORM_VALUE 0x4D2790',prepare);self.assertIn('NFS_CACHED_MENU_CAPTION_NAMESPACE nfs_setup_caption',live);self.assertIn('selector=0,resource=0',cap);self.assertIn('measureWidth=75,textLeft=93,textY=126',cap);self.assertIn('nfs_background_scene_live::nextRequest()',queue);self.assertIn('nfs_private_setup_scene_begin',queue);self.assertNotIn('preview',live.lower());self.assertNotIn('preview',queue.lower())
if __name__=='__main__':unittest.main()

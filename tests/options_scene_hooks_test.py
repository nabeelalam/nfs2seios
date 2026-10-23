# SPDX-License-Identifier: MIT
"""Synthetic source tests for exact Options ownership and reversible original calls."""
import re,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from options_scene_hooks import add_options_scene_hooks,NAMES
from location_scene_hooks_test import original as prior,fn,call
from location_scene_hooks import add_location_scene_hooks,NAMES as LOC_NAMES
STORE='guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov'
def original(name):
 if name in LOC_NAMES:
  t=prior(name)
  if name=='Methods_01.cpp':
   for symbol,body in [('405CF0',''),('405580',call('41B710')+''.join(' nfs_audio_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_audio_sidebar_text::SourceMeasureAlignment::'+k+');\n'for k in('right','center')))]:
    assert 'Fn(void) Game::_sub_'+symbol+'()'not in t;t+=fn(symbol,body)
  if name=='Methods_13.cpp':t+=fn('4A5830',(STORE+'\n')*4)
  return add_location_scene_hooks(t,name)
 t='#include "FunctionIds.h"\n'
 if name=='Methods_09.cpp':return t+fn('4824CC')
 return t+fn('4AC934')+fn('4AC9C8')+fn('4B3EE0',STORE+'\n')
def reverse(t):return re.sub(r'\n// NFS_OPTIONS_SCENE_BEGIN:([^\n]+)\n.*?// NFS_OPTIONS_SCENE_END:\1\n','',t,flags=re.S)
class OptionsHooks(unittest.TestCase):
 def test_all_original_bytes_reverse_and_double_apply_rejected(self):
  for name in NAMES:
   a=original(name);b=add_options_scene_hooks(a,name);self.assertEqual(reverse(b),a);self.assertEqual(add_options_scene_hooks(a,name),b)
   with self.assertRaises(ValueError):add_options_scene_hooks(b,name)
 def test_scope_order_and_single_original_route_arm(self):
  a=original('Methods_01.cpp');b=add_options_scene_hooks(a,'Methods_01.cpp')
  self.assertEqual(b.count('NativePresentationScope'),a.count('NativePresentationScope'))
  for before,after in [('nfs_logo_publisher::OptionsMenuOwnerScope','nfs_options_caption::OwnerScope'),('nfs_options_caption::OwnerScope','nfs_options_scene_live::OptionsScope'),('nfs_options_scene_live::TailScope','nfs_options_caption::TailScope')]:self.assertLess(b.index(before),b.index(after))
  start=b.index('Fn(void) Game::_sub_40C160()');part=b[start:];part=part[part.index('nfs_logo_publisher::NativePresentationScope'):];self.assertLess(part.index('nfs_options_scene_live::begin'),part.index('_sub_427B30'));self.assertLess(part.index('_sub_427B30'),part.index('nfs_options_scene_live::finish'))
 def test_exact_lazy_gate_measurement_and_source_store_observers(self):
  b=add_options_scene_hooks(original('Methods_01.cpp'),'Methods_01.cpp');self.assertLess(b.index('loc_40BC20:'),b.index('nfs_options_iteration.start()'));self.assertEqual(b.count('nfs_options_caption::measured'),2)
  for name,count in [('Methods_13.cpp',4),('Methods_14.cpp',1)]:self.assertEqual(add_options_scene_hooks(original(name),name).count('nfs_options_caption::WordCapture'),count)
 def test_missing_source_anchors_fail_closed(self):
  for anchor in ['loc_40C3CC:','loc_40C5C6:','guest_address(aBlk4)','nfs_audio_sidebar_text::SourceMeasureAlignment::right','nfs_hd_scene_iteration.start();']:
   with self.assertRaises(ValueError):add_options_scene_hooks(original('Methods_01.cpp').replace(anchor,'changed',1),'Methods_01.cpp')
 def test_native_cache_scope_order_and_unrelated_units(self):
  b=add_options_scene_hooks(original('Methods_03.cpp'),'Methods_03.cpp');self.assertLess(b.index('nfs_options_scene_live::UploadEndScope'),b.index('nfs_logo_publisher::UploadScope'));self.assertEqual(add_options_scene_hooks('unrelated','Methods_15.cpp'),'unrelated')
 def test_exact_owner_caption_and_no_preview(self):
  root=Path(__file__).resolve().parents[1];cap=(root/'src/hd/OptionsCaptionGameHooks.inc').read_text();live=(root/'src/hd/OptionsSceneLiveGameHooks.inc').read_text();queue=(root/'src/hd/OptionsSceneQueueAdapter.inc').read_text()
  self.assertIn('guest_address(off_4D3A88)',cap);self.assertNotIn('off_4D38D8',cap);self.assertIn('selector=6,resource=7',cap);self.assertIn('measureWidth=48,textLeft=120,textY=364',cap);core=(root/'src/hd/CachedMenuCaptionGameHooks.inc').read_text();self.assertIn('width!=CaptionPolicy::measureWidth',core);self.assertIn('g.edx!=CaptionPolicy::textLeft',core);self.assertIn('g.ebx!=CaptionPolicy::textY',core);self.assertIn('s->measureAnchor!=168',core);self.assertNotIn('preview',live.lower());self.assertNotIn('preview',queue.lower());self.assertIn('nfs_background_scene_live::nextRequest()',queue)
if __name__=='__main__':unittest.main()

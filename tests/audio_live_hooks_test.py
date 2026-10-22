# SPDX-License-Identifier: MIT
import pathlib,re,sys,unittest
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[1]/"scripts"))
from audio_live_hooks import add_audio_live_hooks,NAMES
HEADER='#include "FunctionIds.h"\n'
def fn(s,b=''):return 'Fn(void) Game::_sub_'+s+'()\n{\n'+b+'\n}\n'
def fixtures():
 normal='loc_40A8C8:\n\tesp -= 4; _sub_40EB20(); esp += 4; //call\n\tesp -= 4; _sub_428A20(); esp += 4; //call\n eax = to32i(esi+4);\n edx = 0x400;\n\tesp -= 4; _sub_405B30(); esp += 4; //call\n eax = to32i(dword_4D2314+2);\n eax = to32i(dword_4D231C+eax*4);\n eax = to32i(eax+0x20);\n edx = 0x1000;\n\tesp -= 4; _sub_405B60(); esp += 4; //call\nloc_40AB1D:\n eax = esi;\n\tesp -= 4; _sub_40FA90(); esp += 4; //call\n'
 store='guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov\n'
 return {'Methods_01.cpp':HEADER+fn('40A570',normal)+''.join(fn(s)for s in ['40C110','405B30','405B60','405CF0']),'Methods_09.cpp':HEADER+fn('4824CC'),'Methods_10.cpp':HEADER+''.join(fn(s)for s in ['482DA0','482BD4','483438','486BC8']),'Methods_13.cpp':HEADER+fn('4A5830',store*4),'Methods_14.cpp':HEADER+''.join(fn(s)for s in ['4AC934','4AC9C8'])+fn('4B3EE0',store)}
def reverse(t):return re.sub(r'\n// NFS_AUDIO_LIVE_BEGIN:([^\n]+)\n.*?\n// NFS_AUDIO_LIVE_END:\1\n','',t,flags=re.S)
class Tests(unittest.TestCase):
 def test_all_units_reverse_exactly(self):
  for name,source in fixtures().items():
   out=add_audio_live_hooks(source,name);self.assertEqual(reverse(out),source);self.assertEqual(out.count('guest_store<'),source.count('guest_store<'));self.assertIn('hd/AudioSidebarCaptionGameHooks.inc',out)
 def test_order_and_original_calls(self):
  source=fixtures()['Methods_01.cpp'];out=add_audio_live_hooks(source,'Methods_01.cpp');self.assertLess(out.index('nfs_audio_text::OwnerScope'),out.index('nfs_audio_live::OwnerScope'));self.assertLess(out.index('nfs_audio_live::TailScope'),out.index('nfs_audio_text::TailScope'));self.assertLess(out.index('nfs_audio_text::StringScope'),out.index('nfs_audio_live::bindText'));self.assertNotIn('Game::_sub_40BBE0',out)
 def test_missing_normal_source_is_rejected(self):
  source=fixtures()['Methods_01.cpp'];
  for old in ['edx = 0x400;','eax = to32i(esi+4);','dword_4D231C+eax*4']:
   with self.assertRaises(ValueError):add_audio_live_hooks(source.replace(old,'changed',1),'Methods_01.cpp')
 def test_callback_and_retire_routes(self):
  out=add_audio_live_hooks(fixtures()['Methods_10.cpp'],'Methods_10.cpp');self.assertEqual(out.count('nfs_audio_live::RowScope'),3);self.assertIn('nfs_audio_text::beforeDescriptorRetire',out);self.assertIn('nfs_audio_live::afterRetire',out)
 def test_words_and_duplicate_rejected(self):
  for name in ['Methods_13.cpp','Methods_14.cpp']:
   source=fixtures()[name];out=add_audio_live_hooks(source,name);self.assertEqual(out.count('nfs_audio_text::WordCapture'),4 if name.endswith('13.cpp') else 1);self.assertEqual(out.count('nfs_audio_sidebar_text::WordCapture'),4 if name.endswith('13.cpp') else 1)
   with self.assertRaises(ValueError):add_audio_live_hooks(out,name)
   with self.assertRaises(ValueError):add_audio_live_hooks(source.replace('guest_store<int16_t>','altered_store',1),name)
 def test_unrelated_unchanged(self):self.assertEqual(add_audio_live_hooks('unchanged','Methods_02.cpp'),'unchanged')
if __name__=='__main__':unittest.main()

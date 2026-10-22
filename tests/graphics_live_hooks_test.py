# SPDX-License-Identifier: MIT
import pathlib,re,sys,unittest
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[1]/"scripts"))
from graphics_live_hooks import add_graphics_live_hooks,NAMES
HEADER='#include "FunctionIds.h"\n'
def fn(s,b=''):return 'Fn(void) Game::_sub_'+s+'()\n{\n'+b+'\n}\n'
def fixtures():
 normal='loc_41020E:\n\tesp -= 4; _sub_40EB20(); esp += 4; //call\n\tesp -= 4; _sub_428A20(); esp += 4; //call\n eax = to32i(esi+4);\n edx = 0x400;\n\tesp -= 4; _sub_405B30(); esp += 4; //call\n eax = to32i(dword_4D2314+2);\n eax = to32i(dword_4D231C+eax*4);\n eax = to32i(eax+0x20);\n edx = 0x1000;\n\tesp -= 4; _sub_405B60(); esp += 4; //call\n eax = esi;\n\tesp -= 4; _sub_40FA90(); esp += 4; //call\n'
 store='guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov\n'
 return {'Methods_01.cpp':HEADER+fn('4101C0',normal)+''.join(fn(s)for s in ['405B60','405CF0','4054E0'])+fn('405580','\tesp -= 4; _sub_41B690(); esp += 4; //call'),'Methods_09.cpp':HEADER+fn('4824CC'),'Methods_10.cpp':HEADER+''.join(fn(s)for s in ['482DA0','482BD4','483438','486BC8']),'Methods_13.cpp':HEADER+fn('4A5830',store*4),'Methods_14.cpp':HEADER+''.join(fn(s)for s in ['4AC934','4AC9C8'])+fn('4B3EE0',store)}
def reverse(t):return re.sub(r'\n// NFS_GRAPHICS_LIVE_BEGIN:([^\n]+)\n.*?\n// NFS_GRAPHICS_LIVE_END:\1\n','',t,flags=re.S)
class Tests(unittest.TestCase):
 def test_all_units_reverse_exactly(self):
  for name,source in fixtures().items():
   out=add_graphics_live_hooks(source,name);self.assertEqual(reverse(out),source);self.assertEqual(out.count('guest_store<'),source.count('guest_store<'))
 def test_order_and_original_calls(self):
  source=fixtures()['Methods_01.cpp'];out=add_graphics_live_hooks(source,'Methods_01.cpp');self.assertLess(out.index('nfs_graphics_text::OwnerScope'),out.index('nfs_graphics_live::OwnerScope'));self.assertLess(out.index('nfs_graphics_live::TailScope'),out.index('nfs_graphics_text::TailScope'));self.assertLess(out.index('nfs_graphics_text::StringScope'),out.index('nfs_graphics_live::bindText'));self.assertNotIn('Game::_sub_40BBE0',out)
 def test_missing_normal_source_is_rejected(self):
  source=fixtures()['Methods_01.cpp'];
  for old in ['edx = 0x400;','eax = to32i(esi+4);','dword_4D231C+eax*4']:
   with self.assertRaises(ValueError):add_graphics_live_hooks(source.replace(old,'changed',1),'Methods_01.cpp')
 def test_exact_alignment_policy(self):
  out=add_graphics_live_hooks(fixtures()['Methods_01.cpp'],'Methods_01.cpp');self.assertIn('PolicyScope nfs_graphics_policy(*this,false)',out);self.assertIn('PolicyScope nfs_graphics_policy(*this,true)',out);self.assertIn('resourceAlignment(*this)',out)
  with self.assertRaises(ValueError):add_graphics_live_hooks(fixtures()['Methods_01.cpp'].replace('_sub_41B690()','_sub_other()'),'Methods_01.cpp')
 def test_words_and_duplicate_rejected(self):
  for name in ['Methods_13.cpp','Methods_14.cpp']:
   source=fixtures()[name];out=add_graphics_live_hooks(source,name);self.assertEqual(out.count('nfs_graphics_text::WordCapture'),4 if name.endswith('13.cpp') else 1)
   with self.assertRaises(ValueError):add_graphics_live_hooks(out,name)
   with self.assertRaises(ValueError):add_graphics_live_hooks(source.replace('guest_store<int16_t>','altered_store',1),name)
 def test_unrelated_unchanged(self):self.assertEqual(add_graphics_live_hooks('unchanged','Methods_02.cpp'),'unchanged')
if __name__=='__main__':unittest.main()

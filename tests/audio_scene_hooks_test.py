# SPDX-License-Identifier: MIT
import pathlib,re,sys,unittest
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[1]/"scripts"))
from audio_scene_hooks import add_audio_scene_hooks
HEADER='#include "FunctionIds.h"\n'
def source():return HEADER+'Fn(void) Game::_sub_426950()\n{\n'+''.join('\tesp -= 4; _sub_'+s+'(); esp += 4; //call\n'for s in ['49755C','497738','497D6C'])+' cmp(esi,0x1E0);\n cmp(edi,0x280);\n}\nFn(void) Game::_sub_426A60()\n{\n return;\n}\n'
def reverse(t):return re.sub(r'\n// NFS_AUDIO_SCENE_BEGIN:([^\n]+)\n.*?\n// NFS_AUDIO_SCENE_END:\1\n','',t,flags=re.S)
class Tests(unittest.TestCase):
 def test_exact_reversal(self):
  s=source();out=add_audio_scene_hooks(s,'Methods_03.cpp');self.assertEqual(reverse(out),s);self.assertLess(out.index('nfs_audio_scene_live::begin'),out.index('_sub_49755C();'));self.assertGreater(out.index('nfs_audio_scene_live::finish'),out.index('_sub_497D6C();'));self.assertIn('nfs_audio_scene_live::cancel',out)
 def test_original_operation_changes_reject(self):
  for old in ['49755C','497738','497D6C','0x1E0','0x280']:
   with self.assertRaises(ValueError):add_audio_scene_hooks(source().replace(old,'changed',1),'Methods_03.cpp')
 def test_new_exit_reject(self):
  with self.assertRaises(ValueError):add_audio_scene_hooks(source().replace(' cmp(esi',' return;\n cmp(esi',1),'Methods_03.cpp')
 def test_duplicate_reject(self):
  with self.assertRaises(ValueError):add_audio_scene_hooks(add_audio_scene_hooks(source(),'Methods_03.cpp'),'Methods_03.cpp')
 def test_unrelated_unchanged(self):self.assertEqual(add_audio_scene_hooks('unchanged','Methods_01.cpp'),'unchanged')
if __name__=='__main__':unittest.main()

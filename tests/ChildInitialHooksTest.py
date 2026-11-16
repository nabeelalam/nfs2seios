# SPDX-License-Identifier: MIT
"""Synthetic lexical anchors; no original game source/data."""
import pathlib,re,sys,unittest
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[1]/"scripts"))
from child_initial_sidebar_hooks import add_child_initial_sidebar_hooks
SOURCE=('#include "FunctionIds.h"\nFn(void) Game::_sub_40C160()\n{\n'
 '// NFS_PLAYER_SCENE_END:initial-caption-open\n'
 '\tesp -= 4; _sub_40C110(); esp += 4; //call\n'
 ' nfs_player_scene_live::completeInitialCaption(*this);\n'
 '// NFS_PLAYER_SCENE_END:copy-initial-open\n'
 '\tesp -= 4; _sub_405B60(); esp += 4; //call\n'
 '// NFS_PLAYER_SCENE_BEGIN:copy-initial-close\n'
 '// NFS_PLAYER_SCENE_END:copy-refresh-open\n'
 '\tesp -= 4; _sub_405B60(); esp += 4; //call\n'
 '// NFS_PLAYER_SCENE_BEGIN:copy-refresh-close\n}\n')
class Tests(unittest.TestCase):
 def test_exact_reversal_and_existing_caller_preserved(self):
  value=add_child_initial_sidebar_hooks(SOURCE,"Methods_01.cpp")
  self.assertEqual(re.sub(r"\n// NFS_CHILD_INITIAL_BEGIN:([^\n]+)\n.*?// NFS_CHILD_INITIAL_END:\1\n","",value,flags=re.S),SOURCE)
  self.assertEqual(value.count("_sub_40C110();"),1)
  self.assertLess(value.index("ChildInitialCallerScope"),value.index("_sub_40C110();"))
  self.assertGreater(value.index("nfs_child_initial_sidebar_live::completeInitialCaption"),value.index("_sub_40C110();"))
 def test_full_copy_lexical_and_row(self):
  value=add_child_initial_sidebar_hooks(SOURCE,"Methods_01.cpp")
  self.assertLess(value.index("CopyScope"),value.index("_sub_405B60();"))
  self.assertEqual(value.count("nfs_setup_initial_full_live::CopyScope"),2)
  self.assertLess(value.index("nfs_setup_refresh_full"),value.rindex("_sub_405B60();"))
  self.assertEqual(value.count("_sub_405B60();"),2)
  source='#include "FunctionIds.h"\nFn(void) Game::_sub_483070()\n{\n}\n'
  value=add_child_initial_sidebar_hooks(source,"Methods_10.cpp")
  self.assertEqual(value.count("nfs_setup_initial_full_live::row"),1)
  self.assertEqual(value.count("nfs_location_initial_full_live::row"),1)
  self.assertEqual(value.count("nfs_opponent_initial_full_live::row"),1)
  self.assertEqual(re.sub(r"\n// NFS_CHILD_INITIAL_BEGIN:([^\n]+)\n.*?// NFS_CHILD_INITIAL_END:\1\n","",value,flags=re.S),source)
  with self.assertRaises(ValueError):add_child_initial_sidebar_hooks(source.replace("483070","483071"),"Methods_10.cpp")
 def test_fail_closed_and_unrelated_unchanged(self):
  value=add_child_initial_sidebar_hooks(SOURCE,"Methods_01.cpp")
  with self.assertRaises(ValueError):add_child_initial_sidebar_hooks(value,"Methods_01.cpp")
  for old in ["_sub_40C160()","NFS_PLAYER_SCENE_END:initial-caption-open","nfs_player_scene_live::completeInitialCaption","NFS_PLAYER_SCENE_END:copy-initial-open","NFS_PLAYER_SCENE_BEGIN:copy-initial-close","NFS_PLAYER_SCENE_END:copy-refresh-open","NFS_PLAYER_SCENE_BEGIN:copy-refresh-close"]:
   with self.assertRaises(ValueError):add_child_initial_sidebar_hooks(SOURCE.replace(old,"changed"),"Methods_01.cpp")
  self.assertEqual(add_child_initial_sidebar_hooks("unrelated","Methods_02.cpp"),"unrelated")
if __name__=="__main__":unittest.main()

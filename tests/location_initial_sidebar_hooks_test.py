# SPDX-License-Identifier: MIT
import importlib.util,re,unittest
from pathlib import Path
p=Path(__file__).resolve().parents[1]/"scripts/location_initial_sidebar_hooks.py"
spec=importlib.util.spec_from_file_location("location_initial",p);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
class HookTests(unittest.TestCase):
 def scaffold(self):return '#include "FunctionIds.h"\nFn(void) Game::_sub_40C160()\n{\n// NFS_PLAYER_SCENE_END:initial-caption-open\n esp -= 4; _sub_40C110(); esp += 4;\n}\n'
 def test_reverse(self):
  source=self.scaffold();out=m.add_location_initial_sidebar_hooks(source,"Methods_01.cpp");self.assertIn("InitialCallerScope",out);self.assertEqual(re.sub(r"\n// NFS_LOCATION_INITIAL_BEGIN:([^\n]+)\n.*?// NFS_LOCATION_INITIAL_END:\1\n","",out,flags=re.S),source)
 def test_repeat(self):
  with self.assertRaises(ValueError):m.add_location_initial_sidebar_hooks(m.add_location_initial_sidebar_hooks(self.scaffold(),"Methods_01.cpp"),"Methods_01.cpp")
 def test_missing(self):
  for anchor in ['#include "FunctionIds.h"','Fn(void) Game::_sub_40C160()','// NFS_PLAYER_SCENE_END:initial-caption-open']:
   with self.assertRaises(ValueError):m.add_location_initial_sidebar_hooks(self.scaffold().replace(anchor,'MISSING'),"Methods_01.cpp")
 def test_other(self):self.assertEqual(m.add_location_initial_sidebar_hooks("unchanged","Methods_02.cpp"),"unchanged")
if __name__=="__main__":unittest.main()

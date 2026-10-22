import importlib.util,re,unittest
from pathlib import Path
p=Path(__file__).resolve().parents[1]/"scripts/audio_sidebar_measure_hooks.py"
s=importlib.util.spec_from_file_location("measure",p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
class Hooks(unittest.TestCase):
 def source(self):return "Fn(void) Game::_sub_405580()\n{\n"+"".join(a+b for a,b in m.ANCHORS.values())+"\tsar(eax, (int32_t)1);\n}\nFn(void) Game::_sub_4056F0()\n{\n}\n"
 def test_reverse_and_order(self):
  src=self.source();out=m.add_audio_sidebar_measure_hooks(src,"Methods_01.cpp")
  self.assertEqual(out.count("::measured("),2)
  self.assertEqual(re.sub(r"\n// NFS_AUDIO_SIDEBAR_MEASURE_BEGIN:measured-(right|center)\n.*?// NFS_AUDIO_SIDEBAR_MEASURE_END:measured-\1\n","",out,flags=re.S),src)
  for kind,(prefix,suffix) in m.ANCHORS.items():self.assertIn(prefix+"\n// NFS_AUDIO_SIDEBAR_MEASURE_BEGIN:measured-"+kind,out)
 def test_foreign(self):self.assertEqual(m.add_audio_sidebar_measure_hooks(self.source(),"Methods_09.cpp"),self.source())
 def test_missing_right(self):
  with self.assertRaises(ValueError):m.add_audio_sidebar_measure_hooks(self.source().replace("loc_4055D3:","wrong:"),"Methods_01.cpp")
 def test_missing_center(self):
  with self.assertRaises(ValueError):m.add_audio_sidebar_measure_hooks(self.source().replace("loc_405646:","wrong:"),"Methods_01.cpp")
 def test_duplicate(self):
  with self.assertRaises(ValueError):m.add_audio_sidebar_measure_hooks(m.add_audio_sidebar_measure_hooks(self.source(),"Methods_01.cpp"),"Methods_01.cpp")
 def test_wrong_function(self):
  with self.assertRaises(ValueError):m.add_audio_sidebar_measure_hooks(self.source().replace("sub_405580","sub_405581"),"Methods_01.cpp")
if __name__=="__main__":unittest.main()

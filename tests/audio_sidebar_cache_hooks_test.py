# SPDX-License-Identifier: MIT
import re,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"scripts"))
from audio_sidebar_cache_hooks import add_audio_sidebar_cache_hooks,MARKER

def function(name,body=""):
    return "Fn(void) Game::_sub_"+name+"()\n{\n"+body+"\n}\n"
def fixture():
    lazy="loc_40BC20:\n nfs_hd_scene_iteration.start();\n { nfs_background_scene_live::RoleScope nfs_hd_side(*this,nfs_background_scene_live::Phase::Side);\n }\n { nfs_background_scene_live::RoleScope nfs_hd_word(*this,nfs_background_scene_live::Phase::Word);\n }\n nfs_background_scene_live::beginGroup(*this);\n nfs_background_scene_live::postGroup(*this);"
    return '#include "FunctionIds.h"\n'+function("40A570"," nfs_audio_live::OwnerScope nfs_audio_owner(*this);")+function("40C110"," nfs_audio_sidebar_text::TailScope nfs_audio_sidebar_tail(*this);")+function("405CF0"," nfs_audio_sidebar_text::StringScope nfs_audio_sidebar_string(*this);\n nfs_audio_live::bindText(*this);")+function("40BBE0",lazy)+function("405B60")
class Tests(unittest.TestCase):
    def test_reverse_exact(self):
        original=fixture();out=add_audio_sidebar_cache_hooks(original,"Methods_01.cpp");reverse,n=re.subn(r"\n// "+MARKER+r"_BEGIN:([^\n]+)\n.*?\n// "+MARKER+r"_END:\1\n","",out,flags=re.S);self.assertEqual(original,reverse);self.assertGreater(n,8)
    def test_scope_order(self):
        out=add_audio_sidebar_cache_hooks(fixture(),"Methods_01.cpp");self.assertLess(out.index("nfs_audio_live::OwnerScope"),out.index("nfs_audio_sidebar_live::OwnerScope"));self.assertLess(out.index("nfs_audio_sidebar_live::TailScope"),out.index("nfs_audio_sidebar_text::TailScope"));self.assertLess(out.index("nfs_audio_sidebar_text::StringScope"),out.index("bindCaption"));self.assertLess(out.index("loc_40BC20:"),out.index("nfs_audio_cache_iteration.start()"))
    def test_duplicate_rejected(self):
        out=add_audio_sidebar_cache_hooks(fixture(),"Methods_01.cpp")
        with self.assertRaises(ValueError):add_audio_sidebar_cache_hooks(out,"Methods_01.cpp")
    def test_missing_parent_rejected(self):
        with self.assertRaises(ValueError):add_audio_sidebar_cache_hooks(fixture().replace("nfs_audio_live::OwnerScope","missing"),"Methods_01.cpp")
    def test_dispatch_count_strict(self):
        text='#include "FunctionIds.h"\n'+function("4A14D4")+function("4A10BC","\tcall(to32i(esp+0x38));"*4);out=add_audio_sidebar_cache_hooks(text,"Methods_13.cpp");self.assertEqual(out.count("\tcall(to32i(esp+0x38));"),4);self.assertEqual(out.count("IndexedSideCallScope"),4)
        with self.assertRaises(ValueError):add_audio_sidebar_cache_hooks(text.replace("\tcall(to32i(esp+0x38));","",1),"Methods_13.cpp")
    def test_other_source_unchanged(self):self.assertEqual("original",add_audio_sidebar_cache_hooks("original","Methods_99.cpp"))
if __name__=="__main__":unittest.main()

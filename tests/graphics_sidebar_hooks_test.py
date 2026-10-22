# SPDX-License-Identifier: MIT
import re,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"scripts"))
from graphics_sidebar_hooks import add_graphics_sidebar_hooks,MARKER

def function(name,body=""):
    return "Fn(void) Game::_sub_"+name+"()\n{\n"+body+"\n}\n"
def fixture():
    lazy="loc_40BC20:\n nfs_hd_scene_iteration.start();\n { nfs_background_scene_live::RoleScope nfs_hd_side(*this,nfs_background_scene_live::Phase::Side);\n }\n { nfs_background_scene_live::RoleScope nfs_hd_word(*this,nfs_background_scene_live::Phase::Word);\n }\n nfs_background_scene_live::beginGroup(*this);\n nfs_background_scene_live::postGroup(*this);"
    return '#include "FunctionIds.h"\n'+function("4101C0"," nfs_graphics_live::OwnerScope nfs_graphics_owner(*this);")+function("40C110")+function("405CF0"," nfs_audio_sidebar_text::StringScope nfs_audio_sidebar_string(*this);\n nfs_audio_live::bindText(*this);")+function("40BBE0",lazy)+function("405B60")+function("405580"," nfs_audio_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_audio_sidebar_text::SourceMeasureAlignment::right);\n nfs_audio_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_audio_sidebar_text::SourceMeasureAlignment::center);")
class Tests(unittest.TestCase):
    def test_reverse_exact(self):
        original=fixture();out=add_graphics_sidebar_hooks(original,"Methods_01.cpp");reverse,n=re.subn(r"\n// "+MARKER+r"_BEGIN:([^\n]+)\n.*?\n// "+MARKER+r"_END:\1\n","",out,flags=re.S);self.assertEqual(original,reverse);self.assertGreater(n,8)
    def test_scope_order(self):
        out=add_graphics_sidebar_hooks(fixture(),"Methods_01.cpp");self.assertLess(out.index("nfs_graphics_sidebar_text::OwnerScope"),out.index("nfs_graphics_live::OwnerScope"));self.assertLess(out.index("nfs_graphics_live::OwnerScope"),out.index("nfs_graphics_sidebar_live::OwnerScope"));self.assertLess(out.index("nfs_graphics_sidebar_live::TailScope"),out.index("nfs_graphics_sidebar_text::TailScope"));self.assertLess(out.index("nfs_graphics_sidebar_text::StringScope"),out.index("bindCaption"));self.assertLess(out.index("loc_40BC20:"),out.index("nfs_graphics_cache_iteration.start()"))
    def test_duplicate_rejected(self):
        out=add_graphics_sidebar_hooks(fixture(),"Methods_01.cpp")
        with self.assertRaises(ValueError):add_graphics_sidebar_hooks(out,"Methods_01.cpp")
    def test_missing_parent_rejected(self):
        with self.assertRaises(ValueError):add_graphics_sidebar_hooks(fixture().replace("nfs_graphics_live::OwnerScope","missing"),"Methods_01.cpp")
    def test_dispatch_count_strict(self):
        text='#include "FunctionIds.h"\n'+function("4A14D4")+function("4A5830","guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"*4)+function("4A10BC","\tcall(to32i(esp+0x38));"*4);out=add_graphics_sidebar_hooks(text,"Methods_13.cpp");self.assertEqual(out.count("\tcall(to32i(esp+0x38));"),4);self.assertEqual(out.count("IndexedSideCallScope"),4)
        with self.assertRaises(ValueError):add_graphics_sidebar_hooks(text.replace("\tcall(to32i(esp+0x38));","",1),"Methods_13.cpp")
    def test_other_files_reverse_exact(self):
        store="guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"
        cases={
            "Methods_09.cpp":function("4824CC"),
            "Methods_10.cpp":"".join(function(v) for v in ("482DA0","482BD4","483288","483438")),
            "Methods_13.cpp":function("4A14D4")+function("4A5830",store*4)+function("4A10BC","\tcall(to32i(esp+0x38));"*4),
            "Methods_14.cpp":function("4AC934")+function("4AC9C8")+function("4B3EE0",store)}
        for name,body in cases.items():
            with self.subTest(name=name):
                original='#include "FunctionIds.h"\n'+body
                out=add_graphics_sidebar_hooks(original,name)
                reverse,n=re.subn(r"\n// "+MARKER+r"_BEGIN:([^\n]+)\n.*?\n// "+MARKER+r"_END:\1\n","",out,flags=re.S)
                self.assertEqual(original,reverse);self.assertGreater(n,1)
                with self.assertRaises(ValueError):add_graphics_sidebar_hooks(out,name)
    def test_other_source_unchanged(self):self.assertEqual("original",add_graphics_sidebar_hooks("original","Methods_99.cpp"))
    def test_measurement_observers_preserved(self):
        out=add_graphics_sidebar_hooks(fixture(),"Methods_01.cpp")
        for kind in ("center","right"):
            original=" nfs_audio_sidebar_text::measured(*this,uint32_t(ecx),int32_t(eax),nfs_audio_sidebar_text::SourceMeasureAlignment::"+kind+");"
            self.assertEqual(out.count(original),1)
            self.assertEqual(out.count("nfs_graphics_sidebar_text::SourceMeasureAlignment::"+kind),1)
        with self.assertRaises(ValueError):add_graphics_sidebar_hooks(fixture().replace("SourceMeasureAlignment::right","SourceMeasureAlignment::invalid"),"Methods_01.cpp")
    def test_word_store_preserved_and_guarded(self):
        store="guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov"
        original='#include "FunctionIds.h"\n'+function("4AC934")+function("4AC9C8")+function("4B3EE0",store)
        out=add_graphics_sidebar_hooks(original,"Methods_14.cpp")
        self.assertEqual(out.count(store),1);self.assertEqual(out.count("nfs_graphics_sidebar_text::WordCapture"),1)
        with self.assertRaises(ValueError):add_graphics_sidebar_hooks(original.replace(store,""),"Methods_14.cpp")
if __name__=="__main__":unittest.main()

# SPDX-License-Identifier: MIT
"""Synthetic source scaffolds only; no original engine source is included."""
import re
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"scripts"))
from background_scene_hooks import add_background_scene_hooks


def fn(symbol,body=""):
    return "Fn(void) Game::_sub_"+symbol+"()\n{\n"+body+"\n}\n"


def call(symbol):
    return "\tesp -= 4; _sub_"+symbol+"(); esp += 4; //call\n"


def scaffold(name):
    prefix='#include "FunctionIds.h"\n'
    if name=="Methods_01.cpp":
        redraw=("loc_40BC20:\n\teax = guest_address(aSide);\n"+call("405B30")+
            "\tedi = to32i(off_4D2474+ecx*4);\n"+call("405B30")+
            "\teax = to32i(esp+eax*4); //mov\n"+call("405B30")+
            "\teax = guest_address(aLogo_0);\n"+call("405B30")+
            "loc_40C047:\nloc_40C054:\n"+call("405B60"))
        return prefix+fn("405B60")+fn("40BBE0",redraw)+fn("40D3D0","loc_40D63E:\n"+call("405B30")+call("427B30")+call("40DC40"))
    if name=="Methods_03.cpp":
        return prefix+fn("428250",call("49755C"))+"".join(fn(s)for s in("4284B0","4287F0","4289F0"))
    if name=="Methods_10.cpp":
        return prefix+"".join(fn(s)for s in("482DA0","482BD4","482ED4","482FD4","483070","483100","4831A8","483288","483438"))
    return prefix+fn("4A14D4")+fn("4A10BC","\tcall(to32i(esp+0x38));\n"*4)


def reverse(text):
    return re.sub(r"\n// NFS_HD_BACKGROUND_BEGIN:([^\n]+)\n.*?// NFS_HD_BACKGROUND_END:\1\n","",text,flags=re.S)


class HookTests(unittest.TestCase):
    names=("Methods_01.cpp","Methods_03.cpp","Methods_10.cpp","Methods_13.cpp")
    def test_all_original_bodies_reverse_exactly(self):
        for name in self.names:
            with self.subTest(name=name):
                original=scaffold(name);prepared=add_background_scene_hooks(original,name)
                self.assertEqual(reverse(prepared),original)
                self.assertEqual(prepared,add_background_scene_hooks(original,name))
                self.assertIn('#include "hd/BackgroundSceneQueueAdapter.inc"',prepared)
    def test_repeated_injection_is_rejected(self):
        for name in self.names:
            with self.assertRaises(ValueError):
                add_background_scene_hooks(add_background_scene_hooks(scaffold(name),name),name)
    def test_missing_entry_fails_closed(self):
        for name,symbol in zip(self.names,("40D3D0","428250","482DA0","4A10BC")):
            with self.assertRaises(ValueError):
                add_background_scene_hooks(scaffold(name).replace("_sub_"+symbol+"()","_sub_000000()",1),name)
    def test_wrong_dispatch_count_fails_closed(self):
        for n in(3,5):
            source=scaffold("Methods_13.cpp").replace("\tcall(to32i(esp+0x38));\n"*4,"\tcall(to32i(esp+0x38));\n"*n)
            with self.assertRaises(ValueError):add_background_scene_hooks(source,"Methods_13.cpp")
    def test_native_order_keeps_labels_later(self):
        source=add_background_scene_hooks(scaffold("Methods_01.cpp"),"Methods_01.cpp")
        begin=source.index("nfs_background_scene_live::begin(*this")
        base=source.index("_sub_427B30();")
        finish=source.index("nfs_background_scene_live::finish(nfs_hd_intent)")
        labels=source.index("_sub_40DC40();")
        self.assertLess(begin,base);self.assertLess(base,finish);self.assertLess(finish,labels)
        self.assertIn("cancel(nfs_hd_intent);\n throw;",source)
    def test_upload_completion_scope_constructed_first(self):
        source=add_background_scene_hooks(scaffold("Methods_03.cpp"),"Methods_03.cpp")
        self.assertLess(source.index("UploadEndScope"),source.index("UploadScope"))
    def test_lazy_redraw_and_optional_dispatch_guard(self):
        one=add_background_scene_hooks(scaffold("Methods_01.cpp"),"Methods_01.cpp")
        self.assertLess(one.index("loc_40BC20:"),one.index("nfs_hd_iteration.start();"))
        thirteen=add_background_scene_hooks(scaffold("Methods_13.cpp"),"Methods_13.cpp")
        self.assertEqual(thirteen.count("indexedSideCallee(*this)"),4)
    def test_other_units_unchanged(self):
        self.assertEqual(add_background_scene_hooks("unrelated","Methods_14.cpp"),"unrelated")

if __name__=="__main__":unittest.main()

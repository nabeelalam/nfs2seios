# SPDX-License-Identifier: MIT
"""Synthetic source scaffolds only: strict call sites and exact original reversal."""
import re,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"scripts"))
from location_preview_hooks import add_location_preview_hooks,NAMES

def fn(symbol,body=""):return "Fn(void) Game::_sub_"+symbol+"()\n{\n"+body+"\n}\n"
def call(symbol):return "\tesp -= 4; _sub_"+symbol+"(); esp += 4; //call\n"
def source(name):
    s='#include "FunctionIds.h"\n'
    if name=="Methods_13.cpp":return s+fn("4A10BC","\tcall(to32i(esp+0x38));\n"*4)
    normal=lambda label:label+"\teax = guest_address(aBlk4);\n"+call("405B30")+"\tebx = 0x13C; //mov\n\tedx = 0xD3; //mov\n\teax = esi; //mov\n"+call("40CD70")
    return s+fn("405B30")+fn("405B60")+fn("40C160",normal("loc_40C3CC:\n")+normal("loc_40C5C6:\n"))+fn("40CD70","loc_40CF46:\n"+call("48266C")+call("488620"))
def reverse(s):return re.sub(r"\n// NFS_LOCATION_PREVIEW_BEGIN:([^\n]+)\n.*?\n// NFS_LOCATION_PREVIEW_END:\1\n","",s,flags=re.S)
class Tests(unittest.TestCase):
    def test_all_units_reverse_exactly(self):
        for n in NAMES:
            before=source(n);after=add_location_preview_hooks(before,n);self.assertEqual(reverse(after),before);self.assertEqual(add_location_preview_hooks(before,n),after)
            with self.assertRaises(ValueError):add_location_preview_hooks(after,n)
    def test_only_two_normal_call_sites_own_scope(self):
        after=add_location_preview_hooks(source(NAMES[0]),NAMES[0]);self.assertEqual(after.count("preview_live::Scope"),2);self.assertEqual(after.count("Site::Initial"),1);self.assertEqual(after.count("Site::Refresh"),1)
        body=after[after.index("Fn(void) Game::_sub_40CD70"):];self.assertNotIn("preview_live::Scope",body)
    def test_missing_or_changed_original_arguments_rejected(self):
        for old in("loc_40C3CC:","loc_40C5C6:","guest_address(aBlk4)","loc_40CF46:","_sub_488620();","_sub_40CD70();","ebx = 0x13C;"):
            with self.assertRaises(ValueError):add_location_preview_hooks(source(NAMES[0]).replace(old,"changed",1),NAMES[0])
    def test_dispatch_count_strict(self):
        for count in(3,5):
            with self.assertRaises(ValueError):add_location_preview_hooks(source(NAMES[1]).replace("\tcall(to32i(esp+0x38));\n"*4,"\tcall(to32i(esp+0x38));\n"*count),NAMES[1])
    def test_other_units_unchanged(self):self.assertEqual(add_location_preview_hooks("other","Methods_11.cpp"),"other")
if __name__=="__main__":unittest.main()

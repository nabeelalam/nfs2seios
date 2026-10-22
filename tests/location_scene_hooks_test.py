# SPDX-License-Identifier: MIT
"""Synthetic source only: Location ownership, original-byte reversal and call ordering."""
import re,sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"scripts"))
from location_scene_hooks import add_location_scene_hooks,NAMES
from background_scene_hooks_test import scaffold,fn,call
from background_scene_hooks import add_background_scene_hooks

def original(name):
    s=add_background_scene_hooks(scaffold(name),name)
    if name=="Methods_01.cpp":
        native="\tpush32(0xFFFFFFFF);\n\tecx = 0x20; //mov\n\teax = 1; //mov\n\txor_(edx, edx);\n"+call("427B30")
        base=lambda label:label+"\teax = to32i(dword_4D4D24); //mov\n"+call("4825A8")+"\teax = static_cast<int32_t>(guest_address(aBlk4)); //mov\n"+call("405B30")
        s+=fn("40C160",base("loc_40C3CC:\n")+base("loc_40C5C6:\n")+"loc_40C792:\n"+call("405B30")+native)+fn("40C110")
        # Caption generator runs first; its scopes must remain inside Scene scopes.
        for sym,role in (("40C160","OwnerScope"),("40C110","TailScope")):
            a="Fn(void) Game::_sub_"+sym+"()\n{\n";s=s.replace(a,a+" nfs_location_caption::"+role+" caption(*this);\n")
    return s

def reverse(s):return re.sub(r"\n// NFS_LOCATION_SCENE_BEGIN:([^\n]+)\n.*?// NFS_LOCATION_SCENE_END:\1\n","",s,flags=re.S)
class Tests(unittest.TestCase):
    def test_reversal_exact_all_units(self):
        for n in NAMES:
            s=original(n);p=add_location_scene_hooks(s,n);self.assertEqual(reverse(p),s);self.assertEqual(add_location_scene_hooks(s,n),p)
            with self.assertRaises(ValueError):add_location_scene_hooks(p,n)
    def test_owner_tail_scope_order(self):
        s=add_location_scene_hooks(original(NAMES[0]),NAMES[0])
        for sym,role in (("40C160","OwnerScope"),("40C110","TailScope")):
            body=s[s.index("Fn(void) Game::_sub_"+sym+"()\n{"):]
            self.assertLess(body.index("nfs_location_scene_live::"),body.index("nfs_location_caption::"+role))
        self.assertIn("LocationMenuOwnerScope",s)
    def test_native_and_base_order(self):
        s=add_location_scene_hooks(original(NAMES[0]),NAMES[0]);s=s[s.index("Fn(void) Game::_sub_40C160"):]
        self.assertLess(s.index("Phase::Base"),s.index("_sub_405B30"));self.assertLess(s.index("::begin(*this"),s.index("_sub_427B30"));self.assertLess(s.index("_sub_427B30"),s.index("::finish("));self.assertIn("::cancel(nfs_location_intent);\n throw;",s)
    def test_all_three_exact_base_calls_covered(self):
        s=add_location_scene_hooks(original(NAMES[0]),NAMES[0]);body=s[s.index("Fn(void) Game::_sub_40C160"):]
        self.assertEqual(body.count("Phase::Base"),3)
        self.assertEqual(body.count("BaseArgument::Blk4Direct"),2)
        for label,tag in (("loc_40C3CC:","normal-base-initial-open"),("loc_40C5C6:","normal-base-refresh-open")):
            self.assertLess(body.index(label),body.index("NFS_LOCATION_SCENE_BEGIN:"+tag))
        for old in ("guest_address(aBlk4)","loc_40C3CC:","loc_40C5C6:"):
            with self.assertRaises(ValueError):add_location_scene_hooks(original(NAMES[0]).replace(old,"changed",1),NAMES[0])
    def test_upload_scope_constructed_before_collector(self):
        s=add_location_scene_hooks(original(NAMES[1]),NAMES[1]);self.assertLess(s.index("nfs_location_scene_live::UploadEndScope"),s.index("nfs_logo_publisher::UploadScope"))
    def test_missing_actual_base_or_native_rejects(self):
        for old in ("loc_40C792:","\tpush32(0xFFFFFFFF);\n\tecx = 0x20;"):
            with self.assertRaises(ValueError):add_location_scene_hooks(original(NAMES[0]).replace(old,"changed",1),NAMES[0])
    def test_queue_shares_actual_main_allocator_and_active_owner(self):
        source=(Path(__file__).resolve().parents[1]/"src/hd/LocationSceneQueueAdapter.inc").read_text()
        self.assertIn('#include "BackgroundSceneQueueAdapter.inc"',source)
        self.assertRegex(source,r"inline uint64_t nextRequest\(\)noexcept\{return nfs_background_scene_live::nextRequest\(\);\}")
        self.assertRegex(source,r"inline uint64_t&activeRequest\(\)noexcept\{return nfs_background_scene_live::activeRequest\(\);\}")
        self.assertNotIn("static std::atomic",source);self.assertNotIn("static thread_local",source)
    def test_other_units_unchanged(self):self.assertEqual(add_location_scene_hooks("unrelated","Methods_14.cpp"),"unrelated")
if __name__=="__main__":unittest.main()

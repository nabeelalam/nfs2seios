"""Public synthetic source proof for Controls complete640 observer hooks."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import re
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from controls_live_hooks import NAMES,MARKER,add_controls_live_hooks,remove_controls_live_hooks
from controls_sidebar_hooks_test import fixtures as sidebar_fixtures
from controls_sidebar_hooks import add_controls_sidebar_hooks
from controls_text_hooks_test import section

def fixtures():
    sources={name:add_controls_sidebar_hooks(source,name)for name,source in sidebar_fixtures().items()};text=sources['Methods_01.cpp']
    a='\tesp -= 4; _sub_405B30(); esp += 4; //call'
    prefix='\tesp -= 4; _sub_40EB20(); esp += 4; //call\n\tesp -= 4; _sub_428A20(); esp += 4; //call\n\teax = to32i(esi+4); //mov\n\tedx = 0x400; //mov\n'
    text=text.replace(a,prefix+a,1)
    a='\tesp -= 4; _sub_405B60(); esp += 4; //call'
    prefix='\teax = to32i(dword_4D2314+2); //mov\n\teax = to32i(dword_4D231C+eax*4); //mov\n\teax = to32i(eax+0x20); //mov\n\tedx = 0x1000; //mov\n'
    sources['Methods_01.cpp']=text.replace(a,prefix+a,1)
    return sources

class ControlsLiveHooks(unittest.TestCase):
    def test_all_units_exact_reversal_and_unchanged_originals(self):
        for name,base in fixtures().items():
            for source in (base,base.replace('\n','\r\n')):
                after=add_controls_live_hooks(source,name)
                self.assertEqual(remove_controls_live_hooks(after).encode(),source.encode())
                self.assertEqual(re.findall(r'\besp -= 4; _sub_[A-F0-9]+\(\); esp \+= 4; //call',after),re.findall(r'\besp -= 4; _sub_[A-F0-9]+\(\); esp \+= 4; //call',source))
                self.assertEqual(after.count('guest_store<'),source.count('guest_store<'))
                self.assertEqual(after.count('nfs_controls_sidebar'),source.count('nfs_controls_sidebar'))
                self.assertNotIn('nfs_graphics_live',after)
                for forbidden in ('enqueue_renderer','activeRequest','private_graphics_scene','probeCompleted','dumpFile'):
                    self.assertNotIn(forbidden,after)
    def test_owner_string_tail_constructor_and_destructor_order(self):
        after=add_controls_live_hooks(fixtures()['Methods_01.cpp'],'Methods_01.cpp')
        owner=section(after,'4101C0')
        self.assertLess(owner.index('nfs_controls_text::OwnerScope'),owner.index('nfs_controls_live::OwnerScope'))
        self.assertLess(owner.index('nfs_controls_live::OwnerScope'),owner.index('nfs_controls_sidebar_live::OwnerScope'))
        self.assertLess(owner.index('nfs_controls_live::ResetScope'),owner.index('_sub_405B30()'))
        self.assertLess(owner.index('nfs_controls_live::SidebarScope'),owner.index('_sub_405B60()'))
        self.assertLess(owner.index('nfs_controls_live::TailScope'),owner.index('nfs_controls_text::TailScope'))
        self.assertLess(owner.index('NFS_CONTROLS_TEXT_END:tail-close'),owner.index('NFS_CONTROLS_LIVE_BEGIN:tail-close'))
        string=section(after,'405CF0')
        self.assertLess(string.index('nfs_controls_text::StringScope'),string.index('nfs_controls_live::bindText'))
    def test_retirement_after_text_invalidation(self):
        for name in ('Methods_10.cpp','Methods_14.cpp'):
            after=add_controls_live_hooks(fixtures()[name],name)
            for symbol,hook in (('482DA0','beforeDescriptorRetire'),)if name=='Methods_10.cpp'else(('4AC934','beforeOwnerRetire'),('4AC9C8','beforeDescriptorRetire')):
                part=section(after,symbol)
                self.assertLess(part.index('nfs_controls_text::'+hook),part.index('nfs_controls_live::afterRetire'))
    def test_all_input_policy_sidebar_init_and_glyph_functions_untouched(self):
        for name,source in fixtures().items():
            after=add_controls_live_hooks(source,name)
            symbols={'Methods_01.cpp':('403FC0','414000','414200','405580','4054E0','405B30','40BBE0','40C110'),
                     'Methods_09.cpp':('4824CC','414400'),'Methods_13.cpp':('4A5830','4A14D4','4A10BC')}.get(name,())
            for symbol in symbols:self.assertEqual(section(after,symbol),section(source,symbol))
    def test_missing_duplicate_and_reordered_source_anchors_decline(self):
        source=fixtures()['Methods_01.cpp']
        for old in (' nfs_controls_text::OwnerScope nfs_controls_owner(*this);',' nfs_controls_text::StringScope nfs_controls_string(*this);','loc_41020E:','eax = to32i(esi+4);','edx = 0x400;','dword_4D231C+eax*4','eax+0x20','edx = 0x1000;',' nfs_controls_text::TailScope nfs_controls_tail(*this);','NFS_CONTROLS_TEXT_END:tail-close'):
            with self.subTest(old=old),self.assertRaises(ValueError):add_controls_live_hooks(source.replace(old,'changed-original',1),'Methods_01.cpp')
        with self.assertRaises(ValueError):add_controls_live_hooks(source+source,'Methods_01.cpp')
        for dependency in ('ControlsTextGameHooks.inc','ControlsSidebarLiveGameHooks.inc'):
            with self.subTest(dependency=dependency),self.assertRaises(ValueError):add_controls_live_hooks(source.replace('#include \"hd/'+dependency+'\"','// missing prerequisite',1),'Methods_01.cpp')
        old='// NFS_CONTROLS_TEXT_END:tail-close\n'
        with self.assertRaises(ValueError):add_controls_live_hooks(source.replace(old,'',1).replace('loc_41020E:\n','loc_41020E:\n'+old,1),'Methods_01.cpp')
    def test_lease_duplicate_application_and_invalid_removal_decline(self):
        for name,source in fixtures().items():
            after=add_controls_live_hooks(source,name)
            with self.assertRaises(ValueError):add_controls_live_hooks(after,name)
            with self.assertRaises(ValueError):remove_controls_live_hooks(after.replace(MARKER+'_END:include',MARKER+'_END:broken',1))
            changed='// checked copy transaction begin\n'+source
            with self.assertRaises(ValueError):add_controls_live_hooks(changed,name)
        source=fixtures()['Methods_01.cpp']
        token=' nfs_controls_text::StringScope nfs_controls_string(*this);'
        with self.assertRaises(ValueError):add_controls_live_hooks(source.replace(token,'// checked copy transaction begin\n'+token+'\n// checked copy transaction end',1),'Methods_01.cpp')
        self.assertEqual(add_controls_live_hooks('unchanged','Methods_03.cpp'),'unchanged')
        self.assertEqual(remove_controls_live_hooks('unchanged'),'unchanged')
        self.assertEqual(len(NAMES),5)

if __name__=='__main__':unittest.main()

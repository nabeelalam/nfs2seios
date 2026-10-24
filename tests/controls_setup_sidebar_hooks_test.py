"""Public synthetic source, already carrying independent parent/setup observers."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import re
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
sys.path.insert(0,str(Path(__file__).resolve().parent))
from controls_setup_sidebar_hooks import NAMES,MARKER,add_controls_setup_sidebar_hooks,remove_controls_setup_sidebar_hooks
from controls_setup_text_hooks import add_controls_setup_text_hooks
from controls_sidebar_hooks import add_controls_sidebar_hooks
from controls_setup_text_hooks_test import fixtures as setup_fixtures
from controls_sidebar_hooks_test import LAZY
from controls_text_hooks_test import fn,call,section,LEASE,FOREIGN

def fixtures():
    result=setup_fixtures()
    result['Methods_01.cpp']+=fn('40BBE0',LEASE+LAZY)+fn('40C110',LEASE)
    result['Methods_10.cpp']+=fn('483288',LEASE)+fn('483438',LEASE)
    result['Methods_13.cpp']+=fn('4A14D4',LEASE)+fn('4A10BC',LEASE+('\tcall(to32i(esp+0x38));\n'+FOREIGN)*4)
    return {name:add_controls_setup_text_hooks(add_controls_sidebar_hooks(source,name),name)for name,source in result.items()}

class ControlsSetupSidebarHooks(unittest.TestCase):
    def test_reversal_existing_observations_original_calls_stores_both_endings(self):
        for name,base in fixtures().items():
            for source in (base,base.replace('\n','\r\n')):
                with self.subTest(name=name,crlf='\r\n'in source):
                    after=add_controls_setup_sidebar_hooks(source,name)
                    self.assertEqual(remove_controls_setup_sidebar_hooks(after).encode(),source.encode())
                    pattern=r'\besp -= 4; _sub_[A-F0-9]+\(\); esp \+= 4; //call'
                    self.assertEqual(re.findall(pattern,after),re.findall(pattern,source))
                    for token in ('guest_store<','call(to32i(esp+0x38))','existing_observation','nfs_controls_setup_text::','nfs_controls_sidebar_text::','nfs_controls_sidebar_live::','nfs_controls_text::'):
                        self.assertEqual(after.count(token),source.count(token))
                    self.assertNotIn('enqueue_renderer',after)
                    self.assertNotIn('activeRequest',after)

    def test_child_owners_after_real_setup_owner_no_prompt_owner(self):
        source=fixtures()['Methods_01.cpp'];after=add_controls_setup_sidebar_hooks(source,'Methods_01.cpp')
        for symbol in ('4101C0','403970'):
            part=section(after,symbol)
            self.assertLess(part.index('nfs_controls_setup_text::OwnerScope'),part.index('nfs_controls_setup_sidebar_text::OwnerScope'))
            self.assertLess(part.index('nfs_controls_setup_sidebar_text::OwnerScope'),part.index('nfs_controls_setup_sidebar_live::OwnerScope'))
            self.assertEqual(part.count('nfs_controls_setup_sidebar_text::OwnerScope'),1)
            self.assertEqual(part.count('nfs_controls_setup_sidebar_live::OwnerScope'),1)
        self.assertEqual(section(source,'403FC0'),section(after,'403FC0'))
        part=section(after,'40C110')
        self.assertLess(part.index('nfs_controls_setup_sidebar_live::TailScope'),part.index('nfs_controls_setup_sidebar_text::TailScope'))
        part=section(after,'405CF0')
        self.assertLess(part.index('nfs_controls_setup_sidebar_text::StringScope'),part.index('nfs_controls_setup_sidebar_live::bindCaption'))
        lazy=section(after,'40BBE0')
        for prior in ('loc_40BC20:','dword_5637F4','dword_4D2440'):
            self.assertLess(lazy.index(prior),lazy.index('nfs_controls_setup_cache_iteration.start()'))
        self.assertLess(lazy.index('nfs_background_scene_live::beginGroup'),lazy.index('nfs_controls_setup_sidebar_live::preGroup'))
        self.assertLess(lazy.index('nfs_background_scene_live::postGroup'),lazy.index('nfs_controls_setup_sidebar_live::postGroup'))

    def test_original_measurement_registers_before_arithmetic(self):
        after=add_controls_setup_sidebar_hooks(fixtures()['Methods_01.cpp'],'Methods_01.cpp')
        part=section(after,'405580')
        for label,end,kind,suffix in (('loc_4055D3:','loc_4055DC:','right','sub(esi, eax)'),('loc_405646:','loc_405662:','center','edx = eax')):
            region=part[part.index(label):part.index(end)]
            self.assertLess(region.index('_sub_482424()'),region.index('nfs_controls_setup_sidebar_text::measured'))
            self.assertLess(region.index('nfs_controls_setup_sidebar_text::measured'),region.index(suffix))
            self.assertIn('SourceMeasureAlignment::'+kind,region)
        self.assertEqual(part.count('nfs_controls_setup_sidebar_text::measured'),2)

    def test_low_level_scopes_and_nonobservation_functions_preserved(self):
        for name,count in (('Methods_13.cpp',4),('Methods_14.cpp',1)):
            after=add_controls_setup_sidebar_hooks(fixtures()[name],name)
            self.assertEqual(after.count('nfs_controls_setup_sidebar_text::WordCapture'),count)
        after=add_controls_setup_sidebar_hooks(fixtures()['Methods_13.cpp'],'Methods_13.cpp')
        self.assertEqual(after.count('nfs_controls_setup_sidebar_live::IndexedSideCallScope'),4)
        for name,terms in {'Methods_09.cpp':('SelectionScope',),'Methods_10.cpp':('beforeDescriptorRetire','game_id_sub_482BD4','game_id_sub_483288','game_id_sub_483438','CopyWrapperScope'),'Methods_13.cpp':('game_id_sub_4A14D4','GlyphScope'),'Methods_14.cpp':('beforeOwnerRetire','beforeDescriptorRetire')}.items():
            after=add_controls_setup_sidebar_hooks(fixtures()[name],name)
            for term in terms:self.assertIn(term,after)
        source=fixtures()['Methods_01.cpp'];after=add_controls_setup_sidebar_hooks(source,'Methods_01.cpp')
        for symbol in ('403FC0','414000','414200','405B30','40B2C0'):
            self.assertEqual(section(source,symbol),section(after,symbol))
        self.assertEqual(remove_controls_setup_sidebar_hooks(section(after,'405B60')),section(source,'405B60'))

    def test_missing_duplicate_changed_and_reordered_anchors_refuse(self):
        source=fixtures()['Methods_01.cpp']
        for old in ('loc_40BC20:','dword_5637F4','dword_4D2440',' nfs_hd_scene_iteration.start();','nfs_controls_setup_text::OwnerScope nfs_controls_setup_owner(*this,0x4101c0u);','nfs_controls_setup_text::OwnerScope nfs_controls_setup_owner(*this,0x403970u);','Game::_sub_40C110','Game::_sub_40BBE0','loc_4055D3:','loc_405646:','sub(esi, eax);','sar(eax, (int32_t)1);'):
            with self.subTest(old=old),self.assertRaises(ValueError):add_controls_setup_sidebar_hooks(source.replace(old,'changed-anchor',1),'Methods_01.cpp')
        for symbol in ('4101C0','403970','40BBE0','40C110','405580'):
            with self.assertRaises(ValueError):add_controls_setup_sidebar_hooks(source+fn(symbol),'Methods_01.cpp')
        for symbol in ('4101C0','403970'):
            part=section(source,symbol);owner=re.search(r' nfs_controls_setup_text::OwnerScope[^\n]+',part).group()
            with self.assertRaises(ValueError):add_controls_setup_sidebar_hooks(source.replace(owner,owner+'\n'+owner,1),'Methods_01.cpp')
        for name in ('Methods_13.cpp','Methods_14.cpp'):
            source=fixtures()[name]
            with self.assertRaises(ValueError):add_controls_setup_sidebar_hooks(source.replace('guest_store<int16_t>','changed_store',1),name)
        source=fixtures()['Methods_13.cpp']
        with self.assertRaises(ValueError):add_controls_setup_sidebar_hooks(source.replace('call(to32i(esp+0x38))','call(changed)',1),'Methods_13.cpp')

    def test_lease_reentry_mixed_endings_and_damaged_removal_refuse(self):
        for name,source in fixtures().items():
            if name not in NAMES:continue
            after=add_controls_setup_sidebar_hooks(source,name)
            with self.assertRaises(ValueError):add_controls_setup_sidebar_hooks(after,name)
            with self.assertRaises(ValueError):remove_controls_setup_sidebar_hooks(after.replace(MARKER+'_END:include',MARKER+'_END:broken',1))
            with self.assertRaises(ValueError):add_controls_setup_sidebar_hooks('// checked copy transaction begin\n'+source,name)
        source=fixtures()['Methods_01.cpp']
        for anchor in (' nfs_hd_scene_iteration.start();',call('482424').rstrip('\n')):
            # The first global482424 belongs to the untouched assignment prompt.
            # Target the actual405580 measurement site we instrument instead.
            symbol='405580'if '482424'in anchor else '40BBE0'
            part=section(source,symbol)
            changed=source.replace(part,part.replace(anchor,'// checked copy transaction begin\n'+anchor+'\n// checked copy transaction end',1),1)
            with self.subTest(anchor=anchor),self.assertRaises(ValueError):add_controls_setup_sidebar_hooks(changed,'Methods_01.cpp')
        source=fixtures()['Methods_13.cpp'];store='guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov'
        with self.assertRaises(ValueError):add_controls_setup_sidebar_hooks(source.replace(store,'// checked copy transaction begin\n'+store+'\n// checked copy transaction end',1),'Methods_13.cpp')
        source=fixtures()['Methods_01.cpp']
        with self.assertRaises(ValueError):add_controls_setup_sidebar_hooks(source.replace('\n','\r\n',1),'Methods_01.cpp')
        self.assertEqual(add_controls_setup_sidebar_hooks('unchanged','Methods_03.cpp'),'unchanged')
        self.assertEqual(remove_controls_setup_sidebar_hooks('unchanged'),'unchanged')
        self.assertEqual(len(NAMES),5)

if __name__=='__main__':unittest.main()

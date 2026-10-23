"""Independent synthetic original-call fixtures for Controls retained343 hooks."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import re
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from controls_sidebar_hooks import NAMES,MARKER,add_controls_sidebar_hooks,remove_controls_sidebar_hooks
from controls_text_hooks import add_controls_text_hooks
from controls_text_hooks_test import fixtures as text_fixtures,fn,call,section,LEASE,FOREIGN

LAZY='''\ttest(dx, dx);
\tif (jz()) goto loc_40BD1C;
loc_40BC02:
\tcmp(to16i(esp+0x1C), (int16_t)0);
\tebx = to32i(dword_4D2440); //mov
\tcmp(ebx, to32i(dword_5637F4));
\tif (jge()) goto loc_40BD11;
loc_40BC20:
 nfs_hd_scene_iteration.start();
 { nfs_background_scene_live::RoleScope nfs_hd_side(*this,nfs_background_scene_live::Phase::Side);
\tesp -= 4; _sub_405B60(); esp += 4; //call
 }
 { nfs_background_scene_live::RoleScope nfs_hd_word(*this,nfs_background_scene_live::Phase::Word);
\tesp -= 4; _sub_405B60(); esp += 4; //call
 }
 nfs_background_scene_live::beginGroup(*this);
\tesp -= 4; _sub_405B30(); esp += 4; //call
 nfs_background_scene_live::postGroup(*this);
\tesp -= 4; _sub_40C110(); esp += 4; //call
'''
# Escapes above describe source syntax, never original game bytes or assets.
LAZY=LAZY.replace('\\t','\t')
def fixtures():
    result=text_fixtures()
    result['Methods_01.cpp']+=fn('40BBE0',LEASE+LAZY)+fn('40C110',LEASE)
    result['Methods_10.cpp']+=fn('483288',LEASE)+fn('483438',LEASE)
    result['Methods_13.cpp']+=fn('4A14D4',LEASE)+fn('4A10BC',LEASE+('\tcall(to32i(esp+0x38));\n'+FOREIGN)*4)
    return {name:add_controls_text_hooks(source,name)for name,source in result.items()}

class ControlsSidebarHooks(unittest.TestCase):
    def test_exact_reversal_both_newlines_all_originals_and_foreign_hooks(self):
        for name,base in fixtures().items():
            for source in (base,base.replace('\n','\r\n')):
                after=add_controls_sidebar_hooks(source,name)
                self.assertEqual(remove_controls_sidebar_hooks(after).encode(),source.encode())
                self.assertEqual(re.findall(r'\besp -= 4; _sub_[A-F0-9]+\(\); esp \+= 4; //call',after),re.findall(r'\besp -= 4; _sub_[A-F0-9]+\(\); esp \+= 4; //call',source))
                self.assertEqual(after.count('guest_store<'),source.count('guest_store<'))
                self.assertEqual(after.count('call(to32i(esp+0x38))'),source.count('call(to32i(esp+0x38))'))
                self.assertEqual(after.count('existing_observation'),source.count('existing_observation'))
                self.assertNotIn('enqueue_renderer',after)
                self.assertNotIn('activeRequest',after)
                self.assertNotIn('nfs_graphics_sidebar',after)
    def test_owner_caption_cache_destructor_order_and_lazy_gate(self):
        source=fixtures()['Methods_01.cpp'];after=add_controls_sidebar_hooks(source,'Methods_01.cpp')
        owner=section(after,'4101C0')
        self.assertLess(owner.index('nfs_controls_sidebar_text::OwnerScope'),owner.index('nfs_controls_text::OwnerScope'))
        self.assertLess(owner.index('nfs_controls_text::OwnerScope'),owner.index('nfs_controls_sidebar_live::OwnerScope'))
        caption=section(after,'40C110')
        self.assertLess(caption.index('nfs_controls_sidebar_live::TailScope'),caption.index('nfs_controls_sidebar_text::TailScope'))
        lazy=section(after,'40BBE0')
        self.assertLess(lazy.index('loc_40BC20:'),lazy.index('nfs_controls_cache_iteration.start()'))
        self.assertLess(lazy.index('dword_5637F4'),lazy.index('nfs_controls_cache_iteration.start()'))
        self.assertLess(lazy.index('nfs_background_scene_live::beginGroup'),lazy.index('nfs_controls_sidebar_live::preGroup'))
        self.assertLess(lazy.index('nfs_background_scene_live::postGroup'),lazy.index('nfs_controls_sidebar_live::postGroup'))
        policy=section(after,'405580')
        self.assertEqual(policy.count('nfs_controls_sidebar_text::measured'),2)
        self.assertIn('SourceMeasureAlignment::right',policy)
        self.assertIn('SourceMeasureAlignment::center',policy)
        self.assertLess(policy.index('nfs_controls_text::dialogMeasured'),policy.index('nfs_controls_sidebar_text::measured'))
        self.assertLess(policy.index('nfs_controls_sidebar_text::measured'),policy.index('sub(esi, eax)'))
    def test_row_indexed_word_and_retirement_families(self):
        expected={'Methods_10.cpp':('game_id_sub_482BD4','game_id_sub_483288','game_id_sub_483438','CopyWrapperScope'),
                  'Methods_13.cpp':('game_id_sub_4A14D4','IndexedSideCallScope','GlyphScope','WordCapture'),
                  'Methods_14.cpp':('beforeOwnerRetire','beforeDescriptorRetire','WordCapture')}
        for name,terms in expected.items():
            after=add_controls_sidebar_hooks(fixtures()[name],name)
            for term in terms:self.assertIn('nfs_controls_sidebar',after);self.assertIn(term,after)
        self.assertEqual(add_controls_sidebar_hooks(fixtures()['Methods_13.cpp'],'Methods_13.cpp').count('nfs_controls_sidebar_live::IndexedSideCallScope'),4)
        self.assertEqual(add_controls_sidebar_hooks(fixtures()['Methods_13.cpp'],'Methods_13.cpp').count('nfs_controls_sidebar_text::WordCapture'),4)
        self.assertEqual(add_controls_sidebar_hooks(fixtures()['Methods_14.cpp'],'Methods_14.cpp').count('nfs_controls_sidebar_text::WordCapture'),1)
    def test_inputs_and_reset_sidebar_implementations_unchanged(self):
        source=fixtures()['Methods_01.cpp'];after=add_controls_sidebar_hooks(source,'Methods_01.cpp')
        for symbol in ('403FC0','414000','414200','405B30'):
            self.assertEqual(section(source,symbol),section(after,symbol))
        # 405B60 receives only a scoped observer entry; removing it restores all bytes.
        self.assertEqual(remove_controls_sidebar_hooks(section(after,'405B60')),section(source,'405B60'))
    def test_missing_duplicate_gate_and_measurement_anchors_decline(self):
        source=fixtures()['Methods_01.cpp']
        for old in ('loc_40BC20:','dword_5637F4','dword_4D2440',' nfs_hd_scene_iteration.start();',' nfs_controls_text::OwnerScope nfs_controls_owner(*this);','nfs_controls_text::dialogMeasured','Game::_sub_40C110','Game::_sub_40BBE0'):
            with self.subTest(old=old),self.assertRaises(ValueError):add_controls_sidebar_hooks(source.replace(old,'changed-anchor',1),'Methods_01.cpp')
        for extra in (fn('40BBE0'),fn('40C110')):
            with self.assertRaises(ValueError):add_controls_sidebar_hooks(source+extra,'Methods_01.cpp')
        for name in ('Methods_13.cpp','Methods_14.cpp'):
            source=fixtures()[name]
            with self.assertRaises(ValueError):add_controls_sidebar_hooks(source.replace('guest_store<int16_t>','changed_store',1),name)
        source=fixtures()['Methods_13.cpp']
        with self.assertRaises(ValueError):add_controls_sidebar_hooks(source.replace('call(to32i(esp+0x38))','call(changed)',1),'Methods_13.cpp')
    def test_lease_scope_duplicate_application_and_damaged_removal_decline(self):
        for name,source in fixtures().items():
            after=add_controls_sidebar_hooks(source,name)
            with self.assertRaises(ValueError):add_controls_sidebar_hooks(after,name)
            with self.assertRaises(ValueError):remove_controls_sidebar_hooks(after.replace(MARKER+'_END:include',MARKER+'_END:broken',1))
            changed=source.replace('#include "FunctionIds.h"\n','#include "FunctionIds.h"\n// checked copy transaction begin\n',1)
            with self.assertRaises(ValueError):add_controls_sidebar_hooks(changed,name)
        source=fixtures()['Methods_01.cpp']
        changed=source.replace(' nfs_hd_scene_iteration.start();','// checked copy transaction begin\n nfs_hd_scene_iteration.start();\n// checked copy transaction end',1)
        with self.assertRaises(ValueError):add_controls_sidebar_hooks(changed,'Methods_01.cpp')
        with self.assertRaises(ValueError):add_controls_sidebar_hooks(source.replace('\n','\r\n',1),'Methods_01.cpp')
        self.assertEqual(add_controls_sidebar_hooks('unchanged','Methods_03.cpp'),'unchanged')
        self.assertEqual(remove_controls_sidebar_hooks('unchanged'),'unchanged')
        self.assertEqual(len(NAMES),5)

if __name__=='__main__':unittest.main()

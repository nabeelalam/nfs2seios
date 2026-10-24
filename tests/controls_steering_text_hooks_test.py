"""Reversible source hooks for the steering text, before its original diagram."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import re
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
sys.path.insert(0,str(Path(__file__).resolve().parent))
from controls_setup_text_hooks_test import fixtures as setup_fixtures
from controls_text_hooks_test import section,call
from controls_setup_text_hooks import add_controls_setup_text_hooks
from controls_steering_text_hooks import MARKER,add_controls_steering_text_hooks,remove_controls_steering_text_hooks

def fixtures():
    values=setup_fixtures()
    old='\tcmp(esi, static_cast<int32_t>(guest_address(off_4D349C)));\n'
    chart=('\tcmp(esi, static_cast<int32_t>(guest_address(off_4D36A8)));\n'
           '\tif (jnz())\n\t\tgoto loc_41025D;\n'+call('410330')+'loc_41025D:\n')
    assert values['Methods_01.cpp'].count(old)==1
    values['Methods_01.cpp']=values['Methods_01.cpp'].replace(old,chart+old)
    return values

class Tests(unittest.TestCase):
    def test_reversal_and_coexistence(self):
        for name,source in fixtures().items():
            for nl in ('\n','\r\n'):
                original=source.replace('\n',nl)
                for before in (original,add_controls_setup_text_hooks(original,name)):
                    after=add_controls_steering_text_hooks(before,name)
                    self.assertEqual(remove_controls_steering_text_hooks(after),before)
                    with self.assertRaises(ValueError):add_controls_steering_text_hooks(after,name)
                    blocks=re.findall(r'// '+MARKER+r'_BEGIN:[^\r\n]+\r?\n(.*?)// '+MARKER+r'_END:',after,re.S)
                    self.assertNotIn('guest_store',''.join(blocks))
                    self.assertNotIn('enqueue',''.join(blocks))
    def test_text_finishes_before_diagram_and_no_other_child_owner(self):
        original=fixtures()['Methods_01.cpp'];after=add_controls_steering_text_hooks(original,'Methods_01.cpp');body=section(after,'4101C0')
        self.assertLess(body.index('_sub_405B60()'),body.index('::beginTail'))
        self.assertLess(body.index('::beginTail'),body.index('_sub_40FA90()'))
        self.assertLess(body.index('_sub_40FA90()'),body.index('::finishTail'))
        self.assertLess(body.index('::finishTail'),body.index('_sub_410330()'))
        for symbol in ('403970','403FC0','40B2C0'):
            self.assertEqual(section(original,symbol),section(after,symbol))
        self.assertEqual(after.count('nfs_controls_steering_text::OwnerScope'),1)
    def test_refuses_changed_or_reordered_original_anchors(self):
        original=fixtures()['Methods_01.cpp']
        for token in ('Game::_sub_4101C0','off_4D36A8','_sub_410330()','_sub_40FA90()','_sub_405C70()'):
            with self.assertRaises(ValueError):add_controls_steering_text_hooks(original.replace(section(original,'4101C0'),section(original,'4101C0').replace(token,'changed',1),1),'Methods_01.cpp')
        with self.assertRaises(ValueError):add_controls_steering_text_hooks(original.replace(call('410330'),'').replace(call('40FA90'),call('410330')+call('40FA90')),'Methods_01.cpp')
    def test_original_store_guards_and_unused_unit(self):
        for name in ('Methods_13.cpp','Methods_14.cpp'):
            with self.assertRaises(ValueError):add_controls_steering_text_hooks(fixtures()[name].replace('guest_store<int16_t>','changed_store<int16_t>',1),name)
        self.assertEqual(add_controls_steering_text_hooks('untouched','Methods_02.cpp'),'untouched')
if __name__=='__main__':unittest.main()

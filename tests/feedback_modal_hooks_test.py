# SPDX-License-Identifier: MIT
"""Synthetic hook structure tests; no original game code or assets embedded."""
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from feedback_modal_hooks import add_feedback_modal_hooks as add, remove_feedback_modal_hooks as remove

def call(symbol):
    return '\tesp -= 4; _sub_'+symbol+'(); esp += 4; //call\n'
def fn(symbol, body=''):
    return 'Fn(void) Game::_sub_'+symbol+'()\n{\n'+body+'}\n'
def fixture(name):
    header = '#include "FunctionIds.h"\n'
    store = 'guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov\n'
    if name == 'Methods_01.cpp':
        return header+fn('410414', 'loc_4109D5:\n'+call('402E80')+call('405C70')+'loc_410A1E:\n')+fn('402E80')+fn('4054E0',call('4056F0'))+fn('405CF0')
    if name == 'Methods_09.cpp':
        return header+fn('482424')
    if name == 'Methods_10.cpp':
        return header+fn('482DA0')
    if name == 'Methods_13.cpp':
        return header+fn('4A5830', store*4)
    return header+fn('4AC934')+fn('4AC9C8')+fn('4B3EE0',store)

class Hooks(unittest.TestCase):
    def test_all_units_preserve_exact_lf_and_crlf(self):
        for name in ('Methods_01.cpp','Methods_09.cpp','Methods_10.cpp','Methods_13.cpp','Methods_14.cpp'):
            for nl in ('\n','\r\n'):
                with self.subTest(name=name,nl=repr(nl)):
                    original=fixture(name).replace('\n',nl)
                    self.assertEqual(remove(add(original,name)),original)
    def test_arm_spans_original_presentation(self):
        out=add(fixture('Methods_01.cpp'),'Methods_01.cpp')
        self.assertLess(out.index('nfs_feedback_invocation(*this)'),out.index(call('402E80')))
        self.assertLess(out.index(call('405C70')),out.index('presented(*this)'))
        self.assertLess(out.index('presented(*this)'),out.index('loc_410A1E:'))
    def test_duplicate_rejected(self):
        out=add(fixture('Methods_01.cpp'),'Methods_01.cpp')
        with self.assertRaises(ValueError):add(out,'Methods_01.cpp')
    def test_missing_original_call_rejected(self):
        with self.assertRaises(ValueError):add(fixture('Methods_01.cpp').replace(call('405C70'),''),'Methods_01.cpp')
    def test_changed_order_rejected(self):
        original=fixture('Methods_01.cpp').replace(call('402E80')+call('405C70'),call('405C70')+call('402E80'))
        with self.assertRaises(ValueError):add(original,'Methods_01.cpp')
    def test_control_flow_exit_rejected(self):
        original=fixture('Methods_01.cpp').replace(call('402E80'),call('402E80')+'goto loc_410A1E;\n')
        with self.assertRaises(ValueError):add(original,'Methods_01.cpp')
    def test_intervening_scope_close_rejected(self):
        original=fixture('Methods_01.cpp').replace(call('402E80'),call('402E80')+' }\n')
        with self.assertRaises(ValueError):add(original,'Methods_01.cpp')
    def test_guest_lease_rejected(self):
        original=fixture('Methods_01.cpp').replace(call('402E80'),'// checked copy transaction begin\n'+call('402E80')).replace(call('405C70'),call('405C70')+'// checked copy transaction end\n')
        with self.assertRaises(ValueError):add(original,'Methods_01.cpp')
    def test_missing_glyph_store_rejected(self):
        with self.assertRaises(ValueError):add(fixture('Methods_13.cpp').replace('guest_store<int16_t>', 'other_store',1),'Methods_13.cpp')
    def test_mixed_newlines_rejected(self):
        with self.assertRaises(ValueError):add(fixture('Methods_09.cpp').replace('\n','\r\n',1),'Methods_09.cpp')
    def test_unrelated_file_untouched(self):
        self.assertEqual(add('arbitrary original bytes','Methods_02.cpp'),'arbitrary original bytes')
if __name__ == '__main__':unittest.main()

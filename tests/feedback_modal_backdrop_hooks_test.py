# SPDX-License-Identifier: MIT
"""Synthetic original-call structure only; no original assets or engine bodies."""
import sys
import unittest
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from feedback_modal_hooks import add_feedback_modal_hooks as text_add, remove_feedback_modal_hooks as text_remove
from feedback_modal_backdrop_hooks import add_feedback_backdrop_hooks as add, remove_feedback_backdrop_hooks as remove
from feedback_modal_hooks_test import fixture as text_fixture, call, fn

def fixture(name):
    s = text_fixture(name)
    if name == 'Methods_01.cpp':
        s = s.replace(fn('402E80'), fn('402E80', call('428A20')+call('428A20')+'loc_40301B:\n'))
    elif name == 'Methods_09.cpp':
        s += ''.join(fn(n) for n in ('482474','4825A8','48266C','482740'))
    elif name == 'Methods_10.cpp':
        s += ''.join(fn(n) for n in ('482C78','482E00','482EA8','48330C','4833A0'))
        s += ''.join(fn(n, 'guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov\n') for n in ('482938','4829E4','482BD4','482FD4','483070','483288'))
    elif name == 'Methods_13.cpp':
        s += fn('4A10BC', '\tcall(to32i(esp+0x38));\n'*4)
    return s

class Hooks(unittest.TestCase):
    def test_composition_reverses_every_original_byte(self):
        for name in ('Methods_01.cpp','Methods_09.cpp','Methods_10.cpp','Methods_13.cpp'):
            for nl in ('\n','\r\n'):
                with self.subTest(name=name,newline=repr(nl)):
                    original=fixture(name).replace('\n',nl)
                    text=text_add(original,name)
                    out=add(text,name)
                    self.assertEqual(remove(out),text)
                    self.assertEqual(text_remove(remove(out)),original)
    def test_missing_shared_invocation_rejected(self):
        with self.assertRaises(ValueError):add(fixture('Methods_01.cpp'),'Methods_01.cpp')
    def test_duplicate_rejected(self):
        original=text_add(fixture('Methods_09.cpp'),'Methods_09.cpp')
        with self.assertRaises(ValueError):add(add(original,'Methods_09.cpp'),'Methods_09.cpp')
    def test_main_restore_count_changed_rejected(self):
        original=text_add(fixture('Methods_01.cpp'),'Methods_01.cpp').replace(call('428A20'),'',1)
        with self.assertRaises(ValueError):add(original,'Methods_01.cpp')
    def test_pretext_label_changed_rejected(self):
        original=text_add(fixture('Methods_01.cpp'),'Methods_01.cpp').replace('loc_40301B:','loc_other:')
        with self.assertRaises(ValueError):add(original,'Methods_01.cpp')
    def test_dispatch_count_changed_rejected(self):
        original=text_add(fixture('Methods_13.cpp'),'Methods_13.cpp').replace('\tcall(to32i(esp+0x38));','',1)
        with self.assertRaises(ValueError):add(original,'Methods_13.cpp')
    def test_known_store_changed_rejected(self):
        original=text_add(fixture('Methods_10.cpp'),'Methods_10.cpp').replace('guest_store<int16_t>', 'changed_store',1)
        with self.assertRaises(ValueError):add(original,'Methods_10.cpp')
    def test_observation_inside_guest_lease_rejected(self):
        original=text_add(fixture('Methods_13.cpp'),'Methods_13.cpp')
        original=original.replace('\tcall(to32i(esp+0x38));','// checked copy transaction begin\n\tcall(to32i(esp+0x38));\n// checked copy transaction end',1)
        with self.assertRaises(ValueError):add(original,'Methods_13.cpp')
    def test_unrelated_unit_unchanged(self):
        original=fixture('Methods_14.cpp')
        self.assertEqual(add(original,'Methods_14.cpp'),original)

if __name__ == '__main__':unittest.main()

"""Synthetic original-source fixtures; no generated game or collector execution."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import re
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'scripts'))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from controls_setup_text_hooks import (MARKER, NAMES, add_controls_setup_text_hooks,
                                      remove_controls_setup_text_hooks)
from controls_text_hooks import add_controls_text_hooks
from controls_text_hooks_test import fixtures as parent_fixtures, fn, call, section, HEADER, FOREIGN, LEASE


def fixtures():
    # Match original lexical labels/joins without retaining proprietary source.
    base = parent_fixtures()
    normal = ('loc_41020E:\n'+LEASE+call('405B30')+FOREIGN+call('405B60')+
              '\teax = esi; //mov\n'+call('40FA90')+
              '\tcmp(esi, static_cast<int32_t>(guest_address(off_4D349C)));\n'+
              '\tif (jnz())\n\t\tgoto loc_41030D;\nloc_410269:\n'+
              '\teax = 0x137; //mov\n'+call('4054E0')+
              'loc_410293:\n'+call('405C70')+'\treturn;\nloc_41030D:\n'+
              '\tcmp(esi, static_cast<int32_t>(guest_address(off_4D35E8)));\n'+
              '\tif (jz())\n\t\tgoto loc_410269;\n\tgoto loc_410293;\n')
    alternate = ('loc_4039B5:\n'+LEASE+call('405B30')+FOREIGN+call('405B60')+
                 '\teax = esi; //mov\n'+call('40FA90')+
                 'loc_403A13:\n\teax = 0x137; //mov\n'+call('405580')+
                 call('405C70')+'loc_403A27:\n\treturn;\n')
    prompt = ('\tnfs::NativeControlBindingScope nfs_binding_scope;\n'+LEASE+
              '\tebp = 0x135; //mov\nloc_40419F:\n'+call('405B30')+FOREIGN+
              call('405B60')+'\teax = to32i(esp+0x2C); //mov\n'+call('40FA90')+
              '\teax = 0x120; //mov\n'+call('41B710')+call('482424')+
              '\tif (jnz())\n\t\tgoto loc_4042D1;\nloc_40421C:\n'+
              '\teax = ebp; //mov\n'+call('405580')+call('405C70')+
              '\treturn;\nloc_4042D1:\n'+call('482424')+'\tgoto loc_40421C;\n')
    main = base['Methods_01.cpp']
    main = main.replace(section(main, '4101C0'), fn('4101C0', normal).rstrip('\n'))
    main = main.replace(section(main, '403FC0'), fn('403FC0', prompt).rstrip('\n'))
    main += fn('403970', alternate)+fn('40B2C0', ' ORIGINAL_SLIDER_UNCHANGED();\n')
    base['Methods_01.cpp'] = main
    base['Methods_03.cpp'] = HEADER+fn('423B90', LEASE+' ORIGINAL_INPUT_UNCHANGED();\n')
    # The supported input is the normal already-hooked production preparation.
    return {name: add_controls_text_hooks(source, name) for name, source in base.items()}


class ControlsSetupTextHooks(unittest.TestCase):
    def test_reversal_preserves_parent_observations_calls_stores_and_newlines(self):
        for name, base in fixtures().items():
            for source in (base, base.replace('\n', '\r\n')):
                with self.subTest(name=name, crlf='\r\n' in source):
                    after = add_controls_setup_text_hooks(source, name)
                    self.assertEqual(remove_controls_setup_text_hooks(after).encode(), source.encode())
                    calls = r'\besp -= 4; _sub_[A-F0-9]+\(\); esp \+= 4; //call'
                    self.assertEqual(re.findall(calls, after), re.findall(calls, source))
                    self.assertEqual(after.count('guest_store<'), source.count('guest_store<'))
                    self.assertEqual(after.count('nfs_controls_text::'), source.count('nfs_controls_text::'))
                    self.assertEqual(after.count('existing_observation'), source.count('existing_observation'))
                    self.assertNotIn('enqueue_renderer', after)
                    self.assertNotIn('guest_store', '\n'.join(re.findall(
                        r'// '+MARKER+r'_BEGIN:[^\n]+\n(.*?)// '+MARKER+r'_END:', after, re.S)))

    def test_manual_tail_includes_appended_text_and_shared_prompt_join(self):
        source = fixtures()['Methods_01.cpp']
        after = add_controls_setup_text_hooks(source, 'Methods_01.cpp')
        self.assertEqual(after.count('nfs_controls_setup_text::beginTail'), 3)
        self.assertEqual(after.count('nfs_controls_setup_text::finishTail'), 3)
        self.assertNotIn('nfs_controls_setup_text::TailScope', after)
        for symbol, label, appended in (('4101C0', 'loc_410293:', '_sub_4054E0()'),
                                         ('403970', 'loc_403A13:', '_sub_405580()'),
                                         ('403FC0', 'loc_40421C:', '_sub_405580()')):
            body = section(after, symbol)
            self.assertLess(body.index('_sub_405B60()'), body.index('nfs_controls_setup_text::beginTail'))
            self.assertLess(body.index('nfs_controls_setup_text::beginTail'), body.index('_sub_40FA90()'))
            self.assertLess(body.index(appended), body.index('nfs_controls_setup_text::finishTail'))
            self.assertLess(body.index(label), body.index('nfs_controls_setup_text::finishTail'))
            self.assertLess(body.index('nfs_controls_setup_text::finishTail'), body.index('_sub_405C70()'))
        prompt = section(after, '403FC0')
        self.assertLess(prompt.index('NativeControlBindingScope'), prompt.index('nfs_controls_setup_text::PromptScope'))
        for symbol in ('4101C0', '403970'):
            self.assertIn('OwnerScope nfs_controls_setup_owner(*this,0x'+symbol.lower()+'u)', section(after, symbol))
        self.assertIn('PolicyScope nfs_controls_setup_policy(*this,true)', after)
        self.assertIn('PolicyScope nfs_controls_setup_policy(*this,false)', after)
        policy = section(after, '405580')
        self.assertLess(policy.index('_sub_41B690()'), policy.index('nfs_controls_setup_text::resourceAlignment'))
        for symbol in ('405B30', '405B60', '40B2C0', '414000', '414200'):
            self.assertEqual(section(source, symbol), section(after, symbol))

    def test_low_level_store_selection_retirement_and_include_only_methods03(self):
        for name, count in (('Methods_13.cpp', 4), ('Methods_14.cpp', 1)):
            source = fixtures()[name]
            after = add_controls_setup_text_hooks(source, name)
            self.assertEqual(after.count('nfs_controls_setup_text::WordCapture'), count)
            with self.assertRaises(ValueError):
                add_controls_setup_text_hooks(source.replace('guest_store<int16_t>', 'other_store<int16_t>', 1), name)
        three = fixtures()['Methods_03.cpp']
        after = add_controls_setup_text_hooks(three, 'Methods_03.cpp')
        self.assertEqual(section(three, '423B90'), section(after, '423B90'))
        self.assertNotIn('nfs_controls_setup_text::', after)
        nine = add_controls_setup_text_hooks(fixtures()['Methods_09.cpp'], 'Methods_09.cpp')
        self.assertIn('nfs_controls_setup_text::SelectionScope', nine)
        ten = add_controls_setup_text_hooks(fixtures()['Methods_10.cpp'], 'Methods_10.cpp')
        self.assertIn('nfs_controls_setup_text::beforeDescriptorRetire', ten)

    def test_missing_changed_duplicate_and_reordered_source_anchors_decline(self):
        source = fixtures()['Methods_01.cpp']
        for token in ('#include "FunctionIds.h"', 'Game::_sub_4101C0', 'Game::_sub_403970',
                      'Game::_sub_403FC0', 'Game::_sub_405CF0', 'Game::_sub_4054E0',
                      'loc_41020E:', 'loc_410269:', 'loc_410293:', 'loc_41030D:',
                      'loc_4039B5:', 'loc_403A13:', 'loc_40419F:', 'loc_40421C:',
                      'loc_4042D1:', 'off_4D349C', 'off_4D35E8',
                      'NativeControlBindingScope nfs_binding_scope;',
                      'eax = 0x137; //mov', 'eax = 0x120; //mov', 'ebp = 0x135; //mov'):
            with self.subTest(token=token), self.assertRaises(ValueError):
                add_controls_setup_text_hooks(source.replace(token, token+'changed', 1), 'Methods_01.cpp')
        for symbol in ('4101C0', '403970', '403FC0', '405580'):
            with self.assertRaises(ValueError):
                add_controls_setup_text_hooks(source+fn(symbol), 'Methods_01.cpp')
        for symbol in ('405B30', '405B60', '40FA90', '405C70'):
            changed = source.replace(call(symbol), call(symbol)+call(symbol), 1)
            with self.subTest(symbol=symbol), self.assertRaises(ValueError):
                add_controls_setup_text_hooks(changed, 'Methods_01.cpp')
        changed = source.replace(call('405B30'), ' TEMP_RESET();\n', 1).replace(call('405B60'), call('405B60')+call('405B30'), 1)
        with self.assertRaises(ValueError):
            add_controls_setup_text_hooks(changed, 'Methods_01.cpp')

    def test_checked_copy_leases_refuse_every_new_observation_family(self):
        for name, source in fixtures().items():
            # Put even the helper include inside the lease. Methods03 has no
            # runtime hooks: opening a lease AFTER its include does not place
            # any injected observation inside that lease and is not this test.
            with self.subTest(name=name), self.assertRaises(ValueError):
                add_controls_setup_text_hooks('// checked copy transaction begin\n'+source, name)
        source = fixtures()['Methods_01.cpp']
        for anchor in (call('40FA90'), call('405C70'), call('41B690')):
            changed = source.replace(anchor, '// checked copy transaction begin\n'+anchor+'// checked copy transaction end\n', 1)
            with self.assertRaises(ValueError):
                add_controls_setup_text_hooks(changed, 'Methods_01.cpp')
        for name in ('Methods_13.cpp', 'Methods_14.cpp'):
            source = fixtures()[name]
            store = 'guest_store<int16_t>(esi, static_cast<int16_t>(bx)); //mov'
            with self.assertRaises(ValueError):
                add_controls_setup_text_hooks(source.replace(store, '// checked copy transaction begin\n'+store+'\n// checked copy transaction end', 1), name)

    def test_duplicate_application_and_unmatched_removal_refuse(self):
        for name, source in fixtures().items():
            after = add_controls_setup_text_hooks(source, name)
            with self.assertRaises(ValueError):
                add_controls_setup_text_hooks(after, name)
            with self.assertRaises(ValueError):
                remove_controls_setup_text_hooks(after.replace(MARKER+'_END:include', MARKER+'_END:wrong', 1))
        self.assertEqual(add_controls_setup_text_hooks('unchanged', 'Other.cpp'), 'unchanged')
        self.assertEqual(remove_controls_setup_text_hooks('unchanged'), 'unchanged')
        self.assertEqual(len(NAMES), 6)


if __name__ == '__main__':
    unittest.main()

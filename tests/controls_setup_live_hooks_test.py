"""Synthetic original-call fixtures for independent child complete640 hooks."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import re
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'scripts'))
sys.path.insert(0, str(Path(__file__).resolve().parent))
from controls_setup_live_hooks import (MARKER, NAMES, add_controls_setup_live_hooks,
                                      remove_controls_setup_live_hooks)
from controls_setup_sidebar_hooks_test import fixtures as sidebar_fixtures
from controls_setup_sidebar_hooks import add_controls_setup_sidebar_hooks
from controls_text_hooks_test import section, call, fn


def fixtures():
    sources = {name: add_controls_setup_sidebar_hooks(source, name)
               for name, source in sidebar_fixtures().items()}
    text = sources['Methods_01.cpp']
    for symbol in ('4101C0', '403970', '403FC0'):
        part = section(text, symbol)
        original = part
        prefix = call('428A20')+'\tedx = 0x400; //mov\n'
        if symbol == '403FC0':
            prefix += ('\teax = to32i(esp+0x2C); //mov\n'
                       '\teax = to32i(eax+4); //mov\n'
                       '\tecx = edi; //mov\n\tebx = edi; //mov\n')
            zeros = '\tecx = edi; //mov\n\tebx = edi; //mov\n'
        else:
            prefix = call('40EB20')+prefix
            prefix += '\teax = to32i(esi+4); //mov\n\txor_(ecx, ecx);\n\txor_(ebx, ebx);\n'
            zeros = '\txor_(ecx, ecx);\n\txor_(ebx, ebx);\n'
        part = part.replace(call('405B30'), prefix+call('405B30'), 1)
        prefix = ('\teax = to32i(dword_4D2314+2); //mov\n'
                  '\tsar(eax, (int32_t)0x10);\n\tedx = 0x1000; //mov\n'
                  '\teax = to32i(dword_4D231C+eax*4); //mov\n'+zeros+
                  '\teax = to32i(eax+0x20); //mov\n')
        part = part.replace(call('405B60'), prefix+call('405B60'), 1)
        text = text.replace(original, part, 1)
    sources['Methods_01.cpp'] = text
    return sources


class ControlsSetupLiveHooks(unittest.TestCase):
    def test_exact_reversal_all_units_endings_and_originals(self):
        for name, base in fixtures().items():
            for source in (base, base.replace('\n', '\r\n')):
                with self.subTest(name=name, crlf='\r\n' in source):
                    after = add_controls_setup_live_hooks(source, name)
                    self.assertEqual(remove_controls_setup_live_hooks(after).encode(), source.encode())
                    calls = r'\besp -= 4; _sub_[A-F0-9]+\(\); esp \+= 4; //call'
                    self.assertEqual(re.findall(calls, source), re.findall(calls, after))
                    for token in ('guest_store<', 'existing_observation', 'nfs_controls_text::',
                                  'nfs_controls_setup_text::', 'nfs_controls_setup_sidebar_live::',
                                  'nfs_controls_setup_sidebar_text::'):
                        self.assertEqual(after.count(token), source.count(token))
                    for forbidden in ('enqueue_renderer', 'activeRequest', 'probeCompleted', 'dumpFile'):
                        self.assertNotIn(forbidden, after)

    def test_owners_and_manual_complete_tail_order(self):
        after = add_controls_setup_live_hooks(fixtures()['Methods_01.cpp'], 'Methods_01.cpp')
        for symbol in ('4101C0', '403970'):
            part = section(after, symbol)
            self.assertLess(part.index('nfs_controls_setup_text::OwnerScope'), part.index('nfs_controls_setup_live::OwnerScope'))
            self.assertLess(part.index('nfs_controls_setup_live::OwnerScope'), part.index('nfs_controls_setup_sidebar_text::OwnerScope'))
        self.assertNotIn('nfs_controls_setup_live::OwnerScope', section(after, '403FC0'))
        for symbol, appendix in (('4101C0', '_sub_4054E0()'), ('403970', '_sub_405580()'), ('403FC0', '_sub_405580()')):
            part = section(after, symbol)
            self.assertLess(part.index('nfs_controls_setup_live::ResetScope'), part.index('_sub_405B30()'))
            self.assertLess(part.index('nfs_controls_setup_live::SidebarScope'), part.index('_sub_405B60()'))
            self.assertLess(part.index('_sub_405B60()'), part.index('nfs_controls_setup_live::beginTail'))
            self.assertLess(part.index('nfs_controls_setup_live::beginTail'), part.index('nfs_controls_setup_text::beginTail'))
            self.assertLess(part.index(appendix), part.index('nfs_controls_setup_text::finishTail'))
            self.assertLess(part.index('nfs_controls_setup_text::finishTail'), part.index('nfs_controls_setup_live::finishTail'))
            self.assertLess(part.index('nfs_controls_setup_live::finishTail'), part.index('_sub_405C70()'))
            self.assertNotIn('nfs_controls_setup_live::TailScope', part)
        part = section(after, '405CF0')
        self.assertLess(part.index('nfs_controls_setup_text::StringScope'), part.index('nfs_controls_setup_live::bindText'))
        self.assertIn('nfs_controls_setup_live::ResolvedScope', section(after, '405B60'))

    def test_retirement_after_setup_invalidation(self):
        for name, symbols in (('Methods_10.cpp', (('482DA0', 'beforeDescriptorRetire'),)),
                              ('Methods_14.cpp', (('4AC934', 'beforeOwnerRetire'), ('4AC9C8', 'beforeDescriptorRetire')))):
            after = add_controls_setup_live_hooks(fixtures()[name], name)
            for symbol, hook in symbols:
                part = section(after, symbol)
                self.assertLess(part.index('nfs_controls_setup_text::'+hook), part.index('nfs_controls_setup_live::afterRetire'))

    def test_unrelated_input_glyphs_and_sidebar_render_preserved(self):
        for name, source in fixtures().items():
            after = add_controls_setup_live_hooks(source, name)
            symbols = {'Methods_01.cpp': ('405580', '4054E0', '405B30', '40BBE0', '40C110', '414000', '414200', '40B2C0'),
                       'Methods_03.cpp': ('423B90',), 'Methods_09.cpp': ('4824CC', '414400'),
                       'Methods_13.cpp': ('4A5830', '4A14D4', '4A10BC')}.get(name, ())
            for symbol in symbols:
                self.assertEqual(section(after, symbol), section(source, symbol))

    def test_missing_duplicate_changed_order_and_dependencies_decline(self):
        source = fixtures()['Methods_01.cpp']
        for symbol in ('4101C0', '403970', '403FC0'):
            part = section(source, symbol)
            for token in ('edx = 0x400;', 'dword_4D2314+2', 'dword_4D231C+eax*4', 'eax+0x20',
                          'edx = 0x1000;', 'sar(eax, (int32_t)0x10);',
                          ' nfs_controls_setup_text::beginTail(*this);',
                          ' nfs_controls_setup_text::finishTail(*this);'):
                changed = source.replace(part, part.replace(token, 'changed_anchor', 1), 1)
                with self.subTest(symbol=symbol, token=token), self.assertRaises(ValueError):
                    add_controls_setup_live_hooks(changed, 'Methods_01.cpp')
            for callee in ('405B30', '405B60', '40FA90', '405C70'):
                changed = source.replace(part, part.replace(call(callee), call(callee)*2, 1), 1)
                with self.assertRaises(ValueError):
                    add_controls_setup_live_hooks(changed, 'Methods_01.cpp')
            with self.assertRaises(ValueError):
                add_controls_setup_live_hooks(source+fn(symbol), 'Methods_01.cpp')
            begin = ' nfs_controls_setup_text::beginTail(*this);'
            changed = part.replace(begin, '', 1).replace(call('405C70'), begin+'\n'+call('405C70'), 1)
            with self.assertRaises(ValueError):
                add_controls_setup_live_hooks(source.replace(part, changed, 1), 'Methods_01.cpp')
        for dependency in ('ControlsSetupTextGameHooks.inc', 'ControlsSetupSidebarLiveGameHooks.inc'):
            with self.assertRaises(ValueError):
                add_controls_setup_live_hooks(source.replace(dependency, 'Missing.inc', 1), 'Methods_01.cpp')

    def test_lease_reentry_mixed_endings_and_damaged_removal_decline(self):
        for name, source in fixtures().items():
            if name not in NAMES:
                continue
            after = add_controls_setup_live_hooks(source, name)
            with self.assertRaises(ValueError):
                add_controls_setup_live_hooks(after, name)
            with self.assertRaises(ValueError):
                remove_controls_setup_live_hooks(after.replace(MARKER+'_END:include', MARKER+'_END:broken', 1))
            with self.assertRaises(ValueError):
                add_controls_setup_live_hooks('// checked copy transaction begin\n'+source, name)
        source = fixtures()['Methods_01.cpp']
        for token in (' nfs_controls_setup_text::StringScope nfs_controls_setup_string(*this);',
                      ' nfs_controls_setup_text::beginTail(*this);',
                      ' nfs_controls_setup_text::finishTail(*this);', call('405B30').rstrip('\n'), call('405B60').rstrip('\n')):
            changed = source.replace(token, '// checked copy transaction begin\n'+token+'\n// checked copy transaction end', 1)
            with self.subTest(token=token), self.assertRaises(ValueError):
                add_controls_setup_live_hooks(changed, 'Methods_01.cpp')
        with self.assertRaises(ValueError):
            add_controls_setup_live_hooks(source.replace('\n', '\r\n', 1), 'Methods_01.cpp')
        self.assertEqual(add_controls_setup_live_hooks('unchanged', 'Methods_03.cpp'), 'unchanged')
        self.assertEqual(remove_controls_setup_live_hooks('unchanged'), 'unchanged')
        self.assertEqual(len(NAMES), 5)


if __name__ == '__main__':
    unittest.main()

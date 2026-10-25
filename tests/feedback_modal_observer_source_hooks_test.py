"""Synthetic structural tests; no Joined/source/native receipt is fabricated."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import re
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from feedback_modal_observer_source_hooks import (
    MARKER, NAMES, add_feedback_modal_observer_source_hooks as add,
    remove_feedback_modal_observer_source_hooks as remove)
from controls_scene_hooks import add_controls_scene_hooks
from graphics_scene_hooks import add_graphics_scene_hooks
from audio_scene_hooks import add_audio_scene_hooks

HEADER = '#include "FunctionIds.h"\n'
LEASE = '\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction tx;\n\t} // checked copy transaction end\n'


def call(symbol):
    return '\tesp -= 4; '+symbol+'(); esp += 4; //call\n'


def source():
    # Independently transcribed finite operation/loop skeleton. This is not an
    # executable game fixture or evidence of actual Joined/native admission.
    body = '\tpush32(ebx);\n'+LEASE+'\txor_(edi, edi);\nloc_42697D:\n'+LEASE+'\txor_(esi, esi);\nloc_4269A7:\n'+LEASE
    body += call('___CHP')+LEASE+call('_sub_4850B0')+LEASE+call('_sub_49755C')+call('_sub_497738')
    body += '\tadd(esi, (int32_t)0x20);\n'+call('_sub_497D6C')
    body += '\tcmp(esi, (int32_t)0x1E0);\n\tif (jnz())\n\t\tgoto loc_4269A7;\n\tadd(edi, (int32_t)0x20);\n\tcmp(edi, (int32_t)0x280);\n\tif (jnz())\n\t\tgoto loc_42697D;\n'
    body += '\tadd(esp, (int32_t)4);\n\tpop32(ebp);\n\tpop32(edi);\n\tpop32(esi);\n\tpop32(edx);\n\tpop32(ecx);\n\tpop32(ebx);\n'
    return HEADER+'Fn(void) Game::_sub_426950()\n{\n'+body+'}\nFn(void) Game::_sub_426A60()\n{\n return;\n}\n'


def controls_only():
    return add_controls_scene_hooks(source(), 'Methods_03.cpp')


def composed():
    s = add_audio_scene_hooks(source(), 'Methods_03.cpp')
    s = add_graphics_scene_hooks(s, 'Methods_03.cpp')
    return add_controls_scene_hooks(s, 'Methods_03.cpp')


class FeedbackNativeSourceHooks(unittest.TestCase):
    def test_exact_lf_crlf_reversal_and_original_operations(self):
        for base in (controls_only(), composed()):
            for nl in ('\n', '\r\n'):
                with self.subTest(composed=base == composed(), newline=repr(nl)):
                    original = base.replace('\n', nl)
                    out = add(original, 'Methods_03.cpp')
                    self.assertEqual(remove(out).encode(), original.encode())
                    pattern = r'\besp -= 4; (?:_sub_|___)[A-Za-z0-9]+\(\); esp \+= 4; //call'
                    self.assertEqual(re.findall(pattern, out), re.findall(pattern, original))
                    self.assertEqual(out.count('GuestCopyTransaction'), original.count('GuestCopyTransaction'))
                    self.assertEqual(out[out.index('Fn(void) Game::_sub_426A60'):], original[original.index('Fn(void) Game::_sub_426A60'):])

    def test_native_scope_precedes_all_existing_intents(self):
        base = composed()
        out = add(base, 'Methods_03.cpp')
        scope = out.index('NativeScope nfs_feedback_modal_native(*this);')
        for namespace in ('nfs_controls_scene_live', 'nfs_graphics_scene_live', 'nfs_audio_scene_live'):
            self.assertLess(scope, out.index(namespace+'::Intent'))
            self.assertEqual(out.count(namespace), base.count(namespace))
        self.assertEqual(out.count('NativeScope nfs_feedback_modal_native(*this);'), 1)
        for forbidden in ('Joined::', 'activeRequest()=', 'nextRequest()', 'enqueue_owned_renderer', 'guest_store', 'take_receipt', 'sourceProof'):
            self.assertNotIn(forbidden, out)

    def test_requires_existing_controls_wrappers(self):
        with self.assertRaises(ValueError):
            add(source(), 'Methods_03.cpp')
        for token in ('NFS_CONTROLS_SCENE_BEGIN:begin', 'NFS_CONTROLS_SCENE_BEGIN:finish', 'nfs_controls_scene_live::begin(', 'nfs_controls_scene_live::finish('):
            with self.subTest(token=token), self.assertRaises(ValueError):
                add(controls_only().replace(token, 'changed-wrapper', 1), 'Methods_03.cpp')

    def test_missing_duplicate_original_operations_decline(self):
        tokens = ('#include "FunctionIds.h"', 'Game::_sub_426950', 'push32(ebx);', 'xor_(edi, edi);', 'xor_(esi, esi);', 'loc_42697D:', 'loc_4269A7:', '___CHP();', '_sub_4850B0();', '_sub_49755C();', '_sub_497738();', '_sub_497D6C();', 'add(esi, (int32_t)0x20);', 'cmp(esi, (int32_t)0x1E0);', 'goto loc_4269A7;', 'add(edi, (int32_t)0x20);', 'cmp(edi, (int32_t)0x280);', 'goto loc_42697D;', 'pop32(ebx);')
        for token in tokens:
            with self.subTest(token=token), self.assertRaises(ValueError):
                add(controls_only().replace(token, 'changed-original', 1), 'Methods_03.cpp')
        with self.assertRaises(ValueError):
            add(controls_only()+controls_only(), 'Methods_03.cpp')
        duplicated = controls_only().replace(call('_sub_49755C'), call('_sub_49755C')*2)
        with self.assertRaises(ValueError):
            add(duplicated, 'Methods_03.cpp')

    def test_reordered_upload_select_triangle_decline(self):
        base = controls_only()
        for first, second in (('_sub_4850B0', '_sub_49755C'), ('_sub_49755C', '_sub_497738'), ('_sub_497738', '_sub_497D6C')):
            changed = base.replace(first, 'temporary_symbol').replace(second, first).replace('temporary_symbol', second)
            with self.subTest(first=first), self.assertRaises(ValueError):
                add(changed, 'Methods_03.cpp')

    def test_loop_condition_and_extra_jump_decline(self):
        base = controls_only()
        for changed in (base.replace('if (jnz())', 'if (jz())', 1), base.replace('if (jnz())', 'if (jbe())', 1), base.replace('\tpop32(ebx);', '\tgoto loc_42697D;\n\tpop32(ebx);'), base.replace('\tpop32(ebx);', '\tgoto unexpected_exit;\n\tpop32(ebx);')):
            with self.assertRaises(ValueError):
                add(changed, 'Methods_03.cpp')

    def test_early_exits_and_unbalanced_scopes_decline(self):
        base = controls_only()
        for exit in ('return;', 'co_return;', 'break;', 'continue;'):
            with self.subTest(exit=exit), self.assertRaises(ValueError):
                add(base.replace('\tpop32(ebx);', exit+'\n\tpop32(ebx);'), 'Methods_03.cpp')
        for changed in (base.replace('\tpop32(ebx);', '}\n\tpop32(ebx);'), base.replace('\tpush32(ebx);', '{\n\tpush32(ebx);')):
            with self.assertRaises(ValueError):
                add(changed, 'Methods_03.cpp')

    def test_operations_hidden_in_comments_decline(self):
        base = controls_only()
        for token in (call('_sub_4850B0'), '\txor_(edi, edi);', '\tpop32(ebx);'):
            with self.subTest(token=token), self.assertRaises(ValueError):
                add(base.replace(token, '/* '+token+' */'), 'Methods_03.cpp')

    def test_guest_copy_and_mixed_newlines_decline(self):
        base = controls_only()
        for changed in ('// checked copy transaction begin\n'+base, base.replace('\tpush32(ebx);', '// checked copy transaction end\n\tpush32(ebx);'), base.replace('\tpop32(ebx);', '// checked copy transaction begin\n\tpop32(ebx);'), base.replace('\n', '\r\n', 1)):
            with self.assertRaises(ValueError):
                add(changed, 'Methods_03.cpp')

    def test_duplicate_malformed_and_modified_markers_decline(self):
        out = add(controls_only(), 'Methods_03.cpp')
        with self.assertRaises(ValueError):
            add(out, 'Methods_03.cpp')
        for changed in (out.replace(MARKER+'_END:native-scope', MARKER+'_END:broken'), out.replace('NativeScope nfs_feedback_modal_native(*this);', 'NativeScope changed(*this);'), out+'\n// '+MARKER+'_BEGIN:orphan\n'):
            with self.assertRaises(ValueError):
                remove(changed)
        include_start = out.index('\n// '+MARKER+'_BEGIN:include')
        include_end = out.index('// '+MARKER+'_END:include\n', include_start)+len('// '+MARKER+'_END:include\n')
        with self.assertRaises(ValueError):
            remove(out+out[include_start:include_end])

    def test_other_observers_comments_and_unrelated_source_preserved(self):
        base = composed().replace('Fn(void) Game::_sub_426950()\n{', 'Fn(void) Game::_sub_426950()\n{\n existing_private_observer_scope();\n // A comment may mention return and braces { }.\n const char* diagnostic="return { }";')
        out = add(base, 'Methods_03.cpp')
        self.assertEqual(remove(out), base)
        self.assertEqual(out.count('existing_private_observer_scope();'), 1)
        self.assertEqual(NAMES, ('Methods_03.cpp',))
        self.assertEqual(add('arbitrary original bytes', 'Methods_01.cpp'), 'arbitrary original bytes')
        self.assertEqual(remove('arbitrary original bytes'), 'arbitrary original bytes')


if __name__ == '__main__':
    unittest.main()

# SPDX-License-Identifier: MIT
"""Synthetic structural source hooks only; no original engine bodies/assets.

No authoritative Joined, parent-update or native receipt is fabricated here.
Actual generated-source/game verification remains a separate required gate.
"""
from pathlib import Path
import sys
import unittest
import tempfile
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'scripts'))
from feedback_modal_scene_hooks import add_feedback_modal_scene_hooks as add, remove_feedback_modal_scene_hooks as remove
from feedback_modal_hooks import add_feedback_modal_hooks as text_add, remove_feedback_modal_hooks as text_remove
from feedback_modal_backdrop_hooks import add_feedback_backdrop_hooks as backdrop_add, remove_feedback_backdrop_hooks as backdrop_remove
from feedback_modal_backdrop_hooks_test import fixture as backdrop_fixture
from feedback_modal_hooks_test import fn, call
from feedback_modal_observer_source_hooks_test import composed as native_fixture
from feedback_modal_observer_source_hooks import add_feedback_modal_observer_source_hooks as native_add, remove_feedback_modal_observer_source_hooks as native_remove


def fixture(name):
    s = backdrop_fixture(name)
    if name == 'Methods_01.cpp':
        tail = ('loc_40C054:\n'+call('4825A8')+
                '\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction tx;\n'
                '\tedx = 0x2800; //mov\n'
                '\teax = guest_transaction.load<int32_t>(dword_4D231C+eax*4); //mov\n'
                '\txor_(ebx, ebx);\n\txor_(ecx, ecx);\n'
                '\teax = guest_transaction.load<int32_t>(eax+0x20); //mov\n'
                '\t} // checked copy transaction end\n'+call('405B60'))
        s += fn('40BBE0', tail)
    elif name == 'Methods_10.cpp':
        s = s.replace(fn('48330C'), fn('48330C', call('4A10BC')))
        s += fn('483438', '\tadd(ebx, ebx);\n\t_sub_4A14D4(); return; //jmp\n')
    elif name == 'Methods_13.cpp':
        s += fn('4A1090', '\tsub(esp, (int32_t)0x1400);\n\tcall(to32i(dword_571264));\n\teax = esp; //mov\n\tebx = ecx; //mov\n\tedx = esi; //mov\n\tcall(to32i(dword_571260));\n')
        s += fn('4A14D4', '\tswap(eax, edx);\n\t_sub_4A14D5(); return; //jmp\n')
    return s


def prior(name):
    return backdrop_add(text_add(fixture(name), name), name)


class FeedbackSceneHooks(unittest.TestCase):
    def test_all_supported_units_lf_crlf_exact_reversal(self):
        for name in ('Methods_01.cpp', 'Methods_09.cpp', 'Methods_10.cpp', 'Methods_13.cpp', 'Methods_14.cpp'):
            for nl in ('\n', '\r\n'):
                with self.subTest(name=name, newline=repr(nl)):
                    original = fixture(name).replace('\n', nl)
                    before = backdrop_add(text_add(original, name), name)
                    after = add(before, name)
                    self.assertEqual(remove(after), before)
                    self.assertEqual(text_remove(backdrop_remove(remove(after))), original)

    def test_invocation_parent_and_join_native_order(self):
        out = add(prior('Methods_01.cpp'), 'Methods_01.cpp')
        order = ('nfs_feedback_invocation(*this)', 'acquireForFeedback(*this)', 'nfs_feedback_text_arm(*this)', call('402E80'), 'nfs_feedback_modal_scene_live::acquire(*this,nfs_feedback_parent)', '::Scope nfs_feedback_observer_scope', call('405C70'), 'presented(*this)')
        positions = [out.index(token) for token in order]
        self.assertEqual(positions, sorted(positions))
        for token in ('private_feedback_', 'checkpoint(*this)', 'newRequest', 'take_receipt', 'sourceProof'):
            self.assertNotIn(token, out)

    def test_other_popup_branches_in_same_function_are_preserved(self):
        original = fixture('Methods_01.cpp')
        # The actual 410414 also has popup/presentation calls in other branches;
        # only loc4109D5..loc410A1E is the unavailable-Feedback Invocation route.
        original = original.replace('loc_4109D5:\n', call('405C70')+'loc_4109D5:\n', 1)
        original = original.replace('loc_410A1E:\n', 'loc_410A1E:\n'+call('402E80')+call('405C70'), 1)
        before = backdrop_add(text_add(original, 'Methods_01.cpp'), 'Methods_01.cpp')
        after = add(before, 'Methods_01.cpp')
        self.assertEqual(remove(after), before)
        self.assertEqual(after.count(call('402E80')), before.count(call('402E80')))
        self.assertEqual(after.count(call('405C70')), before.count(call('405C70')))
        route = after[after.index('loc_4109D5:'):after.index('loc_410A1E:')]
        self.assertEqual(route.count('::Scope nfs_feedback_observer_scope'), 1)
        self.assertNotIn('::Scope nfs_feedback_observer_scope', after[after.index('loc_410A1E:'):])
        # A duplicate inside the authenticated route must still reject.
        changed = before.replace('loc_410A1E:', call('402E80')+'loc_410A1E:', 1)
        with self.assertRaises(ValueError):
            add(changed, 'Methods_01.cpp')

    def test_lazy_last_copy_outside_guest_transaction(self):
        out = add(prior('Methods_01.cpp'), 'Methods_01.cpp')
        self.assertLess(out.index('loc_40C054:'), out.index('CopyScope nfs_feedback_lazy_copy'))
        self.assertLess(out.index('// checked copy transaction end'), out.index('CopyScope nfs_feedback_lazy_copy'))
        self.assertEqual(out.count('CopyScope nfs_feedback_lazy_copy'), 1)

    def test_later_cached_upload_reads_same_table_without_becoming_a_copy(self):
        original = fixture('Methods_01.cpp')
        cleanup = ('loc_40C0E5:\n'
                   '\t{ // checked copy transaction begin\n\tnfs::GuestCopyTransaction tx;\n'
                   '\teax = guest_transaction.load<int32_t>(dword_4D2314+2); //mov\n'
                   '\tsar(eax, (int32_t)0x10);\n'
                   '\teax = guest_transaction.load<int32_t>(dword_4D231C+eax*4); //mov\n'
                   '\txor_(edx, edx);\n'
                   '\teax = guest_transaction.load<int32_t>(eax+0x20); //mov\n'
                   '\t} // checked copy transaction end\n'+call('428250'))
        original = original.replace(call('405B60'), call('405B60')+cleanup, 1)
        before = backdrop_add(text_add(original, 'Methods_01.cpp'), 'Methods_01.cpp')
        after = add(before, 'Methods_01.cpp')
        self.assertEqual(remove(after), before)
        self.assertEqual(after.count('CopyScope nfs_feedback_lazy_copy'), 1)
        self.assertLess(after.index('CopyScope nfs_feedback_lazy_copy'), after.index('loc_40C0E5:'))
        self.assertEqual(after[after.index('loc_40C0E5:'):], before[before.index('loc_40C0E5:'):])
        # Duplicating a source binding inside the actual pre-copy setup still
        # fails closed, even though later upload uses are permitted.
        table = '\teax = guest_transaction.load<int32_t>(dword_4D231C+eax*4); //mov'
        with self.assertRaises(ValueError):
            add(before.replace(table, table+'\n'+table, 1), 'Methods_01.cpp')

    def test_all_actual_callback_scopes_present(self):
        ten = add(prior('Methods_10.cpp'), 'Methods_10.cpp')
        self.assertEqual(ten.count('::ResolveScope'), 1)
        self.assertEqual(ten.count('::DispatchScope'), 1)
        thirteen = add(prior('Methods_13.cpp'), 'Methods_13.cpp')
        self.assertEqual(thirteen.count('::TransformScope'), 1)
        self.assertEqual(thirteen.count('.beforeTransform();'), 1)
        self.assertEqual(thirteen.count('.afterTransform();'), 1)
        self.assertEqual(thirteen.count('.beforeForward();'), 1)
        self.assertEqual(thirteen.count('.afterForward();'), 1)
        self.assertEqual(thirteen.count('::observeIndirect(*this);'), 4)
        self.assertEqual(thirteen.count('::ByteScope'), 1)

    def test_transform_and_forward_complete_after_actual_original_calls(self):
        for newline in ('\n', '\r\n'):
            with self.subTest(newline=repr(newline)):
                before = prior('Methods_13.cpp').replace('\n', newline)
                after = add(before, 'Methods_13.cpp')
                normalized = after.replace('\r\n', '\n')
                start = normalized.index('Fn(void) Game::_sub_4A1090()')
                end = normalized.index('\nFn(', start+1)
                wrapper = normalized[start:end]
                transform = '\tcall(to32i(dword_571264));'
                forward = '\tcall(to32i(dword_571260));'
                order = ('nfs_feedback_lazy_transform.beforeTransform();', transform,
                         'nfs_feedback_lazy_transform.afterTransform();',
                         '\teax = esp; //mov', '\tebx = ecx; //mov', '\tedx = esi; //mov',
                         'nfs_feedback_lazy_transform.beforeForward();', forward,
                         'nfs_feedback_lazy_transform.afterForward();')
                positions = [wrapper.index(token) for token in order]
                self.assertEqual(positions, sorted(positions))
                for token in order:
                    self.assertEqual(wrapper.count(token), 1)
                self.assertEqual(after.count(transform), before.count(transform))
                self.assertEqual(after.count(forward), before.count(forward))
                self.assertEqual(remove(after), before)

    def test_after_callbacks_cannot_be_inserted_inside_original_guest_lease(self):
        before = prior('Methods_13.cpp')
        for original_call in ('\tcall(to32i(dword_571264));', '\tcall(to32i(dword_571260));'):
            with self.subTest(original_call=original_call):
                changed = before.replace(original_call, '// checked copy transaction begin\n'+original_call+'\n// checked copy transaction end\n', 1)
                with self.assertRaises(ValueError):
                    add(changed, 'Methods_13.cpp')

    def test_existing_native_transform_remains_composable(self):
        original = native_fixture()
        self.assertEqual(add(original, 'Methods_03.cpp'), original)
        self.assertEqual(native_remove(native_add(add(original, 'Methods_03.cpp'), 'Methods_03.cpp')), original)

    def test_missing_and_duplicate_actual_sites_reject(self):
        for name, token in (('Methods_01.cpp', 'loc_40C054:'), ('Methods_01.cpp', call('405B60')), ('Methods_10.cpp', call('4A10BC')), ('Methods_10.cpp', '\tadd(ebx, ebx);'), ('Methods_13.cpp', '\tcall(to32i(dword_571264));'), ('Methods_13.cpp', '\tcall(to32i(dword_571260));'), ('Methods_13.cpp', '\tswap(eax, edx);')):
            before = prior(name)
            with self.subTest(name=name, token=token), self.assertRaises(ValueError):
                add(before.replace(token, 'changed-site', 1), name)
            with self.subTest(name=name, duplicate=token), self.assertRaises(ValueError):
                add(before.replace(token, token+token, 1), name)

    def test_indirect_branch_count_rejects(self):
        before = prior('Methods_13.cpp')
        for changed in ('', '\tcall(to32i(esp+0x38));'*2):
            with self.assertRaises(ValueError):
                add(before.replace('\tcall(to32i(esp+0x38));', changed, 1), 'Methods_13.cpp')

    def test_popup_scope_order_and_guest_lease_rejects(self):
        before = prior('Methods_01.cpp')
        for changed in (before.replace('nfs_feedback_invocation(*this)', 'changed-invocation', 1), before.replace(call('402E80'), call('402E80')+' }\n', 1), before.replace(call('402E80'), '// checked copy transaction begin\n'+call('402E80'), 1).replace(call('405C70'), call('405C70')+'// checked copy transaction end\n', 1)):
            with self.assertRaises(ValueError):
                add(changed, 'Methods_01.cpp')

    def test_duplicate_unmatched_and_mixed_newlines_rejects(self):
        before = prior('Methods_10.cpp')
        after = add(before, 'Methods_10.cpp')
        with self.assertRaises(ValueError):
            add(after, 'Methods_10.cpp')
        with self.assertRaises(ValueError):
            remove(after.replace('_END:resolve', '_END:broken', 1))
        with self.assertRaises(ValueError):
            add(before.replace('\n', '\r\n', 1), 'Methods_10.cpp')

    def test_unrelated_units_unchanged(self):
        self.assertEqual(add('arbitrary bytes', 'Methods_02.cpp'), 'arbitrary bytes')

    def test_all_game_and_runtime_headers_remain_in_canonical_src(self):
        import prepare_full_mapped as prepare
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            source = root/'src'
            hd = source/'hd'
            hd.mkdir(parents=True)
            dest = root/'generated'
            dest.mkdir()
            (hd/'Feedback.inc').write_text('#pragma once\n#include "Shared.h"\n#include "Bridge.h"\n')
            (hd/'OtherMenu.inc').write_text('#pragma once\n#include "Shared.h"\n')
            (hd/'Shared.h').write_text('#pragma once\n#include "../WorkerControl.h"\nstruct Shared {};\n')
            (source/'WorkerControl.h').write_text('#pragma once\nstruct WorkerControl {};\n')
            # Both hd bare includes and runtime-local includes use this very
            # same src/Bridge.h -> src/WorkerControl.h physical header chain.
            (source/'Bridge.h').write_text('#pragma once\n#include "WorkerControl.h"\n#include "hd/Shared.h"\n#include "Game.h"\n')
            original = '#pragma once\nstruct Game {};\n'
            (dest/'Game.h').write_text(original)
            method = '#include "Game.h"\n#include "hd/Feedback.inc"\n#include "hd/OtherMenu.inc"\n#include "Bridge.h"\n'
            (dest/'Methods_01.cpp').write_text(method)
            source_before = {str(p.relative_to(source)): p.read_bytes() for p in source.rglob('*') if p.is_file()}
            with patch.object(prepare, 'ROOT', root), patch.object(prepare, 'FEEDBACK_SCENE_HELPERS', ('Feedback.inc',)):
                inventory = prepare.validate_game_helper_includes(dest, ('Game.h', 'Methods_01.cpp'))
            self.assertEqual(set(inventory), {'hd/Feedback.inc', 'hd/OtherMenu.inc', 'hd/Shared.h', 'WorkerControl.h', 'Bridge.h'})
            self.assertEqual((dest/'Game.h').read_text(), original)
            self.assertEqual((dest/'Methods_01.cpp').read_text(), method)
            self.assertFalse((dest/'hd').exists())
            self.assertFalse((dest/'WorkerControl.h').exists())
            self.assertFalse((dest/'Bridge.h').exists())
            self.assertEqual({str(p.relative_to(source)): p.read_bytes() for p in source.rglob('*') if p.is_file()}, source_before)
            receipt = (dest/'maintained-header-inventory.json').read_text()
            self.assertIn('canonical-project-src-no-header-copies', receipt)
            self.assertNotIn(str(root), receipt)

    def test_canonical_inventory_rejects_stale_core_or_hd_copies(self):
        import prepare_full_mapped as prepare
        for shadow in ('WorkerControl.h', 'hd/Shared.h'):
            with self.subTest(shadow=shadow), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                source = root/'src'
                hd = source/'hd'
                hd.mkdir(parents=True)
                dest = root/'generated'
                dest.mkdir()
                (hd/'Feedback.inc').write_text('#include "Shared.h"\n#include "WorkerControl.h"\n')
                (hd/'Shared.h').write_text('#pragma once\n')
                (source/'WorkerControl.h').write_text('#pragma once\n')
                (dest/'Game.h').write_text('original generated Game\n')
                copied = dest/shadow
                copied.parent.mkdir(exist_ok=True)
                copied.write_bytes((source/shadow).read_bytes())
                with patch.object(prepare, 'ROOT', root), patch.object(prepare, 'FEEDBACK_SCENE_HELPERS', ('Feedback.inc',)), self.assertRaises(ValueError):
                    prepare.validate_game_helper_includes(dest, ('Game.h',))
                # Refusal preserves both originals and stale diagnostic evidence.
                self.assertEqual((dest/'Game.h').read_text(), 'original generated Game\n')
                self.assertTrue(copied.exists())

    def test_canonical_inventory_rejects_original_generated_name_collision(self):
        import prepare_full_mapped as prepare
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            hd = root/'src/hd'
            hd.mkdir(parents=True)
            dest = root/'generated'
            dest.mkdir()
            (dest/'Game.h').write_text('original generated Game\n')
            (hd/'Feedback.inc').write_text('#include "../Game.h"\n')
            (root/'src/Game.h').write_text('conflicting maintained Game\n')
            with patch.object(prepare, 'ROOT', root), patch.object(prepare, 'FEEDBACK_SCENE_HELPERS', ('Feedback.inc',)), self.assertRaises(ValueError):
                prepare.validate_game_helper_includes(dest, ('Game.h',))
            self.assertEqual((dest/'Game.h').read_text(), 'original generated Game\n')

    def test_canonical_inventory_rejects_escape_absolute_and_source_units(self):
        import prepare_full_mapped as prepare
        for dependency in ('../../private.h', '/private/header.h', 'E:/private/header.h', '../../.build/private.h', '../Implementation.cpp'):
            with self.subTest(dependency=dependency), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                hd = root/'src/hd'
                hd.mkdir(parents=True)
                dest = root/'generated'
                dest.mkdir()
                (dest/'Game.h').write_text('original generated Game\n')
                (hd/'Feedback.inc').write_text('#include "'+dependency+'"\n')
                (root/'private.h').write_text('private data\n')
                (root/'src/Implementation.cpp').write_text('source translation unit\n')
                with patch.object(prepare, 'ROOT', root), patch.object(prepare, 'FEEDBACK_SCENE_HELPERS', ('Feedback.inc',)), self.assertRaises(ValueError):
                    prepare.validate_game_helper_includes(dest, ('Game.h',))
                self.assertEqual((dest/'Game.h').read_text(), 'original generated Game\n')
                self.assertFalse((dest/'Implementation.cpp').exists())

    def test_maintained_preparation_registration(self):
        root = Path(__file__).resolve().parents[1]
        text = (root/'scripts/prepare_full_mapped.py').read_text()
        sequence = ('text = add_controls_steering_scene_hooks(text, path.name)', 'text = add_feedback_modal_hooks(text, path.name)', 'text = add_feedback_backdrop_hooks(text, path.name)', 'text = add_feedback_modal_scene_hooks(text, path.name)', 'text = add_feedback_modal_observer_source_hooks(text, path.name)')
        positions = [text.index(token) for token in sequence]
        self.assertEqual(positions, sorted(positions))
        self.assertIn('validate_game_helper_includes(dest, generated_files)', text)
        self.assertLess(text.index('(dest / "FullDispatch.cpp").write_text(dispatch)'), text.index('validate_game_helper_includes(dest, generated_files)'))
        queue = (root/'src/hd/ControlsSceneQueueAdapter.inc').read_text()
        self.assertEqual(queue.count('nfs_controls_retained_parent::remember(g,owned,dialogProof);'), 1)
        self.assertLess(queue.index('if(!repeatOwner||'), queue.index('nfs_controls_retained_parent::remember(g,owned,dialogProof);'))
        self.assertLess(queue.index('nfs_controls_retained_parent::remember(g,owned,dialogProof);'), queue.index('auto lease=Lease::create(owned);'))

if __name__ == '__main__':
    unittest.main()

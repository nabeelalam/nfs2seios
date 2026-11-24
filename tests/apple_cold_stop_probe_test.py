# SPDX-License-Identifier: MIT
"""Protocol, closed-CFG and command-policy checks; no compiler or Apple execution."""
import importlib.util
import io
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'scripts'))
try:
    spec = importlib.util.spec_from_file_location('cold_probe', ROOT / 'scripts/apple_cold_stop_probe.py')
    cold = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(cold)
finally:
    sys.path.pop(0)


def assembly_text():
    return '''_nfs_probe_accessor:
 .cfi_startproc
 adrp x0, __ZZN3nfs22current_worker_controlEvE5value@TLVPPAGE
 ldr x0, [x0, __ZZN3nfs22current_worker_controlEvE5value@TLVPPAGEOFF]
 ldr x8, [x0]
 blr x8
 ldr x8, [x0]
 cbz x8, Lnormal
 ldarb w8, [x8]
 tbz w8, #0, Lnormal
 bl __ZN3nfs24worker_stop_not_deferredEv
 cbnz w0, Lthrow
Lnormal:
 mov w1, #4
 bl _nfs_host_address
 ldr w0, [x0]
 ret
Lthrow:
 bl ___cxa_throw
 .cfi_endproc
_nfs_probe_transform:
 ret
 .cfi_endproc
_nfs_probe_rep8:
 ret
 .cfi_endproc
__ZN3nfs24worker_stop_not_deferredEv:
 adrp x0, __ZZN3nfs20owned_critical_depthEvE5value@TLVPPAGE
 ldr x0, [x0, __ZZN3nfs20owned_critical_depthEvE5value@TLVPPAGEOFF]
 ldr x8, [x0]
 blr x8
 ldr w8, [x0]
 cbnz w8, Lblocked
 adrp x0, __ZZN3nfs21worker_stop_deferralsEvE5value@TLVPPAGE
 ldr x0, [x0, __ZZN3nfs21worker_stop_deferralsEvE5value@TLVPPAGEOFF]
 ldr x8, [x0]
 blr x8
 ldr w8, [x0]
 cmp w8, #0
 cset w0, eq
 ret
Lblocked:
 mov w0, #0
 ret
 .cfi_endproc
'''


class CfgTests(unittest.TestCase):
    def proof(self, text):
        return cold.prove_cfg(cold.parse_functions(text))

    def test_observed_false_stop_bypasses_live_helper(self):
        proof = self.proof(assembly_text())
        self.assertTrue(proof['proven'])
        self.assertTrue(proof['false_path_bypasses_helper'])
        self.assertEqual(proof['accessor_tls_lookups'], 1)
        self.assertEqual(proof['helper_tls_lookups'], 2)

    def test_source_descriptor_destination_must_remain_x0(self):
        with self.assertRaises(ValueError):
            self.proof(assembly_text().replace('ldr x0, [x0, __ZZN3nfs22', 'ldr x9, [x0, __ZZN3nfs22'))

    def test_true_taken_branch_form(self):
        text = assembly_text().replace('tbz w8, #0, Lnormal', 'tbnz w8, #0, Lslow\n b Lnormal\nLslow:')
        self.assertTrue(self.proof(text)['proven'])

    def test_rejects_helper_hoisted_before_stop_or_on_false_path(self):
        text = assembly_text()
        call = ' bl ' + cold.HELPER + '\n'
        for bad in (text.replace(' ldarb w8, [x8]\n', call + ' ldarb w8, [x8]\n'),
                    text.replace('Lnormal:\n', 'Lnormal:\n' + call),
                    text.replace('tbz w8, #0, Lnormal', 'tbnz w8, #0, Lnormal')):
            with self.assertRaises(ValueError):
                self.proof(bad)

    def test_rejects_atomic_value_clobber_or_changed_test_bit(self):
        for bad in (assembly_text().replace(' tbz w8', ' mov w8, #0\n tbz w8'),
                    assembly_text().replace('tbz w8, #0', 'tbz w8, #1'),
                    assembly_text().replace('tbz w8, #0', 'tbz w9, #0')):
            with self.assertRaises(ValueError):
                self.proof(bad)

    def test_rejects_lost_control_provenance_and_missing_nonnull_gate(self):
        for bad in (assembly_text().replace(' ldr x8, [x0]\n cbz', ' ldr x8, [x0]\n mov x8, #123\n cbz'),
                    assembly_text().replace(' cbz x8, Lnormal\n', ''),
                    assembly_text().replace(' ldarb w8, [x8]', ' ldarb w8, [x9]')):
            with self.assertRaises(ValueError):
                self.proof(bad)

    def test_rejects_no_atomic_or_changed_helper_target(self):
        for bad in (assembly_text().replace('ldarb w8, [x8]', 'ldrb w8, [x8]'),
                    assembly_text().replace('bl ' + cold.HELPER, 'bl _unrecognized_helper'),
                    assembly_text().replace('tbz w8, #0, Lnormal', 'tbz w8, #0, MissingLabel')):
            with self.assertRaises(ValueError):
                self.proof(bad)

    def test_missing_live_helper_and_accessors_fail_closed(self):
        text = assembly_text()
        for bad in (text[:text.index(cold.HELPER + ':')],
                    text.replace('__ZZN3nfs21worker_stop_deferralsEvE5value@TLVP', 'other_symbol@TLVP'),
                    text.replace('_nfs_probe_rep8:', '_other_entry:')):
            with self.assertRaises(ValueError):
                self.proof(bad)

    def test_depth_materialization_inside_accessor_is_not_allowed(self):
        text = assembly_text().replace(' ldarb w8', ' adrp x9, __ZZN3nfs20owned_critical_depthEvE5value@TLVPPAGE\n ldarb w8', 1)
        with self.assertRaises(ValueError):
            self.proof(text)

    def test_outlined_helper_forwarding_is_followed_or_refused(self):
        text = assembly_text().replace('bl ' + cold.HELPER, 'bl OUTLINED_FUNCTION_0')
        with self.assertRaises(ValueError):
            self.proof(text)
        text += 'OUTLINED_FUNCTION_0:\n bl ' + cold.HELPER + '\n ret\n .cfi_endproc\n'
        self.assertTrue(self.proof(text)['proven'])

    def test_false_path_outlined_external_tail_helper_is_rejected(self):
        text = assembly_text().replace('Lnormal:\n', 'Lnormal:\n bl OUTLINED_FUNCTION_0\n')
        text += 'OUTLINED_FUNCTION_0:\n b ' + cold.HELPER + '\n .cfi_endproc\n'
        with self.assertRaises(ValueError):
            self.proof(text)

    def test_linked_absolute_targets_and_symbols(self):
        source = cold.parse_functions(assembly_text())
        addresses, lines = {}, ['/temporary/private/header/not/emitted:']
        cursor = 0x100000000
        starts = {}
        for name, fn in source.items():
            starts[name] = cursor
            addresses[cursor] = name
            cursor += 4 * len(fn['code']) + 16
        for name, fn in source.items():
            lines.append(name + ':')
            for index, row in enumerate(fn['code']):
                args = cold.operand(row['args'])
                for label, where in fn['labels'].items():
                    args = args.replace(label, hex(starts[name] + 4 * where))
                if row['op'] == 'bl' and args in starts:
                    args = hex(starts[args])
                lines.append('%016x %s %s' % (starts[name] + 4 * index, row['op'], args))
        text = '\n'.join(lines)
        slots = {}
        for offset, symbol in enumerate(cold.SLOT_SYMBOLS.values()):
            text = text.replace(symbol + '@TLVPPAGEOFF', '#0x%x' % (8 * offset))
            text = text.replace(symbol + '@TLVPPAGE', '0x100005000')
            slots[0x100005000 + 8 * offset] = symbol
        functions = cold.parse_functions(text, True, addresses)
        proof = cold.prove_cfg(functions, linked=True, symbol_addresses=addresses, tls_addresses={'direct': {}, 'indirect': slots})
        self.assertTrue(proof['proven'])
        for fn in functions.values():
            for row in fn['code']:
                if row['op'] == 'adrp':
                    delta = (0x100005000 - (row['address'] & ~4095)) >> 12
                    row['args'] = 'x0, ' + str(delta)
        self.assertTrue(cold.prove_cfg(functions, linked=True, symbol_addresses=addresses, tls_addresses={'direct': {}, 'indirect': slots})['proven'])
        replay = cold.replay_metadata(functions, addresses, {'direct': {}, 'indirect': slots})
        self.assertTrue(cold.prove_cfg(replay['functions'], linked=True,
            symbol_addresses={int(k,16):v for k,v in replay['call_target_symbols'].items()},
            tls_addresses={kind:{int(k,16):v for k,v in table.items()} for kind,table in replay['tls_symbols'].items()})['proven'])
        self.assertNotIn('temporary', json.dumps(cold.preview(functions)))
        with self.assertRaises(ValueError):
            cold.prove_cfg(functions, linked=True, symbol_addresses={})

    def test_unsupported_writes_and_unreachable_guard_fail(self):
        for insertion in ('ldp x0, x8, [sp]', 'swp w0, w8, [x1]', 'stxr w8, x0, [x1]', 'ldr x0, [x8], #8', 'blraaz x8', 'b.al Lnormal', 'bl _nfs_probe_accessor.cold.1'):
            with self.assertRaises(ValueError):
                self.proof(assembly_text().replace(' tbz w8', ' ' + insertion + '\n tbz w8'))
        with self.assertRaises(ValueError):
            self.proof(assembly_text().replace(' cbz x8, Lnormal', ' b Lunguarded\n cbz x8, Lnormal\nLunguarded:'))

    def test_exact_unwind_resume_is_exception_terminal_not_general_whitelist(self):
        text = assembly_text().replace(' bl ___cxa_throw', ' bl __Unwind_Resume')
        self.assertTrue(self.proof(text)['proven'])
        fn = dict(code=[dict(op='bl', args='0x100007000', address=0x100001000,
                           resolved_call_target='__Unwind_Resume'),
                        dict(op='ret', args='', address=0x100001004)], labels={})
        self.assertEqual(cold.graph(fn, {0x100007000:'__Unwind_Resume'})[0], [])
        self.assertEqual(cold.graph(fn, {0x100007000:'_unrecognized_alias'})[0], [1])
        with self.assertRaises(ValueError):cold.graph(fn)
        for name in ('__Unwind_Resume_or_Rethrow', '_Unwind_Resume', '__Unwind_ResumeAlias'):
            with self.assertRaises(ValueError):
                self.proof(assembly_text().replace(' bl ___cxa_throw', ' bl ' + name))

    def test_numeric_otool_adrp_displacement_is_pc_relative_and_identity_checked(self):
        fn = dict(code=[
            dict(op='adrp', args='x0, 10', address=0x1000031fc),
            dict(op='add', args='x0, x0, #0x80', address=0x100003200),
            dict(op='ldr', args='x8, [x0]', address=0x100003204),
            dict(op='blr', args='x8', address=0x100003208)])
        tls = dict(direct={0x10000d080:cold.SLOT_SYMBOLS['current']}, indirect={})
        self.assertEqual(cold.tls_lookup(fn, 3, True, tls), 'current')
        fn['code'][0]['args'] = 'x0, -2'
        tls['direct'] = {0x100001080:cold.SLOT_SYMBOLS['critical']}
        self.assertEqual(cold.tls_lookup(fn, 3, True, tls), 'critical')
        for operand in ('x0, 1048576', 'x0, -1048577', 'x0, nonsense'):
            fn['code'][0]['args'] = operand
            with self.assertRaises(ValueError):cold.tls_lookup(fn, 3, True, tls)
        fn['code'][0]['args'] = 'x0, 10'
        with self.assertRaises(ValueError):cold.tls_lookup(fn, 3, True, tls)
        fn['code'][0]['args'] = 'x0, 10 ; 0x10000d000'
        tls['direct'] = {0x10000d080:cold.SLOT_SYMBOLS['current']}
        self.assertEqual(cold.tls_lookup(fn, 3, True, tls), 'current')
        fn['code'][2]['args'] = 'x8, [x9]'
        with self.assertRaises(ValueError):cold.tls_lookup(fn, 3, True, tls)

    def test_replay_preserves_labels_pcs_and_sanitizes_failure_reason(self):
        functions = cold.parse_functions(assembly_text())
        replay = cold.replay_metadata(functions)
        self.assertTrue(cold.prove_cfg(replay['functions'])['proven'])
        self.assertIn('Lnormal', replay['functions']['_nfs_probe_accessor']['labels'])
        self.assertIn('address', replay['functions']['_nfs_probe_accessor']['code'][0])
        self.assertEqual(cold.proof_failure_reason(ValueError('/private/compiler/path')), 'Unrecognized proof failure')
        self.assertEqual(cold.proof_failure_reason(ValueError('Unknown linked TLS descriptor identity')), 'Unknown linked TLS descriptor identity')

    def test_function_bounds_never_accept_truncated_proof(self):
        text = assembly_text().replace(' tbz w8', ' nop\n' * 769 + ' tbz w8')
        with self.assertRaises(ValueError):
            cold.parse_functions(text)


class InputAndProtocolTests(unittest.TestCase):
    def test_materialize_exact_old_cold_and_mutation_refusal(self):
        with tempfile.TemporaryDirectory() as tmp:
            base = Path(tmp); root = base / 'source'; stage = base / 'closures'; stage.mkdir()
            for name in (*cold.assembly.PINNED_INPUTS, cold.COLD_FILE):
                target = root / name; target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes((ROOT / name).read_bytes())
            report = cold.materialize(root, stage)
            self.assertEqual(set(report), {'old', 'cold'})
            self.assertEqual([n for n in cold.assembly.CLOSURE if report['old'][n] != report['cold'][n]], ['WorkerControl.h'])
            self.assertEqual((stage / 'cold/GuestCopyTransaction.h').read_bytes(), (ROOT / 'tests/fixtures/native_span/control/GuestCopyTransaction.h').read_bytes())
            (root / cold.COLD_FILE).write_text('changed')
            other = base / 'other'; other.mkdir()
            with self.assertRaises(ValueError):
                cold.materialize(root, other)

    def test_modes_preserve_old_default_and_enforce_deferred_marker(self):
        self.assertEqual(cold.measure_args('normal', 1), ['--measure', '1'])
        for mode in ('critical', 'deferrals'):
            self.assertEqual(cold.measure_args(mode, 2), ['--measure-' + mode, '2'])
            cold.validate_mode([dict(deferred_mode=mode, stop_after_clear_verified=True)], mode)
            for row in ({}, dict(deferred_mode=mode, stop_after_clear_verified=False)):
                with self.assertRaises(ValueError):
                    cold.validate_mode([row], mode)
        cold.validate_mode([{}], 'normal')
        with self.assertRaises(ValueError):
            cold.validate_mode([dict(deferred_mode='normal')], 'normal')
        with self.assertRaises(ValueError):
            cold.measure_args('normal', True)

    def test_native_reference_proof_is_exact(self):
        self.assertEqual(cold.EXPECTED_PROOF['checks'], 126)
        self.assertEqual(cold.EXPECTED_PROOF['resolver_calls'], 583)
        self.assertEqual(cold.EXPECTED_PROOF['memory_checksum'], 5252414852042320239)

    def test_normal_gate_does_not_hide_deferred_cost(self):
        rows = [dict(mode=m, variant=v, kernel=k, round=r, iterations=100,
                     cpu_ns=1000 if v == 'old' else (800 if m == 'normal' else 3000), wall_ns=1000)
                for m in cold.MODES for v in cold.VARIANTS for k in cold.runtime.KERNELS for r in range(12)]
        _, decision = cold.summarize(rows)
        self.assertTrue(decision['component_gate_passed'])
        self.assertEqual(decision['deferred_cpu_improvement_percent']['critical']['accessor'], -200)
        with self.assertRaises(ValueError):
            cold.summarize(rows[:-1])

    def test_ios_compile_and_link_policy_contains_no_execution(self):
        flags = cold.ios_flags('clang++', '/SDK', Path('/probe'), 'cold')
        self.assertIn('arm64-apple-ios16.0', flags)
        self.assertIn('-DNDEBUG', flags)
        source = (ROOT / 'scripts/apple_cold_stop_probe.py').read_text()
        for value in ('-flto=thin', '-Wl,-mllvm,-threads=2', 'otool', 'stop_after_clear_verified'):
            self.assertIn(value, source)
        self.assertNotIn('setattr(', source)
        self.assertNotIn('runtime.VARIANTS =', source)
        self.assertNotIn('assembly.VARIANTS =', source)
        self.assertEqual(source.count('runtime.run_native('), 3)
        self.assertIn('executables[variant]', source)
        self.assertNotIn('run_native(binary', source)
        self.assertNotIn('upload-artifact', source)

    def test_subprocess_timeout_is_sanitized(self):
        stream = io.StringIO()
        with patch.object(sys, 'argv', ['probe']), patch.object(cold, 'execute', side_effect=subprocess.TimeoutExpired(['/private/secret/compiler'], 1)), patch('sys.stderr', stream):
            with self.assertRaises(SystemExit):
                cold.main()
        self.assertNotIn('/private', stream.getvalue())
        self.assertIn('time bound', stream.getvalue())


class MachoTests(unittest.TestCase):
    class Binary:
        def __init__(self, platform=2, minimum=16 << 16, cmdsize=24):
            self.data = struct.pack('<IIIIIIII', 0xfeedfacf, 0x0100000c, 0, 2, 1, 24, 0, 0) + struct.pack('<IIIIII', 0x32, cmdsize, platform, minimum, 0x00120500, 0)
        def is_symlink(self):return False
        def is_file(self):return True
        def stat(self):
            from types import SimpleNamespace
            return SimpleNamespace(st_size=len(self.data), st_mode=0o700)
        def open(self, _):return io.BytesIO(self.data)
        def read_bytes(self):return self.data

    def test_macho_indirect_tls_identity_and_bounds(self):
        names = list(cold.SLOT_SYMBOLS.values())
        strings = b'\0' + b''.join(n.encode() + b'\0' for n in names)
        command_size = 152 + 24 + 80
        table = 32 + command_size
        stroff = table + 48
        indirectoff = stroff + len(strings)
        data = bytearray(struct.pack('<IIIIIIII', 0xfeedfacf, 0x0100000c, 0, 2, 3, command_size, 0, 0))
        segment = bytearray(152)
        struct.pack_into('<II', segment, 0, 0x19, 152)
        struct.pack_into('<I', segment, 64, 1)
        struct.pack_into('<QQ', segment, 72 + 32, 0x100005000, 24)
        struct.pack_into('<II', segment, 72 + 64, 0x14, 0)
        data += segment
        data += struct.pack('<IIIIII', 2, 24, table, 3, stroff, len(strings))
        dysym = bytearray(80)
        struct.pack_into('<II', dysym, 0, 0xb, 80)
        struct.pack_into('<II', dysym, 56, indirectoff, 3)
        data += dysym
        index = 1
        for k,name in enumerate(names):
            data += struct.pack('<IBBHQ', index, 0xe, 1, 0, 0x100006000 + 24*k)
            index += len(name) + 1
        data += strings + struct.pack('<III', 0, 1, 2)
        blob = self.Binary();blob.data = bytes(data)
        result = cold.macho_tls_symbols(blob)
        self.assertEqual(result['indirect'], {0x100005000 + 8*k:n for k,n in enumerate(names)})
        self.assertEqual(result['direct'], {0x100006000 + 24*k:n for k,n in enumerate(names)})
        struct.pack_into('<I', data, indirectoff, 0x80000000)
        blob.data = bytes(data)
        self.assertNotIn(0x100005000, cold.macho_tls_symbols(blob)['indirect'])
        blob.data = bytes(data[:-1])
        with self.assertRaises(ValueError):cold.macho_tls_symbols(blob)

    def test_ios_build_version_required_not_just_arm64(self):
        self.assertEqual(cold.macho_platform(self.Binary(), 2, 16 << 16)['platform'], 2)
        with self.assertRaises(ValueError):
            cold.macho_platform(self.Binary(), 2, 16 << 16, cold.sdk_number('18.6'))
        self.assertEqual(cold.macho_platform(self.Binary(), 2, 16 << 16, cold.sdk_number('18.5'))['sdk'], 0x00120500)
        for blob in (self.Binary(platform=1), self.Binary(minimum=15 << 16), self.Binary(cmdsize=8)):
            with self.assertRaises(ValueError):
                cold.macho_platform(blob, 2, 16 << 16)


if __name__ == '__main__':
    unittest.main()

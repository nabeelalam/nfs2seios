# SPDX-License-Identifier: MIT
import importlib.util
import json
from pathlib import Path
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('data_probe', ROOT / 'scripts/apple_data_cache_probe.py')
probe = importlib.util.module_from_spec(spec)
spec.loader.exec_module(probe)


class DataCacheProbeChecks(unittest.TestCase):
    def test_frozen_closure_and_header_identity(self):
        rows = probe.sources()
        self.assertEqual(len(rows), len(probe.EXPECTED))
        self.assertEqual(next(row['sha256'] for row in rows if row['file'].endswith('CachedFrameTransaction.h')),
                         '42887c8f31c1b3c368d95f265198f943e9da46f531b99a0e1ff47ea7b85ae26c')
        with patch.object(probe.helper, 'sha', return_value='0' * 64):
            with self.assertRaises(ValueError):
                probe.sources()

    def test_manifest_cannot_admit_unreviewed_inputs(self):
        rows = probe.sources()
        for changed in [rows[:-1], rows + [rows[0]],
                        [dict(row, file='game/original.bin') if index == 0 else row for index, row in enumerate(rows)]]:
            with patch.object(probe, 'FIXTURE') as fixture:
                (fixture / 'source-provenance.json').read_text.return_value = json.dumps({'sources': changed})
                with self.assertRaises(ValueError):
                    probe.sources()

    def test_oracle_requires_all_exact_boolean_coverage(self):
        self.assertEqual(probe.verify_result(dict(probe.COVERAGE)), probe.COVERAGE)
        for key in probe.COVERAGE:
            for value in [False, 1, 'true', None]:
                changed = dict(probe.COVERAGE, **{key: value})
                with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                    probe.verify_result(changed)
        for changed in [{}, dict(probe.COVERAGE, extra=True)]:
            with self.assertRaises(ValueError):
                probe.verify_result(changed)

    def test_platform_gate_precedes_compilation(self):
        with patch.object(probe.sys, 'platform', 'win32'), patch.object(probe.helper, 'checked') as checked:
            with self.assertRaisesRegex(ValueError, 'Linux required'):
                probe.execute(True)
            with self.assertRaisesRegex(ValueError, 'Native Apple ARM'):
                probe.execute(False)
            checked.assert_not_called()

    def test_sdk_kernel_exercises_separate_data_window(self):
        source = probe.access_source()
        self.assertIn('tx{frame}', source)
        self.assertIn('tx.load<uint32_t>(data)', source)
        self.assertIn('tx.store<uint32_t>(data+4,first)', source)
        self.assertIn('memory.release(data);memory.release(frame)', source)
        self.assertNotIn('NFS_PRIVATE_', source)


if __name__ == '__main__':
    unittest.main()

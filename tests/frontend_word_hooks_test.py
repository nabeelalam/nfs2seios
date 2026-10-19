"""Preparation rejects source drift and preserves original function code."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from frontend_word_hooks import add_frontend_word_hooks

ENTRY = ('Fn(void) Game::_sub_423CA0()\n{\n'
         '\tnfs_trace_guest_call(game_id_sub_423CA0,uint32_t(esp));')
BODY = '\n\tpush32(ebx);\n\treturn;\n}\n'


class FrontendWordHooks(unittest.TestCase):
    def test_other_units_unchanged(self):
        self.assertEqual(add_frontend_word_hooks(ENTRY + BODY, 'Methods_02.cpp'), ENTRY + BODY)

    def test_original_tail_and_trace_unchanged(self):
        original = '// preceding source\n' + ENTRY + BODY
        updated = add_frontend_word_hooks(original, 'Methods_03.cpp')
        trace = '\tnfs_trace_guest_call(game_id_sub_423CA0,uint32_t(esp));'
        self.assertEqual(updated.split(trace)[1], original.split(trace)[1])
        self.assertEqual(updated.count(trace), 1)
        self.assertIn('// preceding source\nFn(void)', updated)
        self.assertIn('guest_store<int16_t>(dword_4E8A56 + 2 + 2*i, value)', updated)

    def test_drift_and_duplicates_rejected(self):
        for original in ('', ENTRY.replace('423CA0()', '423CA1()') + BODY,
                         (ENTRY + BODY)*2):
            with self.subTest(original=original), self.assertRaises(ValueError):
                add_frontend_word_hooks(original, 'Methods_03.cpp')

    def test_double_preparation_rejected(self):
        updated = add_frontend_word_hooks(ENTRY + BODY, 'Methods_03.cpp')
        with self.assertRaises(ValueError):
            add_frontend_word_hooks(updated, 'Methods_03.cpp')


if __name__ == '__main__':
    unittest.main()

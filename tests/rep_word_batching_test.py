import sys
import unittest
from unittest.mock import patch
import tempfile
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from batch_rep_words import TARGETS, ORIGINAL_LOOP, BOUNDED_LOOP, batch_rep_words
import prepare_full_mapped


def fixture(unit):
    return "\n".join(f"Fn(void) Game::_sub_{method}()\n{{\n" +
                     (ORIGINAL_LOOP + "\n") * count + "}\n"
                     for method, count in TARGETS[unit].items())


class RepWordBatchingTests(unittest.TestCase):
    def test_reviewed_methods_only_and_exact_reverse(self):
        for unit in TARGETS:
            source = fixture(unit)
            result, manifest = batch_rep_words(source, unit)
            self.assertEqual(result.replace(BOUNDED_LOOP, ORIGINAL_LOOP), source)
            self.assertEqual(sum(row["loops"] for row in manifest), sum(TARGETS[unit].values()))
            self.assertTrue(all(row["maximumWordsPerScope"] == 8 for row in manifest))

    def test_other_units_are_unchanged(self):
        source = fixture("Methods_03.cpp")
        self.assertEqual(batch_rep_words(source, "Methods_02.cpp"), (source, []))

    def test_other_methods_and_surrounding_statements_are_unchanged(self):
        other = "Fn(void) Game::_sub_000000()\n{\n" + ORIGINAL_LOOP + "\n}\n"
        source = other + fixture("Methods_03.cpp") + other
        result, _ = batch_rep_words(source, "Methods_03.cpp")
        self.assertTrue(result.startswith(other) and result.endswith(other))

    def test_missing_and_duplicate_entries_are_rejected(self):
        source = fixture("Methods_03.cpp")
        for bad in ("", source + source, source.replace("42CFB0", "42CFB1")):
            with self.assertRaises(ValueError):
                batch_rep_words(bad, "Methods_03.cpp")

    def test_changed_count_and_reapplication_are_rejected(self):
        source = fixture("Methods_03.cpp")
        for bad in (source.replace(ORIGINAL_LOOP, "", 1), source.replace(ORIGINAL_LOOP, ORIGINAL_LOOP * 2, 1), batch_rep_words(source, "Methods_03.cpp")[0]):
            with self.assertRaises(ValueError):
                batch_rep_words(bad, "Methods_03.cpp")

    def test_other_widths_calls_and_control_flow_are_rejected(self):
        for change in ("movsb();", "movsw();", "callback();", "movsd();\n\t\tcallback();", "if (ecx) movsd();"):
            with self.subTest(change=change), self.assertRaises(ValueError):
                batch_rep_words(fixture("Methods_03.cpp").replace("movsd();", change, 1), "Methods_03.cpp")

    def test_comment_or_string_disguised_loops_are_rejected(self):
        source = fixture("Methods_03.cpp")
        for bad in (source.replace(ORIGINAL_LOOP, "/*\n" + ORIGINAL_LOOP + "\n*/", 1), source.replace("{\n", '{\n"hidden";\n', 1)):
            with self.assertRaises(ValueError):
                batch_rep_words(bad, "Methods_03.cpp")

    def test_opt_in_requires_new_output_before_other_preparation(self):
        with patch.object(prepare_full_mapped, "prepare_mapped") as prepare:
            with self.assertRaises(ValueError):
                prepare_full_mapped.prepare_full(bounded_rep_words=True)
            prepare.assert_not_called()

    def test_existing_and_outside_output_are_rejected_before_preparation(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / "project"
            existing = root / ".build" / "existing"
            existing.mkdir(parents=True)
            sentinel = existing / "keep.txt"
            sentinel.write_text("preserve")
            with patch.object(prepare_full_mapped, "ROOT", root), patch.object(prepare_full_mapped, "prepare_mapped") as prepare:
                with self.assertRaises(FileExistsError):
                    prepare_full_mapped.prepare_full(True, existing)
                with self.assertRaises(ValueError):
                    prepare_full_mapped.prepare_full(True, root / "outside")
                prepare.assert_not_called()
            self.assertEqual(sentinel.read_text(), "preserve")


if __name__ == "__main__":
    unittest.main()

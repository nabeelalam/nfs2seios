"""Memory effects of overloaded multiply operands in the source generator."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from prepare_mapped_engine import rewrite_stores


class MultiplyOperandTests(unittest.TestCase):
    def test_one_operand_multiply_reads_memory_without_writeback(self):
        result = rewrite_stores("\timul32(to32i(edi+0x18)); // source operand\n")
        self.assertIn("imul32(guest_load<int32_t>(edi+0x18));", result)
        self.assertNotIn("guest_store", result)

    def test_destination_forms_keep_writeback(self):
        for statement in ("imul32(to32i(edi), eax);", "imul32(to32i(edi), eax, edx);", "imul16(to16i(edi), ax);"):
            with self.subTest(statement=statement):
                result = rewrite_stores(statement)
                self.assertEqual(result.count("guest_store<"), 1)
                self.assertEqual(result.count("guest_load<"), 1)

    def test_adjacent_mutating_instruction_is_preserved(self):
        result = rewrite_stores("imul32(to32i(edi));\nadd(to32i(edi), eax);\n")
        self.assertEqual(result.count("guest_store<int32_t>"), 1)
        self.assertIn("add(mapped_value, eax)", result)


if __name__ == "__main__":
    unittest.main()

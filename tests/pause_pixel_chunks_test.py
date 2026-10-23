"""Pause source chunks preserve all runs and reject unbounded copy inventories."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import re
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
from pause_pixel_chunks import (PAUSE_PIXEL_MAX_PARTS, PAUSE_PIXEL_PART_BYTES,
                                chunked_pause_pixels, pause_pixel_parts)


class PausePixelChunksTests(unittest.TestCase):
    def test_one_two_ten_and_maximum_chunks_roundtrip_with_file_bounds(self):
        sizes = (6, PAUSE_PIXEL_PART_BYTES, PAUSE_PIXEL_PART_BYTES+6,
                 606852, PAUSE_PIXEL_PART_BYTES*PAUSE_PIXEL_MAX_PARTS)
        for size in sizes:
            encoded = bytes((1, 0, 255, 254, 253, 252)) * (size//6)
            aggregate, outputs = chunked_pause_pixels(encoded)
            with self.subTest(size=size):
                self.assertEqual(pause_pixel_parts(aggregate), list(outputs))
                self.assertEqual(len(outputs), (size+PAUSE_PIXEL_PART_BYTES-1)//PAUSE_PIXEL_PART_BYTES)
                self.assertTrue(aggregate.startswith("static const unsigned char nfs_pause_pixels_rle[]={\n"))
                self.assertTrue(all(len(text.encode()) < 1048576 for text in outputs.values()))
                parts = [bytes(int(value) for value in re.findall(r"\b[0-9]+\b", text.split("\n", 1)[1]))
                         for text in outputs.values()]
                self.assertTrue(all(len(part) % 6 == 0 and len(part) <= PAUSE_PIXEL_PART_BYTES for part in parts))
                self.assertEqual(b"".join(parts), encoded)
                self.assertEqual(pause_pixel_parts(aggregate.replace("\n", "\r\n")), list(outputs))
        encoded = bytearray((1, 0, 1, 2, 3, 4))
        self.assertEqual(chunked_pause_pixels(encoded), chunked_pause_pixels(bytes(encoded)))

    def test_empty_unaligned_excessive_and_nonbyte_runs_decline(self):
        for encoded in (b"", bytes(7), bytes(PAUSE_PIXEL_MAX_PARTS*PAUSE_PIXEL_PART_BYTES+6),
                        "123456", [1, 0, 1, 2, 3, 4], None):
            with self.subTest(type=type(encoded).__name__), self.assertRaises(ValueError):
                chunked_pause_pixels(encoded)

    def test_missing_duplicate_out_of_order_foreign_or_seventeenth_include_decline(self):
        first = '#include "PauseFontPixelsPart0.inc"\n'
        second = '#include "PauseFontPixelsPart1.inc"\n'
        cases = ("", first+first, second, second+first, first+second+first,
                 first+'#include "other.inc"\n', first+'#include "../private.inc"\n',
                 first+'# include "foreign.inc"\n', first+'  #include "foreign.inc"\n',
                 first.replace("Part0", "Part00"), first.replace("Part0", "Part-1"),
                 "".join('#include "PauseFontPixelsPart%d.inc"\n' % index
                         for index in range(PAUSE_PIXEL_MAX_PARTS+1)), None)
        for index, text in enumerate(cases):
            with self.subTest(case=index), self.assertRaises(ValueError):
                pause_pixel_parts(text)


if __name__ == "__main__":
    unittest.main()

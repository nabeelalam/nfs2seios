"""Pinned complete-word Main atlas replacement and rejection tests."""
# SPDX-License-Identifier: MIT
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location(
    "frontend_main_word_art", Path(__file__).resolve().parents[1] / "scripts/frontend_main_word_art.py")
art = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(art)


def placements():
    result, y = [], 7
    for file, _, width, height, _ in art.WORDS:
        result.append(dict(file=file, atlas=[11, y, width, height],
                           sourceCrop=[0, 0, width, height], preservedInputField="untouched"))
        y += height + 3
    return result


class MainWordArtTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raw = art.ART_PATH.read_bytes()
        cls.document = json.loads(cls.raw)
        cls.images = art._load()

    def test_pinned_whole_words_and_white_rgb_even_when_transparent(self):
        self.assertEqual(len(self.raw), art.SOURCE_BYTES)
        self.assertEqual(hashlib.sha256(self.raw).hexdigest(), art.SOURCE_SHA)
        self.assertEqual(len(self.images), 8)
        for image, (_, _, width, height, sha) in zip(self.images, art.WORDS):
            self.assertEqual(len(image), width * height * 4)
            self.assertEqual(hashlib.sha256(image).hexdigest(), sha)
            self.assertEqual(image[0::4], b"\xff" * (width * height))
            self.assertEqual(image[1::4], image[0::4])
            self.assertEqual(image[2::4], image[0::4])
            self.assertIn(0, image[3::4])
            self.assertTrue(any(image[3::4]))

    def test_exact_eight_rectangles_all_other_pixels_preserved_and_idempotent(self):
        chosen = placements()
        original_chosen = copy.deepcopy(chosen)
        pixels = bytearray(b"\x11\x22\x33\x44") * (art.ATLAS_WIDTH * art.ATLAS_HEIGHT)
        expected = pixels[:]
        for image, placement in zip(self.images, chosen):
            x, y, width, height = placement["atlas"]
            for row in range(height):
                at = ((y + row) * art.ATLAS_WIDTH + x) * 4
                expected[at:at + width * 4] = image[row * width * 4:(row + 1) * width * 4]
        metadata = art.apply_main_word_art(pixels, chosen)
        self.assertEqual(pixels, expected)
        self.assertEqual(chosen, original_chosen)
        self.assertEqual(metadata["changedPixels"], sum(w * h for _, _, w, h, _ in art.WORDS))
        self.assertEqual(metadata["sourceSha"], art.SOURCE_SHA)
        self.assertEqual(metadata["newRgba"], art._identities())
        self.assertEqual(metadata["placements"], art._placements(chosen))
        self.assertIsNone(art.verify_main_word_art(metadata))
        self.assertEqual(art.apply_main_word_art(pixels, chosen)["changedPixels"], 0)
        self.assertEqual(pixels, expected)

    def test_invalid_placements_are_atomic_including_last_entry(self):
        cases = []
        cases.append(placements()[:-1])
        cases.append(placements() + [placements()[0]])
        p = placements(); p[0], p[1] = p[1], p[0]; cases.append(p)
        for key, value in (("file", "unknown.svg"), ("atlas", [0, 0, 381, 119]),
                           ("atlas", [-1, 0, 380, 119]), ("atlas", [True, 0, 380, 119]),
                           ("atlas", [1.0, 0, 380, 119]), ("atlas", [900, 0, 380, 119]),
                           ("atlas", [0, 2000, 380, 119]), ("sourceCrop", [1, 0, 380, 119]),
                           ("sourceCrop", [False, 0, 380, 119])):
            p = placements(); p[0][key] = value; cases.append(p)
        p = placements(); p[1]["atlas"][1] = p[0]["atlas"][1]; cases.append(p)
        p = placements(); p[-1]["sourceCrop"][0] = 1; cases.append(p)
        pixels = bytearray(art.ATLAS_WIDTH * art.ATLAS_HEIGHT * 4)
        before = pixels[:]
        for case in cases:
            with self.subTest(case=case):
                with self.assertRaises(ValueError):
                    art.apply_main_word_art(pixels, case)
                self.assertEqual(pixels, before)
        for pixels in (bytes(before), bytearray(3)):
            with self.assertRaises(ValueError):
                art.apply_main_word_art(pixels, placements())

    def test_decoder_rejects_untrusted_identity_geometry_and_runs(self):
        cases = []
        for key, value in (("schema", True), ("schema", 2), ("typeface", "other"),
                           ("revision", {}), ("words", [])):
            d = copy.deepcopy(self.document); d[key] = value; cases.append(d)
        d = copy.deepcopy(self.document); d["unexpected"] = 1; cases.append(d)
        d = copy.deepcopy(self.document); d["words"][0], d["words"][1] = d["words"][1], d["words"][0]; cases.append(d)
        for key, value in (("file", "other.svg"), ("width", 381), ("height", 120),
                           ("text", "game"), ("rgbaSha256", "0" * 64),
                           ("alphaRuns", [1]), ("alphaRuns", [0, 0]),
                           ("alphaRuns", [65536, 0]), ("alphaRuns", [True, 0]),
                           ("alphaRuns", [1, True]), ("alphaRuns", [1, -1]),
                           ("alphaRuns", [1, 256]), ("alphaRuns", [45221, 0]),
                           ("alphaRuns", [1, 255]), ("alphaRuns", [45220, 0]),
                           ("alphaRuns", [1, 0] * 45221)):
            d = copy.deepcopy(self.document); d["words"][0][key] = value; cases.append(d)
        d = copy.deepcopy(self.document); d["words"][0]["extra"] = 1; cases.append(d)
        d = copy.deepcopy(self.document); d["words"][0]["alphaRuns"][1] = 1; cases.append(d)
        for index, case in enumerate(cases):
            with self.subTest(case=index), self.assertRaises(ValueError):
                art._decode(case)
        with self.assertRaises(ValueError):
            art._pairs([("schema", 1), ("schema", 1)])

    def test_explicit_project_source_path_pin_and_no_fallback(self):
        with tempfile.TemporaryDirectory() as directory:
            selected = Path(directory) / "main-word-art.json"
            selected.write_bytes(self.raw)
            self.assertEqual(art._load(selected), self.images)
            pixels = bytearray(art.ATLAS_WIDTH * art.ATLAS_HEIGHT * 4)
            metadata = art.apply_main_word_art(pixels, placements(), source_path=selected)
            self.assertIsNone(art.verify_main_word_art(metadata, source_path=selected))
            for raw in (self.raw[:-1], self.raw + b"x", bytes([self.raw[0] ^ 1]) + self.raw[1:]):
                selected.write_bytes(raw)
                before = pixels[:]
                with self.assertRaises(ValueError):
                    art.apply_main_word_art(pixels, placements(), source_path=selected)
                self.assertEqual(pixels, before)
                with self.assertRaises(ValueError):
                    art.verify_main_word_art(metadata, source_path=selected)
            with self.assertRaises(OSError):
                art.verify_main_word_art(metadata, source_path=Path(directory) / "missing.json")

    def test_metadata_tampering_rejected(self):
        metadata = dict(sourceSha=art.SOURCE_SHA, newRgba=art._identities(),
                        placements=art._placements(placements()), changedPixels=0)
        cases = []
        for key, value in (("sourceSha", "0" * 64), ("newRgba", []),
                           ("changedPixels", True), ("changedPixels", -1),
                           ("changedPixels", sum(w * h for _, _, w, h, _ in art.WORDS) + 1)):
            m = copy.deepcopy(metadata); m[key] = value; cases.append(m)
        m = copy.deepcopy(metadata); m["extra"] = 1; cases.append(m)
        m = copy.deepcopy(metadata); m["newRgba"][0]["rgbaSha256"] = "0" * 64; cases.append(m)
        m = copy.deepcopy(metadata); m["placements"][0]["extra"] = 1; cases.append(m)
        m = copy.deepcopy(metadata); m["placements"][-1]["atlas"][0] = 1000; cases.append(m)
        for index, case in enumerate(cases):
            with self.subTest(case=index), self.assertRaises(ValueError):
                art.verify_main_word_art(case)


if __name__ == "__main__":
    unittest.main()

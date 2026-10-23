"""Finite colored pause artwork, original geometry and inert rejection."""
# SPDX-License-Identifier: MIT
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("pause_word_art", ROOT / "scripts/pause_word_art.py")
art = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(art)


class PauseWordArtTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.catalog_raw = (art.ART_DIR / "catalog.json").read_bytes()
        cls.catalog = json.loads(cls.catalog_raw)
        cls.raw = [(art.ART_DIR / name).read_bytes() for name, _, _ in art.SOURCE_FILES]
        parts = [json.loads(raw) for raw in cls.raw]
        cls.document = {key:value for key, value in parts[0].items() if key != "part"}
        cls.document["words"] = [word for part in parts for word in part["words"]]
        cls.images = art._load()

    def test_six_frozen_colored_images_and_original_catalog(self):
        self.assertEqual(len(self.images), 6)
        self.assertEqual(sum(art.PART_COUNTS), 6)
        self.assertEqual([w[0] for w in art.WORDS],
                         ["continue", "restart", "racing-music", "sound-effects", "quit", "quit-to-system"])
        self.assertEqual(len(self.catalog_raw), art.CATALOG_BYTES)
        self.assertEqual(hashlib.sha256(self.catalog_raw).hexdigest(), art.CATALOG_SHA256)
        self.assertIs(art._catalog(self.catalog), self.catalog)
        for raw, (_, size, sha) in zip(self.raw, art.SOURCE_FILES):
            self.assertLess(size, 1048576)
            self.assertEqual(len(raw), size)
            self.assertEqual(hashlib.sha256(raw).hexdigest(), sha)
        for image, (_, _, atlas, _, sha) in zip(self.images, art.WORDS):
            self.assertEqual(len(image), atlas[2] * atlas[3] * 4)
            self.assertEqual(hashlib.sha256(image).hexdigest(), sha)
            # Metallic RGBA is retained, rather than replacing all ink with white.
            colors = {image[i:i+3] for i in range(0, len(image), 4) if image[i+3]}
            self.assertGreater(len(colors), 1)
            self.assertTrue(any(rgb != b"\xff\xff\xff" for rgb in colors))
            self.assertTrue(all(image[i:i+3] == b"\x00\x00\x00" for i in range(0, len(image), 4) if not image[i+3]))

    def test_exact_six_rectangles_preserve_other_region_bytes_and_catalog(self):
        catalog = copy.deepcopy(self.catalog)
        region = bytearray(b"\x11\x22\x33\x44") * (704 * 384)
        expected = region[:]
        for image, (_, _, (x, y, width, height), _, _) in zip(self.images, art.WORDS):
            for row in range(height):
                at = ((y-640+row)*704+x-320)*4
                expected[at:at+width*4] = image[row*width*4:(row+1)*width*4]
        metadata = art.apply_pause_word_art(region, catalog)
        self.assertEqual(region, expected)
        self.assertEqual(catalog, self.catalog)
        self.assertEqual(metadata["changedPixels"], sum(w[2][2]*w[2][3] for w in art.WORDS))
        self.assertIsNone(art.verify_pause_word_art(metadata, catalog))
        self.assertEqual(art.apply_pause_word_art(region, catalog)["changedPixels"], 0)
        self.assertEqual(region, expected)

    def test_catalog_order_geometry_overlap_and_types_fail_before_writes(self):
        cases = []
        p = copy.deepcopy(self.catalog); p["region"][0] += 1; cases.append(p)
        p = copy.deepcopy(self.catalog); p["rasterScale"] = 4.0; cases.append(p)
        p = copy.deepcopy(self.catalog); p["entries"][0],p["entries"][1] = p["entries"][1],p["entries"][0]; cases.append(p)
        p = copy.deepcopy(self.catalog); p["entries"] = p["entries"][:-1]; cases.append(p)
        p = copy.deepcopy(self.catalog); p["placements"]["continue"]["atlasInkBounds"][0] += 1; cases.append(p)
        p = copy.deepcopy(self.catalog); p["placements"]["continue"]["originalRasterInkBounds"][0] += 1; cases.append(p)
        p = copy.deepcopy(self.catalog); p["placements"]["restart"]["atlasInkBounds"] = p["placements"]["continue"]["atlasInkBounds"][:]; cases.append(p)
        p = copy.deepcopy(self.catalog); p["placements"]["continue"]["allNonzeroAlphaPreserved"] = 1; cases.append(p)
        p = copy.deepcopy(self.catalog); p["extra"] = 1; cases.append(p)
        region = bytearray(704*384*4); before = region[:]
        for index, case in enumerate(cases):
            with self.subTest(case=index), self.assertRaises(ValueError):
                art.apply_pause_word_art(region, case)
            self.assertEqual(region, before)
        for wrong in (bytes(region), bytearray(4)):
            with self.assertRaises(ValueError): art.apply_pause_word_art(wrong, self.catalog)

    def test_decoder_rejects_schema_identity_geometry_and_bad_colored_runs(self):
        for key, value in (("schema", True), ("schema", 2), ("geometryChanged", 0), ("words", []),
                           ("typeface", "other"), ("scope", "other")):
            changed = copy.deepcopy(self.document); changed[key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError): art._decode(changed)
        count = art.WORDS[0][2][2] * art.WORDS[0][2][3]
        wrong_atlas = list(art.WORDS[0][2]); wrong_atlas[0] += 1
        float_atlas = list(art.WORDS[0][2]); float_atlas[0] = float(float_atlas[0])
        wrong_crop = list(art.WORDS[0][3]); wrong_crop[0] += 1
        cases = [("name", "unapproved"), ("text", "unapproved"), ("atlas", wrong_atlas),
                 ("atlas", float_atlas), ("sourceCrop", wrong_crop), ("rgbaSha256", "0"*64),
                 ("colorRuns", []), ("colorRuns", [1]), ("colorRuns", [0,0,0,0,0]),
                 ("colorRuns", [65536,0,0,0,0]), ("colorRuns", [count+1,0,0,0,255]),
                 ("colorRuns", [count,0,0,0,0]), ("colorRuns", [1,0,0,0,255]),
                 ("colorRuns", [True,0,0,0,255]), ("colorRuns", [1,0,0,0,0]*(count+1))]
        for channel in range(1,5):
            for value in (-1, 256, True, 0.5):
                bad = [1, 0, 0, 0, 255]; bad[channel] = value
                cases.append(("colorRuns", bad))
        for index, (key, value) in enumerate(cases):
            changed = copy.deepcopy(self.document); changed["words"][0][key] = value
            with self.subTest(case=index), self.assertRaises(ValueError): art._decode(changed)
        changed = copy.deepcopy(self.document); changed["words"][0]["colorRuns"][1] ^= 1
        with self.assertRaises(ValueError): art._decode(changed)
        changed = copy.deepcopy(self.document); changed["words"][0],changed["words"][1] = changed["words"][1],changed["words"][0]
        with self.assertRaises(ValueError): art._decode(changed)
        changed = copy.deepcopy(self.document); changed["words"][0]["extra"] = 1
        with self.assertRaises(ValueError): art._decode(changed)
        with self.assertRaises(ValueError): art._pairs([("schema", 1), ("schema", 1)])

    def test_selected_source_parts_missing_or_corrupt_are_inert_without_fallback(self):
        region = bytearray(704*384*4)
        metadata = art.apply_pause_word_art(region, self.catalog)
        with tempfile.TemporaryDirectory() as temporary:
            selected = Path(temporary)
            (selected/"catalog.json").write_bytes(self.catalog_raw)
            for raw, (name, _, _) in zip(self.raw, art.SOURCE_FILES): (selected/name).write_bytes(raw)
            self.assertIsNone(art.verify_pause_word_art(metadata, self.catalog, source_dir=selected))
            for original, (name, _, _) in zip(self.raw, art.SOURCE_FILES):
                path = selected/name
                for raw in (original[:-1], original+b"x", bytes([original[0]^1])+original[1:]):
                    path.write_bytes(raw); before = region[:]
                    with self.assertRaises(ValueError): art.apply_pause_word_art(region, self.catalog, source_dir=selected)
                    self.assertEqual(region, before)
                    with self.assertRaises(ValueError): art.verify_pause_word_art(metadata, self.catalog, source_dir=selected)
                path.unlink(); before = region[:]
                with self.assertRaises(OSError): art.apply_pause_word_art(region, self.catalog, source_dir=selected)
                self.assertEqual(region, before)
                with self.assertRaises(OSError): art.verify_pause_word_art(metadata, self.catalog, source_dir=selected)
                path.write_bytes(original)
            path = selected/"catalog.json"
            for raw in (self.catalog_raw[:-1], self.catalog_raw+b" ", bytes([self.catalog_raw[0]^1])+self.catalog_raw[1:]):
                path.write_bytes(raw); before = region[:]
                with self.assertRaises(ValueError): art.apply_pause_word_art(region, self.catalog, source_dir=selected)
                self.assertEqual(region, before)
                with self.assertRaises(ValueError): art.verify_pause_word_art(metadata, self.catalog, source_dir=selected)
            path.unlink(); before = region[:]
            with self.assertRaises(OSError): art.apply_pause_word_art(region, self.catalog, source_dir=selected)
            self.assertEqual(region, before)

    def test_metadata_corruption_declines(self):
        metadata = art.apply_pause_word_art(bytearray(704*384*4), self.catalog)
        for key, value in (("sourceHashes", {}), ("newRgba", []), ("changedPixels", True),
                           ("changedPixels", -1), ("changedPixels", sum(w[2][2]*w[2][3] for w in art.WORDS)+1)):
            changed = copy.deepcopy(metadata); changed[key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError): art.verify_pause_word_art(changed, self.catalog)
        changed = copy.deepcopy(metadata); changed["extra"] = 1
        with self.assertRaises(ValueError): art.verify_pause_word_art(changed, self.catalog)


if __name__ == "__main__":
    unittest.main()

"""Finite remaining-menu graphics, source authentication and inert rejection."""
# SPDX-License-Identifier: MIT
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("frontend_remaining_word_art", ROOT / "scripts/frontend_remaining_word_art.py")
art = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(art)
EXPECTED_COUNTS = {"main":0, "location":15, "setup":17, "player":0, "opponents":9, "options":9}


def families():
    result = {family: [dict(file="unrelated.svg", atlas=[1020, 2044, 4, 4], sourceCrop=[0, 0, 4, 4])]
              for family in art.FAMILIES}
    for _, _, atlas, crop, _, aliases in art.WORDS:
        for alias in aliases:
            family, file = alias.split("/")
            result[family].append(dict(file=file, atlas=list(atlas), sourceCrop=list(crop)))
    return result


class RemainingWordArtTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raw = [(art.ART_DIR / name).read_bytes() for name, _, _ in art.SOURCE_FILES]
        cls.parts = [json.loads(raw) for raw in cls.raw]
        cls.document = {key:value for key, value in cls.parts[0].items() if key != "part"}
        cls.document["words"] = [word for part in cls.parts for word in part["words"]]
        cls.images = art._load()

    def test_fifty_frozen_unique_body_and_value_graphics(self):
        self.assertEqual(len(self.images), 50)
        self.assertEqual(sum(art.PART_COUNTS), 50)
        self.assertEqual(len(art.SOURCE_FILES), len(art.PART_COUNTS))
        for raw, (_, size, sha) in zip(self.raw, art.SOURCE_FILES):
            self.assertLess(size, 1048576)
            self.assertEqual(len(raw), size)
            self.assertEqual(hashlib.sha256(raw).hexdigest(), sha)
        self.assertTrue(all(len(w[5]) == 1 for w in art.WORDS))
        self.assertEqual(len({w[2] for w in art.WORDS}), 50)
        counts = dict.fromkeys(art.FAMILIES, 0)
        for word in art.WORDS:
            counts[word[5][0].split("/")[0]] += 1
        self.assertEqual(counts, EXPECTED_COUNTS)
        for image, (_, _, atlas, _, sha, _) in zip(self.images, art.WORDS):
            count = atlas[2] * atlas[3]
            self.assertEqual(len(image), count * 4)
            self.assertEqual(hashlib.sha256(image).hexdigest(), sha)
            for channel in range(3):
                self.assertEqual(image[channel::4], b"\xff" * count)

    def test_actual_six_family_inventory_and_each_missing_use(self):
        metadata = json.loads((ROOT / "assets/hud/frontend/generated.json").read_bytes())
        actual = {family:metadata[family+"Placements"] for family in art.FAMILIES if family != "options"}
        actual["options"] = metadata["optionsAtlas"]["optionsPlacements"]
        checked = art._placements(actual)
        self.assertEqual(sum(len(record["sharedUses"]) for record in checked), 50)
        for _, _, _, _, _, aliases in art.WORDS:
            alias = aliases[0]; family, file = alias.split("/")
            missing = copy.deepcopy(actual)
            missing[family] = [p for p in missing[family] if p["file"] != file]
            with self.subTest(alias=alias), self.assertRaises(ValueError):
                art._placements(missing)

    def test_exact_fifty_rectangles_preserve_every_other_byte_and_geometry(self):
        chosen = families(); original = copy.deepcopy(chosen)
        pixels = bytearray(b"\x11\x22\x33\x44") * (1024 * 2048)
        expected = pixels[:]
        for image, (_, _, atlas, _, _, _) in zip(self.images, art.WORDS):
            x, y, width, height = atlas
            for row in range(height):
                at = ((y+row)*1024+x)*4
                expected[at:at+width*4] = image[row*width*4:(row+1)*width*4]
        metadata = art.apply_remaining_word_art(pixels, chosen)
        self.assertEqual(pixels, expected)
        self.assertEqual(chosen, original)
        self.assertEqual(metadata["changedPixels"], sum(w[2][2]*w[2][3] for w in art.WORDS))
        self.assertIsNone(art.verify_remaining_word_art(metadata, chosen))
        self.assertEqual(art.apply_remaining_word_art(pixels, chosen)["changedPixels"], 0)
        self.assertEqual(pixels, expected)

    def test_alias_geometry_family_and_type_failures_are_inert(self):
        cases = []
        p = families(); del p["main"]; cases.append(p)
        p = families(); p["foreign"] = []; cases.append(p)
        p = families(); p["options"] = p["options"][:1]; cases.append(p)
        p = families(); p["location"][-1]["atlas"][0] += 1; cases.append(p)
        p = families(); p["location"][-1]["sourceCrop"][1] += 1; cases.append(p)
        p = families(); p["location"].append(copy.deepcopy(p["location"][1])); cases.append(p)
        p = families(); p["main"][0]["atlas"] = list(art.WORDS[0][2]); p["main"][0]["sourceCrop"] = list(art.WORDS[0][3]); cases.append(p)
        p = families(); p["player"][0]["atlas"] = list(art.WORDS[-1][2]); p["player"][0]["sourceCrop"] = list(art.WORDS[-1][3]); cases.append(p)
        p = families(); p["location"][1]["atlas"][0] = True; cases.append(p)
        p = families(); p["location"][1]["atlas"][0] = 1024; cases.append(p)
        p = families(); p["location"][1]["file"] = "unapproved.svg"; cases.append(p)
        pixels = bytearray(1024*2048*4); before = pixels[:]
        for index, case in enumerate(cases):
            with self.subTest(case=index), self.assertRaises(ValueError):
                art.apply_remaining_word_art(pixels, case)
            self.assertEqual(pixels, before)
        for wrong in (bytes(pixels), bytearray(4)):
            with self.assertRaises(ValueError):
                art.apply_remaining_word_art(wrong, families())

    def test_decoder_rejects_schema_identity_alias_geometry_and_unbounded_runs(self):
        # Decode one mutated document at a time, keeping the 50-word fixture bounded.
        for key, value in (("schema", True), ("schema", 2), ("geometryChanged", 0), ("words", []), ("typeface", "other")):
            changed = copy.deepcopy(self.document); changed[key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                art._decode(changed)
        first = art.WORDS[0]; count = first[2][2] * first[2][3]
        wrong_atlas = list(first[2]); wrong_atlas[2] += 1
        wrong_crop = list(first[3]); wrong_crop[1] += 1
        float_atlas = list(first[2]); float_atlas[0] = float(float_atlas[0])
        cases = (("text", "unapproved"), ("file", "other.svg"), ("sharedUses", []),
                 ("sharedUses", [first[5][0], "main/unapproved.svg"]),
                 ("atlas", wrong_atlas), ("atlas", float_atlas), ("sourceCrop", wrong_crop),
                 ("rgbaSha256", "0"*64), ("alphaRuns", [1]), ("alphaRuns", [0, 0]),
                 ("alphaRuns", [65536, 0]), ("alphaRuns", [1, 256]),
                 ("alphaRuns", [True, 0]), ("alphaRuns", [1, True]),
                 ("alphaRuns", [count+1, 255]), ("alphaRuns", [count, 0]),
                 ("alphaRuns", [1, 255]), ("alphaRuns", [1, 0]*(count+1)))
        for index, (key, value) in enumerate(cases):
            changed = copy.deepcopy(self.document); changed["words"][0][key] = value
            with self.subTest(case=index), self.assertRaises(ValueError):
                art._decode(changed)
        changed = copy.deepcopy(self.document); changed["words"][0]["alphaRuns"][1] ^= 1
        with self.assertRaises(ValueError): art._decode(changed)
        changed = copy.deepcopy(self.document); changed["words"][0], changed["words"][1] = changed["words"][1], changed["words"][0]
        with self.assertRaises(ValueError): art._decode(changed)
        changed = copy.deepcopy(self.document); changed["words"][0]["extra"] = 1
        with self.assertRaises(ValueError): art._decode(changed)
        with self.assertRaises(ValueError): art._pairs([("schema", 1), ("schema", 1)])

    def test_each_selected_source_part_pin_missing_and_metadata_corruption(self):
        pixels = bytearray(1024*2048*4)
        metadata = art.apply_remaining_word_art(pixels, families())
        with tempfile.TemporaryDirectory() as directory:
            selected = Path(directory)
            for raw, (name, _, _) in zip(self.raw, art.SOURCE_FILES):
                (selected / name).write_bytes(raw)
            self.assertIsNone(art.verify_remaining_word_art(metadata, families(), source_dir=selected))
            for original, (name, _, _) in zip(self.raw, art.SOURCE_FILES):
                path = selected / name
                for raw in (original[:-1], original+b"x", bytes([original[0]^1])+original[1:]):
                    path.write_bytes(raw); before = pixels[:]
                    with self.assertRaises(ValueError):
                        art.apply_remaining_word_art(pixels, families(), source_dir=selected)
                    self.assertEqual(pixels, before)
                    with self.assertRaises(ValueError):
                        art.verify_remaining_word_art(metadata, families(), source_dir=selected)
                path.unlink(); before = pixels[:]
                with self.assertRaises(OSError):
                    art.apply_remaining_word_art(pixels, families(), source_dir=selected)
                self.assertEqual(pixels, before)
                with self.assertRaises(OSError):
                    art.verify_remaining_word_art(metadata, families(), source_dir=selected)
                path.write_bytes(original)
        for key, value in (("sourceHashes", {}), ("newRgba", []), ("placements", []),
                           ("changedPixels", True), ("changedPixels", -1),
                           ("changedPixels", sum(w[2][2]*w[2][3] for w in art.WORDS)+1)):
            changed = copy.deepcopy(metadata); changed[key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                art.verify_remaining_word_art(changed, families())


if __name__ == "__main__":
    unittest.main()

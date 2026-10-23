"""Finite Player text graphics, exact sharing, and inert rejection tests."""
# SPDX-License-Identifier: MIT
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("frontend_player_word_art", ROOT / "scripts/frontend_player_word_art.py")
art = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(art)


def families():
    # Explicit fixed allocation fixture; only these aliases may share a write.
    result = {family: [] for family in art.FAMILIES}
    for _, _, atlas, crop, _, aliases in art.WORDS:
        for alias in aliases:
            family, file = alias.split("/")
            result[family].append(dict(file=file, atlas=list(atlas), sourceCrop=list(crop)))
    # Unrelated families are still supplied and must survive unchanged.
    for family in ("main", "options"):
        result[family].append(dict(file="unrelated.svg", atlas=[1020, 2044, 4, 4], sourceCrop=[0, 0, 4, 4]))
    return result


class PlayerWordArtTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raw = [(art.ART_DIR / name).read_bytes() for name, _, _ in art.SOURCE_FILES]
        parts = [json.loads(raw) for raw in cls.raw]
        cls.document = {key:value for key, value in parts[0].items() if key != "part"}
        cls.document["words"] = parts[0]["words"] + parts[1]["words"]
        cls.images = art._load()

    def test_frozen_thirty_graphics_and_fixed_colon(self):
        self.assertEqual(len(self.images), 30)
        for raw, (_, size, sha) in zip(self.raw, art.SOURCE_FILES):
            self.assertLess(size, 1048576)
            self.assertEqual(len(raw), size)
            self.assertEqual(hashlib.sha256(raw).hexdigest(), sha)
        self.assertEqual(sum(len(w[5])-1 for w in art.WORDS), 26)
        self.assertEqual(sum(len(w[5]) > 1 for w in art.WORDS), 19)
        colon = [word for word in art.WORDS if word[0] == "colon.svg"]
        self.assertEqual(len(colon), 1)
        self.assertEqual(colon[0][1:4], (":", (964, 1529, 24, 54), (0, 28, 24, 54)))
        for image, (_, _, atlas, _, sha, _) in zip(self.images, art.WORDS):
            count = atlas[2] * atlas[3]
            self.assertEqual(len(image), count * 4)
            self.assertEqual(hashlib.sha256(image).hexdigest(), sha)
            for channel in range(3):
                self.assertEqual(image[channel::4], b"\xff" * count)

    def test_actual_current_metadata_includes_nested_options_aliases(self):
        metadata = json.loads((ROOT / "assets/hud/frontend/generated.json").read_bytes())
        actual = {family:metadata[family+"Placements"]
                  for family in art.FAMILIES if family != "options"}
        actual["options"] = metadata["optionsAtlas"]["optionsPlacements"]
        checked = art._placements(actual)
        self.assertEqual(sum(len(record["sharedUses"]) for record in checked), 56)
        for file in ("colon.svg", "on.svg", "off.svg"):
            record = next(record for record in checked if record["file"] == file)
            self.assertIn("options/"+file, record["sharedUses"])
        # Removing any one genuine Options alias must still decline.
        for file in ("colon.svg", "on.svg", "off.svg"):
            missing = copy.deepcopy(actual)
            missing["options"] = [p for p in missing["options"] if p["file"] != file]
            with self.subTest(file=file), self.assertRaises(ValueError):
                art._placements(missing)

    def test_exact_thirty_rectangles_preserve_every_other_byte_and_geometry(self):
        chosen = families(); original = copy.deepcopy(chosen)
        pixels = bytearray(b"\x11\x22\x33\x44") * (1024 * 2048)
        expected = pixels[:]
        for image, (_, _, atlas, _, _, _) in zip(self.images, art.WORDS):
            x, y, width, height = atlas
            for row in range(height):
                at = ((y+row)*1024+x)*4
                expected[at:at+width*4] = image[row*width*4:(row+1)*width*4]
        metadata = art.apply_player_word_art(pixels, chosen)
        self.assertEqual(pixels, expected)
        self.assertEqual(chosen, original)
        self.assertEqual(metadata["changedPixels"], sum(w[2][2]*w[2][3] for w in art.WORDS))
        self.assertIsNone(art.verify_player_word_art(metadata, chosen))
        self.assertEqual(art.apply_player_word_art(pixels, chosen)["changedPixels"], 0)
        self.assertEqual(pixels, expected)

    def test_all_family_alias_failures_are_inert(self):
        cases = []
        p = families(); del p["main"]; cases.append(p)
        p = families(); p["foreign"] = []; cases.append(p)
        p = families(); p["opponents"] = p["opponents"][1:]; cases.append(p)
        p = families(); p["player"][-1]["atlas"][0] += 1; cases.append(p)
        p = families(); p["location"][0]["sourceCrop"][1] += 1; cases.append(p)
        p = families(); p["player"].append(copy.deepcopy(p["player"][0])); cases.append(p)
        p = families(); p["main"][0]["atlas"] = list(art.WORDS[0][2]); p["main"][0]["sourceCrop"] = list(art.WORDS[0][3]); cases.append(p)
        p = families(); p["options"][0]["atlas"] = [920, 0, 4, 4]; cases.append(p)
        p = families(); p["player"][0]["atlas"][0] = True; cases.append(p)
        p = families(); p["player"][0]["atlas"][0] = 1024; cases.append(p)
        p = families(); p["player"][0]["file"] = "unapproved.svg"; cases.append(p)
        pixels = bytearray(1024*2048*4); before = pixels[:]
        for index, case in enumerate(cases):
            with self.subTest(case=index), self.assertRaises(ValueError):
                art.apply_player_word_art(pixels, case)
            self.assertEqual(pixels, before)
        for wrong in (bytes(pixels), bytearray(4)):
            with self.assertRaises(ValueError):
                art.apply_player_word_art(wrong, families())

    def test_decoder_rejects_identity_alias_geometry_and_unbounded_runs(self):
        cases = []
        for key, value in (("schema", True), ("schema", 2), ("geometryChanged", 0), ("words", []), ("typeface", "other")):
            d = copy.deepcopy(self.document); d[key] = value; cases.append(d)
        for key, value in (("text", "CAR"), ("file", "other.svg"), ("sharedUses", ["player/car.svg"]),
                           ("atlas", [920, 0, 85, 55]), ("sourceCrop", [0, 26, 84, 55]),
                           ("rgbaSha256", "0"*64), ("alphaRuns", [1]), ("alphaRuns", [0, 0]),
                           ("alphaRuns", [65536, 0]), ("alphaRuns", [1, 256]),
                           ("alphaRuns", [True, 0]), ("alphaRuns", [1, True]),
                           ("alphaRuns", [4621, 255]), ("alphaRuns", [4620, 0]),
                           ("alphaRuns", [1, 255]), ("alphaRuns", [1, 0]*4621)):
            d = copy.deepcopy(self.document); d["words"][0][key] = value; cases.append(d)
        d = copy.deepcopy(self.document); d["words"][0]["alphaRuns"][1] ^= 1; cases.append(d)
        d = copy.deepcopy(self.document); d["words"][0], d["words"][1] = d["words"][1], d["words"][0]; cases.append(d)
        d = copy.deepcopy(self.document); d["words"][0]["extra"] = 1; cases.append(d)
        for index, case in enumerate(cases):
            with self.subTest(case=index), self.assertRaises(ValueError):
                art._decode(case)
        with self.assertRaises(ValueError):
            art._pairs([("schema", 1), ("schema", 1)])

    def test_selected_project_source_pin_and_metadata_corruption(self):
        pixels = bytearray(1024*2048*4)
        metadata = art.apply_player_word_art(pixels, families())
        with tempfile.TemporaryDirectory() as directory:
            selected = Path(directory)
            for raw, (name, _, _) in zip(self.raw, art.SOURCE_FILES):
                (selected / name).write_bytes(raw)
            self.assertIsNone(art.verify_player_word_art(metadata, families(), source_dir=selected))
            # Either part fails closed before any writes, without root fallback.
            for original, (name, _, _) in zip(self.raw, art.SOURCE_FILES):
                path = selected / name
                for raw in (original[:-1], original+b"x", bytes([original[0]^1])+original[1:]):
                    path.write_bytes(raw); before = pixels[:]
                    with self.assertRaises(ValueError):
                        art.apply_player_word_art(pixels, families(), source_dir=selected)
                    self.assertEqual(pixels, before)
                    with self.assertRaises(ValueError):
                        art.verify_player_word_art(metadata, families(), source_dir=selected)
                path.write_bytes(original)
            (selected / art.SOURCE_FILES[1][0]).unlink()
            with self.assertRaises(OSError):
                art.verify_player_word_art(metadata, families(), source_dir=selected)
        for key, value in (("sourceHashes", {}), ("newRgba", []), ("placements", []),
                           ("changedPixels", True), ("changedPixels", -1),
                           ("changedPixels", sum(w[2][2]*w[2][3] for w in art.WORDS)+1)):
            changed = copy.deepcopy(metadata); changed[key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                art.verify_player_word_art(changed, families())
        p = families(); p["setup"] = [entry for entry in p["setup"] if entry["file"] != "colon.svg"]
        with self.assertRaises(ValueError):
            art.verify_player_word_art(metadata, p)


if __name__ == "__main__":
    unittest.main()

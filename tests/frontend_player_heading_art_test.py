"""Finite uppercase Player graphics, exact sharing and inert rejection."""
# SPDX-License-Identifier: MIT
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("frontend_player_heading_art", ROOT / "scripts/frontend_player_heading_art.py")
art = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(art)


def families():
    result = {family: [dict(file="unrelated.svg", atlas=[1020, 2044, 4, 4], sourceCrop=[0, 0, 4, 4])]
              for family in art.FAMILIES}
    for _, _, atlas, crop, _, aliases in art.WORDS:
        for alias in aliases:
            family, file = alias.split("/")
            result[family].append(dict(file=file, atlas=list(atlas), sourceCrop=list(crop)))
    return result


class PlayerHeadingArtTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.raw = (art.ART_DIR / art.SOURCE_FILE).read_bytes()
        cls.document = json.loads(cls.raw)
        cls.images = art._load()

    def test_four_frozen_uppercase_graphics(self):
        self.assertLess(art.SOURCE_BYTES, 1048576)
        self.assertEqual(len(self.raw), art.SOURCE_BYTES)
        self.assertEqual(hashlib.sha256(self.raw).hexdigest(), art.SOURCE_SHA256)
        self.assertEqual([word[1] for word in art.WORDS], ["DONE", "CANCEL", "PLAYER ONE", "PLAYER TWO"])
        self.assertEqual(len(self.images), 4)
        self.assertEqual(sum(len(w[5]) for w in art.WORDS), 11)
        self.assertEqual(sum(len(w[5])-1 for w in art.WORDS), 7)
        for image, (_, _, atlas, _, sha, _) in zip(self.images, art.WORDS):
            count = atlas[2] * atlas[3]
            self.assertEqual(len(image), count * 4)
            self.assertEqual(hashlib.sha256(image).hexdigest(), sha)
            for channel in range(3):
                self.assertEqual(image[channel::4], b"\xff" * count)

    def test_actual_six_family_inventory(self):
        metadata = json.loads((ROOT / "assets/hud/frontend/generated.json").read_bytes())
        actual = {family:metadata[family+"Placements"] for family in art.FAMILIES if family != "options"}
        actual["options"] = metadata["optionsAtlas"]["optionsPlacements"]
        checked = art._placements(actual)
        self.assertEqual(sum(len(record["sharedUses"]) for record in checked), 11)
        done = next(record for record in checked if record["file"] == "done.svg")
        self.assertIn("options/done.svg", done["sharedUses"])
        for _, _, _, _, _, aliases in art.WORDS:
            for alias in aliases:
                family, file = alias.split("/")
                missing = copy.deepcopy(actual)
                missing[family] = [p for p in missing[family] if p["file"] != file]
                with self.subTest(alias=alias), self.assertRaises(ValueError):
                    art._placements(missing)

    def test_exact_four_rectangles_preserve_every_other_byte_and_geometry(self):
        chosen = families(); original = copy.deepcopy(chosen)
        pixels = bytearray(b"\x11\x22\x33\x44") * (1024 * 2048)
        expected = pixels[:]
        for image, (_, _, atlas, _, _, _) in zip(self.images, art.WORDS):
            x, y, width, height = atlas
            for row in range(height):
                at = ((y+row)*1024+x)*4
                expected[at:at+width*4] = image[row*width*4:(row+1)*width*4]
        metadata = art.apply_player_heading_art(pixels, chosen)
        self.assertEqual(pixels, expected)
        self.assertEqual(chosen, original)
        self.assertEqual(metadata["changedPixels"], sum(w[2][2]*w[2][3] for w in art.WORDS))
        self.assertIsNone(art.verify_player_heading_art(metadata, chosen))
        self.assertEqual(art.apply_player_heading_art(pixels, chosen)["changedPixels"], 0)
        self.assertEqual(pixels, expected)

    def test_alias_geometry_family_and_type_failures_are_inert(self):
        cases = []
        p = families(); del p["main"]; cases.append(p)
        p = families(); p["foreign"] = []; cases.append(p)
        p = families(); p["options"] = p["options"][:1]; cases.append(p)
        p = families(); p["player"][-1]["atlas"][0] += 1; cases.append(p)
        p = families(); p["location"][-1]["sourceCrop"][1] += 1; cases.append(p)
        p = families(); p["player"].append(copy.deepcopy(p["player"][1])); cases.append(p)
        p = families(); p["main"][0]["atlas"] = list(art.WORDS[0][2]); p["main"][0]["sourceCrop"] = list(art.WORDS[0][3]); cases.append(p)
        p = families(); p["options"][0]["atlas"] = list(art.WORDS[3][2]); p["options"][0]["sourceCrop"] = list(art.WORDS[3][3]); cases.append(p)
        p = families(); p["player"][1]["atlas"][0] = True; cases.append(p)
        p = families(); p["player"][1]["atlas"][0] = 1024; cases.append(p)
        p = families(); p["player"][1]["file"] = "unapproved.svg"; cases.append(p)
        pixels = bytearray(1024*2048*4); before = pixels[:]
        for index, case in enumerate(cases):
            with self.subTest(case=index), self.assertRaises(ValueError):
                art.apply_player_heading_art(pixels, case)
            self.assertEqual(pixels, before)
        for wrong in (bytes(pixels), bytearray(4)):
            with self.assertRaises(ValueError):
                art.apply_player_heading_art(wrong, families())

    def test_decoder_rejects_identity_alias_geometry_and_unbounded_runs(self):
        cases = []
        for key, value in (("schema", True), ("schema", 2), ("geometryChanged", 0), ("words", []), ("typeface", "other")):
            d = copy.deepcopy(self.document); d[key] = value; cases.append(d)
        count = art.WORDS[0][2][2] * art.WORDS[0][2][3]
        for key, value in (("text", "player one"), ("file", "other.svg"), ("sharedUses", ["main/player-one-heading.svg"]),
                           ("atlas", [592, 961, 285, 76]), ("sourceCrop", [0, 10, 284, 76]),
                           ("rgbaSha256", "0"*64), ("alphaRuns", [1]), ("alphaRuns", [0, 0]),
                           ("alphaRuns", [65536, 0]), ("alphaRuns", [1, 256]),
                           ("alphaRuns", [True, 0]), ("alphaRuns", [1, True]),
                           ("alphaRuns", [count+1, 255]), ("alphaRuns", [count, 0]),
                           ("alphaRuns", [1, 255]), ("alphaRuns", [1, 0]*(count+1))):
            d = copy.deepcopy(self.document); d["words"][0][key] = value; cases.append(d)
        d = copy.deepcopy(self.document); d["words"][0]["atlas"][0] = float(d["words"][0]["atlas"][0]); cases.append(d)
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
        metadata = art.apply_player_heading_art(pixels, families())
        with tempfile.TemporaryDirectory() as directory:
            selected = Path(directory); path = selected / art.SOURCE_FILE
            path.write_bytes(self.raw)
            self.assertIsNone(art.verify_player_heading_art(metadata, families(), source_dir=selected))
            for raw in (self.raw[:-1], self.raw+b"x", bytes([self.raw[0]^1])+self.raw[1:]):
                path.write_bytes(raw); before = pixels[:]
                with self.assertRaises(ValueError):
                    art.apply_player_heading_art(pixels, families(), source_dir=selected)
                self.assertEqual(pixels, before)
                with self.assertRaises(ValueError):
                    art.verify_player_heading_art(metadata, families(), source_dir=selected)
            path.unlink(); before = pixels[:]
            with self.assertRaises(OSError):
                art.apply_player_heading_art(pixels, families(), source_dir=selected)
            self.assertEqual(pixels, before)
            with self.assertRaises(OSError):
                art.verify_player_heading_art(metadata, families(), source_dir=selected)
        for key, value in (("sourceHashes", {}), ("newRgba", []), ("placements", []),
                           ("changedPixels", True), ("changedPixels", -1),
                           ("changedPixels", sum(w[2][2]*w[2][3] for w in art.WORDS)+1)):
            changed = copy.deepcopy(metadata); changed[key] = value
            with self.subTest(key=key, value=value), self.assertRaises(ValueError):
                art.verify_player_heading_art(changed, families())


if __name__ == "__main__":
    unittest.main()

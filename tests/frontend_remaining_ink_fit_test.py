"""New menu ink preserves original catalog and scaled Main-summary geometry."""
# SPDX-License-Identifier: MIT
import copy
import json
from pathlib import Path
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/"scripts"))
sys.path.insert(0, str(ROOT/"tests"))
from frontend_remaining_ink_fit import remaining_ink_fit, FAMILIES
from frontend_remaining_word_art import WORDS
from frontend_right_labels import BEGIN, END, base_layout
from frontend_options_atlas_test import decode_current


class RemainingInkFit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory = ROOT/"assets/hud/frontend"
        metadata = json.loads((cls.directory/"generated.json").read_bytes())
        cls.placements = {family+"Placements":metadata["optionsAtlas"]["optionsPlacements"]
                          if family == "options" else metadata[family+"Placements"]
                          for family in FAMILIES}
        layout = (ROOT/"src/FrontendWordLayout.inc").read_text()
        cls.rows = [line.rstrip(",") for line in base_layout(layout).splitlines()
                    if line.startswith("{")]
        extension = layout.split(BEGIN, 1)[1].split(END, 1)[0]
        cls.summary = [line.rstrip(",") for line in extension.splitlines() if line.startswith("{")]
        cls.pixels = decode_current()

    def check(self, pixels=None, rows=None, placements=None, summary=None, directory=None):
        return remaining_ink_fit(self.directory if directory is None else directory,
                                 self.pixels if pixels is None else pixels,
                                 self.rows if rows is None else rows,
                                 self.placements if placements is None else placements,
                                 self.summary if summary is None else summary)

    def test_fifty_images_hundred_catalog_and_four_scaled_summary_placements(self):
        proof = self.check()
        self.assertEqual({key:proof[key] for key in ("images","catalogUses","catalogPlacements",
                                                    "summaryUses","summaryPlacements")},
                         dict(images=50,catalogUses=50,catalogPlacements=100,
                              summaryUses=15,summaryPlacements=30))
        self.assertEqual(len(proof["placements"]), 130)
        self.assertTrue(all(record["outsideOriginalBounds"] == 0 for record in proof["placements"]))
        summary = [record for record in proof["placements"] if record["kind"] == "summary"]
        self.assertEqual([(r["x"],r["y"],r["right"],r["bottom"]) for r in summary[:4]],
                         [(509,73,558,110),(510,74,559,111),
                          (570,73,620,110),(571,74,621,111)])

    def test_uniform_original_output_and_crop_shift_or_stretch_decline(self):
        x,y,width,_ = WORDS[0][2]
        for field in (10,11,12):
            rows = self.rows[:]; changed = 0
            for i,row in enumerate(rows):
                values = row.strip("{}").split(",")
                if list(map(float,values[7:10])) == [x/4,y/8,width/4]:
                    values[field] = str(float(values[field])+1)
                    rows[i] = "{"+",".join(values)+"}"; changed += 1
            self.assertGreater(changed, 1)
            with self.subTest(field=field), self.assertRaisesRegex(ValueError,"original layout/context"):
                self.check(rows=rows)

    def test_original_glyph_partition_and_draw_context_decline(self):
        x,y,width,_ = WORDS[0][2]
        chosen = next(i for i,row in enumerate(self.rows)
                      if list(map(float,row.strip("{}").split(",")[7:10])) == [x/4,y/8,width/4])
        for field in (0,1,2,3,4,5,6):
            rows = self.rows[:]; values = rows[chosen].strip("{}").split(",")
            values[field] = str(float(values[field])+.25)
            rows[chosen] = "{"+",".join(values)+"}"
            with self.subTest(field=field), self.assertRaisesRegex(ValueError,"original layout/context"):
                self.check(rows=rows)
        rows = self.rows[:]; values = rows[chosen].strip("{}").split(",")
        values[13] = "UINT64_C(0x0000000000000000)";rows[chosen] = "{"+",".join(values)+"}"
        with self.assertRaisesRegex(ValueError,"original layout/context"):
            self.check(rows=rows)

    def test_summary_scale_crop_context_and_partition_mutations_decline(self):
        for field in (0,5,7,10,11,12,13,14,15):
            rows = self.summary[:]; values = rows[0].strip("{}").split(",")
            values[field] = ("UINT64_C(0x0000000000000000)" if field == 13
                             else str(float(values[field])+.25))
            rows[0] = "{"+",".join(values)+"}"
            with self.subTest(field=field), self.assertRaisesRegex(ValueError,"Main-summary layout/scales"):
                self.check(summary=rows)
        with self.assertRaisesRegex(ValueError,"Main-summary layout/scales"):
            self.check(summary=self.summary[:-1])

    def test_every_new_image_requires_original_vertical_padding(self):
        for word in WORDS:
            pixels = self.pixels[:];x,y,_,_ = word[2]
            pixels[((y*1024+x)*4)+3] = 255
            with self.subTest(word=word[0]), self.assertRaisesRegex(ValueError,"padded original vertical band"):
                self.check(pixels=pixels)

    def test_original_horizontal_bounds_and_allocation_coverage_decline(self):
        word = next(word for word in WORDS if "location/track.svg" in word[-1])
        x,y,_,height = word[2];pixels = self.pixels[:]
        pixels[(((y+height//2)*1024+x)*4)+3] = 255
        with self.assertRaisesRegex(ValueError,"original horizontal bounds"):
            self.check(pixels=pixels)
        changed = copy.deepcopy(self.placements)
        next(p for p in changed["locationPlacements"] if p["file"] == "track.svg")["atlas"][0] += 1
        with self.assertRaisesRegex(ValueError,"original allocation/group"):
            self.check(placements=changed)
        with self.assertRaises(ValueError):
            self.check(rows=self.rows[:-1])
        changed = copy.deepcopy(self.placements); del changed["optionsPlacements"]
        with self.assertRaisesRegex(ValueError,"placement families"):
            self.check(placements=changed)

    def test_selected_original_summary_catalog_is_pinned(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            names = ["catalog.json", "right-labels/catalog.json"] + [
                family+"/catalog.json" for family in FAMILIES if family != "main"]
            for name in names:
                target = directory/name;target.parent.mkdir(parents=True,exist_ok=True)
                target.write_bytes((self.directory/name).read_bytes())
            path = directory/"right-labels/catalog.json"
            path.write_bytes(path.read_bytes()+b" ")
            with self.assertRaisesRegex(ValueError,"Main-summary source catalog"):
                self.check(directory=directory)


if __name__ == "__main__":
    unittest.main()

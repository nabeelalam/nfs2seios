"""Adopted Player ink must fit every preserved original interaction area."""
# SPDX-License-Identifier: MIT
import json
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
sys.path.insert(0,str(ROOT/'tests'))
from generate_frontend_pixels import player_ink_fit
from frontend_options_atlas_test import decode_current

class PlayerInkFit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory=ROOT/'assets/hud/frontend'
        cls.placements=json.loads((cls.directory/'generated.json').read_text())['playerPlacements']
        cls.rows=[line.rstrip(',') for line in (ROOT/'src/FrontendWordLayout.inc').read_text().splitlines() if line.startswith('{')]
        cls.pixels=decode_current()
    def test_all_sixty_seven_original_areas(self):
        result=player_ink_fit(self.directory,self.pixels,self.rows,self.placements)
        self.assertEqual(len(result),34)
        self.assertEqual(sum(r['placements'] for r in result),67)
        self.assertTrue(all(r['outsideMatchingHorizontalBounds']==0 for r in result))
    def test_rejects_the_observed_half_pixel_left_overhang(self):
        # Recreate the earlier two-atlas-pixel error without changing the real art.
        for filename in ('car.svg','color.svg'):
            pixels=self.pixels[:]
            x,y,w,h=next(p['atlas'] for p in self.placements if p['file']==filename)
            for row in range(h):
                at=((y+row)*1024+x)*4
                old=pixels[at:at+w*4]
                pixels[at:at+w*4]=old[8:]+b'\xff\xff\xff\0'*2
            with self.subTest(filename=filename),self.assertRaisesRegex(ValueError,'leaves original interaction bounds'):
                player_ink_fit(self.directory,pixels,self.rows,self.placements)
    def test_missing_geometry_declines(self):
        with self.assertRaises(ValueError):
            player_ink_fit(self.directory,self.pixels,self.rows[:726],self.placements)
        with self.assertRaises(ValueError):
            player_ink_fit(self.directory,self.pixels,self.rows,self.placements[:-1])

if __name__=='__main__':unittest.main()

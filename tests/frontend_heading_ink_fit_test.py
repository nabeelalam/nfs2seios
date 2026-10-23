"""Shared classic headings stay inside every original text placement."""
# SPDX-License-Identifier: MIT
import copy
import json
from pathlib import Path
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
sys.path.insert(0,str(ROOT/'tests'))
from generate_frontend_pixels import heading_ink_fit
from frontend_options_atlas_test import decode_current
from frontend_player_heading_art import FAMILIES,WORDS

class HeadingInkFit(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory=ROOT/'assets/hud/frontend'
        m=json.loads((cls.directory/'generated.json').read_bytes())
        cls.placements={f+'Placements':m['optionsAtlas']['optionsPlacements'] if f=='options' else m[f+'Placements'] for f in FAMILIES}
        count=0
        for family in FAMILIES:
            c=json.loads((cls.directory/('catalog.json' if family=='main' else family+'/catalog.json')).read_bytes())['words']
            count+=sum(len(w['glyphs']) if family=='main' else sum(len(p['glyphs']) for p in w['placements']) for w in c)
        cls.rows=[line.rstrip(',') for line in (ROOT/'src/FrontendWordLayout.inc').read_text().splitlines() if line.startswith('{')][:count]
        cls.pixels=decode_current()
    def test_all_eleven_shared_uses_and_twenty_two_original_placements(self):
        result=heading_ink_fit(self.directory,self.pixels,self.rows,self.placements)
        self.assertEqual(len(result),22)
        self.assertEqual(set(r['use'] for r in result),{a for w in WORDS for a in w[-1]})
        self.assertTrue(all(r['outsideOriginalBounds']==0 for r in result))
    def test_vertical_padding_is_enforced_for_all_four_images(self):
        for word in WORDS:
            pixels=self.pixels[:];x,y,w,h=word[2]
            pixels[((y*1024+x)*4)+3]=255
            with self.subTest(word=word[0]),self.assertRaisesRegex(ValueError,'padded original vertical band'):
                heading_ink_fit(self.directory,pixels,self.rows,self.placements)
    def test_uniform_vertical_layout_shift_and_stretch_decline(self):
        for field, value in ((11, 100.0), (12, 100.0)):
            rows=self.rows[:];changed=0
            for i, row in enumerate(rows):
                values=row.strip('{}').split(',')
                if list(map(float,values[7:10]))==[592/4,961/8,284/4]:
                    values[field]=str(value);rows[i]='{'+','.join(values)+'}';changed+=1
            self.assertEqual(changed,20)
            with self.subTest(field=field),self.assertRaisesRegex(ValueError,'layout/ink changed'):
                heading_ink_fit(self.directory,self.pixels,rows,self.placements)
    def test_missing_rows_and_shifted_geometry_decline(self):
        with self.assertRaises(ValueError):
            heading_ink_fit(self.directory,self.pixels,self.rows[:-1],self.placements)
        p=copy.deepcopy(self.placements)
        next(v for v in p['optionsPlacements'] if v['file']=='done.svg')['atlas'][0]+=1
        with self.assertRaisesRegex(ValueError,'allocation changed'):
            heading_ink_fit(self.directory,self.pixels,self.rows,p)

if __name__=='__main__':unittest.main()

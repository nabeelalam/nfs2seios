# SPDX-License-Identifier: MIT
"""Finite authored bonus additions; no original game data or renderer required."""
from pathlib import Path
import copy
import hashlib
import json
import re
import sys
import tempfile
import unittest
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path[:0]=[str(ROOT/'scripts'),str(ROOT/'tests')]
import frontend_bonus_car_word_art as bonus
from frontend_options_atlas_test import decode_current
from generate_frontend_pixels import text_identity
from prepare_renderer import frontend_pixel_parts

class BonusWords(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.directory=ROOT/'assets/hud/frontend'
        cls.geometry=ROOT/'src/FrontendFontLayout.inc'
        cls.pixels=decode_current(full=True)
        cls.table=(ROOT/'src/FrontendWordLayout.inc').read_text()
        cls.meta=json.loads((cls.directory/'generated.json').read_text())

    def test_full_image_and_legacy_prefix_are_authenticated(self):
        legacy=bonus.verify(self.pixels,self.table,self.meta['bonusCarWordArt'],self.directory,self.geometry)
        self.assertEqual(hashlib.sha256(self.pixels[:bonus.BASE_BYTES]).hexdigest(),bonus.BASE_SHA)
        self.assertEqual(hashlib.sha256(legacy.encode()).hexdigest(),bonus.LAYOUT_SHA)
        self.assertEqual(self.meta['raster'],[1024,2120])
        self.assertEqual(len(self.pixels)-bonus.BASE_BYTES,294912)
        self.assertEqual(self.pixels[bonus.BASE_BYTES:bonus.BASE_BYTES+4096],self.pixels[bonus.BASE_BYTES-4096:bonus.BASE_BYTES])
        self.assertEqual(len(frontend_pixel_parts((ROOT/'src/FrontendFontPixels.inc').read_text())),82)

    def test_three_complete_words_source_cells_and_sampling_guards(self):
        images=bonus.load(self.directory,self.geometry)
        for word,rgba in images:
            self.assertEqual(text_identity(word['text']),int(word['context'],16))
            x,y,w,h=word['atlas']
            self.assertEqual(y,2050)
            self.assertEqual(sum(c[1]for c in word['sourceCells'])*4,w)
            for row in range(-1,h+2):
                for col in range(-2,w+2):
                    sy=min(h-1,max(0,row));sx=min(w-1,max(0,col));at=((y+row)*1024+x+col)*4
                    self.assertEqual(self.pixels[at:at+4],rgba[(sy*w+sx)*4:(sy*w+sx+1)*4])
        rows=self.table.split(bonus.BEGIN)[1].split(bonus.END)[0].splitlines()
        records=[s for s in rows if s.startswith('{')]
        self.assertEqual(len(records),54)
        for word,_ in images:
            own=[r for r in records if '0x'+word['context']in r]
            self.assertEqual(len(own),2*len(word['sourceCells']))
            for shadow in(0,1):
                edge=0
                for row,(cell,width)in zip(own[shadow*len(word['sourceCells']):(shadow+1)*len(word['sourceCells'])],word['sourceCells']):
                    values=list(map(float,row.split(',UINT64_C')[0].lstrip('{').split(',')))
                    self.assertEqual(values[:5],[cell,365+shadow+edge,84+shadow,width,51])
                    self.assertAlmostEqual(values[5],edge/(word['atlas'][2]/4),places=10)
                    edge+=width
                self.assertEqual(edge*4,word['atlas'][2])

    def test_source_semantics_fail_even_if_outer_digest_is_resealed(self):
        original=json.loads((self.directory/'bonus-car-word-art.json').read_text())
        for label,change in [
            ('context',lambda d:d['words'][0].update(context='0000000000000000')),
            ('source-cell',lambda d:d['words'][0]['sourceCells'][0].__setitem__(0,1)),
            ('source-width',lambda d:d['words'][0]['sourceCells'][0].__setitem__(1,99)),
            ('foreign-name',lambda d:d['words'][0].update(text='Arbitrary text')),
            ('old-bank',lambda d:d['words'][0]['atlas'].__setitem__(1,2000)),
            ('odd-runs',lambda d:d['words'][0]['alphaRuns'].append(1)),
            ('zero-run',lambda d:d['words'][0]['alphaRuns'].__setitem__(0,0)),
            ('run-overflow',lambda d:d['words'][0]['alphaRuns'].__setitem__(0,65535)),
            ('bad-alpha',lambda d:d['words'][0]['alphaRuns'].__setitem__(1,256)),
            ('image-digest',lambda d:d['words'][0].update(rgbaSha256='0'*64)),
        ]:
            with self.subTest(label=label),tempfile.TemporaryDirectory()as tmp:
                d=copy.deepcopy(original);change(d);raw=json.dumps(d).encode();folder=Path(tmp);(folder/'bonus-car-word-art.json').write_bytes(raw)
                with patch.object(bonus,'SOURCE_SHA',hashlib.sha256(raw).hexdigest()):
                    with self.assertRaises(ValueError):bonus.load(folder,self.geometry)

    def test_old_prefix_and_new_pixels_or_metadata_cannot_drift(self):
        legacy=bonus.strip_layout(self.table)
        changed=bytearray(self.pixels[:bonus.BASE_BYTES]);changed[0]^=1
        with self.assertRaises(ValueError):bonus.apply(changed,legacy,self.directory,self.geometry)
        changed=bytearray(self.pixels);changed[-1]^=1
        with self.assertRaises(ValueError):bonus.verify(changed,self.table,self.meta['bonusCarWordArt'],self.directory,self.geometry)
        changed=copy.deepcopy(self.meta['bonusCarWordArt']);changed['newRows']-=1
        with self.assertRaises(ValueError):bonus.verify(self.pixels,self.table,changed,self.directory,self.geometry)
        with self.assertRaises(ValueError):bonus.strip_layout(self.table+bonus.END)
        with self.assertRaises(FileNotFoundError):bonus.load(Path('/missing-bonus-fixture'),self.geometry)

if __name__=='__main__':unittest.main()

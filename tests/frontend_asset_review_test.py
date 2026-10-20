"""Only the exact reviewed condensed menu vectors may cross publication gates."""
import hashlib
import json
import tempfile
import copy
import sys
import unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'scripts'))
from frontend_player_atlas import _player_words
from frontend_opponents_atlas import validate_words
from hud_asset_review import ROOT, FRONTEND_WORDS, LOCATION_WORDS, SETUP_WORDS, SETUP_SHARED, PLAYER_WORDS, PLAYER_SHARED, OPPONENTS_WORDS, OPPONENTS_SHARED, reviewed_remaster_svg


class FrontendAssetReview(unittest.TestCase):
    def test_reviewed_words(self):
        for name in list(FRONTEND_WORDS)+['location/'+n for n in LOCATION_WORDS]+['setup/'+n for n in SETUP_WORDS if n not in SETUP_SHARED]+['player/'+n for n in PLAYER_WORDS if n not in PLAYER_SHARED]+['opponents/'+n for n in OPPONENTS_WORDS if n not in OPPONENTS_SHARED]:
            relative='assets/hud/frontend/'+name+'.svg'
            with self.subTest(name=name):
                self.assertTrue(reviewed_remaster_svg(relative,(ROOT/relative).read_bytes()))

    def test_changed_art_or_unreviewed_path(self):
        relative='assets/hud/frontend/location.svg'
        data=(ROOT/relative).read_bytes()
        self.assertFalse(reviewed_remaster_svg(relative,data+b' '))
        for path in ('location.svg','assets/hud/frontend/private.svg','assets/hud/frontend/LOCATION.svg'):
            self.assertFalse(reviewed_remaster_svg(path,data))

    def test_schema_stays_bounded_even_with_pin(self):
        relative='assets/hud/frontend/location.svg'
        data=(ROOT/relative).read_bytes()
        for mutated in (data.replace(b'location',b'private label'),
                        data.replace(b'width="66"',b'width="67"'),
                        data.replace(b'<text ',b'<image '),
                        data.replace(b'</svg>',b'<text x="0">arbitrary</text></svg>'),
                        data.replace(b'</svg>',b'<image href="data:fixture"/></svg>')):
            manifest={'reviewedSvgSha256':{relative:hashlib.sha256(mutated).hexdigest()}}
            with self.subTest(mutated=mutated), patch('hud_asset_review.json.loads',return_value=manifest):
                self.assertFalse(reviewed_remaster_svg(relative,mutated))

    def test_location_schema_does_not_allow_other_text_even_with_pin(self):
        relative='assets/hud/frontend/location/pacific-spirit.svg'
        data=(ROOT/relative).read_bytes()
        for mutated in (data.replace(b'Pacific Spirit',b'private label'),
                        data.replace(b'height="51"',b'height="52"'),
                        data.replace(b'</svg>',b'<image href="private.png"/></svg>')):
            manifest={'reviewedSvgSha256':{relative:hashlib.sha256(mutated).hexdigest()}}
            with patch('hud_asset_review.json.loads',return_value=manifest):
                self.assertFalse(reviewed_remaster_svg(relative,mutated))

    def test_setup_schema_does_not_allow_other_text_even_with_pin(self):
        relative='assets/hud/frontend/setup/game-type.svg'
        data=(ROOT/relative).read_bytes()
        for mutated in (data.replace(b'game type',b'private label'),
                        data.replace(b'height="51"',b'height="52"'),
                        data.replace(b'</svg>',b'<image href="private.png"/></svg>')):
            manifest={'reviewedSvgSha256':{relative:hashlib.sha256(mutated).hexdigest()}}
            with patch('hud_asset_review.json.loads',return_value=manifest):
                self.assertFalse(reviewed_remaster_svg(relative,mutated))

    def test_setup_shared_words_have_no_independent_file_alias(self):
        for name in SETUP_SHARED:
            existing='assets/hud/frontend/location/'+name+'.svg'
            data=(ROOT/existing).read_bytes()
            self.assertTrue(reviewed_remaster_svg(existing,data))
            self.assertFalse(reviewed_remaster_svg('assets/hud/frontend/setup/'+name+'.svg',data))


    def test_player_schema_stays_bounded_even_with_pin(self):
        relative='assets/hud/frontend/player/italdesign-cala.svg'
        data=(ROOT/relative).read_bytes()
        for mutated in (data.replace('Italdesign Calà'.encode(),b'private label'),
                        data.replace(b'height="51"',b'height="52"'),
                        data.replace(b'</svg>',b'<image href="private.png"/></svg>')):
            self.assertNotEqual(data,mutated)
            manifest={'reviewedSvgSha256':{relative:hashlib.sha256(mutated).hexdigest()}}
            with patch('hud_asset_review.json.loads',return_value=manifest):
                self.assertFalse(reviewed_remaster_svg(relative,mutated))

    def test_player_shared_words_have_no_independent_alias(self):
        for name,family in PLAYER_SHARED.items():
            existing='assets/hud/frontend/'+family+'/'+name+'.svg'
            data=(ROOT/existing).read_bytes()
            self.assertTrue(reviewed_remaster_svg(existing,data))
            self.assertFalse(reviewed_remaster_svg('assets/hud/frontend/player/'+name+'.svg',data))


    def test_player_catalog_rejects_unknown_text_sharing_and_proportions(self):
        catalog=json.loads((ROOT/'assets/hud/frontend/player/catalog.json').read_text())
        changes=[('text','unreviewed'),('width',640),('sharedFamily','private'),
                 ('font','unreviewed'),('size',48),('condensation',1),('baseline',0),
                 ('kind','unreviewed'),('placements',[]),('placements',[{}]*33)]
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory);(root/'player').mkdir();path=root/'player/catalog.json'
            path.write_text(json.dumps(catalog))
            self.assertEqual(len(_player_words(root)[0]),34)
            for field,value in changes:
                changed=copy.deepcopy(catalog);changed['words'][0][field]=value
                path.write_text(json.dumps(changed))
                with self.subTest(field=field),self.assertRaises(ValueError):_player_words(root)
            changed=copy.deepcopy(catalog);changed['words'].reverse()
            path.write_text(json.dumps(changed))
            with self.assertRaises(ValueError):_player_words(root)


    def test_opponents_schema_stays_bounded_even_with_pin(self):
        relative='assets/hud/frontend/opponents/skill-level.svg'
        data=(ROOT/relative).read_bytes()
        for mutated in (data.replace(b'skill level',b'private label'),
                        data.replace(b'height="51"',b'height="52"'),
                        data.replace(b'</svg>',b'<image href="private.png"/></svg>')):
            self.assertNotEqual(data,mutated)
            manifest={'reviewedSvgSha256':{relative:hashlib.sha256(mutated).hexdigest()}}
            with patch('hud_asset_review.json.loads',return_value=manifest):
                self.assertFalse(reviewed_remaster_svg(relative,mutated))

    def test_opponents_shared_words_have_no_independent_alias(self):
        for name,family in OPPONENTS_SHARED.items():
            existing='assets/hud/frontend/'+family+'/'+name+'.svg'
            data=(ROOT/existing).read_bytes()
            self.assertTrue(reviewed_remaster_svg(existing,data))
            self.assertFalse(reviewed_remaster_svg('assets/hud/frontend/opponents/'+name+'.svg',data))

    def test_opponents_catalog_rejects_malformed_source_geometry(self):
        words=json.loads((ROOT/'assets/hud/frontend/opponents/catalog.json').read_text())['words']
        validate_words(words)
        changes=[]
        for k,v in [('file','../private.svg'),('text','unreviewed'),('width',640),('kind','unreviewed'),('sharedFamily','private'),('font','unreviewed'),('size',48),('condensation',1),('baseline',0),('placements',[])]:
            changes.append(('word-'+k,lambda w,k=k,v=v:w[0].__setitem__(k,v)))
        for k,v in [('x',-1),('y',430),('right',641),('width',float('nan')),('alignment','center'),('glyphs',[])]:
            changes.append(('placement-'+k,lambda w,k=k,v=v:w[0]['placements'][0].__setitem__(k,v)))
        for k,v in [('cell',121),('sourcePage',1),('x',-1),('w',float('inf')),('h',52),('left',.5),('drawText','unreviewed')]:
            changes.append(('glyph-'+k,lambda w,k=k,v=v:w[0]['placements'][0]['glyphs'][0].__setitem__(k,v)))
        for name,mutate in changes:
            candidate=copy.deepcopy(words);mutate(candidate)
            with self.subTest(name=name),self.assertRaises(ValueError):validate_words(candidate)


if __name__=='__main__':unittest.main()

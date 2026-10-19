"""Only the exact reviewed condensed menu vectors may cross publication gates."""
import hashlib
import sys
import unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'scripts'))
from hud_asset_review import ROOT, FRONTEND_WORDS, reviewed_remaster_svg


class FrontendAssetReview(unittest.TestCase):
    def test_reviewed_words(self):
        for name in FRONTEND_WORDS:
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


if __name__=='__main__':unittest.main()

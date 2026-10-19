"""Publication exceptions stay limited to pinned fonts and reviewed safe SVGs."""
from pathlib import Path
import json
import sys
import unittest
import hashlib
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from hud_asset_review import ROOT,FONT_FILES,reviewed_font_file,reviewed_remaster_svg
from publication_audit import content_findings

class SourceReview(unittest.TestCase):
    def test_only_exact_font_and_license_files(self):
        for name in FONT_FILES:
            data=(ROOT/name).read_bytes()
            self.assertTrue(reviewed_font_file(name,data))
            self.assertEqual(content_findings(name,data),[])
            self.assertFalse(reviewed_font_file(name,data+b'changed'))
            self.assertFalse(reviewed_font_file('assets/hud/fonts/unreviewed.ttf',data))
            self.assertTrue(content_findings(name,data+b'changed'))

    def test_reviewed_vectors_and_mutation_rejection(self):
        sources=json.loads((ROOT/'assets/hud/remaster-sources.json').read_text())['reviewedSvgSha256']
        self.assertEqual(len(sources),76)
        for name in sources:
            data=(ROOT/name).read_bytes()
            self.assertTrue(reviewed_remaster_svg(name,data),name)
            self.assertEqual(content_findings(name,data),[],name)
            self.assertFalse(reviewed_remaster_svg(name,data+b'<script/>'))
            self.assertFalse(reviewed_remaster_svg(name.replace('assets/hud/','assets/private/'),data))

    def test_surface_schema_stays_bounded_even_with_updated_digest(self):
        cases=[('assets/hud/gauge/lower.svg',b'rgb(66.7,65.55,66.7)',b'rgb(999,65.55,66.7)'),
               ('assets/hud/gear/face.svg',b'<circle ',b'<image '),
               ('assets/hud/small-W.svg',b'font-weight="500"',b'font-weight="900"'),
               ('assets/hud/record-colon.svg',b'font-family="Oxanium"',b'font-family="Unreviewed"')]
        for name,old,new in cases:
            data=(ROOT/name).read_bytes()
            self.assertIn(old,data)
            changed=data.replace(old,new)
            ledger={'reviewedSvgSha256':{name:hashlib.sha256(changed).hexdigest()}}
            with patch('hud_asset_review.json.loads',return_value=ledger):
                self.assertFalse(reviewed_remaster_svg(name,changed))

if __name__=='__main__':unittest.main()

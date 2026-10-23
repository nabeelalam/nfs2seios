"""Finite authored volume material trust, reproducible decoder bytes and provenance."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import hashlib
import json
import re
import sys
import unittest
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from generate_volume_bar_pixels import SOURCE,SOURCE_SHA256,RECTANGLE,reviewed_volume_bar_source,encode_rle,generate
from hud_asset_review import reviewed_remaster_svg
from publication_audit import content_findings

class VolumeMaterialSources(unittest.TestCase):
    def test_only_exact_authored_source(self):
        data=(ROOT/SOURCE).read_bytes()
        self.assertEqual(hashlib.sha256(data).hexdigest(),SOURCE_SHA256)
        self.assertTrue(reviewed_volume_bar_source(SOURCE,data))
        self.assertTrue(reviewed_remaster_svg(SOURCE,data))
        self.assertEqual(content_findings(SOURCE,data),[])
        for bad in (data+b' ',data.replace(b'width="80"',b'width="81"'),data.replace(b'<path ',b'<image ',1)):
            self.assertFalse(reviewed_volume_bar_source(SOURCE,bad))
            self.assertFalse(reviewed_remaster_svg(SOURCE,bad))
        self.assertFalse(reviewed_volume_bar_source(SOURCE.replace('/pause/','/private/'),data))
    def test_generated_receipt_and_independent_full_stream(self):
        text,receipt=generate()
        self.assertEqual((ROOT/'src/VolumeBarPixels.inc').read_bytes(),text.encode())
        self.assertEqual(json.loads((ROOT/'assets/hud/volume-bar-generated.json').read_text()),receipt)
        self.assertEqual(RECTANGLE,[920,398,1000,510])
        self.assertEqual(receipt['logicalProfile'],[20,28])
        self.assertEqual(receipt['logicalDivider'],18)
        self.assertEqual(receipt['logicalChannelStart'],19)
        self.assertEqual(receipt['continuousMirrorX'],167)
        self.assertEqual(receipt['transparentTrailingColumns'],[167,168,169])
        self.assertEqual(receipt['preservedGutterRows'],[396,397,510,511])
        self.assertFalse(receipt['originalGamePixelsIncluded'])
        self.assertLess(len(text.encode()),1048576)
        match=re.search(r'nfs_volume_bar_rle\[\]=\{([^}]*)\};',text,re.S)
        self.assertIsNotNone(match)
        raw=bytes(int(n) for n in re.findall(r'\d+',match[1]));self.assertEqual(len(raw)%6,0)
        pixels=bytearray()
        for at in range(0,len(raw),6):
            count=int.from_bytes(raw[at:at+2],'little');self.assertGreater(count,0)
            self.assertLessEqual(len(pixels)//4+count,80*112)
            pixels+=raw[at+2:at+6]*count
        self.assertEqual(len(pixels),80*112*4)
        self.assertEqual(hashlib.sha256(raw).hexdigest(),receipt['rleSha256'])
        self.assertEqual(hashlib.sha256(pixels).hexdigest(),receipt['rgbaSha256'])
        for y in list(range(4))+list(range(104,112)):
            self.assertEqual(pixels[(y*80)*4+3:((y+1)*80)*4:4],bytes(80))
        self.assertTrue(any(0<a<255 for a in pixels[3::4]))
    def test_encoder_bounds_and_roundtrip(self):
        for bad in (None,b'',bytes(80*112*4-1),bytes(80*112*4+4)):
            with self.assertRaises(ValueError):encode_rle(bad)
        color=bytes([2,3,4,255]);raw=encode_rle(color*(80*112))
        self.assertEqual(raw,(80*112).to_bytes(2,'little')+color)

if __name__=='__main__':unittest.main()

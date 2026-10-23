"""Authored pause header has exact source identities and reproducible bounded pixels."""
# SPDX-License-Identifier: MIT
from pathlib import Path
import hashlib
import json
import re
import sys
import unittest
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
from generate_pause_header_pixels import SOURCES, reviewed_pause_header_source, generate, encode_rle
from hud_asset_review import reviewed_remaster_svg
from publication_audit import content_findings

class PauseHeaderSources(unittest.TestCase):
    def test_exact_reviewed_sources_and_mutation_rejection(self):
        self.assertEqual(len(SOURCES),2)
        for name,label,w,h,x,y,digest,viewbox in SOURCES:
            data=(ROOT/name).read_bytes()
            self.assertTrue(reviewed_pause_header_source(name,data))
            self.assertTrue(reviewed_remaster_svg(name,data))
            self.assertEqual(content_findings(name,data),[])
            self.assertEqual(hashlib.sha256(data).hexdigest(),digest)
            for changed in (data+b' ',data.replace(b'<path ',b'<image ',1),data.replace(b'height="112"',b'height="113"',1)):
                self.assertFalse(reviewed_pause_header_source(name,changed))
                self.assertFalse(reviewed_remaster_svg(name,changed))
            self.assertFalse(reviewed_pause_header_source(name.replace('/pause/','/private/'),data))
            other=SOURCES[1 if label=='title' else 0][0]
            self.assertFalse(reviewed_pause_header_source(other,data))
    def test_independent_generated_stream_and_receipt(self):
        text,receipt=generate() # Includes project_fonts: no ambient font fallback.
        self.assertEqual((ROOT/'src/PauseHeaderPixels.inc').read_bytes(),text.encode())
        self.assertEqual(json.loads((ROOT/'assets/hud/pause-header-generated.json').read_text()),receipt)
        self.assertLess(len(text.encode()),1048576)
        self.assertFalse(receipt['originalGamePixelsIncluded'])
        self.assertEqual(receipt['preservedNumericGutterRows'],[396,397])
        self.assertEqual(receipt['preservedGlyphGutterRows'],[510,511])
        for entry,spec in zip(receipt['entries'],SOURCES):
            name,label,w,h,x,y,digest,_=spec
            match=re.search(r'static const unsigned char nfs_pause_header_'+label+r'_rle\[\]=\{([^}]*)\};',text,re.S)
            self.assertIsNotNone(match)
            raw=bytes(int(n) for n in re.findall(r'\d+',match[1]))
            self.assertEqual(len(raw)%6,0)
            pixels=bytearray()
            for at in range(0,len(raw),6):
                count=int.from_bytes(raw[at:at+2],'little')
                self.assertGreater(count,0)
                self.assertLessEqual(len(pixels)//4+count,w*h)
                pixels+=raw[at+2:at+6]*count
            self.assertEqual(len(pixels),w*h*4)
            self.assertTrue(all(a==255 for a in pixels[3::4]))
            self.assertEqual(hashlib.sha256(pixels).hexdigest(),entry['rgbaSha256'])
            self.assertEqual(hashlib.sha256(raw).hexdigest(),entry['rleSha256'])
            self.assertEqual(entry['rectangle'],[x,y,x+w,y+h])
            self.assertEqual(h,112)
            self.assertEqual(y+h,510)
    def test_encoder_rejects_invalid_or_translucent_data(self):
        for bad in (None,b'',b'123',b'\x01\x02\x03\xfe'):
            with self.assertRaises(ValueError):encode_rle(bad)
        self.assertEqual(encode_rle(bytes([1,2,3,255])*4),bytes([4,0,1,2,3,255]))

if __name__=='__main__':unittest.main()

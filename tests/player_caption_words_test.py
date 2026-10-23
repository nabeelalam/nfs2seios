"""Two fixed-word caption graphics: identity, geometry and bounded decoder."""
# SPDX-License-Identifier: MIT
import copy
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('caption_art',ROOT/'scripts/generate_player_caption_words.py');art=importlib.util.module_from_spec(spec);spec.loader.exec_module(art)
class Words(unittest.TestCase):
 @classmethod
 def setUpClass(cls):cls.document,cls.images=art.load()
 def test_exact_two_complete_images_and_generated_headers(self):
  self.assertEqual(len(self.images),2)
  for image,word in zip(self.images,art.WORDS):
   self.assertEqual(len(image),word[2]*word[3]*4);self.assertEqual(image[0::4],b'\xff'*(word[2]*word[3]));self.assertEqual(image[0::4],image[1::4]);self.assertEqual(image[1::4],image[2::4]);self.assertTrue(any(image[3::4]));self.assertIn(0,image[3::4])
  for name,text in art.outputs().items():self.assertEqual((ROOT/'src/hd'/name).read_bytes(),text.encode('ascii'))
 def test_identity_geometry_and_malformed_payloads_decline(self):
  for field,value in [('word',True),('text','player three'),('width',273),('destination',[99,198,168,230]),('rgbaSha256','0'*64),('alphaRuns',[]),('alphaRuns',[1]),('alphaRuns',[0,0]),('alphaRuns',[True,1]),('alphaRuns',[1,256]),('alphaRuns',[65536,0]),('alphaRuns',[34817,0]),('alphaRuns',[34816,0])]:
   d=copy.deepcopy(self.document);d['words'][0][field]=value
   with self.subTest(field=field,value=value),self.assertRaises(ValueError):art.decode(d)
  d=copy.deepcopy(self.document);d['words'].reverse()
  with self.assertRaises(ValueError):art.decode(d)
  with self.assertRaises(ValueError):art._pairs([('schema',1),('schema',1)])
 def test_selected_source_is_exact_and_bounded(self):
  raw=(ROOT/'assets/hud/frontend/player-caption-word-art.json').read_bytes()
  with tempfile.TemporaryDirectory() as directory:
   p=Path(directory)/'caption.json';p.write_bytes(raw);self.assertEqual(art.load(p)[1],self.images)
   for bad in (raw[:-1],raw+b' ',b'x'+raw[1:]):
    p.write_bytes(bad)
    with self.assertRaises(ValueError):art.load(p)
if __name__=='__main__':unittest.main()

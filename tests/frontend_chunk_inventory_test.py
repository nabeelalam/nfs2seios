# SPDX-License-Identifier: MIT
import sys,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from generate_frontend_pixels import chunked_pixels
from prepare_renderer import frontend_pixel_parts
class Chunks(unittest.TestCase):
 def test_run_aligned_chunks_and_copy_inventory(self):
  for size in [6,65520,65526,65520*22]:
   data=bytes([1,0,255,255,255,255])*(size//6);header,outputs=chunked_pixels(data)
   self.assertEqual(frontend_pixel_parts(header),list(outputs));self.assertTrue(all(len(x.encode())<1048576 for x in outputs.values()))
   import re
   restored=bytes(int(v)for content in outputs.values()for v in re.findall(r'\b[0-9]+\b',content.split('\n',1)[1]))
   self.assertEqual(restored,data)
 def test_empty_unaligned_or_excessive_runs_rejected(self):
  for data in [b'',bytes(7),bytes(64*65520+6)]:
   with self.assertRaises(ValueError):chunked_pixels(data)
 def test_missing_duplicate_out_of_order_and_foreign_include_rejected(self):
  valid='#include "FrontendFontPixelsPart0.inc"\n'
  for text in ['',valid+valid,valid.replace('Part0','Part1'),valid+'#include "other.inc"\n',valid+'#include "../private.inc"\n',''.join('#include "FrontendFontPixelsPart%d.inc"\n'%i for i in range(65))]:
   with self.assertRaises(ValueError):frontend_pixel_parts(text)
if __name__=='__main__':unittest.main()

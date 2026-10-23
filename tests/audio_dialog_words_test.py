"""Eleven finite Audio dialog images: exact source, padding and generation."""
# SPDX-License-Identifier: MIT
import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('audio_dialog_art', ROOT/'scripts/generate_audio_dialog_words.py')
art = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(art)


class AudioDialogWordsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.document, cls.images = art.load()
        cls.raw = (ROOT/'assets/hud/frontend/audio-dialog-word-art.json').read_bytes()

    def test_exact_eleven_images_dimensions_destinations_and_generated_headers(self):
        self.assertEqual(len(self.raw), 648833)
        self.assertEqual(hashlib.sha256(self.raw).hexdigest(), art.SOURCE_SHA)
        self.assertEqual(len(self.images), 11)
        self.assertEqual([(w[0],w[1],w[2],w[3],w[4]) for w in art.WORDS],
                         [(1, 'menu music', 304, 128, [292, 144, 368, 176]), (2, 'racing music', 324, 128, [287, 170, 368, 202]), (3, 'sound effects', 352, 128, [280, 196, 368, 228]), (4, 'interactive music', 440, 128, [258, 222, 368, 254]), (5, 'racing tune', 292, 128, [295, 248, 368, 280]), (6, 'engine sounds', 376, 128, [274, 274, 368, 306]), (7, 'audio mode', 300, 128, [293, 300, 368, 332]), (8, 'audio quality', 336, 128, [284, 326, 368, 358]), (9, 'DONE', 140, 128, [333, 369, 368, 401]), (10, 'CANCEL', 204, 128, [388, 369, 439, 401]), (11, 'AUDIO', 164, 128, [348, 105, 389, 137])])
        for image, word in zip(self.images, art.WORDS):
            _, _, width, height, destination, sha = word
            count = width*height
            self.assertEqual(len(image), count*4)
            self.assertEqual(hashlib.sha256(image).hexdigest(), sha)
            for channel in range(3): self.assertEqual(image[channel::4], b'\xff'*count)
            alpha = image[3::4]
            self.assertTrue(any(alpha))
            ink = [i for i, value in enumerate(alpha) if value]
            self.assertGreaterEqual(min(i%width for i in ink), 1)
            self.assertLess(max(i%width for i in ink), width-1)
            self.assertGreaterEqual(min(i//width for i in ink), 1)
            self.assertLess(max(i//width for i in ink), height-1)
            self.assertEqual((destination[2]-destination[0])*4, width)
            self.assertEqual((destination[3]-destination[1])*4, height)
        self.assertEqual(set(art.outputs()), {'AudioDialogWordIdentity.h','AudioDialogWordData.h'})
        for name, text in art.outputs().items():
            encoded = text.encode('ascii')
            self.assertLess(len(encoded), 1048576)
            self.assertEqual((ROOT/'src/hd'/name).read_bytes(), encoded)

    def test_first_eight_images_retain_the_accepted_original_identities(self):
        expected=('b2136101eb36d12ab84949302b27487a156adf4eb7ad639cc3bb25a14c238e3f',
                  'd71d6eb73948783011532572694719d8c08b0f2b4c8dffa6a4559eb379400378',
                  '62e099128222e6814ad3ff74602441fbf451711ad579fd31c008c6424338ce22',
                  '2b0769dfc94742c342980f34eba59df5d4c659dbdc556053b71151a86d629d81',
                  '0f2a94bf83527685bb7bbbe450b450befc75015fc8fbd2546f4785aec6ead948',
                  'ad64979b4907b4b0c18efc0c77e20f88f553c17001c92a2f9a43bd46e5c9f5d7',
                  '3735d11c3ba16150a5c55eef0726b2eeaf24070e2c3644d24c9c4d7fd3d32f9b',
                  '25bb6f0e68748be0e3c6e5973de897f3319625049f4ccaf1f664aabae6d8e55e')
        self.assertEqual(tuple(hashlib.sha256(image).hexdigest() for image in self.images[:8]),expected)

    def test_schema_identity_geometry_and_malformed_runs_decline(self):
        for key,value in (('schema',True),('schema',2),('scope','other'),('typeface','other'),
                          ('baselineOriginalUiPixels',24.0),('baselineOriginalUiPixels',True),
                          ('rasterScale',4.0),('rasterScale',True),('words',[])):
            document=copy.deepcopy(self.document);document[key]=value
            with self.subTest(key=key,value=value),self.assertRaises(ValueError): art.decode(document)
        count=art.WORDS[0][2]*art.WORDS[0][3]
        cases=(('word',True),('word',12),('text','player one'),('width',304.0),('height',True),
               ('destination',[292.0,144,368,176]),('destination',[291,144,368,176]),
               ('rgbaSha256','0'*64),('alphaRuns',[]),('alphaRuns',[1]),('alphaRuns',[0,0]),
               ('alphaRuns',[True,1]),('alphaRuns',[1,True]),('alphaRuns',[1,256]),
               ('alphaRuns',[1,-1]),('alphaRuns',[65536,0]),('alphaRuns',[count+1,0]),
               ('alphaRuns',[count,0]),('alphaRuns',[1,255]),('alphaRuns',[1,0]*(count+1)))
        for key,value in cases:
            document=copy.deepcopy(self.document);document['words'][0][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError): art.decode(document)
        document=copy.deepcopy(self.document);document['words'].reverse()
        with self.assertRaises(ValueError): art.decode(document)
        document=copy.deepcopy(self.document);document['words'][0]['extra']=1
        with self.assertRaises(ValueError): art.decode(document)
        document=copy.deepcopy(self.document);document['extra']=1
        with self.assertRaises(ValueError): art.decode(document)
        with self.assertRaises(ValueError): art._pairs([('schema',1),('schema',1)])

    def test_all_four_padding_edges_decline_even_with_matching_decoded_digest(self):
        # Matching digest fixtures ensure padding itself, rather than SHA mismatch, rejects.
        width,height=art.WORDS[0][2:4]
        for position,axis in ((width,'horizontal'),(width*2-1,'horizontal'),
                              (width//2,'vertical'),((height-1)*width+width//2,'vertical')):
            document=copy.deepcopy(self.document)
            alpha=bytearray(self.images[0][3::4]);alpha[position]=255
            runs=[];start=0
            while start<len(alpha):
                end=start+1
                while end<len(alpha) and alpha[end]==alpha[start] and end-start<65535: end+=1
                runs.extend((end-start,alpha[start]));start=end
            rgba=bytearray(b'\xff'*(width*height*4));rgba[3::4]=alpha
            sha=hashlib.sha256(rgba).hexdigest()
            expected=list(art.WORDS);word=list(expected[0]);word[-1]=sha;expected[0]=tuple(word)
            document['words'][0]['rgbaSha256']=sha;document['words'][0]['alphaRuns']=runs
            with self.subTest(position=position),patch.object(art,'WORDS',tuple(expected)),self.assertRaisesRegex(ValueError,axis):
                art.decode(document)

    def test_selected_source_is_pinned_and_generation_checks_reproduce_bytes(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory=Path(temporary);source=directory/'caption.json';source.write_bytes(self.raw)
            self.assertEqual(art.load(source)[1],self.images)
            art.generate(source=source,output_dir=directory)
            art.generate(check=True,source=source,output_dir=directory)
            headers={name:(directory/name).read_bytes() for name in art.outputs()}
            for bad in (self.raw[:-1],self.raw+b' ',b'x'+self.raw[1:]):
                source.write_bytes(bad)
                with self.assertRaises(ValueError): art.load(source)
                with self.assertRaises(ValueError): art.generate(source=source,output_dir=directory)
                self.assertEqual({name:(directory/name).read_bytes() for name in headers},headers)
            source.write_bytes(self.raw)
            name=next(iter(headers));(directory/name).write_bytes(headers[name]+b' ')
            with self.assertRaises(ValueError): art.generate(check=True,source=source,output_dir=directory)
            source.unlink()
            with self.assertRaises(OSError): art.load(source)


if __name__=='__main__': unittest.main()

"""Five finite Controls dialog images: exact source, padding and generation."""
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
SPEC = importlib.util.spec_from_file_location('controls_dialog_art', ROOT/'scripts/generate_controls_dialog_words.py')
art = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(art)


class ControlsDialogWordsTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.document, cls.images = art.load()
        cls.raw = (ROOT/'assets/hud/frontend/controls-dialog-word-art.json').read_bytes()

    def test_exact_five_images_dimensions_destinations_and_generated_headers(self):
        self.assertEqual(len(self.raw), 293786)
        self.assertEqual(hashlib.sha256(self.raw).hexdigest(), art.SOURCE_SHA)
        self.assertEqual(len(self.images), 5)
        self.assertEqual([(w[0],w[1],w[2],w[3],w[4]) for w in art.WORDS],
                         [(1, 'CONTROLS', 272, 128, [382, 105, 450, 137]), (2, 'setup', 144, 128, [380, 170, 416, 202]), (3, 'steering sensitivity', 492, 128, [293, 196, 416, 228]), (4, 'force feedback', 384, 128, [320, 224, 416, 256]), (5, 'DONE', 140, 128, [381, 267, 416, 299])])
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
        self.assertEqual(set(art.outputs()), {'ControlsDialogWordIdentity.h','ControlsDialogWordData.h'})
        for name, text in art.outputs().items():
            encoded = text.encode('ascii')
            self.assertLess(len(encoded), 1048576)
            self.assertEqual((ROOT/'src/hd'/name).read_bytes(), encoded)

    def test_done_reuses_accepted_audio_pixels_without_geometry_aliasing(self):
        prior=json.loads((ROOT/'assets/hud/frontend/audio-dialog-word-art.json').read_bytes())['words'][8]
        current=self.document['words'][4]
        self.assertEqual(current['rgbaSha256'],prior['rgbaSha256'])
        self.assertEqual(current['alphaRuns'],prior['alphaRuns'])
        self.assertNotEqual(current['destination'],prior['destination'])
        with self.assertRaises(ValueError):
            art.load(ROOT/'assets/hud/frontend/graphics-dialog-word-art.json')

    def test_schema_identity_geometry_and_malformed_runs_decline(self):
        for key,value in (('schema',True),('schema',2),('scope','other'),('typeface','other'),
                          ('baselineOriginalUiPixels',24.0),('baselineOriginalUiPixels',True),
                          ('rasterScale',4.0),('rasterScale',True),('words',[])):
            document=copy.deepcopy(self.document);document[key]=value
            with self.subTest(key=key,value=value),self.assertRaises(ValueError): art.decode(document)
        count=art.WORDS[0][2]*art.WORDS[0][3]
        cases=(('word',True),('word',9),('text','player one'),('width',360.0),('height',True),
               ('destination',[336.0,144,426,176]),('destination',[335,144,426,176]),
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

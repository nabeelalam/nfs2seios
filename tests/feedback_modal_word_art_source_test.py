"""Finite authored-word source tests; no game or private assets required."""
# SPDX-License-Identifier: MIT
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('modal_words', ROOT / 'scripts/generate_feedback_modal_words.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)

class ModalWordSource(unittest.TestCase):
    def test_identity_and_exact_sources(self):
        m.check()
        self.assertEqual(len(m.decode(m.read_runs())), 432 * 128 * 4)
        data = (ROOT / 'src/hd/FeedbackModalWordData.h').read_text()
        self.assertNotIn('title_alpha', data)
        self.assertNotIn(m.TITLE_SHA, data)

    def test_invalid_runs(self):
        good = m.read_runs()
        for value in (None, [], good[:-1], [0, 0], [65535, 0], [55296, 0], [55296, 255], [True, 0], [55296, 256]):
            with self.assertRaises(ValueError): m.decode(value)
        bad = good.copy()
        bad[1] ^= 1
        with self.assertRaises(ValueError): m.decode(bad)

    def manifest(self):
        return dict(schema=1, word=2, text='device not found', width=432, height=128,
                    rasterScale=4, logicalAdvance=108, logicalHeight=32,
                    baselineOriginalUiPixels=24, typeface='Haettenschweiler',
                    horizontalScale=.88, lighterMasterMinFilter=9, rgbaSha256=m.MESSAGE_SHA,
                    inkBounds=[6, 29, 426, 93], fitX=1.0, alphaRuns=m.read_runs())

    def test_manifest_rejections_and_no_overwrite(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            source, out = root / 'art.json', root / 'headers'
            def write(doc): source.write_text(json.dumps(doc), encoding='utf-8')
            write(self.manifest())
            m.generate(source, out)
            m.check(out)
            for key, value in [('schema', True), ('fitX', 1), ('inkBounds', [True,29,426,93]),
                               ('inkBounds', [0,0,432,128]), ('text','different'), ('extra',0)]:
                doc=self.manifest();doc[key]=value;write(doc)
                with self.assertRaises(ValueError): m.generate(source, out)
            source.write_text('{"schema":1,"schema":1}', encoding='utf-8')
            with self.assertRaises(ValueError): m.generate(source, out)
            write(self.manifest())
            protected=out / 'FeedbackModalWordData.h'
            protected.write_bytes(b'preserve this candidate')
            with self.assertRaises(FileExistsError): m.generate(source, out)
            self.assertEqual(protected.read_bytes(), b'preserve this candidate')

if __name__ == '__main__': unittest.main()

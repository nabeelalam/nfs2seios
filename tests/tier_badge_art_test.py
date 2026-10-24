"""Structural authored tier-badge checks; no raster, original-data or device claim."""
# SPDX-License-Identifier: MIT
import hashlib
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
import unittest
from unittest import mock
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
GENERATOR = ROOT / 'scripts/generate_tier_badge_art.py'
SPEC = importlib.util.spec_from_file_location('tier_badge_art', GENERATOR)
art = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(art)
SVG = '{http://www.w3.org/2000/svg}'
FROZEN_SHA = {
    'A': '70f1fe57a22a3b3a377c5213bd26772f4afa80ce794afa15be1865e3f0b8b952',
    'B': 'dfeadf99542fccf2b7624497dd5b34000993e54ca8779c108c46dfe8fe54442b',
    'C': '688d3590c4b76e3f41def40a9dfde119ff4cf6a0b5fed9c521f5425f8c6bf1f9',
}


def snapshot(directory):
    return {path.name: path.read_bytes() for path in directory.iterdir() if path.is_file()}


class TierBadgeArtTests(unittest.TestCase):
    def test_import_is_inert_even_with_unrelated_cli_arguments(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            spec = importlib.util.spec_from_file_location('tier_badge_inert_import', GENERATOR)
            module = importlib.util.module_from_spec(spec)
            with mock.patch.object(sys, 'argv', ['unrelated', '--unsupported']), \
                    mock.patch('argparse.ArgumentParser.parse_args', side_effect=AssertionError('CLI executed during import')), \
                    mock.patch.object(Path, 'mkdir', side_effect=AssertionError('Import attempted output')):
                spec.loader.exec_module(module)
            self.assertEqual(list(root.iterdir()), [])
            self.assertEqual(tuple(module.LETTERS), ('A', 'B', 'C'))

    def test_exact_finite_inventory_and_review_manifest(self):
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / 'new-review'
            returned = art.generate(output)
            self.assertEqual(set(snapshot(output)), {'tier-a.svg', 'tier-b.svg', 'tier-c.svg', 'tier-badge-art.json'})
            manifest = json.loads((output / 'tier-badge-art.json').read_text(encoding='utf-8'))
            self.assertEqual(returned, manifest)
            self.assertEqual(manifest['schema'], 1)
            self.assertEqual(manifest['viewBox'], [0, 0, 123, 44])
            self.assertTrue(manifest['explicitPathsOnly'])
            self.assertTrue(manifest['letterAndCoupeAreTransparentCutouts'])
            self.assertFalse(manifest['fontOrOriginalPixelsEmbedded'])
            self.assertFalse(manifest['accepted'])
            self.assertFalse(manifest['nativeBinding'])
            self.assertEqual([row['tier'] for row in manifest['badges']], ['A', 'B', 'C'])
            self.assertEqual([row['svg'] for row in manifest['badges']], ['tier-a.svg', 'tier-b.svg', 'tier-c.svg'])

    def test_bounded_vector_geometry_without_text_images_fonts_or_external_references(self):
        # Ellipses are explicit wheel geometry, not glyph/font references. All
        # other visible surfaces and each complete letter use authored paths.
        allowed = {'svg', 'title', 'defs', 'linearGradient', 'stop', 'clipPath', 'path', 'mask', 'g', 'ellipse'}
        for letter in 'ABC':
            with self.subTest(tier=letter):
                raw = art.svg(letter)
                root = ET.fromstring(raw)
                self.assertEqual(root.tag, SVG + 'svg')
                self.assertEqual(root.attrib['viewBox'], '0 0 123 44')
                self.assertEqual((root.attrib['width'], root.attrib['height']), ('123', '44'))
                self.assertNotIn('<!DOCTYPE', raw.upper())
                self.assertNotIn('<!ENTITY', raw.upper())
                identifiers = [node.attrib['id'] for node in root.iter() if 'id' in node.attrib]
                self.assertEqual(len(identifiers), len(set(identifiers)))
                for node in root.iter():
                    self.assertIn(node.tag.removeprefix(SVG), allowed)
                    for key, value in node.attrib.items():
                        self.assertNotIn('href', key.lower())
                        self.assertFalse(key.lower().startswith('on'))
                        self.assertNotIn('font', key.lower())
                        self.assertNotIn('data:', value.lower())
                        self.assertNotIn('javascript:', value.lower())
                        if 'url(' in value:
                            self.assertTrue(value.startswith('url(#') and value.endswith(')'))
                            self.assertIn(value[5:-1], identifiers)
                self.assertGreater(len(root.findall('.//' + SVG + 'path')), 8)

    def test_negative_relief_mask_and_transparent_interior_structure(self):
        # This proves SVG alpha intent and grouping, not final raster alpha or
        # the original native5551 conversion. Those are separate observations.
        letters = []
        for letter in 'ABC':
            root = ET.fromstring(art.svg(letter))
            mask = root.find('.//' + SVG + 'mask')
            self.assertIsNotNone(mask)
            self.assertEqual(mask.attrib['id'], 'negative-relief')
            self.assertEqual(mask.attrib['maskUnits'], 'userSpaceOnUse')
            self.assertEqual([mask.attrib[key] for key in ('x', 'y', 'width', 'height')], ['0', '0', '123', '44'])
            paths = list(mask)
            self.assertEqual(len(paths), 3)
            self.assertTrue(all(path.tag == SVG + 'path' for path in paths))
            self.assertEqual(paths[0].attrib, {'d': 'M0 0H123V44H0Z', 'fill': 'white'})
            self.assertEqual(paths[1].attrib['fill'], 'black')
            self.assertEqual(paths[1].attrib['fill-rule'], 'evenodd')
            self.assertEqual(paths[1].attrib['d'], art.LETTERS[letter])
            self.assertEqual(paths[2].attrib, {'d': art.COUPE_CUTOUT, 'fill': 'black'})
            letters.append(paths[1].attrib['d'])
            painted = root.findall(SVG + 'g')
            self.assertEqual(len(painted), 2)
            self.assertEqual(painted[0].attrib, {'mask': 'url(#negative-relief)'})
            self.assertEqual(painted[1].attrib['id'], 'coupe-metal-relief')
            self.assertEqual(painted[1].attrib['fill'], 'none')
            self.assertTrue(all('fill' not in node.attrib for node in painted[1]))
            # Letter/car mask paths must not also paint opaque dark interiors.
            for node in painted[0].iter():
                self.assertNotIn(node.attrib.get('d'), (art.LETTERS[letter], art.COUPE_CUTOUT))
        self.assertEqual(len(set(letters)), 3)

    def test_one_shared_coupe_material_and_heavy_single_wheel_rings(self):
        documents = [ET.fromstring(art.svg(letter)) for letter in 'ABC']
        material = [ET.tostring(root.find(SVG + 'defs')) for root in documents]
        coupe = [ET.tostring(root.find(".//%sg[@id='coupe-metal-relief']" % SVG)) for root in documents]
        self.assertEqual(material[0], material[1])
        self.assertEqual(material[0], material[2])
        self.assertEqual(coupe[0], coupe[1])
        self.assertEqual(coupe[0], coupe[2])
        for root in documents:
            group = root.find(".//%sg[@id='coupe-metal-relief']" % SVG)
            wheels = group.findall(SVG + 'ellipse')
            self.assertEqual(len(wheels), 2)
            self.assertEqual(wheels[0].attrib['cx'], '69.7')
            self.assertEqual(wheels[0].attrib['stroke-width'], '2.15')
            self.assertTrue(all('fill' not in wheel.attrib for wheel in wheels))
            self.assertEqual(len(group.findall(SVG + 'path')), 2)

    def test_deterministic_frozen_source_hash_and_manifest_bytes(self):
        with tempfile.TemporaryDirectory() as temporary:
            first, second = Path(temporary) / 'first', Path(temporary) / 'second'
            art.generate(first)
            art.generate(second)
            self.assertEqual(snapshot(first), snapshot(second))
            manifest = json.loads((first / 'tier-badge-art.json').read_bytes())
            for row in manifest['badges']:
                digest = hashlib.sha256((first / row['svg']).read_bytes()).hexdigest()
                self.assertEqual(digest, row['sha256'])
                self.assertEqual(digest, FROZEN_SHA[row['tier']])

    def test_nonempty_output_refusal_preserves_all_prior_bytes(self):
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            (directory / 'keep.bin').write_bytes(b'\x00original unrelated sentinel\xff')
            before = snapshot(directory)
            with self.assertRaises(ValueError):
                art.generate(directory)
            self.assertEqual(snapshot(directory), before)
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / 'review'
            art.generate(output)
            before = snapshot(output)
            with self.assertRaises(ValueError):
                art.generate(output)
            self.assertEqual(snapshot(output), before)


if __name__ == '__main__':
    unittest.main()

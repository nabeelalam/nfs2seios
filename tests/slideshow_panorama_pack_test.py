"""Synthetic side-only resource packaging; no game/artwork inputs."""
from pathlib import Path
import copy
import hashlib
import importlib.util
import json
import struct
import tempfile
import unittest
import zlib

spec = importlib.util.spec_from_file_location("packer", Path(__file__).resolve().parents[1] / "scripts/slideshow_panorama_pack.py")
packer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(packer)


def pixels(values):
    return b"".join(bytes((x, x, x, 255)) for x in values)


class Tests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.center = pixels(range(10, 22))
        self.sides = pixels(range(1, 7))
        (self.root / "center.rgba").write_bytes(self.center)
        (self.root / "sides.rgba").write_bytes(self.sides)
        self.row = dict(width=6, height=3, centerWidth=4, reviewed=True,
                        centerRaw="center.rgba", sideRaw="sides.rgba",
                        archiveSha256="01" * 32, entrySha256="02" * 32,
                        centerRgbaSha256=hashlib.sha256(self.center).hexdigest(), footerRgbaSha256="03" * 32)

    def build(self, rows=None):
        manifest = self.root / "manifest.json"
        manifest.write_text(json.dumps({"images": rows if rows is not None else [self.row]}))
        return packer.build(manifest, self.root / "output")

    def test_roundtrip_exact_layout_and_no_center_payload(self):
        receipt = self.build()
        data = (self.root / "output/slideshow-panorama.ownedz").read_bytes()
        self.assertEqual(struct.unpack_from("<8sIIIIQ", data), (b"NFSSPAN\0", 1, 1, 216, 0, len(data) - 248))
        self.assertEqual(struct.unpack_from("<IIIIQ", data, 32 + 192), (6, 3, 4, 1, len(data) - 248))
        self.assertEqual(zlib.decompress(data[248:]), self.sides)
        expected = pixels([1, 10, 11, 12, 13, 4, 2, 14, 15, 16, 17, 5, 3, 18, 19, 20, 21, 6])
        self.assertEqual(data[32 + 160:32 + 192], hashlib.sha256(expected).digest())
        self.assertFalse(receipt["centerPixelsStored"])
        self.assertNotIn(str(self.root), json.dumps(receipt))

    def test_rejects_wrong_center_alpha_and_lengths(self):
        for fault in ("center-hash", "center-alpha", "side-alpha", "short-center", "long-side"):
            with self.subTest(fault=fault):
                center, sides = bytearray(self.center), bytearray(self.sides)
                if fault == "center-hash": center[0] ^= 1
                elif fault == "center-alpha": center[3] = 254
                elif fault == "side-alpha": sides[3] = 0
                elif fault == "short-center": center.pop()
                else: sides.append(0)
                (self.root / "center.rgba").write_bytes(center)
                (self.root / "sides.rgba").write_bytes(sides)
                with self.assertRaises(ValueError): self.build()

    def test_center_alpha_rejected_even_with_matching_hash(self):
        center = bytearray(self.center)
        center[3] = 254
        (self.root / "center.rgba").write_bytes(center)
        row = dict(self.row, centerRgbaSha256=hashlib.sha256(center).hexdigest())
        with self.assertRaises(ValueError): self.build([row])

    def test_rejects_geometry_review_hashes_and_duplicates(self):
        for changes in ({"width": 5}, {"width": 0xffffffff}, {"centerWidth": 2}, {"height": True},
                        {"reviewed": False}, {"reviewed": 1}, {"footerRgbaSha256": "00" * 32},
                        {"sideRgbaSha256": "04" * 32}, {"composedRgbaSha256": "04" * 32}):
            with self.subTest(changes=changes):
                row = dict(self.row, **changes)
                with self.assertRaises(ValueError): self.build([row])
        with self.assertRaises(ValueError): self.build([self.row, copy.deepcopy(self.row)])
        with self.assertRaises(ValueError): self.build([])
        with self.assertRaises(ValueError): self.build([self.row] * 99)

    def test_rejects_input_escape(self):
        for path in (str(self.root / "center.rgba"), "../center.rgba"):
            with self.assertRaises(ValueError): self.build([dict(self.row, centerRaw=path)])

    def test_two_records_payload_order(self):
        other = dict(self.row, entrySha256="09" * 32)
        self.build([self.row, other])
        data = (self.root / "output/slideshow-panorama.ownedz").read_bytes()
        size = struct.unpack_from("<Q", data, 32 + 208)[0]
        start = 32 + 2 * 216
        self.assertEqual(zlib.decompress(data[start:start + size]), self.sides)
        self.assertEqual(zlib.decompress(data[start + size:]), self.sides)


if __name__ == "__main__":
    unittest.main()

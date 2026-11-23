"""Checks for matched History background/panorama resource generation."""
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
import zlib

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
import history_background_pair as pair


def seed():
    rows = []
    for i in range(49):
        metadata = struct.pack("<4I8f", i, 4, 3, 0, 0, 0, 640, 480, 0, 0, 640, 480)
        rows.append((metadata, bytes([i, 2, 3, 255]) * 12))
    payload = struct.pack("<II", 49, 0) + b"".join(a + b for a, b in rows)
    return rows, b"NFSHIST1" + struct.pack("<II", 1, len(payload)) + zlib.compress(payload)


def panorama():
    return bytes(v for y in range(18) for x in range(32) for v in (x, y, 77, 255))


class HistoryBackgroundPair(unittest.TestCase):
    def test_crop_joins_and_unrelated_sources(self):
        rows, _ = seed()
        pixels = panorama()
        packed, sides, receipt = pair.derive(rows, [(7, 32, 18, pixels)])
        decoded = pair.unpack_history(packed, pair.sha(packed))
        meta, center = decoded[7]
        self.assertEqual(struct.unpack_from("<II", meta, 4), (24, 18))
        self.assertEqual(meta[12:], rows[7][0][12:])
        self.assertEqual(center[:4], bytes([4, 0, 77, 255]))
        self.assertEqual(center[23 * 4:24 * 4], bytes([27, 0, 77, 255]))
        self.assertEqual(center[-4:], bytes([27, 17, 77, 255]))
        side = sides["history-panorama-7.ownedz"]
        self.assertEqual(zlib.decompress(side[32:]), pixels)
        self.assertEqual(center, b"".join(pixels[y * 128 + 16:y * 128 + 112] for y in range(18)))
        for index in range(49):
            if index != 7:
                self.assertEqual(decoded[index], rows[index])
        self.assertEqual(receipt["unchangedSourceCount"], 48)
        self.assertFalse(receipt["runtimeVerified"])

    def test_base_digest_and_complete_stream(self):
        _, packed = seed()
        with self.assertRaises(ValueError):
            pair.unpack_history(packed, "0" * 64)
        for bad in (packed[:-1], packed + b"extra", packed + zlib.compress(b"extra")):
            with self.assertRaises(ValueError):
                pair.unpack_history(bad, pair.sha(bad))

    def test_reject_incomplete_catalog(self):
        payload = struct.pack("<II", 48, 0)
        bad = b"NFSHIST1" + struct.pack("<II", 1, len(payload)) + zlib.compress(payload)
        with self.assertRaises(ValueError):
            pair.unpack_history(bad, pair.sha(bad))

    def test_reject_invalid_panorama_or_duplicate_source(self):
        rows, _ = seed()
        raw = panorama()
        for entries in ([], [(1, 32, 18, raw)], [(7, 32, 18, raw)] * 2,
                        [(7, 32, 17, raw)], [(7, 32, 18, raw[:-1])],
                        [(7, 32, 18, raw[:3] + b"\0" + raw[4:])]):
            with self.assertRaises(ValueError):
                pair.derive(rows, entries)

    def test_multiple_backgrounds_keep_titles(self):
        rows, _ = seed()
        packed, sides, receipt = pair.derive(rows, [(0, 32, 18, panorama()), (42, 32, 18, panorama())])
        decoded = pair.unpack_history(packed, pair.sha(packed))
        self.assertEqual(set(sides), {"history-panorama-0.ownedz", "history-panorama-42.ownedz"})
        for i in range(49):
            if i % 7:
                self.assertEqual(decoded[i], rows[i])
        self.assertEqual(receipt["unchangedSourceCount"], 47)

    def test_cli_refuses_output_reuse(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            _, packed = seed()
            (root / "base.ownedz").write_bytes(packed)
            (root / "image.rgba").write_bytes(panorama())
            (root / "manifest.json").write_text(json.dumps([
                {"sourceIndex": 7, "width": 32, "height": 18, "rgba": "image.rgba"}]))
            command = [sys.executable, str(ROOT / "scripts/history_background_pair.py"),
                       "--base", str(root / "base.ownedz"), "--base-sha256", pair.sha(packed),
                       "--manifest", str(root / "manifest.json"), "--output-dir", str(root / "output")]
            first = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(first.returncode, 0, first.stderr)
            output = root / "output/history-ui.ownedz"
            digest = pair.sha(output.read_bytes())
            second = subprocess.run(command, capture_output=True, text=True)
            self.assertNotEqual(second.returncode, 0)
            self.assertEqual(pair.sha(output.read_bytes()), digest)
            self.assertEqual((root / "base.ownedz").read_bytes(), packed)


if __name__ == "__main__":
    unittest.main()

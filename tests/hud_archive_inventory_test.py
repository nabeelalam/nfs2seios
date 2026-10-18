import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location("inventory", Path(__file__).resolve().parents[1] / "scripts/inspect_hud_archive.py")
inventory = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inventory)


def archive():
    return struct.pack("<4sII4s4sIIhh4h", b"SHPI", 40, 1, b"TEST", b"demo", 24, 0x7e, 16, 8, 0, 0, 0, 0)


class InventoryTests(unittest.TestCase):
    def test_metadata_only(self):
        report = inventory.inspect(archive())
        self.assertEqual(report["entry_count"], 1)
        self.assertEqual(report["entries"][0]["width_raw"], 16)
        self.assertEqual(report["entries"][0]["format_code"], 0x7e)
        self.assertNotIn("pixels", report["entries"][0])

    def test_bad_headers_and_offsets(self):
        for offset, value in [(4, 41), (8, 4097), (20, 0), (20, 40)]:
            data = bytearray(archive())
            struct.pack_into("<I", data, offset, value)
            with self.assertRaises(ValueError):
                inventory.inspect(data)
        with self.assertRaises(ValueError):
            inventory.inspect(archive()[:15])
        with self.assertRaises(ValueError):
            inventory.inspect(b"QFS!" + archive()[4:])

    def test_flags_and_signed_metadata(self):
        data = bytearray(archive())
        struct.pack_into("<Ih", data, 24, 0x1234fb, -16)
        entry = inventory.inspect(data)["entries"][0]
        self.assertTrue(entry["entry_compressed"])
        self.assertEqual(entry["next_attachment_offset"], 0x1234)
        self.assertEqual(entry["width_raw"], -16)


if __name__ == "__main__":
    unittest.main()

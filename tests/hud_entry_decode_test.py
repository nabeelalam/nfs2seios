from pathlib import Path
import struct
import sys
import unittest
import zlib
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/"scripts"))
from decode_hud_entry import decode, png


def archive():
    return (struct.pack("<4sII4s", b"SHPI", 76, 2, b"TEST")+
            struct.pack("<4sI4sI", b"demo", 32, b"!pal", 52)+
            struct.pack("<Ihh4h", 0x7b, 2, 2, 0, 0, 0, 0)+bytes([0, 1, 1, 0])+
            struct.pack("<Ihh4h", 0x2a, 2, 1, 0, 0, 0, 0)+bytes([0, 0, 0, 0, 0, 0, 255, 255]))


class DecodeTests(unittest.TestCase):
    def test_rgba_and_png_pixels(self):
        width, height, rgba = decode(archive(), 0)
        self.assertEqual((width, height), (2, 2))
        self.assertEqual(rgba, b"\0\0\0\0"+b"\xff\0\0\xff"*2+b"\0\0\0\0")
        encoded = png(width, height, rgba)
        self.assertEqual(encoded[:8], b"\x89PNG\r\n\x1a\n")
        length = struct.unpack_from(">I", encoded, 33)[0]
        self.assertEqual(zlib.decompress(encoded[41:41+length]), b"\0"+rgba[:8]+b"\0"+rgba[8:])

    def test_bad_geometry_indices_and_palette(self):
        for offset, fmt, value in [(36, "<h", 4097), (48, "<B", 2), (56, "<h", 257), (52, "<I", 0x7e), (32, "<I", 0xfb)]:
            data = bytearray(archive())
            struct.pack_into(fmt, data, offset, value)
            with self.assertRaises(ValueError):
                decode(data, 0)
        with self.assertRaises(ValueError):
            decode(archive(), -1)

    def test_local_palette_precedes_global(self):
        data = (struct.pack("<4sII4s", b"SHPI", 100, 2, b"TEST")+
                struct.pack("<4sI4sI", b"demo", 32, b"!pal", 76)+
                struct.pack("<Ihh4h", (20<<8)|0x7b, 2, 2, 0, 0, 0, 0)+bytes([0, 1, 1, 0])+
                struct.pack("<Ihh4h", 0x2a, 2, 1, 0, 0, 0, 0)+bytes([0, 0, 0, 0, 0, 255, 0, 255])+
                archive()[52:])
        self.assertEqual(decode(data, 0)[2][4:8], b"\0\xff\0\xff")
        bad = bytearray(data)
        struct.pack_into("<I", bad, 32, (4<<8)|0x7b)
        with self.assertRaises(ValueError):
            decode(bad, 0)


if __name__ == "__main__":
    unittest.main()

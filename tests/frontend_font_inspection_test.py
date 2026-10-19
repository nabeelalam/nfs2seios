"""Synthetic font records verify bounds and metrics without original assets."""
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from inspect_frontend_font import inspect,LIMIT

def font():
    data = bytearray(80)
    struct.pack_into('<4sIHH',data,0,b'FNTF',80,100,2)
    struct.pack_into('<4b',data,16,-1,2,4,1)
    struct.pack_into('<I',data,20,32)
    struct.pack_into('<I',data,28,56)
    struct.pack_into('<HBBHHbbb',data,32,32,1,1,0,0,2,-1,-2)
    struct.pack_into('<HBBHHbbb',data,43,65,2,3,1,1,3,0,1)
    struct.pack_into('<IHH',data,56,0x7a,4,4)
    return bytes(data)

class FontInspection(unittest.TestCase):
    def test_offsets_advances_and_bitmap_metrics(self):
        report = inspect(font())
        self.assertEqual(report['originOffset'],[-1,2])
        self.assertEqual((report['lineHeight'],report['lineGap']),(4,1))
        self.assertEqual(report['glyphs'][0]['advance'],2)
        self.assertEqual((report['glyphs'][0]['offsetX'],report['glyphs'][0]['offsetY']),(-1,-2))
        self.assertEqual(report['bitmap'],dict(offset=56,width=4,height=4,bitsPerPixel=4,stride=2))
        self.assertFalse(report['bitmapPixelsExported'])
        self.assertNotIn('pixels',report)

    def test_bitmap_change_does_not_change_metrics_identity(self):
        changed = bytearray(font());changed[-1] = 255
        self.assertEqual(inspect(bytes(changed))['metricsSha256'],inspect(font())['metricsSha256'])
        changed[51] = 4
        self.assertNotEqual(inspect(bytes(changed))['metricsSha256'],inspect(font())['metricsSha256'])

    def test_original_short_length_is_reported(self):
        data = bytearray(font());struct.pack_into('<I',data,4,len(data)-4)
        self.assertEqual(inspect(bytes(data))['declaredLengthDifference'],4)

    def test_header_and_length_rejection(self):
        for value in (b'',b'FNTF',b'x'*(LIMIT+1),font()[:-1],font()+b'\0',b'BAD!'+font()[4:]):
            with self.subTest(length=len(value)),self.assertRaises(ValueError):inspect(value)

    def test_structural_rejection(self):
        for at,fmt,value in [(8,'H',101),(10,'H',0),(10,'H',4097),(20,'I',31),
                             (20,'I',60),(28,'I',40),(56,'I',0x7b),(60,'H',0),
                             (60,'H',4097),(43,'H',32),(47,'H',4),(45,'B',0),
                             (4,'I',85),(4,'I',75)]:
            data = bytearray(font());struct.pack_into('<'+fmt,data,at,value)
            with self.subTest(offset=at,value=value),self.assertRaises(ValueError):inspect(bytes(data))

if __name__=='__main__':
    unittest.main()

"""Focused synthetic class-resource delivery checks; no app or original assets."""
from pathlib import Path
import importlib.util,hashlib,plistlib,struct,tempfile,unittest,zipfile
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('inject_class_test',ROOT/'scripts/inject_personal_artwork.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
H=lambda data:hashlib.sha256(data).hexdigest()
class Delivery(unittest.TestCase):
 def test_public_pins_and_route(self):
  self.assertEqual(m.resource_pins()['class-picture-ui.ownedz'],(5207398,'c2c9cea9e69bd307cf4c76c5bbd87bc28e7fe6cac08162911b531dd44cea4747'))
  self.assertEqual(m.MAX_IPA,768*1024*1024)
  self.assertEqual(m.MAX_BASE_IPA,512*1024*1024)
  self.assertEqual(m.MAX_MEMBER_BYTES,512*1024*1024)
  activation=(ROOT/'src/hd/ArtworkActivation.cpp').read_text();self.assertIn('#include "OpponentPreviewCatalogData.h"',activation);self.assertNotIn('private/OpponentPreviewCatalogData.h',activation)
  self.assertNotIn('OpponentPreviewCatalogData.h',(ROOT/'scripts/classic_screens_assets.py').read_text())
 def test_inject_exact_bytes_and_rejections(self):
  with tempfile.TemporaryDirectory(prefix='class-picture-delivery-')as tmp:
   d=Path(tmp);template=b'OWNZ0001'+bytes(24);art=b'NFSCLPIC'+bytes(range(24));release='42'*32
   native=struct.pack('<IiiIIIII6I',0xfeedfacf,0x0100000c,0,2,1,24,0,0,0x32,24,2,0,0,0)+bytes.fromhex(release)+b'unchanged-native'
   members={'Payload/Test.app/Info.plist':plistlib.dumps({'CFBundleExecutable':'Test'}),'Payload/Test.app/Test':native,'Payload/Test.app/notice.txt':b'unchanged notice'}
   base=d/'base.ipa'
   with zipfile.ZipFile(base,'w',compression=zipfile.ZIP_DEFLATED)as z:
    for name,value in members.items():z.writestr(name,value)
   (d/'template').write_bytes(template);(d/'art').write_bytes(art)
   with patch.multiple(m,TEMPLATE_BYTES=len(template),TEMPLATE_SHA256=H(template),OUTPUT_PACK_SHA256=release,CLASS_PICTURE_BYTES=len(art),CLASS_PICTURE_SHA256=H(art)):
    result=m.inject(base,H(base.read_bytes()),d/'template',d/'out.ipa',class_picture=d/'art')
    self.assertEqual(result['addedMembers'],2);self.assertEqual(result['additionalResources']['class-picture-ui.ownedz']['sha256'],H(art));self.assertTrue(result['allExistingMemberBytesIdentical']);self.assertTrue(result['allExistingCompressedStorageIdentical'])
    with zipfile.ZipFile(d/'out.ipa')as z:
     for name,value in members.items():self.assertEqual(z.read(name),value)
     self.assertEqual(z.read('Payload/Test.app/class-picture-ui.ownedz'),art)
    with zipfile.ZipFile(d/'edited.ipa', 'w') as edited:
     for name, value in members.items():
      edited.writestr(name, value)
     edited.writestr('Payload/Test.app/class-picture-ui.ownedz', art[:-1] + b'x')
    with self.assertRaisesRegex(ValueError, 'artwork digest'):
     m.inspect(d/'edited.ipa', allow_resource=True)
    (d/'badsize').write_bytes(art[:-1]);(d/'badhash').write_bytes(art[:-1]+b'x')
    for name in ['badsize','badhash']:
     with self.assertRaises(ValueError):m.inject(base,H(base.read_bytes()),d/'template',d/(name+'.ipa'),class_picture=d/name)
    with self.assertRaises(ValueError):m.inject(d/'out.ipa',H((d/'out.ipa').read_bytes()),d/'template',d/'duplicate.ipa',class_picture=d/'art')
    with patch.object(m,'MAX_BASE_IPA',base.stat().st_size-1):
     with self.assertRaisesRegex(ValueError,'Base IPA size limit'):m.inject(base,H(base.read_bytes()),d/'template',d/'large-base.ipa',class_picture=d/'art')
    with patch.object(m,'MAX_UNPACKED',sum(map(len,members.values()))+len(template)+len(art)-1):
     with self.assertRaisesRegex(ValueError,'Combined unpacked size limit'):m.inject(base,H(base.read_bytes()),d/'template',d/'unpacked.ipa',class_picture=d/'art')
     self.assertFalse((d/'unpacked.ipa').exists())
    with patch.object(m,'MAX_IPA',base.stat().st_size+len(template)+len(art)+65535):
     with self.assertRaisesRegex(ValueError,'Combined IPA size limit'):m.inject(base,H(base.read_bytes()),d/'template',d/'oversize.ipa',class_picture=d/'art')
if __name__=='__main__':unittest.main()

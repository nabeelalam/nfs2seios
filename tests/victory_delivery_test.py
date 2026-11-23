"""Synthetic Victory delivery checks; no real app or game assets."""
from pathlib import Path
import importlib.util,hashlib,json,plistlib,re,struct,tempfile,unittest,zipfile
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('inject_victory_test',ROOT/'scripts/inject_personal_artwork.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
H=lambda data:hashlib.sha256(data).hexdigest()
class Delivery(unittest.TestCase):
 def test_release_pins_match_loader(self):
  release=json.loads((ROOT/'src/hd/ClassicScreens/VictoryArtworkRelease.json').read_text());header=(ROOT/'src/hd/ClassicScreens/VictoryArtworkPin.h').read_text()
  for macro,key in [('METADATA_SHA256','metadataSha256'),('SHA256','sha256'),('FILE_BYTES','fileBytes')]:
   value=re.search(r'^#define NFS_VICTORY_ARTWORK_'+macro+r' (.+)$',header,re.MULTILINE)[1];self.assertEqual(value,str(release[key])+'u' if key=='fileBytes' else '"'+release[key]+'"')
  self.assertEqual(m.resource_pins()['victory-ui.ownedz'],(release['fileBytes'],release['sha256']))
 def test_inject_exact_bytes_and_rejections(self):
  with tempfile.TemporaryDirectory(prefix='class-picture-delivery-')as tmp:
   d=Path(tmp);template=b'OWNZ0001'+bytes(24);art=b'NFSVIC01'+bytes(range(24));release='42'*32
   native=struct.pack('<IiiIIIII6I',0xfeedfacf,0x0100000c,0,2,1,24,0,0,0x32,24,2,0,0,0)+bytes.fromhex(release)+bytes.fromhex(m._VICTORY_RELEASE['metadataSha256'])+b'unchanged-native'
   members={'Payload/Test.app/Info.plist':plistlib.dumps({'CFBundleExecutable':'Test'}),'Payload/Test.app/Test':native,'Payload/Test.app/notice.txt':b'unchanged notice'}
   base=d/'base.ipa'
   with zipfile.ZipFile(base,'w',compression=zipfile.ZIP_DEFLATED)as z:
    for name,value in members.items():z.writestr(name,value)
   (d/'template').write_bytes(template);(d/'art').write_bytes(art)
   with patch.multiple(m,TEMPLATE_BYTES=len(template),TEMPLATE_SHA256=H(template),OUTPUT_PACK_SHA256=release,VICTORY_BYTES=len(art),VICTORY_SHA256=H(art)):
    result=m.inject(base,H(base.read_bytes()),d/'template',d/'out.ipa',victory=d/'art')
    self.assertEqual(result['addedMembers'],2);self.assertEqual(result['additionalResources']['victory-ui.ownedz']['sha256'],H(art));self.assertTrue(result['allExistingMemberBytesIdentical']);self.assertTrue(result['allExistingCompressedStorageIdentical'])
    with zipfile.ZipFile(d/'out.ipa')as z:
     for name,value in members.items():self.assertEqual(z.read(name),value)
     self.assertEqual(z.read('Payload/Test.app/victory-ui.ownedz'),art)
    with zipfile.ZipFile(d/'edited.ipa', 'w') as edited:
     for name, value in members.items():
      edited.writestr(name, value)
     edited.writestr('Payload/Test.app/victory-ui.ownedz', art[:-1] + b'x')
    with self.assertRaisesRegex(ValueError, 'artwork digest'):
     m.inspect(d/'edited.ipa', allow_resource=True)
    (d/'badsize').write_bytes(art[:-1]);(d/'badhash').write_bytes(art[:-1]+b'x')
    for name in ['badsize','badhash']:
     with self.assertRaises(ValueError):m.inject(base,H(base.read_bytes()),d/'template',d/(name+'.ipa'),victory=d/name)
    with self.assertRaises(ValueError):m.inject(d/'out.ipa',H((d/'out.ipa').read_bytes()),d/'template',d/'duplicate.ipa',victory=d/'art')
    with zipfile.ZipFile(d/'wrong-base.ipa','w')as wrong:
     for name,value in members.items():wrong.writestr(name,value.replace(bytes.fromhex(m._VICTORY_RELEASE['metadataSha256']),bytes(32)) if name.endswith('/Test') else value)
    with self.assertRaisesRegex(ValueError,'required artwork release pin'):
     m.inject(d/'wrong-base.ipa',H((d/'wrong-base.ipa').read_bytes()),d/'template',d/'wrong-output.ipa',victory=d/'art')
    with patch.object(m,'MAX_IPA',base.stat().st_size+len(template)+len(art)+65535):
     with self.assertRaisesRegex(ValueError,'Combined IPA size limit'):m.inject(base,H(base.read_bytes()),d/'template',d/'oversize.ipa',victory=d/'art')
if __name__=='__main__':unittest.main()

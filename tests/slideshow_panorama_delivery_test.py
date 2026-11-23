"""Synthetic delivery/metadata fixtures only; no private art, runnable app or game."""
from pathlib import Path
import copy,hashlib,importlib.util,json,plistlib,shutil,struct,subprocess,sys,tempfile,unittest,zipfile
from unittest.mock import patch
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'scripts'))
import prepare_ci_sources as prep
import slideshow_panorama_release as release
spec=importlib.util.spec_from_file_location('inject_panorama_test',ROOT/'scripts/inject_personal_artwork.py')
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
H=lambda b:hashlib.sha256(b).hexdigest()
class Delivery(unittest.TestCase):
 def test_catalog_optional_exact_and_bounded(self):
  with tempfile.TemporaryDirectory() as tmp:
   p=Path(tmp)/'pin.json';self.assertIsNone(release.load(p))
   good=dict(schema=1,filename='slideshow-panorama.ownedz',fileBytes=300,sha256='12'*32,imageCount=98)
   p.write_text(json.dumps(good));self.assertEqual(release.load(p),good)
   for key,val in [('fileBytes',release.MAX_BYTES+1),('fileBytes',True),('sha256','0'*64),('imageCount',97),('filename','extra.ownedz'),('extra',1)]:
    bad={**good,key:val};p.write_text(json.dumps(bad))
    with self.assertRaises(ValueError):release.load(p)
 def test_injection_and_native_pair_pins_sizes_budget_extra(self):
  with tempfile.TemporaryDirectory() as tmp:
   d=Path(tmp);data={'template':b'OWNZ0001'+bytes(24),'photo':b'photo-fixture','footer':b'footer-fixture','panorama':b'NFSSPAN\0'+bytes(range(80))}
   for n,b in data.items():(d/n).write_bytes(b)
   pin=dict(schema=1,filename=m.SLIDESHOW_PANORAMA_RESOURCE,fileBytes=len(data['panorama']),sha256=H(data['panorama']),imageCount=98)
   owner='42'*32
   pins={m.SLIDESHOW_RESOURCE:H(data['photo']),m.SLIDESHOW_FOOTER_RESOURCE:H(data['footer']),m.SLIDESHOW_PANORAMA_RESOURCE:pin['sha256']}
   marker=('NFS-SLIDESHOW-PANORAMA:'+pin['sha256']+':'+str(pin['fileBytes'])).encode()
   native=struct.pack('<IiiIIIII6I',0xfeedfacf,0x0100000c,0,2,1,24,0,0,0x32,24,2,0,0,0)+bytes.fromhex(owner)+b''.join(bytes.fromhex(v)for v in pins.values())+marker
   members={'Payload/Test.app/Info.plist':plistlib.dumps({'CFBundleExecutable':'Test'}),'Payload/Test.app/Test':native,'Payload/Test.app/notice.txt':b'unchanged'}
   def base(name,edit=None,extra=None):
    p=d/name
    with zipfile.ZipFile(p,'w',compression=zipfile.ZIP_DEFLATED)as z:
     for n,b in members.items():z.writestr(n,edit(b)if edit and n.endswith('/Test')else b)
     if extra:z.writestr('Payload/Test.app/'+extra,data['panorama'])
    return p
   b=base('base.ipa')
   def inject(b,out='out.ipa',**kw):
    return m.inject(b,H(b.read_bytes()),d/'template',d/out,slideshow=d/'photo',slideshow_footer=d/'footer',slideshow_panorama=d/'panorama',**kw)
   with patch.multiple(m,TEMPLATE_BYTES=len(data['template']),TEMPLATE_SHA256=H(data['template']),OUTPUT_PACK_SHA256=owner,SLIDESHOW_BYTES=len(data['photo']),SLIDESHOW_SHA256=pins[m.SLIDESHOW_RESOURCE],SLIDESHOW_FOOTER_BYTES=len(data['footer']),SLIDESHOW_FOOTER_SHA256=pins[m.SLIDESHOW_FOOTER_RESOURCE],_SLIDESHOW_PANORAMA_RELEASE=pin):
    result=inject(b);self.assertEqual(result['addedMembers'],4);self.assertTrue(result['slideshowPanoramaReleasePinsVerified']);self.assertTrue(result['allExistingCompressedStorageIdentical'])
    with zipfile.ZipFile(d/'out.ipa')as z:
     self.assertEqual(z.read('Payload/Test.app/slideshow-panorama.ownedz'),data['panorama'])
     for n,v in members.items():self.assertEqual(z.read(n),v)
    for label,edit in [('digest',lambda v:v.replace(bytes.fromhex(pin['sha256']),bytes(32)).replace(pin['sha256'].encode(),b'0'*64)),('size',lambda v:v.replace(marker,marker[:-1]+b'9')),('photo',lambda v:v.replace(bytes.fromhex(pins[m.SLIDESHOW_RESOURCE]),bytes(32)))]:
     bad=base(label+'.ipa',edit)
     with self.assertRaisesRegex(ValueError,'required artwork release pin'):inject(bad,label+'-out.ipa')
    for bad in (data['panorama'][:-1],data['panorama'][:-1]+b'x'):
     (d/'panorama').write_bytes(bad)
     with self.assertRaises(ValueError):inject(b,'bad-art.ipa')
    (d/'panorama').write_bytes(data['panorama'])
    existing=base('extra.ipa',extra=m.SLIDESHOW_PANORAMA_RESOURCE)
    with self.assertRaisesRegex(ValueError,'already present'):inject(existing,'extra-out.ipa')
    with patch.object(m,'MAX_IPA',b.stat().st_size+sum(map(len,data.values()))+65535):
     with self.assertRaisesRegex(ValueError,'Combined IPA size limit'):inject(b,'budget.ipa')
    with patch.object(m,'_SLIDESHOW_PANORAMA_RELEASE',None):
     with self.assertRaisesRegex(ValueError,'approved pin'):inject(b,'disabled.ipa')
 def test_workflow_compiles_only_prepared_catalog_identity(self):
  text=(ROOT/'.github/workflows/apple-device.yml').read_text()
  for name in ('SHA256','BYTES'):
   flag='-DNFS_CLASSIC_SLIDESHOW_PANORAMA_'+name+'="$NFS_CLASSIC_SLIDESHOW_PANORAMA_'+name+'"'
   self.assertEqual(text.count(flag),1)
  template=(ROOT/'src/hd/ClassicScreens/SlideshowPanoramaConfiguration.template.h').read_text()
  self.assertIn('static const volatile char nfsSlideshowPanoramaDelivery[]',template)
 @unittest.skipUnless(shutil.which('cmake'),'CMake metadata interpreter unavailable')
 def test_actual_deferred_uppercase_configuration_marker(self):
  # Execute the actual normalization/pin-byte commands, without compiling an app.
  cmake=(ROOT/'cmake/BundledOwnedArtwork.cmake').read_text()
  start=cmake.index('  set(NFS_SLIDESHOW_PANORAMA_PIN_BYTES "")')
  end=cmake.index('  if(NOT NFS_CLASSIC_SLIDESHOW_PANORAMA STREQUAL "")',start)
  commands=cmake[start:end]
  with tempfile.TemporaryDirectory()as tmp:
   d=Path(tmp);out=d/'config.h';script=d/'check.cmake';pin='ABCDEF12'*8
   script.write_text('set(NFS_CLASSIC_SLIDESHOW_PANORAMA "")\nset(NFS_CLASSIC_SLIDESHOW_PANORAMA_SHA256 "'+pin+'")\nset(NFS_CLASSIC_SLIDESHOW_PANORAMA_BYTES 73331466)\n'+commands+'\nconfigure_file("'+(ROOT/'src/hd/ClassicScreens/SlideshowPanoramaConfiguration.template.h').as_posix()+'" "'+out.as_posix()+'" @ONLY)\n')
   subprocess.run(['cmake','-P',str(script)],check=True,capture_output=True)
   configured=out.read_text();self.assertIn('NFS-SLIDESHOW-PANORAMA:'+pin.lower()+':73331466',configured)
   self.assertIn('0xAB,0xCD,0xEF,0x12,',configured)
class Metadata(unittest.TestCase):
 def fixture(self,count=15):
  digest=lambda i:[[i%255+1]+[0]*31]
  sources=[[614440,24,614416,120,640,480,digest(i),digest(i+98)]for i in range(98)]
  images=[[114,16+i,1,*sources[i][6:],digest(i+1),[4,3,digest(i+2),2],True,4]for i in range(98)]
  resources=[[16+i,1,4,3,digest(i),digest(i+1)]for i in range(count)]
  bindings=[[*s[6:],i%count]for i,s in enumerate(sources)]
  def agg(v):return '{'+','.join(agg(x)if isinstance(x,list)else 'true'if x is True else str(x)for x in v)+'}'
  prefix='// SPDX-License-Identifier: MIT\n// Slideshow compatibility fingerprints and resource-layout metadata only; no image pixels.\n'
  c=prefix+'#pragma once\n#include "hd/SlideshowSourceIdentity.h"\n#include "hd/SlideshowRuntimeImage.h"\nnamespace nfs_slideshow_private { static const nfs_slideshow::SourcePin sources[]='+agg(sources)+'; static const nfs_slideshow::RuntimeImagePin images[]='+agg(images)+'; }'
  f=prefix+'#pragma once\n#include "hd/SlideshowFooterResource.h"\nnamespace nfs_slideshow_footer_private { static constexpr uint64_t fileBytes='+str(16+count)+'; static const nfs_slideshow_footer::ResourcePin images[]='+agg(resources)+'; inline std::vector<nfs_slideshow_footer::Binding> bindings(){return '+agg(bindings)+';} }'
  return c.encode(),f.encode(),dict(sourcePins=98,runtimeImagePins=98,resourcePins=count,bindings=98)
 def test_exact_approved_counts_and_binding_bounds(self):
  c,f,n=self.fixture();self.assertEqual(prep._slideshow_structure(c,f,n),n)
  for counts in (None,{**n,'resourcePins':17},{**n,'sourcePins':97},{**n,'resourcePins':True}):
   with self.assertRaises(ValueError):prep._slideshow_structure(c,f,counts)
  c17,f17,n17=self.fixture(17);self.assertEqual(prep._slideshow_structure(c17,f17,n17),n17)
  bad=f.replace(b',0},{',b',15},{',1)
  self.assertNotEqual(bad,f)
  with self.assertRaises(ValueError):prep._slideshow_structure(c,bad,n)
 def test_manifest_hash_pair_and_count_mismatch(self):
  c,f,n=self.fixture()
  with tempfile.TemporaryDirectory()as tmp:
   d=Path(tmp);assets=d/prep.ASSETS;assets.mkdir(parents=True)
   (assets/'SlideshowCatalogData.h').write_bytes(c);(assets/'SlideshowFooterData.h').write_bytes(f)
   manifest=dict(schema=1,kind='slideshow-compatibility-metadata',pairRequired=True,imagePayloads=False,metadataApproved=True,headers={name:dict(sha256=H(raw),kind=prep.SLIDESHOW_HEADERS[name],license='MIT',notice=prep.SLIDESHOW_NOTICE.as_posix())for name,raw in [('SlideshowCatalogData.h',c),('SlideshowFooterData.h',f)]},counts=n)
   p=d/prep.SLIDESHOW_METADATA;p.write_text(json.dumps(manifest));(d/prep.SLIDESHOW_NOTICE).write_text('fixture MIT')
   with patch.object(prep,'ROOT',d),patch.object(prep,'tracked',lambda relative:d/relative):
    prep.metadata_approval()
    manifest['counts']['resourcePins']=17;p.write_text(json.dumps(manifest))
    with self.assertRaises(ValueError):prep.metadata_approval()
    manifest['counts']['resourcePins']=15;p.write_text(json.dumps(manifest));(assets/'SlideshowFooterData.h').write_bytes(f+b' ')
    with self.assertRaisesRegex(ValueError,'SHA mismatch'):prep.metadata_approval()
if __name__=='__main__':unittest.main()

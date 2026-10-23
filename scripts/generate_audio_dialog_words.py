"""Generate eleven fixed Audio dialog word images; no fonts or game data are loaded."""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SOURCE_BYTES=648833
SOURCE_SHA='e892bae75b465be3a98045d785166793cd0380ef68873342441746fbfab889a9'
WORDS=((1, 'menu music', 304, 128, [292, 144, 368, 176], 'b2136101eb36d12ab84949302b27487a156adf4eb7ad639cc3bb25a14c238e3f'), (2, 'racing music', 324, 128, [287, 170, 368, 202], 'd71d6eb73948783011532572694719d8c08b0f2b4c8dffa6a4559eb379400378'), (3, 'sound effects', 352, 128, [280, 196, 368, 228], '62e099128222e6814ad3ff74602441fbf451711ad579fd31c008c6424338ce22'), (4, 'interactive music', 440, 128, [258, 222, 368, 254], '2b0769dfc94742c342980f34eba59df5d4c659dbdc556053b71151a86d629d81'), (5, 'racing tune', 292, 128, [295, 248, 368, 280], '0f2a94bf83527685bb7bbbe450b450befc75015fc8fbd2546f4785aec6ead948'), (6, 'engine sounds', 376, 128, [274, 274, 368, 306], 'ad64979b4907b4b0c18efc0c77e20f88f553c17001c92a2f9a43bd46e5c9f5d7'), (7, 'audio mode', 300, 128, [293, 300, 368, 332], '3735d11c3ba16150a5c55eef0726b2eeaf24070e2c3644d24c9c4d7fd3d32f9b'), (8, 'audio quality', 336, 128, [284, 326, 368, 358], '25bb6f0e68748be0e3c6e5973de897f3319625049f4ccaf1f664aabae6d8e55e'), (9, 'DONE', 140, 128, [333, 369, 368, 401], 'dcca47f4cdceb4011aff18d22fb419c5bbd3ebb4abdd8a7abca1e74853d447aa'), (10, 'CANCEL', 204, 128, [388, 369, 439, 401], 'ecb5896d7a00ed0167ca508f81adf335fc972d944c3b3764bcc68256a880912e'), (11, 'AUDIO', 164, 128, [348, 105, 389, 137], '68a2920f6d0a5d1a13987efae2dd71214cf7f2445c7f018c662700b9424334af'))

def _pairs(pairs):
 result={}
 for k,v in pairs:
  if k in result:raise ValueError('Duplicate caption artwork key')
  result[k]=v
 return result

def decode(document):
 if type(document)is not dict or set(document)!={'schema','scope','typeface','baselineOriginalUiPixels','rasterScale','words'} or type(document['schema'])is not int or document['schema']!=1 or document['scope']!='Eleven fixed Audio dialog body, button and heading word graphics; no font file, alphabet or original game pixels.' or document['typeface']!='Haettenschweiler' or type(document['baselineOriginalUiPixels'])is not int or document['baselineOriginalUiPixels']!=24 or type(document['rasterScale'])is not int or document['rasterScale']!=4:raise ValueError('Caption artwork schema changed')
 if type(document['words'])is not list or len(document['words'])!=11:raise ValueError('Caption word inventory changed')
 images=[]
 for record,expected in zip(document['words'],WORDS):
  word,text,w,h,dest,sha=expected
  if type(record)is not dict or set(record)!={'word','text','width','height','destination','rgbaSha256','alphaRuns'} or tuple(record[k]for k in ('word','text','width','height','destination','rgbaSha256'))!=expected:raise ValueError('Caption word identity changed')
  if any(type(record[k])is not int for k in ('word','width','height')) or type(record['destination'])is not list or any(type(v)is not int for v in record['destination']):raise ValueError('Caption word numeric type changed')
  runs=record['alphaRuns'];n=w*h
  if type(runs)is not list or not runs or len(runs)%2 or len(runs)>2*n:raise ValueError('Caption alpha run inventory changed')
  alpha=bytearray()
  for i in range(0,len(runs),2):
   length,value=runs[i:i+2]
   if type(length)is not int or not 1<=length<=65535 or type(value)is not int or not 0<=value<=255 or length>n-len(alpha):raise ValueError('Caption alpha run overflow')
   alpha.extend(bytes((value,))*length)
  if len(alpha)!=n or not any(alpha):raise ValueError('Caption alpha ink/length changed')
  rgba=bytearray(b'\xff'*(n*4));rgba[3::4]=alpha
  if hashlib.sha256(rgba).hexdigest()!=sha:raise ValueError('Caption decoded digest changed')
  # Pixel centers must remain strictly within the preserved source text box.
  ink=[i for i,a in enumerate(alpha)if a]
  if min(i%w for i in ink)<1 or max(i%w for i in ink)>=w-1:raise ValueError('Caption ink exceeds original horizontal bounds')
  if min(i//w for i in ink)<1 or max(i//w for i in ink)>=h-1:raise ValueError('Caption ink exceeds original vertical band')
  images.append(bytes(rgba))
 return tuple(images)

def load(source=None):
 path=ROOT/'assets/hud/frontend/audio-dialog-word-art.json'if source is None else Path(source)
 with path.open('rb')as f:raw=f.read(SOURCE_BYTES+1)
 if len(raw)!=SOURCE_BYTES or hashlib.sha256(raw).hexdigest()!=SOURCE_SHA:raise ValueError('Caption artwork source digest changed')
 document=json.loads(raw,object_pairs_hook=_pairs);return document,decode(document)

def outputs(source=None):
 document,_=load(source)
 notice='// SPDX-License-Identifier: MIT\n// Eleven complete-word graphics, no font or original pixels. Source SHA256 '+SOURCE_SHA+'\n'
 identity=notice+'#ifndef NFS_AUDIO_DIALOG_WORD_IDENTITY_H\n#define NFS_AUDIO_DIALOG_WORD_IDENTITY_H\nstatic const unsigned char nfs_audio_dialog_word_sha256[11][32]={\n'+',\n'.join('{'+','.join(str(v)for v in bytes.fromhex(word[-1]))+'}'for word in WORDS)+'\n};\n#endif\n'
 data=notice+'#ifndef NFS_AUDIO_DIALOG_WORD_DATA_H\n#define NFS_AUDIO_DIALOG_WORD_DATA_H\n'
 for record in document['words']:
  values=record['alphaRuns'];data+='static const unsigned short nfs_audio_dialog_word_alpha_rle_'+str(record['word'])+'[]={\n'+',\n'.join(','.join(map(str,values[i:i+24]))for i in range(0,len(values),24))+'\n};\n'
 data+='#endif\n'
 return {'AudioDialogWordIdentity.h':identity,'AudioDialogWordData.h':data}

def generate(check=False,*,source=None,output_dir=None):
 directory=ROOT/'src/hd'if output_dir is None else Path(output_dir)
 # Authenticate and validate all source/output bytes before any header changes.
 encoded={name:text.encode('ascii')for name,text in outputs(source).items()}
 if any(len(data)>=1048576 for data in encoded.values()):raise ValueError('Caption generated source too large')
 for name,data in encoded.items():
  path=directory/name
  if check:
   if path.read_bytes()!=data:raise ValueError('Stale caption artwork header '+name)
  elif not path.exists()or path.read_bytes()!=data:path.write_bytes(data)

if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');generate(parser.parse_args().check);print('Audio dialog artwork verified')

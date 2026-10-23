"""Generate eight fixed Graphics dialog word images; no fonts or game data are loaded."""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SOURCE_BYTES=435255
SOURCE_SHA='cfd64aed926130a87a5fb3e56799428a8dce2772cba06839ebb21cf3afcfe8a8'
WORDS=((1, 'car detail (F2)', 360, 128, [336, 144, 426, 176], 'bcf1df4ebf3c2a2b3c1c272e0372556496004a895f00e0fbac39e22b1f09c212'), (2, 'brightness (F10)', 420, 128, [321, 170, 426, 202], '343fa1080adfb2d53d5aa8ce89cc0f6912cc9d6239a5f7b53d70598d061c43f2'), (3, 'view distance (F3)', 480, 128, [306, 196, 426, 228], '8453f8d689c3903778845183780304eecc7e00035310551418507221fab25728'), (4, 'horizon (F4)', 304, 128, [350, 222, 426, 254], '2c36b813293fbc3d301643b06efa7d3d483d73bb2876e0393a2aebdf4abdd367'), (5, 'reset', 132, 128, [393, 274, 426, 306], '92e18d8156ccb9f0c6e04a626b61e7f940a7323b8093b9f6dba71828231b371b'), (6, 'DONE', 140, 128, [391, 317, 426, 349], 'dcca47f4cdceb4011aff18d22fb419c5bbd3ebb4abdd8a7abca1e74853d447aa'), (7, 'CANCEL', 204, 128, [466, 317, 517, 349], 'ecb5896d7a00ed0167ca508f81adf335fc972d944c3b3764bcc68256a880912e'), (8, 'GRAPHICS', 272, 128, [358, 105, 426, 137], '98a3e870e635a7410f1dec0d7c85d4cc117d70b10b822fe02ad11553e4a62035'))

def _pairs(pairs):
 result={}
 for k,v in pairs:
  if k in result:raise ValueError('Duplicate caption artwork key')
  result[k]=v
 return result

def decode(document):
 if type(document)is not dict or set(document)!={'schema','scope','typeface','baselineOriginalUiPixels','rasterScale','words'} or type(document['schema'])is not int or document['schema']!=1 or document['scope']!='Eight fixed Graphics dialog word graphics; no font file, alphabet or original game pixels.' or document['typeface']!='Haettenschweiler' or type(document['baselineOriginalUiPixels'])is not int or document['baselineOriginalUiPixels']!=24 or type(document['rasterScale'])is not int or document['rasterScale']!=4:raise ValueError('Caption artwork schema changed')
 if type(document['words'])is not list or len(document['words'])!=8:raise ValueError('Caption word inventory changed')
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
 path=ROOT/'assets/hud/frontend/graphics-dialog-word-art.json'if source is None else Path(source)
 with path.open('rb')as f:raw=f.read(SOURCE_BYTES+1)
 if len(raw)!=SOURCE_BYTES or hashlib.sha256(raw).hexdigest()!=SOURCE_SHA:raise ValueError('Caption artwork source digest changed')
 document=json.loads(raw,object_pairs_hook=_pairs);return document,decode(document)

def outputs(source=None):
 document,_=load(source)
 notice='// SPDX-License-Identifier: MIT\n// Eight complete-word graphics, no font or original pixels. Source SHA256 '+SOURCE_SHA+'\n'
 identity=notice+'#ifndef NFS_GRAPHICS_DIALOG_WORD_IDENTITY_H\n#define NFS_GRAPHICS_DIALOG_WORD_IDENTITY_H\nstatic const unsigned char nfs_graphics_dialog_word_sha256[8][32]={\n'+',\n'.join('{'+','.join(str(v)for v in bytes.fromhex(word[-1]))+'}'for word in WORDS)+'\n};\n#endif\n'
 data=notice+'#ifndef NFS_GRAPHICS_DIALOG_WORD_DATA_H\n#define NFS_GRAPHICS_DIALOG_WORD_DATA_H\n'
 for record in document['words']:
  values=record['alphaRuns'];data+='static const unsigned short nfs_graphics_dialog_word_alpha_rle_'+str(record['word'])+'[]={\n'+',\n'.join(','.join(map(str,values[i:i+24]))for i in range(0,len(values),24))+'\n};\n'
 data+='#endif\n'
 return {'GraphicsDialogWordIdentity.h':identity,'GraphicsDialogWordData.h':data}

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
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');generate(parser.parse_args().check);print('Graphics dialog artwork verified')

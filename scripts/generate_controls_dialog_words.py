"""Generate five fixed Controls dialog word images; no fonts or game data are loaded."""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SOURCE_BYTES=293786
SOURCE_SHA='264ac8c6a2a198d12dadd4673deaa641bc70418864c4d164e0f8e67342bb8f36'
WORDS=((1, 'CONTROLS', 272, 128, [382, 105, 450, 137], '1776f2c15b5936b59f44a5346ed0524149026b4a1f8f8b666ab400bc6db31f53'), (2, 'setup', 144, 128, [380, 170, 416, 202], '8129b04761c19702a89f5a705ac5252db8a92ac50dd0e70d8feacd529ec0d197'), (3, 'steering sensitivity', 492, 128, [293, 196, 416, 228], '8c68421bdcb0a2a5eaa3f89c5cf97104da5989ab241e6669a4035719b590320f'), (4, 'force feedback', 384, 128, [320, 224, 416, 256], '603163b4df5f7b5b02a26906c60827f681593d56a1ca6ca14159665302812bdc'), (5, 'DONE', 140, 128, [381, 267, 416, 299], 'dcca47f4cdceb4011aff18d22fb419c5bbd3ebb4abdd8a7abca1e74853d447aa'))

def _pairs(pairs):
 result={}
 for k,v in pairs:
  if k in result:raise ValueError('Duplicate caption artwork key')
  result[k]=v
 return result

def decode(document):
 if type(document)is not dict or set(document)!={'schema','scope','typeface','baselineOriginalUiPixels','rasterScale','words'} or type(document['schema'])is not int or document['schema']!=1 or document['scope']!='Five fixed Controls landing word graphics; no font file, alphabet or original game pixels.' or document['typeface']!='Haettenschweiler' or type(document['baselineOriginalUiPixels'])is not int or document['baselineOriginalUiPixels']!=24 or type(document['rasterScale'])is not int or document['rasterScale']!=4:raise ValueError('Caption artwork schema changed')
 if type(document['words'])is not list or len(document['words'])!=5:raise ValueError('Caption word inventory changed')
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
 path=ROOT/'assets/hud/frontend/controls-dialog-word-art.json'if source is None else Path(source)
 with path.open('rb')as f:raw=f.read(SOURCE_BYTES+1)
 if len(raw)!=SOURCE_BYTES or hashlib.sha256(raw).hexdigest()!=SOURCE_SHA:raise ValueError('Caption artwork source digest changed')
 document=json.loads(raw,object_pairs_hook=_pairs);return document,decode(document)

def outputs(source=None):
 document,_=load(source)
 notice='// SPDX-License-Identifier: MIT\n// Five complete-word controls, no font or original pixels. Source SHA256 '+SOURCE_SHA+'\n'
 identity=notice+'#ifndef NFS_CONTROLS_DIALOG_WORD_IDENTITY_H\n#define NFS_CONTROLS_DIALOG_WORD_IDENTITY_H\nstatic const unsigned char nfs_controls_dialog_word_sha256[5][32]={\n'+',\n'.join('{'+','.join(str(v)for v in bytes.fromhex(word[-1]))+'}'for word in WORDS)+'\n};\n#endif\n'
 data=notice+'#ifndef NFS_CONTROLS_DIALOG_WORD_DATA_H\n#define NFS_CONTROLS_DIALOG_WORD_DATA_H\n'
 for record in document['words']:
  values=record['alphaRuns'];data+='static const unsigned short nfs_controls_dialog_word_alpha_rle_'+str(record['word'])+'[]={\n'+',\n'.join(','.join(map(str,values[i:i+24]))for i in range(0,len(values),24))+'\n};\n'
 data+='#endif\n'
 return {'ControlsDialogWordIdentity.h':identity,'ControlsDialogWordData.h':data}

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
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');generate(parser.parse_args().check);print('Controls dialog artwork verified')

"""Generate two fixed Player caption images; no fonts or game data are loaded."""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SOURCE_BYTES=107630
SOURCE_SHA='a25e745a01e87d261c5d4c0831e5071e710bb8f973287917813a31e373ab9072'
WORDS=((1,'player one',272,128,[100,198,168,230],'a138031f041437e225f80fc1c56426a046d9e81aa70c6f3f930dbf3e596ad4d7'),(2,'player two',280,128,[98,234,168,266],'21419b71691285b643a7960e6c55ca0994ec60bf2558aeafc38bd8258597c573'))

def _pairs(pairs):
 result={}
 for k,v in pairs:
  if k in result:raise ValueError('Duplicate caption artwork key')
  result[k]=v
 return result

def decode(document):
 if type(document)is not dict or set(document)!={'schema','scope','typeface','baselineOriginalUiPixels','rasterScale','words'} or type(document['schema'])is not int or document['schema']!=1 or document['scope']!='Two fixed lowercase Player sidebar word graphics; no font file, alphabet or original game pixels.' or document['typeface']!='Haettenschweiler' or type(document['baselineOriginalUiPixels'])is not int or document['baselineOriginalUiPixels']!=24 or type(document['rasterScale'])is not int or document['rasterScale']!=4:raise ValueError('Caption artwork schema changed')
 if type(document['words'])is not list or len(document['words'])!=2:raise ValueError('Caption word inventory changed')
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
  images.append(bytes(rgba))
 return tuple(images)

def load(source=None):
 path=ROOT/'assets/hud/frontend/player-caption-word-art.json'if source is None else Path(source)
 with path.open('rb')as f:raw=f.read(SOURCE_BYTES+1)
 if len(raw)!=SOURCE_BYTES or hashlib.sha256(raw).hexdigest()!=SOURCE_SHA:raise ValueError('Caption artwork source digest changed')
 document=json.loads(raw,object_pairs_hook=_pairs);return document,decode(document)

def outputs(source=None):
 document,_=load(source)
 notice='// SPDX-License-Identifier: MIT\n// Two complete-word graphics, no font or original pixels. Source SHA256 '+SOURCE_SHA+'\n'
 identity=notice+'#ifndef NFS_PLAYER_CAPTION_WORD_IDENTITY_H\n#define NFS_PLAYER_CAPTION_WORD_IDENTITY_H\nstatic const unsigned char nfs_player_caption_word_sha256[2][32]={\n'+',\n'.join('{'+','.join(str(v)for v in bytes.fromhex(word[-1]))+'}'for word in WORDS)+'\n};\n#endif\n'
 data=notice+'#ifndef NFS_PLAYER_CAPTION_WORD_DATA_H\n#define NFS_PLAYER_CAPTION_WORD_DATA_H\n'
 for record in document['words']:
  values=record['alphaRuns'];data+='static const unsigned short nfs_player_caption_word_alpha_rle_'+str(record['word'])+'[]={\n'+',\n'.join(','.join(map(str,values[i:i+24]))for i in range(0,len(values),24))+'\n};\n'
 data+='#endif\n'
 return {'PlayerCaptionWordIdentity.h':identity,'PlayerCaptionWordData.h':data}

def generate(check=False):
 for name,text in outputs().items():
  p=ROOT/'src/hd'/name;b=text.encode('ascii')
  if len(b)>1048576:raise ValueError('Caption generated source too large')
  if check:
   if p.read_bytes()!=b:raise ValueError('Stale caption artwork header '+name)
  elif not p.exists()or p.read_bytes()!=b:p.write_bytes(b)

if __name__=='__main__':
 parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');generate(parser.parse_args().check);print('Player caption artwork verified')

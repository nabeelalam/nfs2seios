"""Add a separately supplied widescreen photograph to an existing private intro pack.
No embedded images, machine paths or artwork are part of this script.
The original photo, logo rectangle and all six copyright planes stay byte exact.
"""
# SPDX-License-Identifier: MIT
import argparse,hashlib,json,struct,zlib
from pathlib import Path
MAX=96*1024*1024

def extend(original,wide,width,height):
    if len(original)>64*1024*1024 or len(original)<17 or original[:8]!=b'NFSINTRO':raise ValueError('Invalid original intro file')
    version,count=struct.unpack_from('<II',original,8)
    if version!=1 or not 72<=count<=MAX:raise ValueError('Expected original version1 intro pack')
    stream=zlib.decompressobj();payload=stream.decompress(original[16:],count+1)
    if len(payload)!=count or not stream.eof or stream.unused_data or stream.unconsumed_tail:raise ValueError('Invalid original compressed payload')
    pw,ph=struct.unpack_from('<II',payload)
    if not 0<pw<=8192 or not 0<ph<=8192:raise ValueError('Original photograph bounds')
    expected=72+pw*ph*4
    for k in range(6):
        cw,ch=struct.unpack_from('<II',payload,24+8*k)
        if not 0<cw<=8192 or not 0<ch<=8192:raise ValueError('Original copyright bounds')
        expected+=cw*ch
    if expected!=len(payload):raise ValueError('Original image planes do not match payload')
    if not 0<width<=8192 or not 0<height<=8192 or abs(width*9-height*16)>16:raise ValueError('Wide photograph must be 16:9 with at most one pixel rounding')
    if len(wide)!=width*height*4 or len(payload)+8+len(wide)>MAX:raise ValueError('Wide image byte bounds')
    if any(alpha!=255 for alpha in wide[3::4]):raise ValueError('Wide photograph must be opaque')
    result=payload[:72]+struct.pack('<II',width,height)+payload[72:]+wide
    return b'NFSINTRO'+struct.pack('<II',2,len(result))+zlib.compress(result,9)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source',type=Path,required=True)
    parser.add_argument('--source-sha256',required=True)
    parser.add_argument('--wide-photo',type=Path,required=True)
    parser.add_argument('--output',type=Path,required=True)
    args=parser.parse_args()
    if args.output.exists():raise ValueError('Refusing to overwrite an existing pack')
    original=args.source.read_bytes()
    if hashlib.sha256(original).hexdigest()!=args.source_sha256:raise ValueError('Original pack identity differs')
    from PIL import Image
    with Image.open(args.wide_photo) as image:
        if not 0<image.width<=8192 or not 0<image.height<=8192:raise ValueError('Wide image dimensions')
        rgba=image.convert('RGBA');packed=extend(original,rgba.tobytes(),rgba.width,rgba.height)
    with args.output.open('xb') as output:output.write(packed)
    print(json.dumps({'version':2,'fileBytes':len(packed),'payloadBytes':struct.unpack_from('<I',packed,12)[0],'sha256':hashlib.sha256(packed).hexdigest(),'originalPhotoAndCopyrightUnchanged':True}))
if __name__=='__main__':main()

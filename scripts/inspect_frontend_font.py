"""Inspect external FFN metrics for faithful replacements; never export bitmap pixels."""
import argparse
import hashlib
import json
import struct
from pathlib import Path

LIMIT = 1024*1024

def inspect(data):
    if not isinstance(data,bytes) or not 32<=len(data)<=LIMIT or data[:4]!=b'FNTF':
        raise ValueError('Expected a bounded FFN font')
    declared,version,count = struct.unpack_from('<IHH',data,4)
    table,bitmap = struct.unpack_from('<I4xI',data,20)
    # An original numeric font declares four fewer bytes than its full bitmap.
    # Bound all reads by the actual file, and report that discrepancy explicitly.
    if version!=100 or not len(data)-4<=declared<=len(data) or not 0<count<=4096:
        raise ValueError('Unsupported font version, length or glyph count')
    end = table+count*11
    if not 32<=table<=end<=bitmap or bitmap+16>len(data):
        raise ValueError('Font tables overlap or exceed the file')
    code,width,height = struct.unpack_from('<IHH',data,bitmap)
    if code!=0x7a or not 0<width<=4096 or not 0<height<=4096:
        raise ValueError('Expected a bounded original four-bit font bitmap')
    stride = (width*4+7)//8
    if bitmap+16+stride*height!=len(data):
        raise ValueError('Font bitmap has an unexpected byte span')
    glyphs = []
    previous = -1
    for i in range(count):
        char,w,h,x,y,advance,dx,dy = struct.unpack_from('<HBBHHbbb',data,table+i*11)
        if char<=previous or x+w>width or y+h>height or (w==0)!=(h==0):
            raise ValueError('Unsorted glyphs or glyph outside the bitmap')
        previous = char
        glyphs.append(dict(code=char,width=w,height=h,bitmapX=x,bitmapY=y,
                           advance=advance,offsetX=dx,offsetY=dy))
    origin_x,origin_y,line_height,line_gap = struct.unpack_from('<4b',data,16)
    return dict(schema='nfsiise-frontend-font-metrics-v1',fileBytes=len(data),
                declaredBytes=declared,declaredLengthDifference=len(data)-declared,
                version=version,glyphCount=count,glyphTableOffset=table,
                metricsSha256=hashlib.sha256(data[:end]).hexdigest(),
                originOffset=[origin_x,origin_y],lineHeight=line_height,lineGap=line_gap,
                bitmap=dict(offset=bitmap,width=width,height=height,bitsPerPixel=4,stride=stride),
                glyphs=glyphs,bitmapPixelsExported=False)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('font',type=Path)
    args = parser.parse_args()
    try:
        with args.font.open('rb') as stream:
            data = stream.read(LIMIT+1)
        print(json.dumps(inspect(data),indent=2))
    except (OSError,ValueError,struct.error):
        parser.exit(1,'Unable to inspect: unsupported or malformed FFN font.\n')

if __name__=='__main__':
    main()

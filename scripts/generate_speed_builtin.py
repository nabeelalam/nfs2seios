"""Generate new gold numeral pixels from reviewed vectors; no original assets."""
import argparse
import hashlib
import json
from pathlib import Path
from rasterize_hud_svg import rasterize
from publication_audit import reviewed_timing_svg

ROOT=Path(__file__).resolve().parents[1]

def generate():
    directory=ROOT/'assets/hud'
    catalog=json.loads((directory/'speed-font.json').read_text())
    if catalog['vectorSize']!=[29,24] or catalog['rasterSize']!=[116,96] or len(catalog['entries'])!=10:
        raise ValueError('Unexpected new speed glyph layout')
    rgba=bytearray();sources={}
    for digit,entry in enumerate(catalog['entries']):
        if entry['digit']!=digit or entry['file']!='speed-'+str(digit)+'.svg':
            raise ValueError('Unexpected speed glyph filename')
        source=(directory/entry['file']).read_bytes()
        digest=hashlib.sha256(source).hexdigest()
        if digest!=entry['sha256'] or not reviewed_timing_svg('assets/hud/'+entry['file'],source):
            raise ValueError('Unreviewed speed vector source')
        sources[entry['file']]=digest
        pixels=rasterize(source,116,96)
        if len(pixels)!=11136*4:raise ValueError('Unexpected speed raster size')
        rgba+=pixels
    encoded=bytearray();position=0
    while position<len(rgba):
        color=rgba[position:position+4];count=1
        while count<65535 and position+(count+1)*4<=len(rgba) and rgba[position+count*4:position+(count+1)*4]==color:count+=1
        encoded+=bytes((count&255,count>>8))+color;position+=count*4
    def array(name,values):
        lines=[','.join('0x%02x'%b for b in values[i:i+24]) for i in range(0,len(values),24)]
        return 'static const unsigned char '+name+'[]={\n'+',\n'.join(lines)+'\n};\n'
    text='// SPDX-License-Identifier: MIT\n// Generated from new vector paths only; no original game pixels.\n'
    text+=array('nfs_speed_raster_digest',hashlib.sha256(rgba).digest())
    text+=array('nfs_speed_pixels_rle',encoded)
    return text,dict(generatedFrom='New vector gold numerals only',originalGamePixelsIncluded=False,
        sourceHashes=sources,rasterSha256=hashlib.sha256(rgba).hexdigest(),rasterBytes=len(rgba),
        rleBytes=len(encoded),headerSha256=hashlib.sha256(text.encode()).hexdigest())

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    text,manifest=generate();header=ROOT/'src/SpeedFontPixels.inc';meta=ROOT/'assets/hud/speed-generated.json'
    if args.check:
        if header.read_text()!=text or json.loads(meta.read_text())!=manifest:
            raise SystemExit('Generated speed artwork differs; regenerate and review')
    else:
        header.write_text(text);meta.write_text(json.dumps(manifest,indent=2)+'\n')
    print('Verified generated speed pixels' if args.check else 'Generated new speed pixels without reading original assets')

if __name__=='__main__':main()

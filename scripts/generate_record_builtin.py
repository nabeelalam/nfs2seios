"""Generate newly drawn record-time glyphs; zero shares the reviewed O art."""
import argparse
import hashlib
import json
from pathlib import Path
from publication_audit import reviewed_timing_svg
from rasterize_hud_svg import rasterize

ROOT=Path(__file__).resolve().parents[1]


def generate():
    directory=ROOT/'assets/hud'
    catalog=json.loads((directory/'record-font.json').read_text())
    characters='0123456789.:'
    if catalog['version']!=1 or len(catalog['entries'])!=12:
        raise ValueError('Unexpected record font catalog')
    sources={};rgba=bytearray();alias=None
    for index,entry in enumerate(catalog['entries']):
        character=characters[index];width=4 if character in '1.:' else 8
        filename=('small-O.svg' if index==0 else 'record-'+('dot' if character=='.' else 'colon' if character==':' else character)+'.svg')
        if (entry['character']!=character or entry['file']!=filename or
                entry['vectorSize']!=[width,8] or entry['rasterSize']!=[width*8,64]):
            raise ValueError('Unexpected record glyph layout')
        source=(directory/filename).read_bytes();digest=hashlib.sha256(source).hexdigest()
        if digest!=entry['sha256'] or not reviewed_timing_svg('assets/hud/'+filename,source):
            raise ValueError('Unreviewed record vector')
        sources[filename]=digest;pixels=rasterize(source,width*8,64)
        if len(pixels)!=width*8*64*4:raise ValueError('Unexpected record raster size')
        if index==0:
            if entry.get('aliasOf')!='O':raise ValueError('Zero must share O')
            alias=hashlib.sha256(pixels).hexdigest();continue
        for y in range(64):
            rgba+=pixels[y*width*32:(y+1)*width*32]
            rgba+=bytes((64-width*8)*4)
    encoded=bytearray();position=0
    while position<len(rgba):
        color=rgba[position:position+4];count=1
        while count<65535 and position+(count+1)*4<=len(rgba) and rgba[position+count*4:position+(count+1)*4]==color:count+=1
        encoded+=bytes((count&255,count>>8))+color;position+=count*4
    def array(name,values):
        lines=[','.join('0x%02x'%b for b in values[i:i+24]) for i in range(0,len(values),24)]
        return 'static const unsigned char '+name+'[]={\n'+',\n'.join(lines)+'\n};\n'
    text='// SPDX-License-Identifier: MIT\n// New vector pixels only; zero uses the existing O tile.\n'
    text+=array('nfs_record_font_raster_digest',hashlib.sha256(rgba).digest())
    text+=array('nfs_record_font_pixels_rle',encoded)
    return text,dict(generatedFrom='New record vectors only; zero aliases reviewed O',originalGamePixelsIncluded=False,
        sourceHashes=sources,zeroAliasRasterSha256=alias,rasterSha256=hashlib.sha256(rgba).hexdigest(),rasterBytes=len(rgba),
        storedTiles=11,rleBytes=len(encoded),headerSha256=hashlib.sha256(text.encode()).hexdigest())


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    text,manifest=generate();header=ROOT/'src/RecordFontPixels.inc';meta=ROOT/'assets/hud/record-generated.json'
    if args.check:
        if header.read_text()!=text or json.loads(meta.read_text())!=manifest:raise SystemExit('Generated record artwork differs')
    else:
        header.write_bytes(text.encode());meta.write_bytes((json.dumps(manifest,indent=2)+'\n').encode())
    print(json.dumps(manifest))


if __name__=='__main__':main()

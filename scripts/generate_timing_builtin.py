"""Generate only new glyph pixels from reviewed SVGs; never reads original game data."""
import argparse
import hashlib
import json
from pathlib import Path
import re
from rasterize_hud_svg import rasterize
ROOT=Path(__file__).resolve().parents[1]

def generate():
    directory=ROOT/'assets/hud';catalog=json.loads((directory/'timing-font.json').read_text())
    if catalog.get('artworkRevision')=='existing-font-v1':
        from generate_remaster_pixels import generate as remaster
        return remaster('timing')
    fingerprint=catalog['originalGpuFingerprint']
    if fingerprint['edge']!=256 or fingerprint['sampleStride']!=4096 or fingerprint['sampleBytes']!=16 or not re.fullmatch('[0-9a-f]{64}',fingerprint['sha256']) or not re.fullmatch('[0-9a-f]{16}',fingerprint['sampleFnv64']):
        raise ValueError('Unsupported atlas fingerprint metadata')
    rgba=bytearray();sources={}
    for digit,entry in enumerate(catalog['entries']):
        if entry['digit']!=digit or entry['file']!='timing-'+str(digit)+'.svg' or entry['atlasRect']!=[48+12*digit,21,12,26]:raise ValueError('Unexpected glyph layout')
        source=(directory/entry['file']).read_bytes();digest=hashlib.sha256(source).hexdigest()
        if digest!=entry['sha256']:raise ValueError('Vector source mismatch')
        sources[entry['file']]=digest
        rgba+=rasterize(source,48,108)[:48*104*4]
    if len(rgba)!=10*48*104*4:raise ValueError('Unexpected glyph size')
    encoded=bytearray();position=0
    while position<len(rgba):
        color=rgba[position:position+4];count=1
        while count<65535 and position+(count+1)*4<=len(rgba) and rgba[position+count*4:position+(count+1)*4]==color:count+=1
        encoded+=bytes((count&255,count>>8))+color;position+=count*4
    def array(name,values):
        lines=[','.join('0x%02x'%b for b in values[i:i+24]) for i in range(0,len(values),24)]
        return 'static const unsigned char '+name+'[]={\n'+',\n'.join(lines)+'\n};\n'
    text='// SPDX-License-Identifier: MIT\n// Generated from new vector paths only; no original game pixels.\n'
    text+=array('nfs_timing_original_digest',bytes.fromhex(fingerprint['sha256']))
    text+=array('nfs_timing_raster_digest',hashlib.sha256(rgba).digest())
    text+='static const uint64_t nfs_timing_sample_expected=UINT64_C(0x'+fingerprint['sampleFnv64']+');\n'
    text+=array('nfs_timing_pixels_rle',encoded)
    return text,dict(generatedFrom='New vector timing glyphs only',originalGamePixelsIncluded=False,sourceHashes=sources,rasterSha256=hashlib.sha256(rgba).hexdigest(),rasterBytes=len(rgba),rleBytes=len(encoded),headerSha256=hashlib.sha256(text.encode()).hexdigest())

def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    text,manifest=generate();header=ROOT/'src/TimingFontPixels.inc';meta=ROOT/'assets/hud/timing-generated.json'
    if args.check:
        if header.read_text()!=text or json.loads(meta.read_text())!=manifest:raise SystemExit('Generated timing artwork differs; regenerate and review')
    else:
        header.write_text(text);meta.write_text(json.dumps(manifest,indent=2)+'\n')
    print('Verified generated timing pixels' if args.check else 'Generated new timing pixels without reading original assets')
if __name__=='__main__':main()

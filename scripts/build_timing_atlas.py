"""Developer-only pack builder from a private captured GPU atlas and new vector art.

Requires locally installed librsvg/Cairo. The resulting pack contains original
pixels, must stay private, and is ignored by Git. No original assets are supplied.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
from rasterize_hud_svg import rasterize

ROOT=Path(__file__).resolve().parents[1]

def build(original,expected_sha256):
    if len(original)!=256*256*2 or not re.fullmatch(r"[0-9a-fA-F]{64}",expected_sha256) or hashlib.sha256(original).hexdigest()!=expected_sha256.lower():
        raise ValueError("Original atlas size or expected fingerprint does not match")
    directory=ROOT/"assets/hud"
    catalog=json.loads((directory/"timing-font.json").read_text(encoding="utf-8"))
    if catalog["version"]!=1 or catalog["scale"]!=4 or len(catalog["entries"])!=10:
        raise ValueError("Unsupported timing font catalog")
    rgba=bytearray(1024*1024*4)
    values=struct.unpack("<65536H",original)
    for y in range(1024):
        for x in range(1024):
            value=values[(y//4)*256+x//4];at=(y*1024+x)*4
            rgba[at:at+4]=bytes((((value>>11)&31)*255//31,((value>>6)&31)*255//31,((value>>1)&31)*255//31,255 if value&1 else 0))
    seen=set()
    for entry in catalog["entries"]:
        digit=entry["digit"]
        if type(digit) is not int or not 0<=digit<=9 or digit in seen or entry["file"]!="timing-"+str(digit)+".svg" or entry["atlasRect"]!=[48+12*digit,21,12,26] or entry["vectorViewBox"]!=[0,0,12,27]:
            raise ValueError("Unexpected timing glyph geometry")
        seen.add(digit)
        source=(directory/entry["file"]).read_bytes()
        if len(source)>65536 or hashlib.sha256(source).hexdigest()!=entry["sha256"]:
            raise ValueError("Timing vector source differs from its catalog")
        glyph=rasterize(source,48,108)
        if len(glyph)!=48*108*4:raise ValueError("Invalid vector raster dimensions")
        left=(48+12*digit)*4;top=21*4
        # 27-unit artwork is clipped to 26 source rows: row47 belongs to the
        # next sprite. Keep every original texel outside these exact rectangles.
        for y in range(104):
            at=((top+y)*1024+left)*4
            rgba[at:at+192]=glyph[y*192:(y+1)*192]
    return struct.pack("<8s6I",b"NFSHDA1\0",256,1024,len(original),len(rgba),0,0)+original+rgba

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("original",type=Path,help="Private 256-square converted GPU RGBA5551 capture")
    parser.add_argument("output",type=Path,help="New private .nfsatlas file; existing files are refused")
    parser.add_argument("--expected-sha256",required=True,help="Trusted original-capture fingerprint")
    args=parser.parse_args()
    if args.output.suffix!=".nfsatlas":parser.error("Private output must use the ignored .nfsatlas extension")
    try:
        with args.original.open("rb") as source:original=source.read(256*256*2+1)
        result=build(original,args.expected_sha256)
        with args.output.open("xb") as target:target.write(result)
    except (OSError,ValueError,KeyError,TypeError,AttributeError):
        parser.exit(1,"Unable to build: check the private capture/fingerprint, vector catalog, native libraries and new output filename.\n")
    print("Created private timing atlas pack; no original file was changed.")

if __name__=="__main__":main()

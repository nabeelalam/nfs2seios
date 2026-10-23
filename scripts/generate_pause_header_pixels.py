"""Generate two digest-pinned authored pause-header tiles; never reads game data."""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
import xml.etree.ElementTree as ET
from pathlib import Path
from hud_asset_review import ROOT
from hud_font_environment import project_fonts
from rasterize_hud_svg import rasterize

SOURCES = (
    ('assets/hud/pause/header-title.svg', 'title', 520, 112, 240, 398,
     '3fad7f2f5c2bdddafee5e1f977cda85b0551bd68a5422d5a876735dc0a970686', '0 0 130 29'),
    ('assets/hud/pause/header-caution.svg', 'caution', 152, 112, 768, 398,
     '46c47dc4490e8ef15369c3f4b3dd4f330c36823335fb4197cda84aeae0c31e7d', '0 0 38 29'),
)

def reviewed_pause_header_source(name, data):
    spec = next((s for s in SOURCES if s[0] == name), None)
    if spec is None or not isinstance(data, bytes) or len(data) > 65536:
        return False
    if hashlib.sha256(data).hexdigest() != spec[6]:
        return False
    if any(token in data.lower() for token in (b'<!', b'<?', b'href', b'data:')):
        return False
    try:
        root = ET.fromstring(data)
        ns = '{http://www.w3.org/2000/svg}'
        if root.tag != ns+'svg' or root.attrib != dict(width=str(spec[2]), height=str(spec[3]),
                preserveAspectRatio='none', viewBox=spec[7]):
            return False
        allowed = {'svg','defs','linearGradient','stop','path','g','text'}
        for node in root.iter():
            if not node.tag.startswith(ns) or node.tag[len(ns):] not in allowed:
                return False
            if node.tag == ns+'text' and (node.text != 'PAUSE' or node.attrib.get('font-family') != 'Michroma'):
                return False
        return True
    except ET.ParseError:
        return False

def encode_rle(pixels):
    if not isinstance(pixels, bytes) or not pixels or len(pixels) % 4:
        raise ValueError('Nonempty RGBA required')
    out = bytearray()
    for at in range(0, len(pixels), 4):
        color = pixels[at:at+4]
        if color[3] != 255:
            raise ValueError('Authored pause header must remain opaque')
        if at and color == previous and count < 65535:
            count += 1
        else:
            if at: out += count.to_bytes(2,'little') + previous
            previous, count = color, 1
    out += count.to_bytes(2,'little') + previous
    return bytes(out)

def array(name, raw):
    return 'static const unsigned char '+name+'[]={\n'+',\n'.join(
        ','.join(str(x) for x in raw[i:i+24]) for i in range(0,len(raw),24))+'\n};\n'

DECODER = r"""
// Validate both complete streams before the first atlas write. Fixed destinations
// preserve all existing art, including the numeric filtering gutter at y396..397 and y510..511.
static int nfs_pause_header_stream_valid(const unsigned char *rle,size_t bytes,unsigned expected){
 if(!rle || !bytes || bytes%6 || bytes>(size_t)expected*6)return 0;
 unsigned count=0;
 for(size_t i=0;i<bytes;i+=6){unsigned n=rle[i]|((unsigned)rle[i+1]<<8);
  if(!n || n>expected-count || rle[i+5]!=255)return 0;count+=n;}
 return count==expected;
}
static void nfs_pause_header_write(const unsigned char *rle,size_t bytes,unsigned char *rgba,unsigned width,unsigned ax){
 unsigned pos=0;
 for(size_t i=0;i<bytes;i+=6){unsigned n=rle[i]|((unsigned)rle[i+1]<<8);
  for(unsigned j=0;j<n;++j,++pos)memcpy(rgba+((size_t)(398+pos/width)*1024+ax+pos%width)*4,rle+i+2,4);}
}
static int nfs_pause_header_expand_checked(unsigned char *rgba,size_t capacity,
 const unsigned char *title,size_t titleBytes,const unsigned char *caution,size_t cautionBytes){
 if(!rgba || capacity<4194304 ||
  !nfs_pause_header_stream_valid(title,titleBytes,520u*112u) ||
  !nfs_pause_header_stream_valid(caution,cautionBytes,152u*112u))return 0;
 nfs_pause_header_write(title,titleBytes,rgba,520,240);
 nfs_pause_header_write(caution,cautionBytes,rgba,152,768);return 1;
}
static int nfs_pause_header_expand(unsigned char *rgba,size_t capacity){
 return nfs_pause_header_expand_checked(rgba,capacity,nfs_pause_header_title_rle,sizeof(nfs_pause_header_title_rle),
  nfs_pause_header_caution_rle,sizeof(nfs_pause_header_caution_rle));
}
"""

def generate():
    text = ('// Renderer code: MIT. Newly authored pause header; Michroma artwork: assets/hud/fonts/Michroma-OFL.txt.\n'
            '#ifndef NFS_PAUSE_HEADER_PIXELS_INC\n#define NFS_PAUSE_HEADER_PIXELS_INC\n'
            '#include <stddef.h>\n#include <string.h>\n')
    entries = []
    with project_fonts():
        for name, label, width, height, left, top, digest, viewbox in SOURCES:
            data = (ROOT/name).read_bytes()
            if not reviewed_pause_header_source(name,data):
                raise ValueError('Unreviewed pause header source: '+name)
            pixels = rasterize(data,width,height)
            if len(pixels) != width*height*4:
                raise ValueError('Pause header raster dimensions differ')
            encoded = encode_rle(pixels)
            text += array('nfs_pause_header_'+label+'_rle', encoded)
            entries.append(dict(source=name, sourceSha256=digest, rectangle=[left,top,left+width,top+height],
                rgbaBytes=len(pixels), rgbaSha256=hashlib.sha256(pixels).hexdigest(),
                rleBytes=len(encoded), rleSha256=hashlib.sha256(encoded).hexdigest()))
    text += DECODER + '#endif\n'
    if len(text.encode()) >= 1048576:
        raise ValueError('Pause header generated file exceeds bounded source size')
    receipt = dict(schema=1, originalGamePixelsIncluded=False,
        generatedFrom='Digest-pinned authored classic yellow plate/caution vectors and pinned Michroma',
        atlasSize=[1024,1024], preservedNumericGutterRows=[396,397], preservedGlyphGutterRows=[510,511], entries=entries,
        headerSha256=hashlib.sha256(text.encode()).hexdigest())
    return text, receipt

def generate_pause_header(check=False):
    text, receipt = generate()
    output = ROOT/'src/PauseHeaderPixels.inc'
    metadata = ROOT/'assets/hud/pause-header-generated.json'
    if check:
        if output.read_bytes() != text.encode() or json.loads(metadata.read_text()) != receipt:
            raise ValueError('Generated pause header or receipt differs')
    else:
        output.write_bytes(text.encode())
        metadata.write_bytes((json.dumps(receipt,indent=2)+'\n').encode())
    return receipt

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check',action='store_true')
    args = parser.parse_args()
    generate_pause_header(args.check)
    print('PASS pause header authored sources, pinned font, bounded pixels and receipt' if args.check else 'Generated authored pause header pixels')

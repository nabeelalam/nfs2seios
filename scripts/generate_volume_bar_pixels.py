"""Generate the newly authored classic pause volume cap/channel material."""
# SPDX-License-Identifier: MIT
import argparse
import hashlib
import json
import xml.etree.ElementTree as ET
from hud_asset_review import ROOT
from rasterize_hud_svg import rasterize

SOURCE='assets/hud/pause/volume-bar.svg'
SOURCE_SHA256='1bc23122925d97b9de8c3859e6cfddead9560535a2dc203e710c55d4e0a43918'
RECTANGLE=[920,398,1000,510]

def reviewed_volume_bar_source(name,data):
    if name!=SOURCE or not isinstance(data,bytes) or len(data)>65536 or hashlib.sha256(data).hexdigest()!=SOURCE_SHA256:
        return False
    if any(t in data.lower() for t in (b'<!',b'<?',b'href',b'data:')):return False
    try:
        root=ET.fromstring(data);ns='{http://www.w3.org/2000/svg}'
        if root.tag!=ns+'svg' or root.attrib!=dict(width='80',height='112',preserveAspectRatio='none',viewBox='0 0 20 28'):return False
        return all(node.tag.startswith(ns) and node.tag[len(ns):] in {'svg','defs','linearGradient','stop','path'} for node in root.iter())
    except ET.ParseError:return False

def encode_rle(pixels):
    if not isinstance(pixels,bytes) or len(pixels)!=80*112*4:raise ValueError('Exact volume material RGBA required')
    out=bytearray();count=0;previous=b''
    for at in range(0,len(pixels),4):
        color=pixels[at:at+4]
        if color==previous and count<65535:count+=1
        else:
            if count:out+=count.to_bytes(2,'little')+previous
            count=1;previous=color
    out+=count.to_bytes(2,'little')+previous
    return bytes(out)

DECODER=r"""
// Fixed finite material. Validate all runs before any write. No original page,
// accepted HUD image or filtering gutter is a decoder destination.
static int nfs_volume_bar_expand_checked(unsigned char *rgba,size_t capacity,const unsigned char *rle,size_t bytes){
 const unsigned pixels=80u*112u;
 if(!rgba || capacity<4194304 || !rle || !bytes || bytes%6 || bytes>(size_t)pixels*6)return 0;
 unsigned count=0;
 for(size_t i=0;i<bytes;i+=6){unsigned n=rle[i]|((unsigned)rle[i+1]<<8);if(!n || n>pixels-count)return 0;count+=n;}
 if(count!=pixels)return 0;
 unsigned pos=0;
 for(size_t i=0;i<bytes;i+=6){unsigned n=rle[i]|((unsigned)rle[i+1]<<8);
  for(unsigned j=0;j<n;++j,++pos)memcpy(rgba+((size_t)(398+pos/80)*1024+920+pos%80)*4,rle+i+2,4);}
 return 1;
}
static int nfs_volume_bar_expand(unsigned char *rgba,size_t capacity){
 return nfs_volume_bar_expand_checked(rgba,capacity,nfs_volume_bar_rle,sizeof(nfs_volume_bar_rle));
}
"""

def generate():
    data=(ROOT/SOURCE).read_bytes()
    if not reviewed_volume_bar_source(SOURCE,data):raise ValueError('Unreviewed volume material')
    pixels=rasterize(data,80,112)
    if len(pixels)!=80*112*4:raise ValueError('Volume material dimensions differ')
    if not any(a==0 for a in pixels[3::4]) or not any(a==255 for a in pixels[3::4]) or not any(0<a<255 for a in pixels[3::4]):
        raise ValueError('Volume cap must retain transparent exterior and smooth coverage')
    # The stretched centre column has no cap/separator contamination.
    if any(pixels[(y*80+79)*4+3]!=255 for y in range(8,104)):
        raise ValueError('Volume centre/rim cross-section must remain opaque')
    encoded=encode_rle(pixels)
    text=('// Renderer code: MIT. Newly authored classic metallic pause volume material.\n'
          '#ifndef NFS_VOLUME_BAR_PIXELS_INC\n#define NFS_VOLUME_BAR_PIXELS_INC\n#include <stddef.h>\n#include <string.h>\n'
          'static const unsigned char nfs_volume_bar_rle[]={\n'+',\n'.join(','.join(str(x) for x in encoded[i:i+24]) for i in range(0,len(encoded),24))+'\n};\n'+DECODER+'#endif\n')
    if len(text.encode())>=1048576:raise ValueError('Volume material source exceeds bounded file size')
    receipt=dict(schema=1,originalGamePixelsIncluded=False,source=SOURCE,sourceSha256=SOURCE_SHA256,
        generatedFrom='Newly authored classic cap, divider and recessed channel vector',rectangle=RECTANGLE,
        logicalProfile=[20,28],originalSprite=[0,172,170,28],originalVisibleExtent=[0,166],
        logicalDivider=18,logicalChannelStart=19,continuousMirrorX=167,transparentTrailingColumns=[167,168,169],preservedGutterRows=[396,397,510,511],
        rgbaSha256=hashlib.sha256(pixels).hexdigest(),rgbaBytes=len(pixels),rleSha256=hashlib.sha256(encoded).hexdigest(),
        rleBytes=len(encoded),headerSha256=hashlib.sha256(text.encode()).hexdigest())
    return text,receipt

def generate_volume_bar(check=False):
    text,receipt=generate();output=ROOT/'src/VolumeBarPixels.inc';metadata=ROOT/'assets/hud/volume-bar-generated.json'
    if check:
        if output.read_bytes()!=text.encode() or json.loads(metadata.read_text())!=receipt:raise ValueError('Volume material or receipt differs')
    else:
        output.write_bytes(text.encode());metadata.write_bytes((json.dumps(receipt,indent=2)+'\n').encode())
    return receipt

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    generate_volume_bar(args.check)
    print('PASS authored volume material and bounded receipt' if args.check else 'Generated authored volume material')

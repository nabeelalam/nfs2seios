"""Generate the authored condensed main-menu atlas without original game data."""
import hashlib
import json
import math
from hud_asset_review import ROOT, FRONTEND_WORDS
from hud_font_environment import project_fonts
from generate_remaster_pixels import array, rle, source


def generate_frontend(check=False):
    directory = ROOT/'assets/hud/frontend'
    catalog = json.loads((directory/'catalog.json').read_text())
    words = catalog['words']
    if len(words) != 8 or [w['file'] for w in words] != [n+'.svg' for n in FRONTEND_WORDS]:
        raise ValueError('Unexpected reviewed main-menu word set')
    pixels = bytearray(1024*1024*4)
    occupied = set()
    hashes, ink, rows = {}, [], []
    with project_fonts():
        for word in words:
            name = word['file'][:-4]
            text, width = FRONTEND_WORDS[name]
            if word['text'] != text or word['width'] != width or word['rightAnchor'] != 168:
                raise ValueError('Unexpected reviewed word dimensions')
            ax, ay, y0 = word['x'], word['y'], word['originalY']
            if any(type(x) is not int for x in (ax, ay, y0)) or not (0 <= ax < ax+width <= 256 and 0 <= ay < ay+51 <= 256):
                raise ValueError('Frontend atlas bounds invalid')
            left, top, right, bottom = word['hitBounds']
            if left != 168-width or right != 180 or not 0 <= top < bottom <= 480:
                raise ValueError('Frontend interactive bounds invalid')
            rgba = source(directory/word['file'], width*4, 204, hashes)
            hits = 0
            for y in range(204):
                at = ((ay*4+y)*1024+ax*4)*4
                pixels[at:at+width*16] = rgba[y*width*16:(y+1)*width*16]
                for x in range(width*4):
                    position = (ay*4+y)*1024+ax*4+x
                    if position in occupied:
                        raise ValueError('Overlapping frontend word allocation')
                    occupied.add(position)
                    if rgba[(y*width*4+x)*4+3]:
                        xx, yy = 168-width+(x+.5)/4, y0+(y+.5)/4
                        if not (left < xx < right and top < yy < bottom):
                            raise ValueError('Frontend ink lies outside its interactive rectangle')
                        hits += 1
            if not hits:
                raise ValueError('Empty reviewed word artwork')
            ink.append(dict(text=text, nontransparentPixels=hits, outsideMatchingHitRectangle=0))
            glyphs = word['glyphs']
            if not 1 <= len(glyphs) <= 16 or glyphs[0]['left'] != 0 or glyphs[-1]['right'] != 1:
                raise ValueError('Incomplete word partitions')
            edge = 0
            for glyph in glyphs:
                values = [glyph[k] for k in ('cell','x','y','w','h','left','right')]
                if any(not isinstance(v, (int,float)) or not math.isfinite(v) for v in values):
                    raise ValueError('Nonfinite word geometry')
                if not (1 <= glyph['cell'] <= 120 and 0 <= glyph['x'] < 200 and glyph['y'] == y0 and 0 < glyph['w'] <= 32 and 0 < glyph['h'] <= 51 and glyph['left'] == edge < glyph['right'] <= 1):
                    raise ValueError('Invalid word compatibility partition')
                edge = glyph['right']
                rows.append('{'+','.join(format(v,'.12g') for v in values+[ax,ay,width])+'}')
    if len(rows) != 62:
        raise ValueError('Unexpected main-menu glyph coverage')
    layout = 'typedef struct {unsigned short cell;float x,y,w,h,left,right,atlasX,atlasY,width;} NfsFrontWordGlyph;\nstatic const NfsFrontWordGlyph nfs_front_word_glyphs[]={\n'+',\n'.join(rows)+'\n};\n'
    template = json.loads((ROOT/'scripts/hud_templates/FrontendFontPixels.json').read_text())['template']
    if template.count('@PIXEL_ARRAY@') != 1:
        raise ValueError('Invalid frontend decoder template')
    encoded = rle(pixels)
    header = template.replace('@PIXEL_ARRAY@', array('nfs_front_font_rle', encoded, decimal=True).rstrip())
    notice = '// SPDX-License-Identifier: MIT\n// Font artwork: pinned Bebas Neue/Oxanium, SIL OFL; see assets/hud/fonts.\n'
    outputs = {'FrontendFontPixels.inc': notice+header, 'FrontendWordLayout.inc': notice+layout}
    metadata = dict(originalGamePixelsIncluded=False, sourceHashes=hashes,
                    catalogSha256=hashlib.sha256((directory/'catalog.json').read_bytes()).hexdigest(),
                    rasterSha256=hashlib.sha256(pixels).hexdigest(), raster=[1024,1024],
                    rleBytes=len(encoded), glyphPartitions=len(rows), inkFit=ink,
                    headerHashes={n:hashlib.sha256(s.encode()).hexdigest() for n,s in outputs.items()})
    for name, content in outputs.items():
        path = ROOT/'src'/name
        if check:
            if path.read_text() != content:
                raise ValueError('Generated frontend artwork differs: '+name)
        else:
            path.write_bytes(content.encode())
    receipt = directory/'generated.json'
    if check:
        if json.loads(receipt.read_text()) != metadata:
            raise ValueError('Generated frontend receipt differs')
    else:
        receipt.write_bytes((json.dumps(metadata,indent=2)+'\n').encode())
    return metadata


if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    print(json.dumps(generate_frontend(parser.parse_args().check), indent=2))

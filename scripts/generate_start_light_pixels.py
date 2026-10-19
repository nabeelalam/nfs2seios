"""Rasterize the reviewed countdown sphere into two overlapping spare atlas banks."""
import hashlib
import json
from hud_asset_review import ROOT
from generate_remaster_pixels import array, rle, source

def generate_start_light(check=False):
    sources = {}
    pixels = source(ROOT/'assets/hud/start/sphere.svg', 222, 222, sources)
    if len(pixels) != 222*222*4:
        raise ValueError('Invalid countdown sphere raster')
    template = json.loads((ROOT/'scripts/hud_templates/StartLightPixels.json').read_text())['template']
    if template.count('@PIXEL_ARRAY@') != 1:
        raise ValueError('Invalid sphere decoder template')
    encoded = rle(pixels)
    text = '// SPDX-License-Identifier: MIT\n// Authored vector artwork; see assets/hud/HUDArtworkNotice.md.\n'
    text += array('nfs_start_light_raster_digest', hashlib.sha256(pixels).digest())
    text += template.replace('@PIXEL_ARRAY@', array('nfs_start_light_rle', encoded, decimal=True).rstrip())
    path = ROOT/'src/StartLightPixels.inc'
    meta = dict(originalGamePixelsIncluded=False, sourceHashes=sources,
                rasterSha256=hashlib.sha256(pixels).hexdigest(), raster=[222,222],
                banks=[[248,188,222,112],[470,188,222,112]], sharedRows=[110,111],
                rleBytes=len(encoded), headerSha256=hashlib.sha256(text.encode()).hexdigest())
    receipt = ROOT/'assets/hud/start-generated.json'
    if check:
        if path.read_text() != text or json.loads(receipt.read_text()) != meta:
            raise ValueError('Generated countdown sphere differs')
    else:
        path.write_bytes(text.encode())
        receipt.write_bytes((json.dumps(meta,indent=2)+'\n').encode())
    return meta

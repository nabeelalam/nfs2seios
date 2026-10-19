"""Generate reviewed clock punctuation and classic slash coverage in spare atlas space."""
import hashlib
import json
from hud_asset_review import ROOT
from hud_font_environment import project_fonts
from generate_remaster_pixels import array, rle, source

def generate_punctuation(check=False):
    result = {}
    with project_fonts():
        for name, header, symbol, width, height in (
            ('clock', 'ClockPunctuationPixels', 'nfs_clock_punctuation_rle', 132, 104),
            ('slash', 'SilverSlashPixels', 'nfs_silver_slash_rle', 36, 112),
        ):
            sources = {}
            pixels = source(ROOT/'assets/hud/punctuation'/(name+'.svg'), width, height, sources)
            if len(pixels) != width*height*4:
                raise ValueError('Unexpected punctuation raster size')
            template = json.loads((ROOT/'scripts/hud_templates'/(header+'.json')).read_text())['template']
            if template.count('@PIXEL_ARRAY@') != 1:
                raise ValueError('Invalid punctuation decoder template')
            text = '// Renderer code: MIT. Silver finish and font artwork: see assets/hud/HUDArtworkNotice.md.\n'
            text += template.replace('@PIXEL_ARRAY@', array(symbol, rle(pixels), decimal=True).rstrip())
            path = ROOT/'src'/(header+'.inc')
            if check:
                if path.read_text() != text:
                    raise ValueError('Generated punctuation differs: '+name)
            else:
                path.write_bytes(text.encode())
            result[name] = dict(sourceHashes=sources, rasterSha256=hashlib.sha256(pixels).hexdigest(),
                                rasterBytes=len(pixels), headerSha256=hashlib.sha256(text.encode()).hexdigest())
    receipt = ROOT/'assets/hud/punctuation-generated.json'
    metadata = dict(originalGamePixelsIncluded=False, generatedFrom='Reviewed vectors and pinned project font', families=result)
    if check:
        if json.loads(receipt.read_text()) != metadata:
            raise ValueError('Generated punctuation receipt differs')
    else:
        receipt.write_bytes((json.dumps(metadata, indent=2)+'\n').encode())
    return metadata

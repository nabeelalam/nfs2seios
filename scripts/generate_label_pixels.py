"""Reproduce the geometric HUD labels from pinned fonts and reviewed SVGs."""
import hashlib
import json
from hud_asset_review import ROOT, reviewed_remaster_svg
from hud_font_environment import project_fonts
from rasterize_hud_svg import rasterize
from generate_remaster_pixels import array, rle

ASSETS = ROOT / 'assets/hud'
HEADERS = {'small': 'SmallFontPixels.inc', 'record': 'RecordFontPixels.inc',
           'wide': 'WideLabelPixels.inc'}

def pixels(name, width, sources):
    path = ASSETS / name
    data = path.read_bytes()
    relative = path.relative_to(ROOT).as_posix()
    if not reviewed_remaster_svg(relative, data):
        raise ValueError('Unreviewed label vector: ' + name)
    sources[relative] = hashlib.sha256(data).hexdigest()
    value = rasterize(data, width * 8, 64)
    if len(value) != width * 8 * 64 * 4:
        raise ValueError('Unexpected label raster dimensions')
    return value

def generate_family(kind):
    sources = {}
    with project_fonts():
        if kind == 'wide':
            rgba = pixels('small-W.svg', 10, sources)
            encoded = rle(rgba)
            template = json.loads((ROOT/'scripts/hud_templates/WideLabelPixels.json').read_text())['template']
            text = template.replace('@PIXEL_ARRAY@', array('nfs_wide_label_w_rle', encoded, decimal=True).rstrip())
        else:
            if kind not in ('small', 'record'):
                raise ValueError('Unknown label family')
            catalog = json.loads((ASSETS/(kind+'-font.json')).read_text())
            expected = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' if kind == 'small' else '0123456789.:'
            if catalog['font'] != 'Oxanium' or catalog['weight'] != 500 or ''.join(e['character'] for e in catalog['entries']) != expected:
                raise ValueError('Unexpected label catalog')
            rgba = bytearray()
            for entry in catalog['entries']:
                char = entry['character']
                width = 10 if char == 'W' else 6 if char == '1' else 4 if char in 'I.:' else 8
                name = 'small-'+char+'.svg' if kind == 'small' else 'small-O.svg' if char == '0' else 'record-'+('dot' if char == '.' else 'colon' if char == ':' else char)+'.svg'
                if entry['file'] != name or entry['vectorSize'] != [width, 8] or entry['rasterSize'] != [width*8, 64]:
                    raise ValueError('Unexpected glyph cell layout')
                image = pixels(name, width, sources)
                if entry['sha256'] != sources['assets/hud/'+name]:
                    raise ValueError('Glyph catalog digest differs')
                if char == '0':
                    if entry.get('aliasOf') != 'O':
                        raise ValueError('Zero must alias the O tile')
                    continue
                if char == 'W':
                    rgba += bytes(64*64*4)  # Its natural wide tile lives in the spare atlas strip.
                else:
                    for y in range(64):
                        rgba += image[y*width*32:(y+1)*width*32]
                        rgba += bytes((64-width*8)*4)
            encoded = rle(rgba)
            prefix = 'nfs_'+kind+'_font'
            text = '// Renderer code: MIT. Font artwork: see assets/hud/fonts/Oxanium-OFL.txt.\n'
            text += array(prefix+'_raster_digest', hashlib.sha256(rgba).digest())
            text += array(prefix+'_pixels_rle', encoded)
    return text, dict(generatedFrom='Reviewed Oxanium SVG artwork with pinned font',
                      originalGamePixelsIncluded=False, sourceHashes=sources,
                      rasterSha256=hashlib.sha256(rgba).hexdigest(), rasterBytes=len(rgba),
                      rleBytes=len(encoded), headerSha256=hashlib.sha256(text.encode()).hexdigest())

def write_family(kind, check=False):
    text, meta = generate_family(kind)
    header = ROOT/'src'/HEADERS[kind]
    receipt = ASSETS/(kind+'-generated.json')
    if check:
        if header.read_text() != text or json.loads(receipt.read_text()) != meta:
            raise ValueError('Generated label artwork differs: '+kind)
    else:
        header.write_bytes(text.encode())
        receipt.write_bytes((json.dumps(meta, indent=2)+'\n').encode())
    return meta

def generate_labels(check=False):
    for kind in HEADERS:
        write_family(kind, check)

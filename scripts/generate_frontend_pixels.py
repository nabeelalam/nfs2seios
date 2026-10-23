"""Generate authored menu/Location/Setup words without original game data."""
import hashlib
import json
import math
from hud_asset_review import ROOT, FRONTEND_WORDS, LOCATION_WORDS, SETUP_WORDS, SETUP_SHARED
from hud_font_environment import project_fonts
from generate_remaster_pixels import rle, source


def text_identity(text):
    if not 1 <= len(text) < 96 or any(not (32 <= ord(c) <= 126 or ord(c)==224) for c in text):
        raise ValueError('Unsupported complete draw string')
    identity = 14695981039346656037
    for c in text.encode('latin1'):
        identity = ((identity ^ c) * 1099511628211) & 0xffffffffffffffff
    return identity


def location_words(directory, pixels, main_words, hashes, rows):
    """Crop new ink only and pack it around unchanged main-menu allocations."""
    path = directory/'location/catalog.json'
    words = json.loads(path.read_text())['words']
    if [w['file'] for w in words] != [n+'.svg' for n in LOCATION_WORDS]:
        raise ValueError('Unexpected reviewed Location word set')
    skyline, placements = [0]*1024, []
    for word in main_words:
        for x in range(word['x']*4, min(1024, (word['x']+word['width']+1)*4)):
            skyline[x] = max(skyline[x], min(1024, (word['y']+52)*4))
    for word in words:
        text, logical_width = LOCATION_WORDS[word['file'][:-4]]
        if word['text'] != text or word['width'] != logical_width:
            raise ValueError('Unexpected reviewed Location dimensions')
        width = logical_width*4
        rgba = source(directory/'location'/word['file'], width, 204, hashes)
        ink = [(x,y) for y in range(204) for x in range(width) if rgba[(y*width+x)*4+3]]
        if not ink:
            raise ValueError('Empty Location artwork')
        top, bottom = max(0,min(y for x,y in ink)-2), min(204,max(y for x,y in ink)+3)
        height = bottom-top
        if bottom > 104:
            raise ValueError('Location ink crosses the next row')
        ay, ax = min((max(skyline[x:x+width+4]),x) for x in range(0,1024-width-3,4))
        if ay+height+4 > 1024:
            raise ValueError('Location artwork exceeds the existing atlas')
        for x in range(ax,ax+width+4):
            skyline[x] = ay+height+4
        for y in range(height):
            at = ((ay+y)*1024+ax)*4
            pixels[at:at+width*4] = rgba[(top+y)*width*4:(top+y+1)*width*4]
        base = word['placements'][0]
        target = base['right']-logical_width
        if word['file'] in ('done.svg','cancel.svg') or word.get('kind') == 'lap':
            target = base['x']+(base['width']-logical_width)/2
        elif word.get('kind') == 'track':
            target = base['x']
        candidates = []
        for step in range(-32,33):
            left = round(target*4)/4+step/4
            if all(base['x'] < left+(x+.5)/4 < base['right'] for x,y in ink):
                candidates.append((abs(left-target),left))
        if not candidates:
            raise ValueError('Location ink exceeds original horizontal text extents')
        left = min(candidates)[1]
        for placement in word['placements']:
            edge = 0
            for glyph in placement['glyphs']:
                values = [glyph[k] for k in ('cell','x','y','w','h','left','right')]
                if any(not isinstance(v,(int,float)) or not math.isfinite(v) for v in values):
                    raise ValueError('Nonfinite Location geometry')
                if not (1 <= glyph['cell'] <= 120 and 0 <= glyph['x'] < glyph['x']+glyph['w'] <= 640
                        and 0 <= glyph['y'] < glyph['y']+glyph['h'] <= 480
                        and glyph['y'] == placement['y'] and 0 < glyph['w'] <= 32 and glyph['h'] == 51
                        and glyph['left'] == edge < glyph['right'] <= 1):
                    raise ValueError('Invalid Location compatibility partition')
                edge = glyph['right']
                draw_text = glyph['drawText']
                if draw_text not in ((text[:-3],'...') if text.endswith('...') else (text,)):
                    raise ValueError('Unrelated complete draw string')
                values += [ax/4,ay/8,logical_width,left+placement['x']-base['x'],top/4,height/4]
                rows.append('{'+','.join(format(v,'.12g') for v in values)+',UINT64_C(0x'+format(text_identity(draw_text),'016x')+')}')
            if edge != 1 or placement['glyphs'][0]['left'] != 0:
                raise ValueError('Incomplete Location word partition')
        placements.append(dict(file=word['file'],atlas=[ax,ay,width,height],sourceCrop=[0,top,width,height],
                               outputX=left,inkFitsOriginalHorizontalTextBounds=True))
    return placements, hashlib.sha256(path.read_bytes()).hexdigest()


def setup_words(directory, pixels, location_placements, hashes, rows):
    """Keep accepted top-half pixels and reuse three already-reviewed words."""
    path = directory/'setup/catalog.json'
    words = json.loads(path.read_text())['words']
    if [w['file'] for w in words] != [n+'.svg' for n in SETUP_WORDS]:
        raise ValueError('Unexpected reviewed Setup word set')
    shared = {p['file']:p for p in location_placements}
    skyline, placements = [1024]*1024, []
    for word in words:
        name = word['file'][:-4]
        text, logical_width = SETUP_WORDS[name]
        if word['text'] != text or word['width'] != logical_width or word['shared'] != (name in SETUP_SHARED):
            raise ValueError('Unexpected Setup word dimensions or alias')
        directory_name = 'location' if word['shared'] else 'setup'
        width = logical_width*4
        rgba = source(directory/directory_name/word['file'], width, 204, hashes)
        ink = [(x,y) for y in range(204) for x in range(width) if rgba[(y*width+x)*4+3]]
        if not ink:
            raise ValueError('Empty Setup artwork')
        top, bottom = max(0,min(y for x,y in ink)-2), min(204,max(y for x,y in ink)+3)
        height = bottom-top
        if bottom > 104:
            raise ValueError('Setup ink crosses the next row')
        if word['shared']:
            entry = shared[word['file']]
            ax, ay, original_width, original_height = entry['atlas']
            if original_width != width or original_height != height or entry['sourceCrop'] != [0,top,width,height]:
                raise ValueError('Shared Setup artwork differs from its reviewed Location crop')
        else:
            ay, ax = min((max(skyline[x:x+width+4]),x) for x in range(0,1024-width-3,4))
            if ay+height+4 > 2048:
                raise ValueError('Setup artwork exceeds reviewed taller atlas')
            for x in range(ax,ax+width+4):
                skyline[x] = ay+height+4
            for y in range(height):
                at = ((ay+y)*1024+ax)*4
                pixels[at:at+width*4] = rgba[(top+y)*width*4:(top+y+1)*width*4]
        if word['kind'] not in ('label','colon','value','button'):
            raise ValueError('Unexpected Setup alignment role')
        for placement in word['placements']:
            target = placement['right']-logical_width
            if word['kind'] == 'value':
                target = placement['x']
            elif word['kind'] == 'button':
                target = placement['x']+(placement['width']-logical_width)/2
            candidates = []
            for step in range(-32,33):
                left = round(target*4)/4+step/4
                if all(placement['x'] < left+(x+.5)/4 < placement['right'] for x,y in ink):
                    candidates.append((abs(left-target),left))
            if not candidates:
                raise ValueError('Setup ink exceeds original horizontal text extents')
            left, edge = min(candidates)[1], 0
            for glyph in placement['glyphs']:
                values = [glyph[k] for k in ('cell','x','y','w','h','left','right')]
                if any(not isinstance(v,(int,float)) or not math.isfinite(v) for v in values):
                    raise ValueError('Nonfinite Setup geometry')
                if not (1 <= glyph['cell'] <= 120 and 0 <= glyph['x'] < glyph['x']+glyph['w'] <= 640
                        and 0 <= glyph['y'] < glyph['y']+glyph['h'] <= 480
                        and glyph['y'] == placement['y'] and 0 < glyph['w'] <= 32 and glyph['h'] == 51
                        and glyph['left'] == edge < glyph['right'] <= 1 and glyph['drawText'] == text):
                    raise ValueError('Invalid Setup word/context partition')
                edge = glyph['right']
                values += [ax/4,ay/8,logical_width,left,top/4,height/4]
                rows.append('{'+','.join(format(v,'.12g') for v in values)+',UINT64_C(0x'+format(text_identity(text),'016x')+')}')
            if edge != 1 or placement['glyphs'][0]['left'] != 0:
                raise ValueError('Incomplete Setup word partition')
        placements.append(dict(file=word['file'],atlas=[ax,ay,width,height],sourceCrop=[0,top,width,height],
                               reusedImage=word['shared'],inkFitsOriginalHorizontalTextBounds=True))
    return placements, hashlib.sha256(path.read_bytes()).hexdigest()


def chunked_pixels(encoded):
    # Run-aligned decimal literals; keep every source below the unchanged 1 MiB gate.
    if not encoded or len(encoded)%6 or len(encoded)>64*65520:
        raise ValueError('Unbounded or unaligned authored frontend pixel runs')
    outputs = {}
    names = []
    for i, start in enumerate(range(0,len(encoded),65520)):
        name = 'FrontendFontPixelsPart'+str(i)+'.inc'
        data = encoded[start:start+65520]
        outputs[name] = '// SPDX-License-Identifier: MIT\n'+',\n'.join(','.join(str(v) for v in data[j:j+24]) for j in range(0,len(data),24))+',\n'
        names.append(name)
    aggregate = 'static const unsigned char nfs_front_font_rle[]={\n'+''.join('#include "'+n+'"\n' for n in names)+'};'
    return aggregate, outputs


def player_ink_fit(directory, pixels, rows, placements):
    """Check adopted ink against every original Player horizontal text bound."""
    words = json.loads((directory/'player/catalog.json').read_text())['words']
    if len(words) != 34 or len(placements) != 34 or len(pixels) != 8388608:
        raise ValueError('Incomplete Player ink-fit inputs')
    parsed = [list(map(float, row.strip('{}').split(',')[:13])) for row in rows]
    index, result = 726, []
    for word, packed in zip(words, placements):
        if word['file'] != packed['file']:
            raise ValueError('Player ink-fit image order changed')
        x, y, width, height = packed['atlas']
        ink = [(xx, yy) for yy in range(height) for xx in range(width)
               if pixels[((y+yy)*1024+x+xx)*4+3]]
        if not ink:
            raise ValueError('Empty adopted Player text')
        low, high = min(xx for xx, _ in ink), max(xx for xx, _ in ink)
        for placement in word['placements']:
            count = len(placement['glyphs'])
            group = parsed[index:index+count]
            index += count
            if len(group) != count or not group or any(
                    row[7:10] != [x/4, y/8, width/4]
                    or row[10:13] != group[0][10:13] for row in group):
                raise ValueError('Player ink-fit layout differs from packed word')
            left = group[0][10]
            if not (placement['x'] < left+(low+.5)/4
                    <= left+(high+.5)/4 < placement['right']):
                raise ValueError('Adopted Player ink leaves original interaction bounds: '+word['file'])
        result.append(dict(file=word['file'], placements=len(word['placements']),
                           nontransparentPixels=len(ink), outsideMatchingHorizontalBounds=0))
    if index != 1267 or sum(row['placements'] for row in result) != 67:
        raise ValueError('Incomplete Player ink-fit placement coverage')
    return result


def generate_frontend(check=False):
    directory = ROOT/'assets/hud/frontend'
    catalog = json.loads((directory/'catalog.json').read_text())
    words = catalog['words']
    if len(words) != 8 or [w['file'] for w in words] != [n+'.svg' for n in FRONTEND_WORDS]:
        raise ValueError('Unexpected reviewed main-menu word set')
    pixels = bytearray(1024*2048*4)
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
                rows.append('{'+','.join(format(v,'.12g') for v in values+[ax,ay/2,width,168-width,0,51])+',UINT64_C(0x0000000000000000)}')
    if len(rows) != 62:
        raise ValueError('Unexpected main-menu glyph coverage')
    with project_fonts():
        location_placements, location_catalog_hash = location_words(directory,pixels,words,hashes,rows)
    if len(rows) != 380:
        raise ValueError('Unexpected reviewed Location coverage')
    top_hash = hashlib.sha256(pixels[:4194304]).hexdigest()
    with project_fonts():
        setup_placements, setup_catalog_hash = setup_words(directory,pixels,location_placements,hashes,rows)
    if len(rows) != 726:
        raise ValueError('Unexpected reviewed Setup coverage')
    from frontend_player_atlas import compact_player_words
    pixels, rows, compact = compact_player_words(directory,pixels,words,location_placements,setup_placements,hashes,rows,text_identity)
    from frontend_opponents_atlas import opponents_words
    pixels, rows, opponents = opponents_words(directory,pixels,rows,hashes,text_identity)
    from frontend_options_atlas import options_words
    pixels, rows, options = options_words(directory,pixels,rows,opponents,hashes,text_identity)
    from frontend_main_word_art import apply_main_word_art
    main_word_art = apply_main_word_art(pixels, options['mainPlacements'], source_path=directory/'main-word-art.json')
    from frontend_player_word_art import apply_player_word_art
    player_word_art = apply_player_word_art(
        pixels, {family: options[family+'Placements'] for family in
                 ('main','location','setup','player','opponents','options')},
        source_dir=directory)
    player_ink = player_ink_fit(directory, pixels, rows, options['playerPlacements'])
    # Fit evidence must describe the adopted complete words, not the historical
    # SVG images used to reproduce the unchanged base packing recipe.
    options['packingPhase'] = 'before-main-and-player-text-art-overrides'
    ink = []
    for word, placement in zip(words, options['mainPlacements']):
        if word['file'] != placement['file']:
            raise ValueError('Main word placement identity changed')
        ax, ay, width, height = placement['atlas']
        left, top, right, bottom = word['hitBounds']
        hits = 0
        for y in range(height):
            for x in range(width):
                if pixels[((ay+y)*1024+ax+x)*4+3]:
                    xx, yy = 168-word['width']+(x+.5)/4, word['originalY']+(y+.5)/4
                    if not (left < xx < right and top < yy < bottom):
                        raise ValueError('Adopted Main word ink leaves original interaction bounds')
                    hits += 1
        if not hits:
            raise ValueError('Empty adopted Main word')
        ink.append(dict(text=word['text'], nontransparentPixels=hits, outsideMatchingHitRectangle=0))

    for family in ('main','location','setup','player','opponents'):
        opponents[family+'Placements']=options[family+'Placements']
    location_placements,setup_placements=opponents['locationPlacements'],opponents['setupPlacements']
    layout = 'typedef struct {unsigned short cell;float x,y,w,h,left,right,atlasX,atlasY,width,outputX,inkTop,inkHeight;uint64_t word;} NfsFrontWordGlyph;\nstatic const NfsFrontWordGlyph nfs_front_word_glyphs[]={\n'+',\n'.join(rows)+'\n};\n'
    base_layout_hash = hashlib.sha256(('// SPDX-License-Identifier: MIT\n// Font artwork: OFL Bebas Neue/Oxanium plus fixed Main/Player text graphics; see assets/hud/frontend/MAIN_WORD_ART.md and PLAYER_WORD_ART.md.\n'+layout).encode()).hexdigest()
    from frontend_right_labels import right_label_rows, extend_layout
    right_path = directory/'right-labels/catalog.json'
    right_rows = right_label_rows(json.loads(right_path.read_text()), setup_placements, text_identity)
    layout = extend_layout(layout, right_rows)
    template = json.loads((ROOT/'scripts/hud_templates/FrontendFontPixels.json').read_text())['template']
    if template.count('@PIXEL_ARRAY@') != 1:
        raise ValueError('Invalid frontend decoder template')
    encoded = rle(pixels)
    aggregate, parts = chunked_pixels(encoded)
    dimensions = 'enum {NFS_FRONT_FONT_WIDTH=1024,NFS_FRONT_FONT_HEIGHT=2048,NFS_FRONT_FONT_PIXELS=2097152,NFS_FRONT_FONT_RGBA_BYTES=8388608};\nstatic const float nfs_front_font_v_scale=.5f;\n'
    header = dimensions+template.replace('@PIXEL_ARRAY@', aggregate)
    notice = '// SPDX-License-Identifier: MIT\n// Font artwork: OFL Bebas Neue/Oxanium plus fixed Main/Player text graphics; see assets/hud/frontend/MAIN_WORD_ART.md and PLAYER_WORD_ART.md.\n'
    outputs = {'FrontendFontPixels.inc': notice+header, 'FrontendWordLayout.inc': notice+layout, **parts}
    metadata = dict(originalGamePixelsIncluded=False, sourceHashes=hashes,
                    mainWordArt=main_word_art, playerWordArt=player_word_art, playerInkFit=player_ink,
                    catalogSha256=hashlib.sha256((directory/'catalog.json').read_bytes()).hexdigest(),
                    rasterSha256=hashlib.sha256(pixels).hexdigest(), raster=[1024,2048],
                    rleBytes=len(encoded), glyphPartitions=len(rows)+len(right_rows), inkFit=ink,
                    baseWordLayoutSha256=base_layout_hash, rightLabelCatalogSha256=hashlib.sha256(right_path.read_bytes()).hexdigest(),
                    locationCatalogSha256=location_catalog_hash, locationPlacements=location_placements,
                    setupCatalogSha256=setup_catalog_hash, setupPlacements=setup_placements,
                    previousAcceptedTopRasterSha256=top_hash,
                    playerCatalogSha256=compact['playerCatalogSha256'],playerPlacements=opponents['playerPlacements'],
                    opponentsCatalogSha256=opponents['catalogSha256'],opponentsPlacements=opponents['opponentsPlacements'],mainPlacements=opponents['mainPlacements'],
                    optionsAtlas=options,
                    packingRecipeSha256=opponents['recipeSha256'],compactAtlas=opponents['packing'],additionalCachedTextureBytesForOpponents=0,additionalCachedTextureBytesForPlayer=0, rasterBytes=len(pixels), additionalCachedTextureBytes=4194304,
                    headerHashes={n:hashlib.sha256(s.encode()).hexdigest() for n,s in outputs.items()})
    for name, content in outputs.items():
        if len(content.encode()) > 1048576:
            raise ValueError('Generated frontend source exceeds publication size gate')
        path = ROOT/'src'/name
        if check:
            if path.read_text() != content:
                raise ValueError('Generated frontend artwork differs: '+name)
        elif not path.exists() or path.read_bytes() != content.encode():
            path.write_bytes(content.encode())
    receipt = directory/'generated.json'
    if check:
        if json.loads(receipt.read_text()) != metadata:
            raise ValueError('Generated frontend receipt differs')
    elif not receipt.exists() or json.loads(receipt.read_text()) != metadata:
        receipt.write_bytes((json.dumps(metadata,indent=2)+'\n').encode())
    return metadata



def generate_frontend_layout(check=False):
    """Update only word geometry using a verified existing atlas and packing receipt."""
    from frontend_right_labels import right_label_rows, extend_layout, base_layout
    directory = ROOT/'assets/hud/frontend'
    receipt = directory/'generated.json'
    metadata = json.loads(receipt.read_text())
    from frontend_main_word_art import verify_main_word_art
    verify_main_word_art(metadata['mainWordArt'], source_path=directory/'main-word-art.json')
    if metadata['mainWordArt']['placements'] != [{k:p[k] for k in ('file','atlas','sourceCrop')} for p in metadata['mainPlacements']]:
        raise ValueError('Whole-word artwork placements differ from generated atlas')
    from frontend_player_word_art import verify_player_word_art
    verify_player_word_art(
        metadata['playerWordArt'],
        dict([(family, metadata[family+'Placements']) for family in
              ('main','location','setup','player','opponents')] +
             [('options', metadata['optionsAtlas']['optionsPlacements'])]),
        source_dir=directory)
    # A stale or edited image, catalog, packing recipe or generated header requires
    # full generation. Never silently reuse metadata from a different atlas.
    for relative, expected in metadata['sourceHashes'].items():
        path = ROOT/relative
        if not relative.startswith('assets/hud/frontend/') or '..' in path.parts or path.suffix != '.svg':
            raise ValueError('Unexpected authored source path')
        if hashlib.sha256(path.read_bytes()).hexdigest() != expected:
            raise ValueError('Stale authored artwork; run full generation')
    catalogs = {'catalogSha256':'catalog.json', 'locationCatalogSha256':'location/catalog.json',
                'setupCatalogSha256':'setup/catalog.json', 'playerCatalogSha256':'player/catalog.json',
                'opponentsCatalogSha256':'opponents/catalog.json', 'packingRecipeSha256':'opponents/packing.json'}
    option_meta=metadata['optionsAtlas']
    for relative,key in [('options/catalog.json','catalogSha256'),('options/packing.json','recipeSha256')]:
        if hashlib.sha256((directory/relative).read_bytes()).hexdigest()!=option_meta[key]:
            raise ValueError('Stale Options catalog or packing; run full generation')
    for key, relative in catalogs.items():
        if hashlib.sha256((directory/relative).read_bytes()).hexdigest() != metadata[key]:
            raise ValueError('Stale catalog or packing; run full generation')
    part_count=len(metadata['headerHashes'])-2
    if not 1<=part_count<=64:raise ValueError('Unbounded generated frontend header inventory')
    expected_names = {'FrontendFontPixels.inc','FrontendWordLayout.inc'} | {
        'FrontendFontPixelsPart'+str(i)+'.inc' for i in range(part_count)}
    if set(metadata['headerHashes']) != expected_names:
        raise ValueError('Unexpected generated header inventory')
    for name, expected in metadata['headerHashes'].items():
        if hashlib.sha256((ROOT/'src'/name).read_bytes()).hexdigest() != expected:
            raise ValueError('Modified generated header; run full generation')
    table_path = ROOT/'src/FrontendWordLayout.inc'
    base = base_layout(table_path.read_text())
    if hashlib.sha256(base.encode()).hexdigest() != metadata['baseWordLayoutSha256']:
        raise ValueError('Stale base word layout')
    # Cross-check metadata regions against the already-authenticated base rows.
    import re
    for text, filename in [('one player','one-player.svg'),('single race','single-race.svg')]:
        placements = [p for p in metadata['setupPlacements'] if p['file'] == filename]
        if len(placements) != 1:
            raise ValueError('Ambiguous packed label')
        p = placements[0]; ax, ay, aw, ah = p['atlas']; top = p['sourceCrop'][1]
        matches = re.findall(r'\{([^{}]+),UINT64_C\(0x'+format(text_identity(text),'016x')+r'\)\}', base)
        if not matches or any([float(v) for v in row.split(',')][7:10] != [ax/4,ay/8,aw/4]
                              or [float(v) for v in row.split(',')][11:13] != [top/4,ah/4] for row in matches):
            raise ValueError('Packed label metadata disagrees with base atlas coordinates')
    catalog_path = directory/'right-labels/catalog.json'
    rows = right_label_rows(json.loads(catalog_path.read_text()), metadata['setupPlacements'], text_identity)
    table = extend_layout(base, rows)
    metadata['rightLabelCatalogSha256'] = hashlib.sha256(catalog_path.read_bytes()).hexdigest()
    metadata['headerHashes']['FrontendWordLayout.inc'] = hashlib.sha256(table.encode()).hexdigest()
    if check:
        if table_path.read_text() != table or json.loads(receipt.read_text()) != metadata:
            raise ValueError('Generated right-label layout differs')
    else:
        if table_path.read_bytes() != table.encode():
            table_path.write_bytes(table.encode())
        if json.loads(receipt.read_text()) != metadata:
            receipt.write_bytes((json.dumps(metadata,indent=2)+'\n').encode())
    return metadata

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--layout-only', action='store_true', help='Reuse verified atlas pixels; update caption geometry only')
    args = parser.parse_args()
    print(json.dumps((generate_frontend_layout if args.layout_only else generate_frontend)(args.check), indent=2))

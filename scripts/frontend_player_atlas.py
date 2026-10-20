# SPDX-License-Identifier: MIT
"""Pack reviewed player typography around existing words without game pixels."""
import hashlib
import json
import math
import re
from hud_asset_review import PLAYER_WORDS, PLAYER_SHARED
from hud_font_environment import project_fonts
from generate_remaster_pixels import source


def _subtract(free, rectangle):
    px, py, width, height = rectangle
    remaining = []
    for x, y, w, h in free:
        if px >= x+w or px+width <= x or py >= y+h or py+height <= y:
            remaining.append((x,y,w,h))
            continue
        if px > x: remaining.append((x,y,px-x,h))
        if px+width < x+w: remaining.append((px+width,y,x+w-px-width,h))
        if py > y: remaining.append((x,y,w,py-y))
        if py+height < y+h: remaining.append((x,py+height,w,y+h-py-height))
    unique = list(dict.fromkeys(remaining))
    return [v for i,v in enumerate(unique) if not any(i != j and v[0] >= z[0] and v[1] >= z[1]
            and v[0]+v[2] <= z[0]+z[2] and v[1]+v[3] <= z[1]+z[3] for j,z in enumerate(unique))]


def _pack(images):
    orders = (
        ('area',lambda v:(-(v[1]['width']+4)*(v[1]['height']+4),v[0])),
        ('height',lambda v:(-v[1]['height'],-v[1]['width'],v[0])),
        ('width',lambda v:(-v[1]['width'],-v[1]['height'],v[0])),
        ('perimeter',lambda v:(-v[1]['width']-v[1]['height'],v[0])),
    )
    attempts = []
    for name, order in orders:
        for heuristic in ('short','area','bottom'):
            free, placed = [(0,0,1024,2048)], {}
            for key,item in sorted(images.items(),key=order):
                w,h = item['width']+4,item['height']+4
                candidates = []
                for x,y,fw,fh in free:
                    if w > fw or h > fh: continue
                    a,z = fw-w,fh-h
                    rank = (min(a,z),max(a,z),y,x) if heuristic == 'short' else (fw*fh-w*h,min(a,z),y,x) if heuristic == 'area' else (y+h,x,min(a,z),max(a,z))
                    candidates.append((rank,x,y))
                if not candidates: break
                _,x,y = min(candidates)
                placed[key] = (x,y,w,h)
                free = _subtract(free,(x,y,w,h))
            fit = len(placed) == len(images)
            attempts.append(dict(order=name,heuristic=heuristic,fit=fit))
            if fit:
                rectangles = list(placed.values())
                for i,(x,y,w,h) in enumerate(rectangles):
                    if not (0 <= x < x+w <= 1024 and 0 <= y < y+h <= 2048 and all(x+w <= a or a+c <= x or y+h <= z or z+d <= y for a,z,c,d in rectangles[:i])):
                        raise ValueError('Overlapping authored atlas allocations')
                return placed,attempts
    raise ValueError('Reviewed words do not fit the existing 8 MiB atlas')


def _player_words(directory):
    path = directory/'player/catalog.json'
    words = json.loads(path.read_text())['words']
    if [w['file'] for w in words] != [name+'.svg' for name in PLAYER_WORDS]:
        raise ValueError('Unexpected player word inventory')
    for word in words:
        name = word['file'][:-4]
        if (word['text'],word['width']) != PLAYER_WORDS[name] or word['sharedFamily'] != PLAYER_SHARED.get(name):
            raise ValueError('Unexpected player dimensions or shared source')
        if word['kind'] not in ('label','colon','value','button','heading'):
            raise ValueError('Unexpected player alignment role')
        caps = word['kind'] in ('button','heading')
        expected = ('Bebas Neue',25,.75,21) if caps else ('Oxanium',24,.54,20)
        if tuple(word[k] for k in ('font','size','condensation','baseline')) != expected:
            raise ValueError('Unexpected reviewed player font proportions')
        if not 1 <= len(word['placements']) <= 32:
            raise ValueError('Unbounded player placements')
    return words,hashlib.sha256(path.read_bytes()).hexdigest()


def compact_player_words(directory, old_pixels, main, location, setup, hashes, old_rows, identity):
    player, catalog_hash = _player_words(directory)
    logical,images = [],{}
    families = [('main',main,None),('location',json.loads((directory/'location/catalog.json').read_text())['words'],location),
                ('setup',json.loads((directory/'setup/catalog.json').read_text())['words'],setup)]
    for family,words,placements in families:
        for i,word in enumerate(words):
            folder = 'location' if word.get('shared') else family
            path = directory/word['file'] if family == 'main' else directory/folder/word['file']
            key = hashlib.sha256(path.read_bytes()).hexdigest()
            if family == 'main':
                x,y,width,height,top = word['x']*4,word['y']*4,word['width']*4,204,0
            else:
                x,y,width,height = placements[i]['atlas']
                top = placements[i]['sourceCrop'][1]
            rgba = b''.join(old_pixels[((y+line)*1024+x)*4:((y+line)*1024+x+width)*4] for line in range(height))
            if key not in images: images[key] = dict(width=width,height=height,top=top,rgba=rgba)
            elif images[key] != dict(width=width,height=height,top=top,rgba=rgba):
                raise ValueError('Shared authored source rectangles differ')
            logical.append((family,word,key))
    with project_fonts():
        for word in player:
            folder = word['sharedFamily'] or 'player'
            path = directory/folder/word['file']
            data = path.read_bytes();key = hashlib.sha256(data).hexdigest()
            width = word['width']*4
            if key not in images:
                rgba = source(path,width,204,hashes)
                ys = [y for y in range(204) for x in range(width) if rgba[(y*width+x)*4+3]]
                if not ys: raise ValueError('Empty player word')
                top,bottom = max(0,min(ys)-2),min(204,max(ys)+3)
                if bottom > 104: raise ValueError('Player ink crosses next row')
                images[key] = dict(width=width,height=bottom-top,top=top,rgba=rgba[top*width*4:bottom*width*4])
            elif images[key]['width'] != width:
                raise ValueError('Shared player artwork width differs')
            logical.append(('player',word,key))
    if len(logical) != 82 or len(images) != 74: raise ValueError('Unexpected reviewed atlas image coverage')
    placed,attempts = _pack(images)
    pixels = bytearray(8388608)
    for key,item in images.items():
        x,y,_,_ = placed[key];w = item['width']
        for line in range(item['height']):
            at = ((y+line)*1024+x)*4;pixels[at:at+w*4] = item['rgba'][line*w*4:(line+1)*w*4]
    parsed = []
    for row in old_rows:
        fields = row.strip('{}').split(',')
        parsed.append((list(map(float,fields[:-1])),int(re.fullmatch(r'UINT64_C\(0x([0-9a-f]+)\)',fields[-1])[1],16)))
    rows,old_index,output = [],0,{'location':[],'setup':[],'player':[]}
    for family,word,key in logical:
        ax,ay,_,_ = placed[key];item = images[key];width,top,height = word['width'],item['top'],item['height']
        placements = [dict(glyphs=word['glyphs'])] if family == 'main' else word['placements']
        ink_x = [x for y in range(height) for x in range(item['width']) if item['rgba'][(y*item['width']+x)*4+3]]
        for placement in placements:
            if family == 'player':
                for name in ('x','y','right','width'):
                    if not isinstance(placement[name],(int,float)) or not math.isfinite(placement[name]):raise ValueError('Nonfinite player geometry')
                if not (0 <= placement['x'] < placement['right'] <= 640 and 0 <= placement['y'] <= 429 and placement['width'] == placement['right']-placement['x']):raise ValueError('Invalid player word extents')
                target = placement['right']-width
                if word['kind'] == 'value': target = placement['x']
                elif word['kind'] in ('button','heading'): target = placement['x']+(placement['width']-width)/2
                candidates = []
                for step in range(-32,33):
                    left = round(target*4)/4+step/4
                    if placement['x'] < left+(min(ink_x)+.5)/4 and left+(max(ink_x)+.5)/4 < placement['right']:candidates.append((abs(left-target),left))
                if not candidates: raise ValueError('Player ink exceeds original horizontal extents')
                left = min(candidates)[1];edge = 0
                if not 1 <= len(placement['glyphs']) <= 64:raise ValueError('Unbounded player glyph count')
            for glyph in placement['glyphs']:
                values = [glyph[k] for k in ('cell','x','y','w','h','left','right')]
                if family != 'player':
                    prior,token = parsed[old_index];old_index += 1
                    if any(abs(a-z)>1e-9 for a,z in zip(prior[:7],values)) or prior[9] != width or prior[11:] != [top/4,height/4]:raise ValueError('Accepted frontend geometry changed')
                    values = prior[:];values[7:9] = [ax/4,ay/8]
                else:
                    if any(not isinstance(v,(int,float)) or not math.isfinite(v) for v in values):raise ValueError('Nonfinite player glyph')
                    page = glyph['sourcePage']
                    if page not in (0,1) or not (1 <= glyph['cell'] <= 121) or (glyph['cell'] == 121) != (page == 1):raise ValueError('Invalid player source page')
                    if page and (word['text'] != 'Italdesign Cal\u00e0' or glyph.get('originalCode') != 224):raise ValueError('Unsupported legacy player glyph')
                    if not (0 <= glyph['x'] < glyph['x']+glyph['w'] <= 640 and glyph['y'] == placement['y'] and 0 < glyph['w'] <= 32 and glyph['h'] == 51 and glyph['left'] == edge < glyph['right'] <= 1):raise ValueError('Invalid player compatibility partition')
                    edge = glyph['right'];draw = glyph['drawText']
                    if draw not in ((word['text'][:-3],'...') if word['text'].endswith('...') else (word['text'],)):raise ValueError('Unrelated player draw text')
                    token = identity(draw)
                    values += [ax/4,ay/8,width,left,top/4,height/4]
                rows.append('{'+','.join(format(v,'.12g') for v in values)+',UINT64_C(0x'+format(token,'016x')+')}')
            if family == 'player' and (edge != 1 or placement['glyphs'][0]['left'] != 0):raise ValueError('Incomplete player word partition')
        if family != 'main':output[family].append(dict(file=word['file'],atlas=[ax,ay,item['width'],height],sourceCrop=[0,top,item['width'],height],inkFitsOriginalHorizontalTextBounds=True))
    if old_index != 726 or len(rows) != 1267:raise ValueError('Unexpected guarded glyph coverage')
    metadata = dict(locationPlacements=output['location'],setupPlacements=output['setup'],playerPlacements=output['player'],playerCatalogSha256=catalog_hash,
                    packing=dict(logicalWords=82,uniqueImages=74,dimensions=[1024,2048],mainCropChanged=False,oldGlyphGeometryAndInkMetricsPreserved=726,attempts=attempts,paddedArea=sum((v['width']+4)*(v['height']+4) for v in images.values()),allocationsNonoverlapping=True))
    return pixels,rows,metadata

"""Main-menu summary captions: reuse atlas ink, scale only output geometry."""
import math

LABELS = {'one player': ('one-player.svg', 509, 49),
          'single race': ('single-race.svg', 570, 50)}
FIELDS = 'float outputScaleX,outputScaleY;'
BEGIN = ',\n/* NFS_RIGHT_LABELS_BEGIN */\n'
END = '\n/* NFS_RIGHT_LABELS_END */'


def right_label_rows(catalog, placements, identity):
    if catalog.get('schema') != 1 or len(catalog.get('words', [])) != 2:
        raise ValueError('Unexpected right-label catalog')
    seen, rows = set(), []
    for word in catalog['words']:
        text = word.get('text')
        if text not in LABELS or text in seen:
            raise ValueError('Unknown or duplicate right label')
        seen.add(text)
        filename, x, width = LABELS[text]
        if (word.get('reuse'), word.get('x'), word.get('y'), word.get('width'), word.get('height')) != (filename, x, 73, width, 37):
            raise ValueError('Unexpected right-label geometry')
        matches = [p for p in placements if p.get('file') == filename]
        if len(matches) != 1:
            raise ValueError('Missing or ambiguous packed word')
        packed = matches[0]
        atlas, crop = packed['atlas'], packed['sourceCrop']
        if len(atlas) != 4 or len(crop) != 4 or any(type(v) is not int for v in atlas+crop):
            raise ValueError('Invalid packed word region')
        ax, ay, aw, ah = atlas
        if not (0 <= ax < ax+aw <= 1024 and 0 <= ay < ay+ah <= 2048
                and crop[0] == 0 and crop[2:] == [aw, ah]
                and 0 <= crop[1] < crop[1]+ah <= 204 and aw > 0 and ah > 0):
            raise ValueError('Packed word out of bounds')
        glyphs, edge = word.get('glyphs', []), 0
        if len(glyphs) != len(text):
            raise ValueError('Incomplete right-label partition')
        for glyph in glyphs:
            if set(glyph) != {'cell', 'x', 'width'}:
                raise ValueError('Unexpected glyph fields')
            cell, gx, gw = glyph['cell'], glyph['x'], glyph['width']
            if (type(cell) is not int or not 1 <= cell <= 120
                    or any(type(v) not in (int, float) or not math.isfinite(v) for v in (gx, gw))
                    or gx != edge or not 0 < gw <= 32 or gx+gw > width):
                raise ValueError('Invalid right-label glyph partition')
            edge += gw
        if edge != width:
            raise ValueError('Incomplete right-label width')
        for shadow in (0, 1):
            for glyph in glyphs:
                gx, gw = glyph['x'], glyph['width']
                values = [glyph['cell'], x+gx+shadow, 73+shadow, gw, 37,
                          gx/width, (gx+gw)/width, ax/4, ay/8, aw/4,
                          x+shadow, crop[1]/4, ah/4]
                rows.append('{'+','.join(format(v, '.12g') for v in values)
                            +',UINT64_C(0x'+format(identity(text), '016x')+'),'
                            +format(width/(aw/4), '.12g')+','+format(37/51, '.12g')+'}')
    if len(rows) != 42:
        raise ValueError('Unexpected right-label coverage')
    return rows


def extend_layout(base, rows):
    if BEGIN in base or FIELDS in base or base.count('uint64_t word;') != 1 or not base.endswith('\n};\n'):
        raise ValueError('Invalid base word layout')
    return (base[:-4].replace('uint64_t word;', 'uint64_t word;'+FIELDS)
            +BEGIN+',\n'.join(rows)+END+'\n};\n')


def base_layout(layout):
    if layout.count(BEGIN) != 1 or layout.count(END) != 1 or layout.count(FIELDS) != 1:
        raise ValueError('Missing or ambiguous right-label extension')
    start, end = layout.index(BEGIN), layout.index(END)
    if end <= start or layout[end+len(END):] != '\n};\n':
        raise ValueError('Invalid right-label extension boundary')
    return (layout[:start]+layout[end+len(END):]).replace(FIELDS, '')

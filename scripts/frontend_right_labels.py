"""Main-menu summary captions: reuse atlas ink, scale only output geometry."""
import math

LABELS = {'one player': ('setup', 'one-player.svg', 509.0, 73.0, 49.0), 'single race': ('setup', 'single-race.svg', 570.0, 73.0, 50.0), 'split screen': ('setup', 'split-screen.svg', 504.0, 73.0, 51.0), 'modem': ('setup', 'modem.svg', 526.0, 73.0, 32.0), 'serial link': ('setup', 'serial-link.svg', 513.0, 73.0, 44.0), 'network': ('setup', 'network.svg', 520.0, 73.0, 37.0), 'tournament': ('setup', 'tournament.svg', 570.0, 73.0, 52.0), 'knockout': ('setup', 'knockout.svg', 570.0, 73.0, 39.0), 'North Country': ('location', 'north-country.svg', 541.0, 181.0, 61.0), 'Pacific Spirit': ('location', 'pacific-spirit.svg', 544.0, 181.0, 56.0), 'Mediterraneo': ('location', 'mediterraneo.svg', 542.0, 181.0, 63.0), 'Mystic Peaks': ('location', 'mystic-peaks.svg', 544.0, 181.0, 56.0), 'Proving Grounds': ('location', 'proving-grounds.svg', 535.0, 181.0, 74.0), 'Outback': ('location', 'outback.svg', 555.0, 181.0, 35.0), 'Last Resort': ('location', 'last-resort.svg', 547.0, 181.0, 48.0)}
EXPECTED_GLYPHS = {'one player': [{'cell': 107, 'x': 0.0, 'width': 6.0}, {'cell': 106, 'x': 6.0, 'width': 6.0}, {'cell': 97, 'x': 12.0, 'width': 6.0}, {'cell': 28, 'x': 18.0, 'width': 3.0}, {'cell': 108, 'x': 21.0, 'width': 6.0}, {'cell': 104, 'x': 27.0, 'width': 3.0}, {'cell': 93, 'x': 30.0, 'width': 6.0}, {'cell': 117, 'x': 36.0, 'width': 4.0}, {'cell': 97, 'x': 40.0, 'width': 6.0}, {'cell': 110, 'x': 46.0, 'width': 3.0}], 'single race': [{'cell': 111, 'x': 0.0, 'width': 4.0}, {'cell': 101, 'x': 4.0, 'width': 3.0}, {'cell': 106, 'x': 7.0, 'width': 6.0}, {'cell': 99, 'x': 13.0, 'width': 6.0}, {'cell': 104, 'x': 19.0, 'width': 3.0}, {'cell': 97, 'x': 22.0, 'width': 6.0}, {'cell': 28, 'x': 28.0, 'width': 3.0}, {'cell': 110, 'x': 31.0, 'width': 3.0}, {'cell': 93, 'x': 34.0, 'width': 6.0}, {'cell': 95, 'x': 40.0, 'width': 4.0}, {'cell': 97, 'x': 44.0, 'width': 6.0}], 'split screen': [{'cell': 111, 'x': 0.0, 'width': 4.0}, {'cell': 108, 'x': 4.0, 'width': 6.0}, {'cell': 104, 'x': 10.0, 'width': 3.0}, {'cell': 101, 'x': 13.0, 'width': 3.0}, {'cell': 112, 'x': 16.0, 'width': 3.0}, {'cell': 28, 'x': 19.0, 'width': 3.0}, {'cell': 111, 'x': 22.0, 'width': 4.0}, {'cell': 95, 'x': 26.0, 'width': 4.0}, {'cell': 110, 'x': 30.0, 'width': 3.0}, {'cell': 97, 'x': 33.0, 'width': 6.0}, {'cell': 97, 'x': 39.0, 'width': 6.0}, {'cell': 106, 'x': 45.0, 'width': 6.0}], 'modem': [{'cell': 105, 'x': 0.0, 'width': 7.0}, {'cell': 107, 'x': 7.0, 'width': 6.0}, {'cell': 96, 'x': 13.0, 'width': 6.0}, {'cell': 97, 'x': 19.0, 'width': 6.0}, {'cell': 105, 'x': 25.0, 'width': 7.0}], 'serial link': [{'cell': 111, 'x': 0.0, 'width': 4.0}, {'cell': 97, 'x': 4.0, 'width': 6.0}, {'cell': 110, 'x': 10.0, 'width': 3.0}, {'cell': 101, 'x': 13.0, 'width': 3.0}, {'cell': 93, 'x': 16.0, 'width': 6.0}, {'cell': 104, 'x': 22.0, 'width': 3.0}, {'cell': 28, 'x': 25.0, 'width': 3.0}, {'cell': 104, 'x': 28.0, 'width': 3.0}, {'cell': 101, 'x': 31.0, 'width': 3.0}, {'cell': 106, 'x': 34.0, 'width': 6.0}, {'cell': 103, 'x': 40.0, 'width': 4.0}], 'network': [{'cell': 106, 'x': 0.0, 'width': 6.0}, {'cell': 97, 'x': 6.0, 'width': 6.0}, {'cell': 112, 'x': 12.0, 'width': 3.0}, {'cell': 115, 'x': 15.0, 'width': 9.0}, {'cell': 107, 'x': 24.0, 'width': 6.0}, {'cell': 110, 'x': 30.0, 'width': 3.0}, {'cell': 103, 'x': 33.0, 'width': 4.0}], 'tournament': [{'cell': 112, 'x': 0.0, 'width': 3.0}, {'cell': 107, 'x': 3.0, 'width': 6.0}, {'cell': 113, 'x': 9.0, 'width': 6.0}, {'cell': 110, 'x': 15.0, 'width': 3.0}, {'cell': 106, 'x': 18.0, 'width': 6.0}, {'cell': 93, 'x': 24.0, 'width': 6.0}, {'cell': 105, 'x': 30.0, 'width': 7.0}, {'cell': 97, 'x': 37.0, 'width': 6.0}, {'cell': 106, 'x': 43.0, 'width': 6.0}, {'cell': 112, 'x': 49.0, 'width': 3.0}], 'knockout': [{'cell': 103, 'x': 0.0, 'width': 4.0}, {'cell': 106, 'x': 4.0, 'width': 6.0}, {'cell': 107, 'x': 10.0, 'width': 6.0}, {'cell': 95, 'x': 16.0, 'width': 4.0}, {'cell': 103, 'x': 20.0, 'width': 4.0}, {'cell': 107, 'x': 24.0, 'width': 6.0}, {'cell': 113, 'x': 30.0, 'width': 6.0}, {'cell': 112, 'x': 36.0, 'width': 3.0}], 'North Country': [{'cell': 74, 'x': 0.0, 'width': 6.0}, {'cell': 107, 'x': 6.0, 'width': 6.0}, {'cell': 110, 'x': 12.0, 'width': 3.0}, {'cell': 112, 'x': 15.0, 'width': 3.0}, {'cell': 100, 'x': 18.0, 'width': 6.0}, {'cell': 28, 'x': 24.0, 'width': 3.0}, {'cell': 63, 'x': 27.0, 'width': 6.0}, {'cell': 107, 'x': 33.0, 'width': 6.0}, {'cell': 113, 'x': 39.0, 'width': 6.0}, {'cell': 106, 'x': 45.0, 'width': 6.0}, {'cell': 112, 'x': 51.0, 'width': 3.0}, {'cell': 110, 'x': 54.0, 'width': 3.0}, {'cell': 117, 'x': 57.0, 'width': 4.0}], 'Pacific Spirit': [{'cell': 76, 'x': 0.0, 'width': 6.0}, {'cell': 93, 'x': 6.0, 'width': 6.0}, {'cell': 95, 'x': 12.0, 'width': 4.0}, {'cell': 101, 'x': 16.0, 'width': 3.0}, {'cell': 98, 'x': 19.0, 'width': 3.0}, {'cell': 101, 'x': 22.0, 'width': 3.0}, {'cell': 95, 'x': 25.0, 'width': 4.0}, {'cell': 28, 'x': 29.0, 'width': 3.0}, {'cell': 79, 'x': 32.0, 'width': 6.0}, {'cell': 108, 'x': 38.0, 'width': 6.0}, {'cell': 101, 'x': 44.0, 'width': 3.0}, {'cell': 110, 'x': 47.0, 'width': 3.0}, {'cell': 101, 'x': 50.0, 'width': 3.0}, {'cell': 112, 'x': 53.0, 'width': 3.0}], 'Mediterraneo': [{'cell': 73, 'x': 0.0, 'width': 9.0}, {'cell': 97, 'x': 9.0, 'width': 6.0}, {'cell': 96, 'x': 15.0, 'width': 6.0}, {'cell': 101, 'x': 21.0, 'width': 3.0}, {'cell': 112, 'x': 24.0, 'width': 3.0}, {'cell': 97, 'x': 27.0, 'width': 6.0}, {'cell': 110, 'x': 33.0, 'width': 3.0}, {'cell': 110, 'x': 36.0, 'width': 3.0}, {'cell': 93, 'x': 39.0, 'width': 6.0}, {'cell': 106, 'x': 45.0, 'width': 6.0}, {'cell': 97, 'x': 51.0, 'width': 6.0}, {'cell': 107, 'x': 57.0, 'width': 6.0}], 'Mystic Peaks': [{'cell': 73, 'x': 0.0, 'width': 9.0}, {'cell': 117, 'x': 9.0, 'width': 4.0}, {'cell': 111, 'x': 13.0, 'width': 4.0}, {'cell': 112, 'x': 17.0, 'width': 3.0}, {'cell': 101, 'x': 20.0, 'width': 3.0}, {'cell': 95, 'x': 23.0, 'width': 4.0}, {'cell': 28, 'x': 27.0, 'width': 3.0}, {'cell': 76, 'x': 30.0, 'width': 6.0}, {'cell': 97, 'x': 36.0, 'width': 6.0}, {'cell': 93, 'x': 42.0, 'width': 6.0}, {'cell': 103, 'x': 48.0, 'width': 4.0}, {'cell': 111, 'x': 52.0, 'width': 4.0}], 'Proving Grounds': [{'cell': 76, 'x': 0.0, 'width': 6.0}, {'cell': 110, 'x': 6.0, 'width': 3.0}, {'cell': 107, 'x': 9.0, 'width': 6.0}, {'cell': 114, 'x': 15.0, 'width': 4.0}, {'cell': 101, 'x': 19.0, 'width': 3.0}, {'cell': 106, 'x': 22.0, 'width': 6.0}, {'cell': 99, 'x': 28.0, 'width': 6.0}, {'cell': 28, 'x': 34.0, 'width': 3.0}, {'cell': 67, 'x': 37.0, 'width': 6.0}, {'cell': 110, 'x': 43.0, 'width': 3.0}, {'cell': 107, 'x': 46.0, 'width': 6.0}, {'cell': 113, 'x': 52.0, 'width': 6.0}, {'cell': 106, 'x': 58.0, 'width': 6.0}, {'cell': 96, 'x': 64.0, 'width': 6.0}, {'cell': 111, 'x': 70.0, 'width': 4.0}], 'Outback': [{'cell': 75, 'x': 0.0, 'width': 6.0}, {'cell': 113, 'x': 6.0, 'width': 6.0}, {'cell': 112, 'x': 12.0, 'width': 3.0}, {'cell': 94, 'x': 15.0, 'width': 6.0}, {'cell': 93, 'x': 21.0, 'width': 6.0}, {'cell': 95, 'x': 27.0, 'width': 4.0}, {'cell': 103, 'x': 31.0, 'width': 4.0}], 'Last Resort': [{'cell': 72, 'x': 0.0, 'width': 4.0}, {'cell': 93, 'x': 4.0, 'width': 6.0}, {'cell': 111, 'x': 10.0, 'width': 4.0}, {'cell': 112, 'x': 14.0, 'width': 3.0}, {'cell': 28, 'x': 17.0, 'width': 3.0}, {'cell': 78, 'x': 20.0, 'width': 6.0}, {'cell': 97, 'x': 26.0, 'width': 6.0}, {'cell': 111, 'x': 32.0, 'width': 4.0}, {'cell': 107, 'x': 36.0, 'width': 6.0}, {'cell': 110, 'x': 42.0, 'width': 3.0}, {'cell': 112, 'x': 45.0, 'width': 3.0}]}
EXPECTED_ROWS = 316
FIELDS = 'float outputScaleX,outputScaleY;'
BEGIN = ',\n/* NFS_RIGHT_LABELS_BEGIN */\n'
END = '\n/* NFS_RIGHT_LABELS_END */'


def right_label_rows(catalog, placements, identity):
    if catalog.get('schema') != 2 or len(catalog.get('words', [])) != len(LABELS):
        raise ValueError('Unexpected right-label catalog')
    if [w.get('text') for w in catalog['words']] != list(LABELS):
        raise ValueError('Unexpected right-label order')
    seen, rows = set(), []
    for word in catalog['words']:
        text = word.get('text')
        if text not in LABELS or text in seen:
            raise ValueError('Unknown or duplicate right label')
        seen.add(text)
        family, filename, x, y, width = LABELS[text]
        if (word.get('reuseFamily'), word.get('reuse'), word.get('x'), word.get('y'), word.get('width'), word.get('height')) != (family, filename, x, y, width, 37):
            raise ValueError('Unexpected right-label geometry')
        matches = [p for p in placements.get(family+'Placements', []) if p.get('file') == filename]
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
        if len(glyphs) != len(text) or glyphs != EXPECTED_GLYPHS[text]:
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
                values = [glyph['cell'], x+gx+shadow, y+shadow, gw, 37,
                          gx/width, (gx+gw)/width, ax/4, ay/8, aw/4,
                          x+shadow, crop[1]/4, ah/4]
                rows.append('{'+','.join(format(v, '.12g') for v in values)
                            +',UINT64_C(0x'+format(identity(text), '016x')+'),'
                            +format(width/(aw/4), '.12g')+','+format(37/51, '.12g')+'}')
    if len(rows) != EXPECTED_ROWS:
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
